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

Whenever quake::cl.time gets past the last received message, another message is
read from the demo file.
==============================================================================
*/

/*
==============
CL_StopPlayback

Called when a demo file runs out, or the user starts a game
==============
*/
void client_static_t::stop_playback(void)
{
	if (!demoplayback)
		return;

	demofile.close();
	demoplayback = false;
	state = ca_disconnected;

	if (timedemo)
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
	id_little_binary_writer bw(quake::cls.demofile);
	bw << net_message.size();
	for (int i = 0; i < 3; i++)
		bw << quake::cl.viewangles[i];
	bw.write(net_message.data(), net_message.size());
	quake::cls.demofile.flush();
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
	
	if	(quake::cls.demoplayback)
	{
	// decide if it is time to grab the next message		
		if (quake::cls.signon == SIGNONS)	// allways grab until fully connected
		{
			if (quake::cls.timedemo)
			{
				if (host_framecount == quake::cls.td_lastframe)
					return 0;		// allready read this frame's message
				quake::cls.td_lastframe = host_framecount;
			// if this is the second frame, grab the real td_starttime
			// so the bogus time on the first frame doesn't count
				if (host_framecount == quake::cls.td_startframe + 1)
					quake::cls.td_starttime = realtime;
			}
			else if ( /* quake::cl.time > 0 && */ quake::cl.time <= quake::cl.mtime[0])
			{
					return 0;		// don't need another message yet
			}
		}
		id_little_binary_reader br(quake::cls.demofile);
		br >> i;
		net_message.resize(i);
		VectorCopy (quake::cl.mviewangles[0], quake::cl.mviewangles[1]);
		for (i = 0; i < 3; i++)
			br >> quake::cl.mviewangles[0][i];

		if (net_message.size() > MAX_MSGLEN)
			Sys_Error ("Demo message > MAX_MSGLEN");
		br.read(net_message.data(), net_message.size());
		if (!quake::cls.demofile.is_open())
		{
			quake::cls.stop_playback();
			return 0;
		}
	
		return 1;
	}

	while (1)
	{
		r = NET_GetMessage (quake::cls.netcon);
		
		if (r != 1 && r != 2)
			return r;
	
	// discard nop keepalive message
		if (net_message.size() == 1 && net_message.data()[0] == svc_nop)
			Con_Printf ("<-- server to client keepalive\n");
		else
			break;
	}

	if (quake::cls.demorecording)
		CL_WriteDemoMessage ();
	
	return r;
}


/*
====================
CL_Stop_f

stop recording a demo
====================
*/
void client_static_t::stop() {
	if (!demorecording)
	{
		Con_Printf("Not recording a demo.\n");
		return;
	}

	// write a disconnect message to the demo file
	net_message.Clear();
	net_message.WriteByte(svc_disconnect);
	CL_WriteDemoMessage();

	// finish up
	demofile.close();
	demorecording = false;
	quake::con << "Completed demo" << std::endl;
}
void CL_Stop_f (cmd_source_t source, const StringArgs& args)
{
	if (source != src_command)
		return;
	quake::cls.stop();

}

/*
====================
CL_Record_f

record <demoname> <map> [cd track]
====================
*/
void CL_Record_f (cmd_source_t source, const StringArgs& args)
{
	int		c;

	int		track;

	if (source != src_command)
		return;

	c = args.size();
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

	if (c == 2 && quake::cls.state == ca_connected)
	{
		Con_Printf("Can not record - already connected to server\nClient demo recording must be started before connecting\n");
		return;
	}

// write the forced cd track number, or -1
	if (c == 4)
	{
		track = Q_atoi(args[3]);
		Con_Printf ("Forcing CD track to %i\n", quake::cls.forcetrack);
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
	quake::cls.demofile.open(name.c_str(), std::ios_base::out);
	if (!quake::cls.demofile)
	{
		Con_Printf ("ERROR: couldn't open.\n");
		return;
	}
	quake::cls.demofile << quake::cls.forcetrack << '\n';
	quake::cls.demofile.flush();
	
	quake::cls.demorecording = true;
}


/*
====================
CL_PlayDemo_f

play [demoname]
====================
*/

void CL_PlayDemo_f(cmd_source_t source, const StringArgs& args)
{
	int c;
	qboolean neg = false;

	if (source != src_command)
		return;

	if (args.size() != 2)
	{
		Con_Printf ("play <demoname> : plays a demo\n");
		return;
	}

//
// disconnect from server
//
	quake::cls.disconnect();
	
//
// open the demo file
//
//	ZString name(args[1]);
	quake::path_string name(args[1]);

	COM_DefaultExtension (name, ".dem");
	

	size_t len = COM_FindFile(name.c_str(), quake::cls.demofile);
	if (!quake::cls.demofile.is_open())
	{
		quake::con << "ERROR: couldn't open demo " << name << std::endl;
		quake::cls.demonum = -1;		// stop demo loop
		return;
	}
	else {
		quake::con << "Playing demo from " << name << std::endl;
	}

	quake::cls.demoplayback = true;
	quake::cls.state = ca_connected;
	quake::cls.forcetrack = 0;

	while ((c = quake::cls.demofile.get()) != '\n')
		if (c == '-')
			neg = true;
		else
			quake::cls.forcetrack = quake::cls.forcetrack * 10 + (c - '0');

	if (neg)
		quake::cls.forcetrack = -quake::cls.forcetrack;
// ZOID, fscanf is evil
//	fscanf (quake::cls.demofile, "%i\n", &quake::cls.forcetrack);
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
	
	quake::cls.timedemo = false;
	
// the first frame didn't count
	frames = (host_framecount - quake::cls.td_startframe) - 1;
	time = realtime - quake::cls.td_starttime;
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
void CL_TimeDemo_f(cmd_source_t source, const StringArgs& args)
{
	if (source != src_command)
		return;

	if (args.size() != 2)
	{
		Con_Printf ("timedemo <demoname> : gets demo speeds\n");
		return;
	}

	CL_PlayDemo_f (source,args);
	
// quake::cls.td_starttime will be grabbed at the second frame of the demo, so
// all the loading time doesn't get counted
	
	quake::cls.timedemo = true;
	quake::cls.td_startframe = host_framecount;
	quake::cls.td_lastframe = -1;		// get a new message this frame
}

