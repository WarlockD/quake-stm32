/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// host.c -- coordinates spawning and killing of local servers
#include "icommon.h"
#include "r_local.h"

/*

A server can allways be started, even if the system started out as a client
to a remote system.

A client can NOT be started if the system started as a dedicated server.

Memory is cleared / released when a server or client begins, not when they end.

*/
using namespace std::chrono;
extern sys_file vcrFile;
quakeparms_t host_parms;

qboolean	host_initialized;		// true if into command execution

idTime		host_frametime;
idTime		host_time;
idTime		realtime;				// without any filtering or bounding
idTime		oldrealtime;			// last frame run
int			host_framecount;

int			host_hunklevel;

int			minimum_memory;

client_t	*host_client;			// current client

jmp_buf 	host_abortserver;

byte		*host_basepal;
byte		*host_colormap;

cvar_t<float> host_framerate = { 0.0f} ;	// set for slow motion
cvar_t<float>  host_speeds = {  0.0f} ;			// set for running times

cvar_t<float> sys_ticrate = { 0.05f} ;
cvar_t<float> serverprofile = { 0.0f} ;

cvar_t<float> fraglimit = { 0.0f,false,true} ;
cvar_t<float> timelimit = { 0.0f,false,true} ;
cvar_t<float> teamplay = { 0.0f,false,true} ;

cvar_t<float> samelevel = { 0.0f} ;
cvar_t<float> noexit = { 0.0f,false,true} ;

#ifdef QUAKE2
cvar_t<float> developer = { 1.0f} ;	// should be 0 for release!
#else
cvar_t<float> developer = { 1.0f} ;
#endif

cvar_t<float> skill = { 1.0f} ;						// 0 - 3
cvar_t<float> deathmatch = { 0.0f} ;			// 0, 1, or 2
cvar_t<float> coop = { 0.0f} ;			// 0 or 1

cvar_t<float> pausable = { 1.0f} ;

cvar_t<float> temp1 = { 0.0f} ;


/*
================
Host_EndGame
================
*/
void Host_EndGame (char *message, ...)
{
	va_list		argptr;
	char		string[1024];
	
	va_start (argptr,message);
	Q_vsprintf(string, message, argptr);
	va_end (argptr);
	Con_DPrintf ("Host_EndGame: %s\n",string);
	
	if (sv.active)
		Host_ShutdownServer (false);

	if (quake::cls.state == ca_dedicated)
		Sys_Error ("Host_EndGame: %s\n",string);	// dedicated servers exit
	
	if (quake::cls.demonum != -1)
		quake::cls.next_demo();
	else
		quake::cls.disconnect();

	longjmp (host_abortserver, 1);
}

/*
================
Host_Error

This shuts down both the client and server
================
*/
void Host_Error (char *error, ...)
{
	va_list		argptr;
	char		string[1024];
	static	qboolean inerror = false;
	
	if (inerror)
		Sys_Error ("Host_Error: recursively entered");
	inerror = true;
	
	SCR_EndLoadingPlaque ();		// reenable screen updates

	va_start (argptr,error);
	Q_vsprintf(string,error,argptr);
	va_end (argptr);
	Con_Printf ("Host_Error: %s\n",string);
	
	if (sv.active)
		Host_ShutdownServer (false);

	if (quake::cls.state == ca_dedicated)
		Sys_Error ("Host_Error: %s\n",string);	// dedicated servers exit

	quake::cls.disconnect();
	quake::cls.demonum = -1;

	inerror = false;

	longjmp (host_abortserver, 1);
}

/*
================
Host_FindMaxClients
================
*/
void	Host_FindMaxClients (void)
{

	svs.maxclients = 1;
	cstring_t value;

	if (host_parms.COM_CheckParmValue("-dedicated", value))
	{
		if (!value.empty())
			svs.maxclients = Q_atoi (value);
		else
			svs.maxclients = 8;
	}
	else
		quake::cls.state = ca_disconnected;

	if (host_parms.COM_CheckParmValue("-listen", value))
	{
		if (quake::cls.state == ca_dedicated)
			Sys_Error ("Only one of -dedicated or -listen can be specified");
		if (!value.empty())
			svs.maxclients = Q_atoi(value);
		else
			svs.maxclients = 8;
	}
	if (svs.maxclients < 1)
		svs.maxclients = 8;
	else if (svs.maxclients > MAX_SCOREBOARD)
		svs.maxclients = MAX_SCOREBOARD;

	svs.maxclientslimit = svs.maxclients;
	if (svs.maxclientslimit < 4)
		svs.maxclientslimit = 4;
	svs.clients = (client_t*)Hunk_AllocName (svs.maxclientslimit*sizeof(client_t), "clients");

	if (svs.maxclients > 1)
		Cvar_Set ("deathmatch", 1.0);
	else
		Cvar_Set ("deathmatch", 0.0);
}


/*
=======================
Host_InitLocal
======================
*/
void Host_InitLocal (void)
{
	Host_InitCommands ();
	
	Cvar_RegisterVariable("host_framerate",host_framerate);
	Cvar_RegisterVariable("host_speeds",host_speeds);

	Cvar_RegisterVariable("sys_ticrate",sys_ticrate);
	Cvar_RegisterVariable("serverprofile",serverprofile);

	Cvar_RegisterVariable("fraglimit",fraglimit);
	Cvar_RegisterVariable("timelimit",timelimit);
	Cvar_RegisterVariable("teamplay",teamplay);
	Cvar_RegisterVariable("samelevel",samelevel);
	Cvar_RegisterVariable("noexit",noexit);
	Cvar_RegisterVariable("skill",skill);
	Cvar_RegisterVariable("developer",developer);
	Cvar_RegisterVariable("deathmatch",deathmatch);
	Cvar_RegisterVariable("coop",coop);

	Cvar_RegisterVariable("pausable",pausable);

	Cvar_RegisterVariable("temp1",temp1);

	Host_FindMaxClients ();
	
	host_time = 1s;		// so a think at time 0 won't get called
}


/*
===============
Host_WriteConfiguration

Writes key bindings and archived cvars to config.cfg
===============
*/
void Host_WriteConfiguration (void)
{
// dedicated servers initialize the host but don't parse and set the
// config.cfg cvars
	if (host_initialized & !isDedicated)
	{
		quake::va_stack<256> va;
		quake::ofstream f(va("%s/config.cfg", COM_GameDir));

		//f = fopen (va("%s/config.cfg",com_gamedir), "w");
		if (f.bad())
		{
			Con_Printf ("Couldn't write config.cfg.\n");
			return;
		}
		
		Key_WriteBindings (f);
		Cvar_WriteVariables (f);
		f.close();
	}
}


/*
=================
SV_ClientPrintf

Sends text across to be displayed 
FIXME: make this just a stuffed echo?
=================
*/
void SV_ClientPrintf (const char *fmt, ...)
{
	va_list		argptr;
	char		string[1024];
	
	va_start (argptr,fmt);
	Q_vsprintf(string, fmt,argptr);
	va_end (argptr);
	
	host_client->message.WriteByte(svc_print);
	host_client->message.WriteString(string);
}

/*
=================
SV_BroadcastPrintf

Sends text to all active clients
=================
*/
void SV_BroadcastPrintf (const char *fmt, ...)
{
	va_list		argptr;
	char		string[1024];
	int			i;
	
	va_start (argptr,fmt);
	Q_vsprintf(string, fmt,argptr);
	va_end (argptr);
	
	for (i=0 ; i<svs.maxclients ; i++)
		if (svs.clients[i].active && svs.clients[i].spawned)
		{
			svs.clients[i].message.WriteByte(svc_print);
			svs.clients[i].message.WriteString(string);
		}
}

/*
=================
Host_ClientCommands

Send text over to the client to be executed
=================
*/
void Host_ClientCommands (char *fmt, ...)
{
	va_list		argptr;
	char		string[1024];
	va_start (argptr,fmt);
	Q_vsprintf(string, fmt,argptr);
	va_end (argptr);
	
	host_client->message.WriteByte(svc_stufftext);
	host_client->message.WriteString(string);
}

/*
=====================
SV_DropClient

Called when the player is getting totally kicked off the host
if (crash = true), don't bother sending signofs
=====================
*/
void SV_DropClient (qboolean crash) {
	int		i;
	client_t *client;

	if (!crash)
	{
		// send any final messages (don't check for errors)
		if (NET_CanSendMessage (host_client->netconnection))
		{
			host_client->message.WriteByte(svc_disconnect);
			NET_SendMessage (host_client->netconnection, &host_client->message);
		}
	
		if (host_client->edict && host_client->spawned)
		{
		// call the prog function for removing a client
		// this will set the body to a dead frame, among other things
			vm.pr_global_struct->ClientDisconnect->call(host_client->edict, nullptr);
		}

		Sys_Printf ("Client %s removed\n",host_client->name);
	}

// break the net connection
	NET_Close (host_client->netconnection);
	host_client->netconnection = NULL;

// free the client (the body stays around)
	host_client->active = false;
	host_client->name = string_t();
	host_client->old_frags = -999999;
	net_activeconnections--;

// send notification to all clients
	for (i=0, client = svs.clients ; i<svs.maxclients ; i++, client++)
	{
		if (!client->active)
			continue;
		client->message.WriteByte(svc_updatename);
		client->message.WriteByte(host_client - svs.clients);
		client->message.WriteString("");
		client->message.WriteByte(svc_updatefrags);
		client->message.WriteByte(host_client - svs.clients);
		client->message.WriteShort(0);
		client->message.WriteByte(svc_updatecolors);
		client->message.WriteByte(host_client - svs.clients);
		client->message.WriteByte(0);
	}
}

/*
==================
Host_ShutdownServer

This only happens at the end of a game, not between levels
==================
*/
void Host_ShutdownServer(qboolean crash)
{
	int		i;
	int		count;
	sizebuf_t	buf;
	char		message[4];
	idTime	start;

	if (!sv.active)
		return;

	sv.active = false;

// stop all client sounds immediately
	if (quake::cls.state == ca_connected)
		quake::cls.disconnect();

// flush any pending messages - like the score!!!
	start = Sys_FloatTime();
	do
	{
		count = 0;
		for (i=0, host_client = svs.clients ; i<svs.maxclients ; i++, host_client++)
		{
			if (host_client->active && host_client->message.size())
			{
				if (NET_CanSendMessage (host_client->netconnection))
				{
					NET_SendMessage(host_client->netconnection, &host_client->message);
					host_client->message.Clear();
				}
				else
				{
					NET_GetMessage(host_client->netconnection);
					count++;
				}
			}
		}
		if ((Sys_FloatTime() - start) > 3s)
			break;
	}
	while (count);

// make sure all the clients know we're disconnecting
	buf=sizebuf_t((byte*)message, 4);
	buf.WriteByte(svc_disconnect);
	count = NET_SendToAll(&buf, 5s);
	if (count)
		Con_Printf("Host_ShutdownServer: NET_SendToAll failed for %u clients\n", count);

	for (i=0, host_client = svs.clients ; i<svs.maxclients ; i++, host_client++)
		if (host_client->active) {
			SV_DropClient(crash);
		}


//
// clear structures
//
	sv.reset();
	//Q_memset (&sv, 0, sizeof(sv));
	for (size_t i = 0; i < svs.maxclientslimit; i++) svs.clients[i].reset();
}

void pr_system_t::Clear() {
	progs = nullptr;
	pr_functions = nullptr;
	pr_xfunction = nullptr;
	pr_xstatement = 0;
	pr_globaldefs = nullptr;
	pr_fielddefs = nullptr;
	pr_statements = nullptr;
	pr_global_struct = nullptr;
	pr_globals = nullptr;
	pr_edict_size = 0;
	pr_max_edicts = 0;

	ED_ClearStrings();
	free_pool.clear();
	used_pool.clear();
}
/*
================
Host_ClearMemory

This clears all the memory used by both the client and server, but does
not reinitialize anything.
================
*/
void Host_ClearMemory (void)
{
	quake::dcon << "Clearing memory" << std::endl;
	D_FlushCaches ();
	Mod_ClearAll ();
	if (host_hunklevel)
		Hunk_FreeToLowMark (host_hunklevel);

	vm.Clear();
	sv.reset();


	quake::cl.clear();
	quake::cls.signon = 0;
}


//============================================================================


/*
===================
Host_FilterTime

Returns false if the time is too short to run a frame
===================
*/
qboolean Host_FilterTime (idTime time)
{
	using namespace std::chrono;
	static const idTime max_framerate= idTime(1.0f / 72.0f);
	realtime += time;

	const idTime new_time = (realtime - oldrealtime);
	if (!quake::cls.timedemo && new_time < max_framerate)
		return false;		// framerate is too high

	host_frametime = new_time;
	oldrealtime = realtime;

	if (host_framerate.value > 0)
		host_frametime = idCast<idTime>(static_cast<float>(host_framerate.value));
	else
	{	// don't allow really long or short frames
		if (host_frametime > 100ms)
			host_frametime = 100ms;
		if (host_frametime < 1ms)
			host_frametime = 1ms;
	}
	
	return true;
}


/*
===================
Host_GetConsoleCommands

Add them exactly as if they had been typed at the console
===================
*/
void Host_GetConsoleCommands (void)
{
	char	*cmd;

	while (1)
	{
		cmd = Sys_ConsoleInput ();
		if (!cmd)
			break;
		Cbuf_AddText (cmd);
	}
}


/*
==================
Host_ServerFrame

==================
*/
#ifdef FPS_20

void _Host_ServerFrame (void)
{
// run the world state	
	vm.pr_global_struct->frametime = host_frametime;

// read client messages
	SV_RunClients ();
	
// move things around and think
// always pause in single player if in console or menus
	if (!sv.paused && (svs.maxclients > 1 || key_dest == key_game) )
		SV_Physics ();
}

void Host_ServerFrame (void)
{
	float	save_host_frametime;
	float	temp_host_frametime;

// run the world state	
	vm.pr_global_struct->frametime = host_frametime;

// set the time and clear the general datagram
	SV_ClearDatagram ();
	
// check for new clients
	SV_CheckForNewClients ();

	temp_host_frametime = save_host_frametime = host_frametime;
	while(temp_host_frametime > (1.0/72.0))
	{
		if (temp_host_frametime > 0.05)
			host_frametime = 0.05;
		else
			host_frametime = temp_host_frametime;
		temp_host_frametime -= host_frametime;
		_Host_ServerFrame ();
	}
	host_frametime = save_host_frametime;

// send all messages to the clients
	SV_SendClientMessages ();
}

#else

void Host_ServerFrame (void)
{
// run the world state	
	vm.pr_global_struct->frametime = static_cast<float>(host_frametime);

// set the time and clear the general datagram
	SV_ClearDatagram ();
	
// check for new clients
	SV_CheckForNewClients ();

// read client messages
	SV_RunClients ();
	
// move things around and think
// always pause in single player if in console or menus
	if (!sv.paused && (svs.maxclients > 1 || key_dest == key_game) )
		SV_Physics ();

// send all messages to the clients
	SV_SendClientMessages ();
}

#endif


/*
==================
Host_Frame

Runs all active servers
==================
*/
void _Host_Frame (idTime time)
{
	static idTime		time1 = idTime::zero();
	static idTime		time2 = idTime::zero();
	static idTime		time3 = idTime::zero();

	if (setjmp (host_abortserver) )
		return;			// something bad happened, or the server disconnected

// keep the random time dependent
	rand ();
	
// decide the simulation time
	if (!Host_FilterTime (time))
		return;			// don't run too fast, or packets will flood out
		
// get new key events
	Sys_SendKeyEvents ();

// allow mice or other external controllers to add commands
	IN_Commands ();

// process console commands
	Cbuf_Execute ();

	NET_Poll();

// if running the server locally, make intentions now
	if (sv.active)
		CL_SendCmd ();
	
//-------------------
//
// server operations
//
//-------------------

// check for commands typed to the host
	Host_GetConsoleCommands ();
	
	if (sv.active)
		Host_ServerFrame ();

//-------------------
//
// client operations
//
//-------------------

// if running the server remotely, send intentions now after
// the incoming messages have been read
	if (!sv.active)
		CL_SendCmd ();

	host_time += host_frametime;

// fetch results from server
	if (quake::cls.state == ca_connected)
	{
		CL_ReadFromServer ();
	}

// update video
	if (host_speeds.value)
		time1 = Sys_FloatTime ();
		
	SCR_UpdateScreen ();

	if (host_speeds.value)
		time2 = Sys_FloatTime ();
		
// update audio
	if (quake::cls.signon == SIGNONS)
	{
		S_Update (r_origin, vpn, vright, vup);
		CL_DecayLights ();
	}
	else
		S_Update (vec3_origin, vec3_origin, vec3_origin, vec3_origin);
	
	CDAudio_Update();

	if (host_speeds.value)
	{
		idTime time3 = Sys_FloatTime();
		idTime pass1 = (time1 - time3);
		idTime pass2 = (time2 - time1);
		idTime pass3 = (time3 - time2);

		quake::con << " total=" << std::setw(7) << (pass1 + pass2 + pass3);
		quake::con << " server=" << std::setw(7) << pass1;
		quake::con << " gfx=" << std::setw(7) << pass2;
		quake::con << " snd=" << std::setw(7) << pass3;
		quake::con << std::endl;
	}
	
	host_framecount++;
}

void Host_Frame (idTime time)
{
	idTime	time1, time2;
	static idTime	timetotal;
	static int		timecount;
	int		i, c;

	if (!serverprofile.value)
	{
		_Host_Frame (time);
		return;
	}
	
	time1 = Sys_FloatTime ();
	_Host_Frame (time);
	time2 = Sys_FloatTime ();	
	
	timetotal += time2 - time1;
	timecount++;
	
	if (timecount < 1000)
		return;

	idTime m(timetotal.count()* 1000/timecount);
	timecount = 0;
	timetotal = idTime::zero();
	c = 0;
	for (i=0 ; i<svs.maxclients ; i++)
	{
		if (svs.clients[i].active)
			c++;
	}

	Con_Printf ("serverprofile: %2i clients %2i msec\n",  c,  m.count());
}

//============================================================================



#define	VCR_SIGNATURE	0x56435231
// "VCR1"
ZVector<char> buffer2;
void Host_InitVCR (quakeparms_t *parms)
{
	int		i, len, n;
	char	*p;
	int arg_count;


	if (host_parms.COM_CheckParm("-playback"))
	{
		if (host_parms.args.size() != 2)
			Sys_Error("No other parameters allowed with -playback\n");
		vcrFile.open("quake.vcr", std::ios_base::in | std::ios_base::binary);
		if (!vcrFile.is_open())
			Sys_Error("playback file not found\n");

		vcrFile.read(&i, sizeof(int));
		if (i != VCR_SIGNATURE)
			Sys_Error("Invalid signature in vcr file\n");
		vcrFile.read(&arg_count, sizeof(int));
		host_parms.COM_ClearArgs();

		host_parms.COM_AddArg("Playback"); // filler for file name
		for (size_t i = 0; i < (size_t)arg_count; i++) {
			ZVector<char> buffer;
			vcrFile.read(&len, sizeof(int));
			buffer.resize(len);
			vcrFile.read(buffer.data(), len);
			buffer[len] = 0;
			host_parms.COM_AddArg(buffer.data());
		}
		vcrFile.close();
	}
#if 0
	if ( (n = COM_CheckParm("-record")) != 0)
	{
		vcrFile = Sys_FileOpenWrite("quake.vcr");

		i = VCR_SIGNATURE;
		Sys_FileWrite(vcrFile, &i, sizeof(int));
		i = com_argc - 1;
		Sys_FileWrite(vcrFile, &i, sizeof(int));
		for (i = 1; i < com_argc; i++)
		{
			if (i == n)
			{
				len = 10;
				Sys_FileWrite(vcrFile, &len, sizeof(int));
				Sys_FileWrite(vcrFile, "-playback", len);
				continue;
			}
			len = Q_strlen(com_argv[i]) + 1;
			Sys_FileWrite(vcrFile, &len, sizeof(int));
			Sys_FileWrite(vcrFile, com_argv[i], len);
		}
	}
#endif
	// lazy
}

/*
====================
Host_Init
====================
*/
void Host_Init (quakeparms_t *parms)
{

	if (standard_quake)
		minimum_memory = MINIMUM_MEMORY;
	else
		minimum_memory = MINIMUM_MEMORY_LEVELPAK;

	if (parms->COM_CheckParm ("-minmemory"))
		parms->memsize = minimum_memory;

	host_parms = *parms;

	if (parms->memsize < minimum_memory)
		Sys_Error ("Only %4.1f megs of memory available, can't execute game", parms->memsize / (float)0x100000);


	Memory_Init (parms->membase, parms->memsize); // in main.c ugh

	Cbuf_Init ();
	Cmd_Init ();	
	V_Init ();
	Chase_Init ();
	Host_InitVCR (parms);
	COM_Init (parms->basedir.c_str());
	Host_InitLocal ();
	W_LoadWadFile ("gfx.wad");
	Key_Init ();
	Con_Init ();	
	M_Init ();	
	vm.Init();
	Mod_Init ();
	NET_Init ();
	SV_Init ();

	Con_Printf ("Exe: " __TIME__ " " __DATE__ "\n");
	Con_Printf ("%4.1f megabyte heap\n",parms->memsize/ (1024*1024.0));
	
	R_InitTextures ();		// needed even for dedicated servers
 
	if (quake::cls.state != ca_dedicated)
	{
		host_basepal = (byte *)COM_LoadHunkFile ("gfx/palette.lmp");
		if (!host_basepal)
			Sys_Error ("Couldn't load gfx/palette.lmp");
		host_colormap = (byte *)COM_LoadHunkFile ("gfx/colormap.lmp");
		if (!host_colormap)
			Sys_Error ("Couldn't load gfx/colormap.lmp");

#ifndef _WIN32 // on non win32, mouse comes before video for security reasons
		IN_Init ();
#endif
		VID_Init (host_basepal);

		Draw_Init ();
		SCR_Init ();
		R_Init ();
#ifndef	_WIN32
	// on Win32, sound initialization has to come before video initialization, so we
	// can put up a popup if the sound hardware is in use
		S_Init ();
#else

#ifdef	GLQUAKE
	// FIXME: doesn't use the new one-window approach yet
		S_Init ();
#endif

#endif	// _WIN32
		CDAudio_Init ();
		Sbar_Init ();
		CL_Init ();
#ifdef _WIN32 // on non win32, mouse comes before video for security reasons
		IN_Init ();
#endif
	}
	Cbuf_InsertText ("exec quake.rc\n");

	Hunk_AllocName (0, "-HOST_HUNKLEVEL-");
	host_hunklevel = Hunk_LowMark ();

	host_initialized = true;
	
	Sys_Printf ("========Quake Initialized=========\n");	
}


/*
===============
Host_Shutdown

FIXME: this is a callback from Sys_Quit and Sys_Error.  It would be better
to run quit through here before the final handoff to the sys code.
===============
*/
void Host_Shutdown(void)
{
	static qboolean isdown = false;
	
	if (isdown)
	{
		printf ("recursive shutdown\n");
		return;
	}
	isdown = true;

// keep Con_Printf from trying to update the screen
	scr_disabled_for_loading = true;

	Host_WriteConfiguration (); 

	CDAudio_Shutdown ();
	NET_Shutdown ();
	S_Shutdown();
	IN_Shutdown ();

	if (quake::cls.state != ca_dedicated)
	{
		VID_Shutdown();
	}
}

