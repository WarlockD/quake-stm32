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
void CL_FinishTimeDemo (void);
using namespace std::chrono;

/*
==============================================================================

DEMO CODE

When a demo is playing back, all NET_SendMessages are skipped, and
NET_GetMessages are read from the demo file.

Whenever cl.time gets past the last received message, another message is
read from the demo file.
==============================================================================
*/

/*
==============
CL_StopPlayback

Called when a demo file runs out, or the user starts a game
==============
*/
void CL_StopPlayback (void)
{
	if (!cls.demoplayback)
		return;

	cls.demofile.close();
	cls.demoplayback = false;
	cls.state = ca_disconnected;

	if (cls.timedemo)
		CL_FinishTimeDemo ();
}

/*
====================
CL_WriteDemoMessage

Dumps the current net message, prefixed by the length and view angles
====================
*/
void CL_WriteDemoMessage (void)
{
	int		len;
	int		i;
	id_little_binary_writer bw(cls.demofile);
	bw << net_message.size();
	for (i = 0; i < 3; i++)
		bw << cl.viewangles[i];
	bw.write(net_message.data(), net_message.size());
	cls.demofile.flush();
}

/*
====================
CL_GetMessage

Handles recording and playback of demos, on top of NET_ code
====================
*/
int CL_GetMessage (void)
{
	int		r, i;
	
	if	(cls.demoplayback)
	{
	// decide if it is time to grab the next message		
		if (cls.signon == SIGNONS)	// allways grab until fully connected
		{
			if (cls.timedemo)
			{
				if (host_framecount == cls.td_lastframe)
					return 0;		// allready read this frame's message
				cls.td_lastframe = host_framecount;
			// if this is the second frame, grab the real td_starttime
			// so the bogus time on the first frame doesn't count
				if (host_framecount == cls.td_startframe + 1)
					cls.td_starttime = realtime;
			}
			else if ( /* cl.time > 0 && */ cl.time <= cl.mtime[0])
			{
					return 0;		// don't need another message yet
			}
		}
		id_little_binary_reader br(cls.demofile);
		br >> i;
		net_message.resize(i);
		VectorCopy (cl.mviewangles[0], cl.mviewangles[1]);
		for (i = 0; i < 3; i++)
			br >> cl.mviewangles[0][i];

		if (net_message.size() > MAX_MSGLEN)
			Sys_Error ("Demo message > MAX_MSGLEN");
		br.read(net_message.data(), net_message.size());
		if (!cls.demofile)
		{
			CL_StopPlayback ();
			return 0;
		}
	
		return 1;
	}

	while (1)
	{
		r = NET_GetMessage (cls.netcon);
		
		if (r != 1 && r != 2)
			return r;
	
	// discard nop keepalive message
		if (net_message.size() == 1 && net_message.data()[0] == svc_nop)
			Con_Printf ("<-- server to client keepalive\n");
		else
			break;
	}

	if (cls.demorecording)
		CL_WriteDemoMessage ();
	
	return r;
}


/*
====================
CL_Stop_f

stop recording a demo
====================
*/
void CL_Stop() {
	if (!cls.demorecording)
	{
		Con_Printf("Not recording a demo.\n");
		return;
	}

	// write a disconnect message to the demo file
	net_message.Clear();
	net_message.WriteByte(svc_disconnect);
	CL_WriteDemoMessage();

	// finish up
	cls.demofile.close();
	cls.demorecording = false;
	Con_Printf("Completed demo\n");
}
void CL_Stop_f (cmd_source_t source, size_t argc, const quake::string_view argv[])
{
	if (source != src_command)
		return;
	CL_Stop();
}

/*
====================
CL_Record_f

record <demoname> <map> [cd track]
====================
*/
void CL_Record_f (cmd_source_t source, size_t argc, const quake::string_view args[])
{
	int		c;

	int		track;

	if (source != src_command)
		return;

	c = argc;
	if (c != 2 && c != 3 && c != 4)
	{
		Con_Printf ("record <demoname> [<map> [cd track]]\n");
		return;
	}

	if (args[1].find("..") != quake::string_view::npos)
	{
		Con_Printf ("Relative pathnames are not allowed.\n");
		return;
	}

	if (c == 2 && cls.state == ca_connected)
	{
		Con_Printf("Can not record - already connected to server\nClient demo recording must be started before connecting\n");
		return;
	}

// write the forced cd track number, or -1
	if (c == 4)
	{
		track = Q_atoi(args[3]);
		Con_Printf ("Forcing CD track to %i\n", cls.forcetrack);
	}
	else
		track = -1;	

	quake::path_string name;

	name += COM_GameDir();
	name += '/';
	name += args[1];

//
// start the map up
//
	if (c > 2) {
		quake::fixed_string_stream<128> va;
		va << "map " << args[2];
		execute_args(va.str(), src_command);
	}

	
//
// open the demo file
//
	COM_DefaultExtension (name, ".dem");

	Con_Printf ("recording to %s.\n", name.c_str());
	cls.demofile.open(name.c_str(), std::ios_base::out);
	if (!cls.demofile)
	{
		Con_Printf ("ERROR: couldn't open.\n");
		return;
	}
	cls.demofile << cls.forcetrack << '\n';
	cls.demofile.flush();
	
	cls.demorecording = true;
}


/*
====================
CL_PlayDemo_f

play [demoname]
====================
*/

void CL_PlayDemo_f(cmd_source_t source, size_t argc, const quake::string_view args[])
{
	int c;
	qboolean neg = false;

	if (source != src_command)
		return;

	if (argc != 2)
	{
		Con_Printf ("play <demoname> : plays a demo\n");
		return;
	}

//
// disconnect from server
//
	CL_Disconnect ();
	
//
// open the demo file
//
//	ZString name(args[1]);
	quake::path_string name(args[1]);

	COM_DefaultExtension (name, ".dem");
	

	cls.demofile = COM_FindFile(name.c_str());
	if (!cls.demofile.is_open())
	{
		quake::con << "ERROR: couldn't open demo " << name << std::endl;
		cls.demonum = -1;		// stop demo loop
		return;
	}
	else {
		quake::con << "Playing demo from " << name << std::endl;
	}

	cls.demoplayback = true;
	cls.state = ca_connected;
	cls.forcetrack = 0;

	while ((c = cls.demofile.get()) != '\n')
		if (c == '-')
			neg = true;
		else
			cls.forcetrack = cls.forcetrack * 10 + (c - '0');

	if (neg)
		cls.forcetrack = -cls.forcetrack;
// ZOID, fscanf is evil
//	fscanf (cls.demofile, "%i\n", &cls.forcetrack);
}

/*
====================
CL_FinishTimeDemo

====================
*/
void CL_FinishTimeDemo (void)
{
	int		frames;
	idTime	time;
	
	cls.timedemo = false;
	
// the first frame didn't count
	frames = (host_framecount - cls.td_startframe) - 1;
	time = realtime - cls.td_starttime;
	if (time != idTime::zero())
		time = 1s;
	Con_Printf ("%i frames %5.1f seconds %5.1f fps\n", frames, idCast<float>(time), frames/ idCast<float>(time));
}

/*
====================
CL_TimeDemo_f

timedemo [demoname]
====================
*/
void CL_TimeDemo_f(cmd_source_t source, size_t argc, const quake::string_view args[])
{
	if (source != src_command)
		return;

	if (argc != 2)
	{
		Con_Printf ("timedemo <demoname> : gets demo speeds\n");
		return;
	}

	CL_PlayDemo_f (source,argc, args);
	
// cls.td_starttime will be grabbed at the second frame of the demo, so
// all the loading time doesn't get counted
	
	cls.timedemo = true;
	cls.td_startframe = host_framecount;
	cls.td_lastframe = -1;		// get a new message this frame
}

