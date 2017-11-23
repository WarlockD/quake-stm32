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

#include "icommon.h"
#include "cmd.h"
extern cvar_t	pausable;

int	current_skill;

void Mod_Print(cmd_source_t source, size_t argc, const quake::string_view argv[]);

/*
==================
Host_Quit_f
==================
*/

extern void M_Menu_Quit_f(cmd_source_t source, size_t argc, const quake::string_view args[]);

void Host_Quit_f(cmd_source_t source, size_t argc, const quake::string_view argv[])
{
	if (key_dest != key_console && cls.state != ca_dedicated)
	{
		M_Menu_Quit_f (source,argc,argv);
		return;
	}
	CL_Disconnect ();
	Host_ShutdownServer(false);		

	Sys_Quit ();
}


/*
==================
Host_Status_f
==================
*/
void Host_Status_f(cmd_source_t source, size_t argc, const quake::string_view argv[])
{
	client_t	*client;
	int			seconds;
	int			minutes;
	int			hours = 0;
	int			j;
	void		(*print) (const char *fmt, ...);
	
	if (source == src_command)
	{
		if (!sv.active)
		{
			Cmd_ForwardToServer (source, argc, argv);
			return;
		}
		print = Con_Printf;
	}
	else
		print = SV_ClientPrintf;

	print ("host:    %s\n", Cvar_VariableString ("hostname"));
	print ("version: %4.2f\n", VERSION);
	if (tcpipAvailable)
		print ("tcp/ip:  %s\n", my_tcpip_address);
	if (ipxAvailable)
		print ("ipx:     %s\n", my_ipx_address);
	print ("map:     %s\n", sv.name);
	print ("players: %i active (%i max)\n\n", net_activeconnections, svs.maxclients);
	for (j=0, client = svs.clients ; j<svs.maxclients ; j++, client++)
	{
		if (!client->active)
			continue;
		seconds = idCast<int>(net_time- client->netconnection->connecttime);
		minutes = seconds / 60;
		if (minutes)
		{
			seconds -= (minutes * 60);
			hours = minutes / 60;
			if (hours)
				minutes -= (hours * 60);
		}
		else
			hours = 0;
		print ("#%-2u %-16.16s  %3i  %2i:%02i:%02i\n", j+1, client->name, (int)client->edict->v.frags, hours, minutes, seconds);
		print ("   %s\n", client->netconnection->address);
	}
}


/*
==================
Host_God_f

Sets client to godmode
==================
*/
void Host_God_f(cmd_source_t source, size_t argc, const quake::string_view argv[])
{
	if (source == src_command)
	{
		Cmd_ForwardToServer (source, argc, argv);
		return;
	}

	if (pr_global_struct->deathmatch && !host_client->privileged)
		return;

	sv_player->v.flags = (int)sv_player->v.flags ^ FL_GODMODE;
	if (!((int)sv_player->v.flags & FL_GODMODE) )
		SV_ClientPrintf ("godmode OFF\n");
	else
		SV_ClientPrintf ("godmode ON\n");
}

void Host_Notarget_f(cmd_source_t source, size_t argc, const quake::string_view argv[])
{
	if (source == src_command)
	{
		Cmd_ForwardToServer (source, argc, argv);
		return;
	}

	if (pr_global_struct->deathmatch && !host_client->privileged)
		return;

	sv_player->v.flags = (int)sv_player->v.flags ^ FL_NOTARGET;
	if (!((int)sv_player->v.flags & FL_NOTARGET) )
		SV_ClientPrintf ("notarget OFF\n");
	else
		SV_ClientPrintf ("notarget ON\n");
}

qboolean noclip_anglehack;

void Host_Noclip_f(cmd_source_t source, size_t argc, const quake::string_view argv[])
{
	if (source == src_command)
	{
		Cmd_ForwardToServer (source, argc, argv);
		return;
	}

	if (pr_global_struct->deathmatch && !host_client->privileged)
		return;

	if (sv_player->v.movetype != MOVETYPE_NOCLIP)
	{
		noclip_anglehack = true;
		sv_player->v.movetype = MOVETYPE_NOCLIP;
		SV_ClientPrintf ("noclip ON\n");
	}
	else
	{
		noclip_anglehack = false;
		sv_player->v.movetype = MOVETYPE_WALK;
		SV_ClientPrintf ("noclip OFF\n");
	}
}

/*
==================
Host_Fly_f

Sets client to flymode
==================
*/
void Host_Fly_f(cmd_source_t source, size_t argc, const quake::string_view argv[])
{
	if (source == src_command)
	{
		Cmd_ForwardToServer (source,argc,argv);
		return;
	}

	if (pr_global_struct->deathmatch && !host_client->privileged)
		return;

	if (sv_player->v.movetype != MOVETYPE_FLY)
	{
		sv_player->v.movetype = MOVETYPE_FLY;
		SV_ClientPrintf ("flymode ON\n");
	}
	else
	{
		sv_player->v.movetype = MOVETYPE_WALK;
		SV_ClientPrintf ("flymode OFF\n");
	}
}


/*
==================
Host_Ping_f

==================
*/
void Host_Ping_f(cmd_source_t source, size_t argc, const quake::string_view argv[])
{
	int		i, j;
	idTime	total;
	client_t	*client;
	
	if (source == src_command)
	{
		Cmd_ForwardToServer (source, argc, argv);
		return;
	}

	SV_ClientPrintf ("Client ping times:\n");
	for (i=0, client = svs.clients ; i<svs.maxclients ; i++, client++)
	{
		if (!client->active)
			continue;
		total = idTime::zero();
		for (j=0 ; j<NUM_PING_TIMES ; j++)
			total+=client->ping_times[j];
		total = static_cast<float>(total) / ((float)NUM_PING_TIMES / 1000.0f);
		SV_ClientPrintf ("%4i %s\n", idCast<int>(total), client->name);
	}
}

/*
===============================================================================

SERVER TRANSITIONS

===============================================================================
*/


/*
======================
Host_Map_f

handle a 
map <servername>
command from the console.  Active clients are kicked off.
======================
*/
void Host_Map_f(cmd_source_t source, size_t argc, const quake::string_view argv[])
{
	quake::fixed_string_stream<MAX_QPATH> name;
	if (source != src_command)
		return;

	cls.demonum = -1;		// stop demo loop in case this fails

	CL_Disconnect ();
	Host_ShutdownServer(false);		

	key_dest = key_game;			// remove console or menu
	SCR_BeginLoadingPlaque ();

	cls.mapstring[0] = 0;
	for (int i=0 ; i<argc ; i++)
		name << argv[i] << ' ';
	name << std::endl; 
	static_cast<quake::string_view>(name.str()).copy(cls.mapstring, sizeof(cls.mapstring));

	svs.serverflags = 0;			// haven't completed an episode yet
	name.clear();
	name << argv[1];
#ifdef QUAKE2
	SV_SpawnServer (name, NULL);
#else
	SV_SpawnServer (name.str().c_str());
#endif
	if (!sv.active)
		return;
	
	if (cls.state != ca_dedicated)
	{
		name.clear();

		for (size_t i = 2; i < argc; i++)
			name << argv[i] << ' ';

		static_cast<quake::string_view>(name.str()).copy(cls.spawnparms, sizeof(cls.spawnparms));
		execute_args("connect local", src_command);
	}	
}

/*
==================
Host_Changelevel_f

Goes to a new map, taking all clients along
==================
*/
void Host_Changelevel_f(cmd_source_t source, size_t argc, const quake::string_view argv[])
{
#ifdef QUAKE2
	char	level[MAX_QPATH];
	char	_startspot[MAX_QPATH];
	char	*startspot;

	if (Cmd_Argc() < 2)
	{
		Con_Printf ("changelevel <levelname> : continue game on a new level\n");
		return;
	}
	if (!sv.active || cls.demoplayback)
	{
		Con_Printf ("Only the server may changelevel\n");
		return;
	}

	strcpy (level, Cmd_Argv(1));
	if (Cmd_Argc() == 2)
		startspot = NULL;
	else
	{
		strcpy (_startspot, Cmd_Argv(2));
		startspot = _startspot;
	}

	SV_SaveSpawnparms ();
	SV_SpawnServer (level, startspot);
#else
	if (argc != 2)
	{
		Con_Printf ("changelevel <levelname> : continue game on a new level\n");
		return;
	}
	if (!sv.active || cls.demoplayback)
	{
		Con_Printf ("Only the server may changelevel\n");
		return;
	}
	SV_SaveSpawnparms ();
	UString str(argv[1].data(), argv[1].size());
	SV_SpawnServer (str.c_str());
#endif
}

/*
==================
Host_Restart_f

Restarts the current server for a dead player
==================
*/
void Host_Restart_f(cmd_source_t source, size_t argc, const quake::string_view argv[])
{
	char	mapname[MAX_QPATH];
#ifdef QUAKE2
	char	startspot[MAX_QPATH];
#endif

	if (cls.demoplayback || !sv.active)
		return;

	if (source != src_command)
		return;
	strcpy (mapname, sv.name);	// must copy out, because it gets cleared
								// in sv_spawnserver
#ifdef QUAKE2
	strcpy(startspot, sv.startspot);
	SV_SpawnServer (mapname, startspot);
#else
	SV_SpawnServer (mapname);
#endif
}

/*
==================
Host_Reconnect_f

This command causes the client to wait for the signon messages again.
This is sent just before a server changes levels
==================
*/
void Host_Reconnect_f(cmd_source_t source, size_t argc, const quake::string_view argv[])
{
	SCR_BeginLoadingPlaque ();
	cls.signon = 0;		// need new connection messages
}

/*
=====================
Host_Connect_f

User command to connect to server
=====================
*/
void Host_Connect_f(cmd_source_t source, size_t argc, const quake::string_view argv[])
{
	cls.demonum = -1;		// stop demo loop in case this fails
	if (cls.demoplayback)
	{
		CL_StopPlayback ();
		CL_Disconnect ();
	}
	CL_EstablishConnection (argv[1],source,argc,argv);
	Host_Reconnect_f (source,argc,argv);
}


/*
===============================================================================

LOAD / SAVE GAME

===============================================================================
*/

#define	SAVEGAME_VERSION	5

/*
===============
Host_SavegameComment

Writes a SAVEGAME_COMMENT_LENGTH character comment describing the current 
===============
*/
using save_game_comment_string = quake::fixed_string<SAVEGAME_COMMENT_LENGTH + 1>;
static void Host_SavegameComment(std::ostream& os){
	quake::fixed_string_stream<SAVEGAME_COMMENT_LENGTH + 1> ss;
	ss << cl.levelname << " kills:" << std::setw(3) << cl.stats[STAT_MONSTERS] << '/' << std::setw(3) << cl.stats[STAT_TOTALMONSTERS];
	size_t len = ss.str().size();
	if (len< SAVEGAME_COMMENT_LENGTH)
		ss << std::setfill('_') << std::setw(SAVEGAME_COMMENT_LENGTH - len) << '_';
	os << ss.rdbuf();
}


/*
===============
Host_Savegame_f
===============
*/
void Host_Savegame_f(cmd_source_t source, size_t argc, const quake::string_view argv[])
{
	quake::fixed_string<256>	name;
	int		i;

	if (source != src_command)
		return;

	if (!sv.active)
	{
		quake::con << "Not playing a local game. " << std::endl;
		return;
	}

	if (cl.intermission)
	{
		quake::con << "Can't save in intermission." << std::endl;
		return;
	}

	if (svs.maxclients != 1)
	{
		quake::con << "Can't save multiplayer games." << std::endl;
		return;
	}

	if (argc != 2)
	{
		quake::con << "save <savename> : save a game." << std::endl;
		return;
	}

	if (argv[1].find("..") != quake::string_view::npos)
	{
		quake::con << "Relative pathnames are not allowed." << std::endl;
		return;
	}
		
	for (i=0 ; i<svs.maxclients ; i++)
	{
		if (svs.clients[i].active && (svs.clients[i].edict->v.health <= 0) )
		{
			quake::con << "Can't savegame with a dead player" << std::endl;
			return;
		}
	}
	name += COM_GameDir();
	name += '/';
	name += argv[1];
	COM_DefaultExtension (name, ".sav");
	
	quake::con << "Saving game to " << name << std::endl;

	quake::ofstream f(name.c_str());

	if (!f.is_open())
	{
		quake::con << "ERROR: couldn't open." << std::endl;
		return;
	}
	f << (int)SAVEGAME_VERSION << std::endl;
	Host_SavegameComment(f);
	for (i = 0; i < NUM_SPAWN_PARMS; i++)
		f << (float)svs.clients->spawn_parms[i] << std::endl;
	f << (int)current_skill << std::endl;
	f << sv.name << std::endl;
	f << idCast<float>(sv.time) << std::endl;

// write the light styles

	for (i=0 ; i<MAX_LIGHTSTYLES ; i++)
	{
		if (sv.lightstyles[i])
			f << sv.lightstyles[i] << std::endl;
		else
			f << 'm' << std::endl;
	}


	ED_WriteGlobals (f);
	for (i=0 ; i<sv.num_edicts ; i++)
	{
		ED_Write (f, EDICT_NUM(i));
		f.flush();
	}
	f.close();
	quake::con << "done." << std::endl;
}


/*
===============
Host_Loadgame_f
===============
*/

void Host_Loadgame_f(cmd_source_t source, size_t argc, const quake::string_view argv[])
{
	quake::fixed_string<MAX_OSPATH>	name;
	FILE	*f;
	char	mapname[MAX_QPATH];
	float	time, tfloat;
	char	str[32768], *start;
	int		i, r;
	edict_t	*ent;
	int		entnum;
	int		version;
	float			spawn_parms[NUM_SPAWN_PARMS];

	if (source != src_command)
		return;

	if (argc != 2)
	{
		quake::con << "load <savename> : load a game" << std::endl;
		return;
	}

	cls.demonum = -1;		// stop demo loop in case this fails

	name += COM_GameDir();
	name += '/';
	name += argv[1];
	COM_DefaultExtension (name, ".sav");
	
// we can't call SCR_BeginLoadingPlaque, because too much stack space has
// been used.  The menu calls it before stuffing loadgame command
//	SCR_BeginLoadingPlaque ();

	Con_Printf ("Loading game from %s...\n", name);
	f = fopen (name.c_str(), "r");
	if (!f)
	{
		Con_Printf ("ERROR: couldn't open.\n");
		return;
	}

	fscanf (f, "%i\n", &version);
	if (version != SAVEGAME_VERSION)
	{
		fclose (f);
		Con_Printf ("Savegame is version %i, not %i\n", version, SAVEGAME_VERSION);
		return;
	}
	fscanf (f, "%s\n", str);
	for (i=0 ; i<NUM_SPAWN_PARMS ; i++)
		fscanf (f, "%f\n", &spawn_parms[i]);
// this silliness is so we can load 1.06 save files, which have float skill values
	fscanf (f, "%f\n", &tfloat);
	current_skill = (int)(tfloat + 0.1);
	Cvar_SetValue ("skill", (float)current_skill);

#ifdef QUAKE2
	Cvar_SetValue ("deathmatch", 0);
	Cvar_SetValue ("coop", 0);
	Cvar_SetValue ("teamplay", 0);
#endif

	fscanf (f, "%s\n",mapname);
	fscanf (f, "%f\n",&time);

	CL_Disconnect_f (source, argc, argv);
	
#ifdef QUAKE2
	SV_SpawnServer (mapname, NULL);
#else
	SV_SpawnServer (mapname);
#endif
	if (!sv.active)
	{
		Con_Printf ("Couldn't load map\n");
		return;
	}
	sv.paused = true;		// pause until all clients connect
	sv.loadgame = true;

// load the light styles

	for (i=0 ; i<MAX_LIGHTSTYLES ; i++)
	{
		fscanf (f, "%s\n", str);
		sv.lightstyles[i] = (char*)Hunk_Alloc (Q_strlen(str)+1);
		strcpy (sv.lightstyles[i], str);
	}

// load the edicts out of the savegame file
	entnum = -1;		// -1 is the globals
	while (!feof(f))
	{
		for (i=0 ; i<sizeof(str)-1 ; i++)
		{
			r = fgetc (f);
			if (r == EOF || !r)
				break;
			str[i] = r;
			if (r == '}')
			{
				i++;
				break;
			}
		}
		if (i == sizeof(str)-1)
			Sys_Error ("Loadgame buffer overflow");
		str[i] = 0;
		start = str;
		COM_Parser parser(str);
		quake::string_view token;
		if (!parser.Next(token)) break; // end of file

		if (token[0] != '{')
			Sys_Error ("First token isn't a brace");
			
		if (entnum == -1)
		{	// parse the global vars
			ED_ParseGlobals (parser); 
		}
		else
		{	// parse an edict

			ent = EDICT_NUM(entnum);
			if (!ent->vars) ent->vars = new edict_t::map_t;
			std::memset (&ent->v, 0, progs->entityfields * 4);
			ent->free = false;
			ED_ParseEdict (parser, ent);
	
		// link it into the bsp tree
			if (!ent->free)
				SV_LinkEdict (ent, false);
		}

		entnum++;
	}
	
	sv.num_edicts = entnum;
	sv.time = idCast<idTime>(time);

	fclose (f);

	for (i=0 ; i<NUM_SPAWN_PARMS ; i++)
		svs.clients->spawn_parms[i] = spawn_parms[i];

	if (cls.state != ca_dedicated)
	{
		CL_EstablishConnection ("local", source, argc, argv);
		Host_Reconnect_f (source,argc,argv);
	}
}

#ifdef QUAKE2
void SaveGamestate()
{
	char	name[256];
	FILE	*f;
	int		i;
	char	comment[SAVEGAME_COMMENT_LENGTH+1];
	edict_t	*ent;

	sprintf (name, "%s/%s.gip", com_gamedir, sv.name);
	
	Con_Printf ("Saving game to %s...\n", name);
	f = fopen (name, "w");
	if (!f)
	{
		Con_Printf ("ERROR: couldn't open.\n");
		return;
	}
	
	fprintf (f, "%i\n", SAVEGAME_VERSION);
	Host_SavegameComment (comment);
	fprintf (f, "%s\n", comment);
//	for (i=0 ; i<NUM_SPAWN_PARMS ; i++)
//		fprintf (f, "%f\n", svs.clients->spawn_parms[i]);
	fprintf (f, "%f\n", skill.value);
	fprintf (f, "%s\n", sv.name);
	fprintf (f, "%f\n", sv.time);

// write the light styles

	for (i=0 ; i<MAX_LIGHTSTYLES ; i++)
	{
		if (sv.lightstyles[i])
			fprintf (f, "%s\n", sv.lightstyles[i]);
		else
			fprintf (f,"m\n");
	}


	for (i=svs.maxclients+1 ; i<sv.num_edicts ; i++)
	{
		ent = EDICT_NUM(i);
		if ((int)ent->v.flags & FL_ARCHIVE_OVERRIDE)
			continue;
		fprintf (f, "%i\n",i);
		ED_Write (f, ent);
		fflush (f);
	}
	fclose (f);
	Con_Printf ("done.\n");
}

int LoadGamestate(char *level, char *startspot)
{
	char	name[MAX_OSPATH];
	FILE	*f;
	char	mapname[MAX_QPATH];
	float	time, sk;
	char	str[32768], *start;
	int		i, r;
	edict_t	*ent;
	int		entnum;
	int		version;
//	float	spawn_parms[NUM_SPAWN_PARMS];

	sprintf (name, "%s/%s.gip", com_gamedir, level);
	
	Con_Printf ("Loading game from %s...\n", name);
	f = fopen (name, "r");
	if (!f)
	{
		Con_Printf ("ERROR: couldn't open.\n");
		return -1;
	}

	fscanf (f, "%i\n", &version);
	if (version != SAVEGAME_VERSION)
	{
		fclose (f);
		Con_Printf ("Savegame is version %i, not %i\n", version, SAVEGAME_VERSION);
		return -1;
	}
	fscanf (f, "%s\n", str);
//	for (i=0 ; i<NUM_SPAWN_PARMS ; i++)
//		fscanf (f, "%f\n", &spawn_parms[i]);
	fscanf (f, "%f\n", &sk);
	Cvar_SetValue ("skill", sk);

	fscanf (f, "%s\n",mapname);
	fscanf (f, "%f\n",&time);

	SV_SpawnServer (mapname, startspot);

	if (!sv.active)
	{
		Con_Printf ("Couldn't load map\n");
		return -1;
	}

// load the light styles
	for (i=0 ; i<MAX_LIGHTSTYLES ; i++)
	{
		fscanf (f, "%s\n", str);
		sv.lightstyles[i] = Hunk_Alloc (strlen(str)+1);
		strcpy (sv.lightstyles[i], str);
	}

// load the edicts out of the savegame file
	while (!feof(f))
	{
		fscanf (f, "%i\n",&entnum);
		for (i=0 ; i<sizeof(str)-1 ; i++)
		{
			r = fgetc (f);
			if (r == EOF || !r)
				break;
			str[i] = r;
			if (r == '}')
			{
				i++;
				break;
			}
		}
		if (i == sizeof(str)-1)
			Sys_Error ("Loadgame buffer overflow");
		str[i] = 0;
		start = str;
		start = COM_Parse(str);
		if (!com_token[0])
			break;		// end of file
		if (strcmp(com_token,"{"))
			Sys_Error ("First token isn't a brace");
			
		// parse an edict

		ent = EDICT_NUM(entnum);
		memset (&ent->v, 0, progs->entityfields * 4);
		ent->free = false;
		ED_ParseEdict (start, ent);
	
		// link it into the bsp tree
		if (!ent->free)
			SV_LinkEdict (ent, false);
	}
	
//	sv.num_edicts = entnum;
	sv.time = time;
	fclose (f);

//	for (i=0 ; i<NUM_SPAWN_PARMS ; i++)
//		svs.clients->spawn_parms[i] = spawn_parms[i];

	return 0;
}

// changing levels within a unit
void Host_Changelevel2_f (void)
{
	char	level[MAX_QPATH];
	char	_startspot[MAX_QPATH];
	char	*startspot;

	if (Cmd_Argc() < 2)
	{
		Con_Printf ("changelevel2 <levelname> : continue game on a new level in the unit\n");
		return;
	}
	if (!sv.active || cls.demoplayback)
	{
		Con_Printf ("Only the server may changelevel\n");
		return;
	}

	strcpy (level, Cmd_Argv(1));
	if (Cmd_Argc() == 2)
		startspot = NULL;
	else
	{
		strcpy (_startspot, Cmd_Argv(2));
		startspot = _startspot;
	}

	SV_SaveSpawnparms ();

	// save the current level's state
	SaveGamestate ();

	// try to restore the new level
	if (LoadGamestate (level, startspot))
		SV_SpawnServer (level, startspot);
}
#endif


//============================================================================

/*
======================
Host_Name_f
======================
*/
void Host_Name_f(cmd_source_t source, size_t argc, const quake::string_view argv[])
{
	quake::fixed_string_stream<16> newName;
	
	if (argc  == 1)
	{
		quake::con << "\"name\" is \"" << cl_name.string << '"' << std::endl;
		return;
	}
	if (argc == 2)
		newName << argv[1];
	else
		newName << argv[0];

	if (source == src_command)
	{
		if (cl_name.string == newName.str())
			return;
		Cvar_Set ("_cl_name", newName.str());
		if (cls.state == ca_connected)
			Cmd_ForwardToServer (source, argc, argv);
		return;
	}

	if (host_client->name[0] && strcmp(host_client->name, "unconnected"))
		if (newName.str() == host_client->name)
			quake::con << host_client->name << "renamed to " << newName.str() << std::endl;
	assert(newName.size() + 1 < sizeof(host_client->name));
	static_cast<quake::string_view>(newName.str()).copy(host_client->name, sizeof(host_client->name));
	host_client->name[newName.size()] = 0;

	host_client->edict->v.netname = host_client->name - pr_strings;
	
// send notification to all clients
	
	sv.reliable_datagram.WriteByte(svc_updatename);
	sv.reliable_datagram.WriteByte(host_client - svs.clients);
	sv.reliable_datagram.WriteString(host_client->name);
}

	
void Host_Version_f(cmd_source_t source, size_t argc, const quake::string_view argv[])
{
	Con_Printf ("Version %4.2f\n", VERSION);
	Con_Printf ("Exe: " __TIME__ " " __DATE__ "\n");
}

#ifdef IDGODS
void Host_Please_f (void)
{
	client_t *cl;
	int			j;
	
	if (cmd_source != src_command)
		return;

	if ((Cmd_Argc () == 3) && Q_strcmp(Cmd_Argv(1), "#") == 0)
	{
		j = Q_atof(Cmd_Argv(2)) - 1;
		if (j < 0 || j >= svs.maxclients)
			return;
		if (!svs.clients[j].active)
			return;
		cl = &svs.clients[j];
		if (cl->privileged)
		{
			cl->privileged = false;
			cl->edict->v.flags = (int)cl->edict->v.flags & ~(FL_GODMODE|FL_NOTARGET);
			cl->edict->v.movetype = MOVETYPE_WALK;
			noclip_anglehack = false;
		}
		else
			cl->privileged = true;
	}

	if (Cmd_Argc () != 2)
		return;

	for (j=0, cl = svs.clients ; j<svs.maxclients ; j++, cl++)
	{
		if (!cl->active)
			continue;
		if (Q_strcasecmp(cl->name, Cmd_Argv(1)) == 0)
		{
			if (cl->privileged)
			{
				cl->privileged = false;
				cl->edict->v.flags = (int)cl->edict->v.flags & ~(FL_GODMODE|FL_NOTARGET);
				cl->edict->v.movetype = MOVETYPE_WALK;
				noclip_anglehack = false;
			}
			else
				cl->privileged = true;
			break;
		}
	}
}
#endif


void Host_Say(cmd_source_t source, size_t argc, const quake::string_view argv[], qboolean teamonly)
{
	client_t *client;
	client_t *save;
	int		j;
	quake::fixed_string_stream<64> text;
	qboolean	fromServer = false;

	if (source == src_command)
	{
		if (cls.state == ca_dedicated)
		{
			fromServer = true;
			teamonly = false;
		}
		else
		{
			Cmd_ForwardToServer (source, argc, argv);
			return;
		}
	}

	if (argc < 2)
		return;

	save = host_client;


// turn on color set 1
	if (!fromServer)
		text << (char)1 << save->name << ':';
	//	sprintf (text, "%c%s: ", 1, save->name);
	else
		text << (char)1 << '<' << save->name << "> ";
		//sprintf (text, "%c<%s> ", 1, hostname.string);
	for (size_t i = 1; i < argc; i++) {
		// remove quotes if present
		quake::string_view p = argv[i];
		text << p << ' ';
	}
	text << std::endl;


	for (j = 0, client = svs.clients; j < svs.maxclients; j++, client++)
	{
		if (!client || !client->active || !client->spawned)
			continue;
		if (teamplay.value && teamonly && client->edict->v.team != save->edict->v.team)
			continue;
		host_client = client;
		SV_ClientPrintf("%s", text.str().c_str());
	}
	host_client = save;

	Sys_Printf("%s", text.str().c_str() +1);
}


void Host_Say_f(cmd_source_t source, size_t argc, const quake::string_view argv[])
{
	Host_Say(source,argc,argv, false);
}


void Host_Say_Team_f(cmd_source_t source, size_t argc, const quake::string_view argv[])
{
	Host_Say(source, argc, argv, true);
}


void Host_Tell_f(cmd_source_t source, size_t argc, const quake::string_view argv[])
{
	client_t *client;
	client_t *save;
	int		j;
	quake::fixed_string_stream<64> text;

	if (source == src_command)
	{
		Cmd_ForwardToServer (source, argc, argv);
		return;
	}

	if (argc < 3)
		return;

	text << host_client->name << ": ";
	for (size_t i = 1; i < argc; i++) {
		// remove quotes if present
		quake::string_view p = argv[i];
		text << p << ' ';
	}
	text << std::endl;


	save = host_client;
	for (j = 0, client = svs.clients; j < svs.maxclients; j++, client++)
	{
		if (!client->active || !client->spawned)
			continue;
		if (Q_strcasecmp(argv[1], client->name))
			continue;
		host_client = client;
		SV_ClientPrintf("%s", text.str().c_str());
		break;
	}
	host_client = save;
}


/*
==================
Host_Color_f
==================
*/
void Host_Color_f(cmd_source_t source, size_t argc, const quake::string_view argv[])
{
	int		top, bottom;
	int		playercolor;
	
	if (argc == 1)
	{
		Con_Printf ("\"color\" is \"%i %i\"\n", ((int)cl_color.value) >> 4, ((int)cl_color.value) & 0x0f);
		Con_Printf ("color <0-13> [0-13]\n");
		return;
	}

	if (argc == 2)
		top = bottom = Q_atoi(argv[1]);
	else
	{
		top = Q_atoi(argv[1]);
		bottom = Q_atoi(argv[2]);
	}
	
	top &= 15;
	if (top > 13)
		top = 13;
	bottom &= 15;
	if (bottom > 13)
		bottom = 13;
	
	playercolor = top*16 + bottom;

	if (source == src_command)
	{
		Cvar_SetValue ("_cl_color", playercolor);
		if (cls.state == ca_connected)
			Cmd_ForwardToServer (source, argc, argv);
		return;
	}

	host_client->colors = playercolor;
	host_client->edict->v.team = bottom + 1;

// send notification to all clients
	sv.reliable_datagram.WriteByte(svc_updatecolors);
	sv.reliable_datagram.WriteByte(host_client - svs.clients);
	sv.reliable_datagram.WriteByte(host_client->colors);
}

/*
==================
Host_Kill_f
==================
*/
void Host_Kill_f(cmd_source_t source, size_t argc, const quake::string_view argv[])
{
	if (source == src_command)
	{
		Cmd_ForwardToServer (source, argc, argv);
		return;
	}

	if (sv_player->v.health <= 0)
	{
		SV_ClientPrintf ("Can't suicide -- allready dead!\n");
		return;
	}
	
	pr_global_struct->time = sv.time;
	pr_global_struct->self = EDICT_TO_PROG(sv_player);
	PR_ExecuteProgram (pr_global_struct->ClientKill);
}


/*
==================
Host_Pause_f
==================
*/
void Host_Pause_f(cmd_source_t source, size_t argc, const quake::string_view argv[])
{
	
	if (source == src_command)
	{
		Cmd_ForwardToServer (source, argc, argv);
		return;
	}
	if (!pausable.value)
		SV_ClientPrintf ("Pause not allowed.\n");
	else
	{
		sv.paused ^= 1;

		if (sv.paused)
		{
			SV_BroadcastPrintf ("%s paused the game\n", pr_strings + sv_player->v.netname);
		}
		else
		{
			SV_BroadcastPrintf ("%s unpaused the game\n",pr_strings + sv_player->v.netname);
		}

	// send notification to all clients
		sv.reliable_datagram.WriteByte(svc_setpause);
		sv.reliable_datagram.WriteByte(sv.paused);
	}
}

//===========================================================================


/*
==================
Host_PreSpawn_f
==================
*/
void Host_PreSpawn_f(cmd_source_t source, size_t argc, const quake::string_view argv[])
{
	if (source == src_command)
	{
		Con_Printf ("prespawn is not valid from the console\n");
		return;
	}

	if (host_client->spawned)
	{
		Con_Printf ("prespawn not valid -- allready spawned\n");
		return;
	}
	
	host_client->message.Write ( sv.signon.data(), sv.signon.size());
	host_client->message.WriteByte(svc_signonnum);
	host_client->message.WriteByte(2);
	host_client->sendsignon = true;
}

/*
==================
Host_Spawn_f
==================
*/
void Host_Spawn_f(cmd_source_t source, size_t argc, const quake::string_view argv[])
{
	int		i;
	client_t	*client;
	edict_t	*ent;

	if (source == src_command)
	{
		Con_Printf ("spawn is not valid from the console\n");
		return;
	}

	if (host_client->spawned)
	{
		Con_Printf ("Spawn not valid -- allready spawned\n");
		return;
	}

// run the entrance script
	if (sv.loadgame)
	{	// loaded games are fully inited allready
		// if this is the last client to be connected, unpause
		sv.paused = false;
	}
	else
	{
		// set up the edict
		ent = host_client->edict;

		memset (&ent->v, 0, progs->entityfields * 4);
		ent->v.colormap = NUM_FOR_EDICT(ent);
		ent->v.team = (host_client->colors & 15) + 1;
		ent->v.netname = host_client->name - pr_strings;

		// copy spawn parms out of the client_t

		for (i=0 ; i< NUM_SPAWN_PARMS ; i++)
			(&pr_global_struct->parm1)[i] = host_client->spawn_parms[i];

		// call the spawn function

		pr_global_struct->time = sv.time;
		pr_global_struct->self = EDICT_TO_PROG(sv_player);
		PR_ExecuteProgram (pr_global_struct->ClientConnect);

		if ((Sys_FloatTime() - host_client->netconnection->connecttime) <= sv.time)
			Sys_Printf ("%s entered the game\n", host_client->name);

		PR_ExecuteProgram (pr_global_struct->PutClientInServer);	
	}


// send all current names, colors, and frag counts
	host_client->message.Clear();

// send time of update
	host_client->message.WriteByte(svc_time);
	host_client->message.WriteFloat(idCast<float>(sv.time));

	for (i=0, client = svs.clients ; i<svs.maxclients ; i++, client++)
	{
		host_client->message.WriteByte(svc_updatename);
		host_client->message.WriteByte(i);
		host_client->message.WriteString(client->name);
		host_client->message.WriteByte(svc_updatefrags);
		host_client->message.WriteByte(i);
		host_client->message.WriteShort(client->old_frags);
		host_client->message.WriteByte(svc_updatecolors);
		host_client->message.WriteByte(i);
		host_client->message.WriteByte(client->colors);
	}
	
// send all current light styles
	for (i=0 ; i<MAX_LIGHTSTYLES ; i++)
	{
		host_client->message.WriteByte(svc_lightstyle);
		host_client->message.WriteByte((char)i);
		host_client->message.WriteString(sv.lightstyles[i]);
	}

//
// send some stats
//
	host_client->message.WriteByte(svc_updatestat);
	host_client->message.WriteByte(STAT_TOTALSECRETS);
	host_client->message.WriteLong(pr_global_struct->total_secrets);

	host_client->message.WriteByte(svc_updatestat);
	host_client->message.WriteByte(STAT_TOTALMONSTERS);
	host_client->message.WriteLong(pr_global_struct->total_monsters);

	host_client->message.WriteByte(svc_updatestat);
	host_client->message.WriteByte(STAT_SECRETS);
	host_client->message.WriteLong(pr_global_struct->found_secrets);

	host_client->message.WriteByte(svc_updatestat);
	host_client->message.WriteByte(STAT_MONSTERS);
	host_client->message.WriteLong(pr_global_struct->killed_monsters);

	
//
// send a fixangle
// Never send a roll angle, because savegames can catch the server
// in a state where it is expecting the client to correct the angle
// and it won't happen if the game was just loaded, so you wind up
// with a permanent head tilt
	ent = EDICT_NUM( 1 + (host_client - svs.clients) );
	host_client->message.WriteByte(svc_setangle);
	for (i=0 ; i < 2 ; i++)
		host_client->message.WriteAngle(ent->v.angles[i] );
	host_client->message.WriteAngle(0 );

	SV_WriteClientdataToMessage (sv_player, &host_client->message);

	host_client->message.WriteByte(svc_signonnum);
	host_client->message.WriteByte(3);
	host_client->sendsignon = true;
}

/*
==================
Host_Begin_f
==================
*/
void Host_Begin_f(cmd_source_t source, size_t argc, const quake::string_view argv[])
{
	if (source == src_command)
	{
		Con_Printf ("begin is not valid from the console\n");
		return;
	}

	host_client->spawned = true;
}

//===========================================================================


/*
==================
Host_Kick_f

Kicks a user off of the server
==================
*/
void Host_Kick_f (cmd_source_t source, size_t argc, const quake::string_view argv[])
{
	const char		*who;
	client_t	*save;
	int			i;
	qboolean	byNumber = false;

	if (source == src_command)
	{
		if (!sv.active)
		{
			Cmd_ForwardToServer (source, argc, argv);
			return;
		}
	}
	else if (pr_global_struct->deathmatch && !host_client->privileged)
		return;

	save = host_client;

	if (argc > 2 && argv[1] ==  "#")
	{
		i = Q_atof(argv[2]) - 1;
		if (i < 0 || i >= svs.maxclients)
			return;
		if (!svs.clients[i].active)
			return;
		host_client = &svs.clients[i];
		byNumber = true;
	}
	else
	{
		for (i = 0, host_client = svs.clients; i < svs.maxclients; i++, host_client++)
		{
			if (!host_client->active)
				continue;
			if (Q_strcasecmp(argv[1],host_client->name) == 0)
				break;
		}
	}

	if (i < svs.maxclients)
	{
		if (source == src_command)
			if (cls.state == ca_dedicated)
				who = "Console";
			else
				who = cl_name.string;
		else
			who = save->name;

		// can't kick yourself!
		if (host_client == save)
			return;

		if (argc > 2)
		{
			quake::fixed_string<128> message;
			for (size_t j = byNumber ? 2 : 1; j < argc; j++) {
				message += argv[i];
				message += ' ';
			}
			if (!message.empty()) {
				SV_ClientPrintf("Kicked by %s: %s\n", who, message.c_str());
			}
			else
				SV_ClientPrintf("Kicked by %s\n", who);
		}

		SV_DropClient (false);
	}

	host_client = save;
}

/*
===============================================================================

DEBUGGING TOOLS

===============================================================================
*/

/*
==================
Host_Give_f
==================
*/
void Host_Give_f(cmd_source_t source, size_t argc, const quake::string_view argv[])
{
	int		v, w;
	eval_t	*val;

	if (source == src_command)
	{
		Cmd_ForwardToServer (source, argc, argv);
		return;
	}

	if (pr_global_struct->deathmatch && !host_client->privileged)
		return;

	quake::string_view t = argv[1];
	v = Q_atoi (argv[2]);
	
	switch (t[0])
	{
   case '0':
   case '1':
   case '2':
   case '3':
   case '4':
   case '5':
   case '6':
   case '7':
   case '8':
   case '9':
      // MED 01/04/97 added hipnotic give stuff
      if (hipnotic)
      {
         if (t[0] == '6')
         {
            if (t[1] == 'a')
               sv_player->v.items = (int)sv_player->v.items | HIT_PROXIMITY_GUN;
            else
               sv_player->v.items = (int)sv_player->v.items | IT_GRENADE_LAUNCHER;
         }
         else if (t[0] == '9')
            sv_player->v.items = (int)sv_player->v.items | HIT_LASER_CANNON;
         else if (t[0] == '0')
            sv_player->v.items = (int)sv_player->v.items | HIT_MJOLNIR;
         else if (t[0] >= '2')
            sv_player->v.items = (int)sv_player->v.items | (IT_SHOTGUN << (t[0] - '2'));
      }
      else
      {
         if (t[0] >= '2')
            sv_player->v.items = (int)sv_player->v.items | (IT_SHOTGUN << (t[0] - '2'));
      }
		break;
	
    case 's':
		if (rogue)
		{
	        val = GetEdictFieldValue(sv_player, "ammo_shells1");
		    if (val)
			    val->_float = v;
		}

        sv_player->v.ammo_shells = v;
        break;		
    case 'n':
		if (rogue)
		{
			val = GetEdictFieldValue(sv_player, "ammo_nails1");
			if (val)
			{
				val->_float = v;
				if (sv_player->v.weapon <= IT_LIGHTNING)
					sv_player->v.ammo_nails = v;
			}
		}
		else
		{
			sv_player->v.ammo_nails = v;
		}
        break;		
    case 'l':
		if (rogue)
		{
			val = GetEdictFieldValue(sv_player, "ammo_lava_nails");
			if (val)
			{
				val->_float = v;
				if (sv_player->v.weapon > IT_LIGHTNING)
					sv_player->v.ammo_nails = v;
			}
		}
        break;
    case 'r':
		if (rogue)
		{
			val = GetEdictFieldValue(sv_player, "ammo_rockets1");
			if (val)
			{
				val->_float = v;
				if (sv_player->v.weapon <= IT_LIGHTNING)
					sv_player->v.ammo_rockets = v;
			}
		}
		else
		{
			sv_player->v.ammo_rockets = v;
		}
        break;		
    case 'm':
		if (rogue)
		{
			val = GetEdictFieldValue(sv_player, "ammo_multi_rockets");
			if (val)
			{
				val->_float = v;
				if (sv_player->v.weapon > IT_LIGHTNING)
					sv_player->v.ammo_rockets = v;
			}
		}
        break;		
    case 'h':
        sv_player->v.health = v;
        break;		
    case 'c':
		if (rogue)
		{
			val = GetEdictFieldValue(sv_player, "ammo_cells1");
			if (val)
			{
				val->_float = v;
				if (sv_player->v.weapon <= IT_LIGHTNING)
					sv_player->v.ammo_cells = v;
			}
		}
		else
		{
			sv_player->v.ammo_cells = v;
		}
        break;		
    case 'p':
		if (rogue)
		{
			val = GetEdictFieldValue(sv_player, "ammo_plasma");
			if (val)
			{
				val->_float = v;
				if (sv_player->v.weapon > IT_LIGHTNING)
					sv_player->v.ammo_cells = v;
			}
		}
        break;		
    }
}

edict_t	*FindViewthing (void)
{
	int		i;
	edict_t	*e;
	
	for (i=0 ; i<sv.num_edicts ; i++)
	{
		e = EDICT_NUM(i);
		if ( !strcmp (pr_strings + e->v.classname, "viewthing") )
			return e;
	}
	Con_Printf ("No viewthing on map\n");
	return NULL;
}

/*
==================
Host_Viewmodel_f
==================
*/
void Host_Viewmodel_f(cmd_source_t source, size_t argc, const quake::string_view argv[])
{
	edict_t	*e;
	model_t	*m;

	e = FindViewthing ();
	if (!e)
		return;

	m = Mod_ForName (argv[1], false);
	if (!m)
	{
		Con_Printf ("Can't load %s\n", argv[1]);
		return;
	}
	
	e->v.frame = 0;
	cl.model_precache[(int)e->v.modelindex] = m;
}

/*
==================
Host_Viewframe_f
==================
*/
void Host_Viewframe_f(cmd_source_t source, size_t argc, const quake::string_view argv[])
{
	edict_t	*e;
	int		f;
	model_t	*m;

	e = FindViewthing ();
	if (!e)
		return;
	m = cl.model_precache[(int)e->v.modelindex];

	f = Q_atoi(argv[1]);
	if (f >= m->numframes)
		f = m->numframes-1;

	e->v.frame = f;		
}


void PrintFrameName (model_t *m, int frame)
{
	aliashdr_t 			*hdr;
	maliasframedesc_t	*pframedesc;

	hdr = (aliashdr_t *)Mod_Extradata (m);
	if (!hdr)
		return;
	pframedesc = &hdr->frames[frame];
	
	Con_Printf ("frame %i: %s\n", frame, pframedesc->name);
}

/*
==================
Host_Viewnext_f
==================
*/
void Host_Viewnext_f(cmd_source_t source, size_t argc, const quake::string_view argv[])
{
	edict_t	*e;
	model_t	*m;
	
	e = FindViewthing ();
	if (!e)
		return;
	m = cl.model_precache[(int)e->v.modelindex];

	e->v.frame = e->v.frame + 1;
	if (e->v.frame >= m->numframes)
		e->v.frame = m->numframes - 1;

	PrintFrameName (m, e->v.frame);		
}

/*
==================
Host_Viewprev_f
==================
*/
void Host_Viewprev_f(cmd_source_t source, size_t argc, const quake::string_view argv[])
{
	edict_t	*e;
	model_t	*m;

	e = FindViewthing ();
	if (!e)
		return;

	m = cl.model_precache[(int)e->v.modelindex];

	e->v.frame = e->v.frame - 1;
	if (e->v.frame < 0)
		e->v.frame = 0;

	PrintFrameName (m, e->v.frame);		
}

/*
===============================================================================

DEMO LOOP CONTROL

===============================================================================
*/


/*
==================
Host_Startdemos_f
==================
*/
void Host_Startdemos_f(cmd_source_t source, size_t argc, const quake::string_view argv[])
{
	int		i, c;

	if (cls.state == ca_dedicated)
	{
		if (!sv.active)
			Cbuf_AddText ("map start\n");
		return;
	}

	c = argc - 1;
	if (c > MAX_DEMOS)
	{
		Con_Printf ("Max %i demos in demoloop\n", MAX_DEMOS);
		c = MAX_DEMOS;
	}
	quake::con << c << " demo(s) in loop" << std::endl;

	for (size_t i = 1; i < c + 1; i++)
		argv[i].copy(cls.demos[i - 1], sizeof(cls.demos[i - 1]));


	if (!sv.active && cls.demonum != -1 && !cls.demoplayback)
	{
		cls.demonum = 0;
		CL_NextDemo ();
	}
	else
		cls.demonum = -1;
}


/*
==================
Host_Demos_f

Return to looping demos
==================
*/
void Host_Demos_f(cmd_source_t source, size_t argc, const quake::string_view argv[])
{
	if (cls.state == ca_dedicated)
		return;
	if (cls.demonum == -1)
		cls.demonum = 1;
	CL_Disconnect_f(source, argc, argv);
	CL_NextDemo ();
}

/*
==================
Host_Stopdemo_f

Return to looping demos
==================
*/
void Host_Stopdemo_f(cmd_source_t source, size_t argc, const quake::string_view argv[])
{
	if (cls.state == ca_dedicated)
		return;
	if (!cls.demoplayback)
		return;
	CL_StopPlayback ();
	CL_Disconnect ();
}

//=============================================================================

/*
==================
Host_InitCommands
==================
*/
void Host_InitCommands (void)
{
	Cmd_AddCommand ("status", Host_Status_f);
	Cmd_AddCommand ("quit", Host_Quit_f);
	Cmd_AddCommand ("god", Host_God_f);
	Cmd_AddCommand ("notarget", Host_Notarget_f);
	Cmd_AddCommand ("fly", Host_Fly_f);
	Cmd_AddCommand ("map", Host_Map_f);
	Cmd_AddCommand ("restart", Host_Restart_f);
	Cmd_AddCommand ("changelevel", Host_Changelevel_f);
#ifdef QUAKE2
	Cmd_AddCommand ("changelevel2", Host_Changelevel2_f);
#endif
	Cmd_AddCommand ("connect", Host_Connect_f);
	Cmd_AddCommand ("reconnect", Host_Reconnect_f);
	Cmd_AddCommand ("name", Host_Name_f);
	Cmd_AddCommand ("noclip", Host_Noclip_f);
	Cmd_AddCommand ("version", Host_Version_f);
#ifdef IDGODS
	Cmd_AddCommand ("please", Host_Please_f);
#endif
	Cmd_AddCommand ("say", Host_Say_f);
	Cmd_AddCommand ("say_team", Host_Say_Team_f);
	Cmd_AddCommand ("tell", Host_Tell_f);
	Cmd_AddCommand ("color", Host_Color_f);
	Cmd_AddCommand ("kill", Host_Kill_f);
	Cmd_AddCommand ("pause", Host_Pause_f);
	Cmd_AddCommand ("spawn", Host_Spawn_f);
	Cmd_AddCommand ("begin", Host_Begin_f);
	Cmd_AddCommand ("prespawn", Host_PreSpawn_f);
	Cmd_AddCommand ("kick", Host_Kick_f);
	Cmd_AddCommand ("ping", Host_Ping_f);
	Cmd_AddCommand ("load", Host_Loadgame_f);
	Cmd_AddCommand ("save", Host_Savegame_f);
	Cmd_AddCommand ("give", Host_Give_f);

	Cmd_AddCommand ("startdemos", Host_Startdemos_f);
	Cmd_AddCommand ("demos", Host_Demos_f);
	Cmd_AddCommand ("stopdemo", Host_Stopdemo_f);

	Cmd_AddCommand ("viewmodel", Host_Viewmodel_f);
	Cmd_AddCommand ("viewframe", Host_Viewframe_f);
	Cmd_AddCommand ("viewnext", Host_Viewnext_f);
	Cmd_AddCommand ("viewprev", Host_Viewprev_f);

	Cmd_AddCommand ("mcache", Mod_Print);
}
