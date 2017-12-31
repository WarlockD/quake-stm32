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
// cl_main.c  -- client main loop


#include "icommon.h"
using namespace std::chrono;
// we need to declare some mouse variables here, because the menu system
// references them even when on a unix system.

// these two are not intended to be set directly
cvar_t<cstring_t> cl_name = {   "player", true} ;
cvar_t<float> cl_color = { 0.0f, true} ;

//cvar_t<float> cl_shownet = { 0.0f} ;	// can be 0, 1, or 2
cvar_t<float> cl_shownet = { 0.0f } ;	// can be 0, 1, or 2
cvar_t<float> cl_nolerp = { 0.0f} ;

cvar_t<float> lookspring = { 0.0f, true} ;
cvar_t<float> lookstrafe = { 0.0f, true} ;
cvar_t<float> sensitivity = { 3.0f, true} ;

cvar_t<float> m_pitch = { 0.022f, true} ;
cvar_t<float> m_yaw = { 0.022f, true} ;
cvar_t<float> m_forward = { 1.0f, true} ;
cvar_t<float> m_side = { 0.8f, true} ;

namespace quake {
	client_static_t	cls;
	client_state_t	cl;
}

client_static_t::client_static_t() {
	clear();
}

void client_static_t::clear() {
	state = ca_disconnected;
	mapstring.clear();
	spawnparms.clear();
	demonum = 0;
	demos.clear();
	demorecording = 0;
	demoplayback=0;
	timedemo=0;
	forcetrack=0;			// -1 = use normal cd track
	demofile.close();
	td_lastframe=0;		// to meter out one message a frame
	td_startframe=0;		// host_framecount at start
	td_starttime=0;		// realtime at second frame of timedemo
	signon=0;			// 0 to SIGNONS
	netcon=nullptr;
	message.Clear();		// writing buffer to send to server
}
client_state_t::client_state_t() { clear(); }


// FIXME: put these on hunk?
efrag_t			cl_efrags[MAX_EFRAGS];
entity_t		cl_entities[MAX_EDICTS];
entity_t		cl_static_entities[MAX_STATIC_ENTITIES];
lightstyle_t	cl_lightstyle[MAX_LIGHTSTYLES];
dlight_t		cl_dlights[MAX_DLIGHTS];

int				cl_numvisedicts;
entity_t		*cl_visedicts[MAX_VISEDICTS];

/*
=====================
CL_ClearState

=====================
*/
void client_state_t::clear() { 
	memset(&quake::cl, 0, sizeof(quake::cl)); 
	// clear other arrays	
	memset(cl_efrags, 0, sizeof(cl_efrags));
	memset(cl_entities, 0, sizeof(cl_entities));
	memset(cl_dlights, 0, sizeof(cl_dlights));
	memset(cl_lightstyle, 0, sizeof(cl_lightstyle));
	memset(cl_temp_entities, 0, sizeof(cl_temp_entities));
	memset(cl_beams, 0, sizeof(cl_beams));

	//
	// allocate the efrags and chain together into a free list
	//
	free_efrags = cl_efrags;
	for (int i = 0; i<MAX_EFRAGS - 1; i++)
		free_efrags[i].entnext = &free_efrags[i + 1];
	//free_efrags[i].entnext = nullptr;


}
void CL_ClearState (void)
{
	int			i;

	if (!sv.active)
		Host_ClearMemory ();

	
// wipe the entire quake::cl structure
	//memset (&quake::cl, 0, sizeof(quake::cl));
	quake::cl.clear(); // host_clear memeroy clears this already
	quake::cls.message.Clear();


}

/*
=====================
CL_Disconnect

Sends a disconnect message to the server
This is also called on Host_Error, so it shouldn't cause any errors
=====================
*/

void client_static_t::disconnect(){
// stop sounds (especially looping!)
	S_StopAllSounds (true);
	
// bring the console down and fade the colors back to normal
//	SCR_BringDownConsole ();

// if running a local server, shut it down
	if (demoplayback)
		stop_playback();
	else if (state == ca_connected)
	{
		if (demorecording)
			stop();
		quake::dcon << "Sending clc_disconnect" << std::endl;
		message.Clear();
		message.WriteByte(clc_disconnect);
		NET_SendUnreliableMessage (netcon, &message);
		message.Clear();;
		NET_Close (quake::cls.netcon);

		state = ca_disconnected;
		if (sv.active)
			Host_ShutdownServer(false);
	}

	demoplayback = timedemo = false;
	signon = 0;
}

void CL_Disconnect_f(cmd_source_t source, const StringArgs& args)
{
	quake::cls.disconnect();
	if (sv.active)
		Host_ShutdownServer (false);
}


void client_static_t::establish_connection(const quake::string_view& host) {
	if (state == ca_dedicated) return;
	if (demoplayback) return;

	disconnect();
	if (sv.active) Host_ShutdownServer(false);

	netcon = NET_Connect(host);
	if (!quake::cls.netcon)
		Host_Error("CL_Connect: connect failed\n");
	quake::dcon << "CL_EstablishConnection: connected to  " << host << std::endl;

	quake::cls.demonum = -1;			// not in the demo loop now
	quake::cls.state = ca_connected;
	quake::cls.signon = 0;				// need all the signon messages before playing
}



/*
=====================
CL_SignonReply

An svc_signonnum has been received, perform a client side setup
=====================
*/
void client_static_t::signon_reply(void)
{
	//char 	str[8192];

	quake::con << "CL_SignonReply: " << signon << std::endl;

	switch (quake::cls.signon)
	{
	case 1:
		message.WriteByte(clc_stringcmd);
		message.WriteString("prespawn");
		break;
		
	case 2:		
	{
		quake::fixed_string_stream<256> ss;

		message.WriteByte(clc_stringcmd);
		ss.rdbuf()->clear();
		ss << "name \"" << cl_name.value << std::endl;
		message.WriteString(ss.str().c_str());

		message.WriteByte(clc_stringcmd);

		ss.rdbuf()->clear();
		ss << "color " << ((int)cl_color.value >> 4) << ' ' << ((int)cl_color.value & 15) << std::endl;
		message.WriteString(ss.str().c_str());

		quake::cls.message.WriteByte(clc_stringcmd);

		ss.rdbuf()->clear();
		ss << "spawn " << quake::cls.spawnparms << std::endl;
		message.WriteString(ss.str().c_str());
	}
		break;
		
	case 3:	
		message.WriteByte(clc_stringcmd);
		message.WriteString("begin");
		Cache_Report ();		// print remaining memory
		break;
		
	case 4:
		SCR_EndLoadingPlaque ();		// allow normal screen updates
		break;
	}
}

/*
=====================
CL_NextDemo

Called to play the next demo in the demo loop
=====================
*/
void client_static_t::next_demo(void)
{

	if (demonum == -1)
		return;		// don't play demos

	SCR_BeginLoadingPlaque ();

	if (demos.empty()) {
		quake::con << "No demos listed with startdemos" << std::endl;
		demonum = -1;
		return;

	}
	if(demonum>= demos.size()) demonum = 0;
		
	quake::fixed_string_stream<256> ss;
	ss << "playdemo " << demos[demonum] << std::endl;

	Cbuf_InsertText(ss.str());
	demonum++;
}

/*
==============
CL_PrintEntities_f
==============
*/
void CL_PrintEntities_f (cmd_source_t source, const StringArgs& args)
{
	entity_t	*ent;
	int			i;
	
	for (i=0,ent=cl_entities ; i<quake::cl.num_entities ; i++,ent++)
	{
		Con_Printf ("%3i:",i);
		if (!ent->model)
		{
			Con_Printf ("EMPTY\n");
			continue;
		} 
		// need to change the formating for using quake::con but ugh
		Con_Printf ("%s:%2i  (%5.1f,%5.1f,%5.1f) [%5.1f %5.1f %5.1f]\n"
		,ent->model->name,ent->frame, ent->origin[0], ent->origin[1], ent->origin[2], ent->angles[0], ent->angles[1], ent->angles[2]);
	}
}


/*
===============
SetPal

Debugging tool, just flashes the screen
===============
*/
void SetPal (int i)
{
#if 0
	static int old;
	byte	pal[768];
	int		c;
	
	if (i == old)
		return;
	old = i;

	if (i==0)
		VID_SetPalette (host_basepal);
	else if (i==1)
	{
		for (c=0 ; c<768 ; c+=3)
		{
			pal[c] = 0;
			pal[c+1] = 255;
			pal[c+2] = 0;
		}
		VID_SetPalette (pal);
	}
	else
	{
		for (c=0 ; c<768 ; c+=3)
		{
			pal[c] = 0;
			pal[c+1] = 0;
			pal[c+2] = 255;
		}
		VID_SetPalette (pal);
	}
#endif
}

/*
===============
CL_AllocDlight

===============
*/
dlight_t *CL_AllocDlight (int key)
{
	int		i;
	dlight_t	*dl;

// first look for an exact key match
	if (key)
	{
		dl = cl_dlights;
		for (i=0 ; i<MAX_DLIGHTS ; i++, dl++)
		{
			if (dl->key == key)
			{
				memset (dl, 0, sizeof(*dl));
				dl->key = key;
				return dl;
			}
		}
	}

// then look for anything else
	dl = cl_dlights;
	for (i=0 ; i<MAX_DLIGHTS ; i++, dl++)
	{
		if (dl->die < quake::cl.time)
		{
			memset (dl, 0, sizeof(*dl));
			dl->key = key;
			return dl;
		}
	}

	dl = &cl_dlights[0];
	memset (dl, 0, sizeof(*dl));
	dl->key = key;
	return dl;
}


/*
===============
CL_DecayLights

===============
*/
void CL_DecayLights (void)
{
	int			i;
	dlight_t	*dl;
	idTime		time;
	
	time = quake::cl.time - quake::cl.oldtime;

	dl = cl_dlights;
	for (i=0 ; i<MAX_DLIGHTS ; i++, dl++)
	{
		if (dl->die < quake::cl.time || !dl->radius)
			continue;
		
		dl->radius -= idCast<float>(time)*dl->decay;
		if (dl->radius < 0.0f)
			dl->radius = 0.0f;
	}
}


/*
===============
CL_LerpPoint

Determines the fraction between the last two messages that the objects
should be put at.
===============
*/
float	CL_LerpPoint(void)
{
	float	f, frac;

	f = idCast<float>(quake::cl.mtime[0] - quake::cl.mtime[1]);

	if (!f || cl_nolerp.value || quake::cls.timedemo || sv.active)
	{
		quake::cl.time = quake::cl.mtime[0];
		return 1;
	}

	if (f > 0.1)
	{	// dropped packet, or start of demo
		quake::cl.mtime[1] = quake::cl.mtime[0] - 100ms;
		f = 0.1f;
	}
	frac = idCast<float>((quake::cl.time - quake::cl.mtime[1])) / f;
	//Con_Printf ("frac: %f\n",frac);
	if (frac < 0)
	{
		if (frac < -0.01)
		{
			SetPal(1);
			quake::cl.time = quake::cl.mtime[1];
			//				Con_Printf ("low frac\n");
		}
		frac = 0;
	}
	else if (frac > 1)
	{
		if (frac > 1.01)
		{
			SetPal(2);
			quake::cl.time = quake::cl.mtime[0];
			//				Con_Printf ("high frac\n");
		}
		frac = 1;
	}
	else
		SetPal(0);

	return frac;
}


/*
===============
CL_RelinkEntities
===============
*/
void CL_RelinkEntities (void)
{
	entity_t	*ent;
	int			i, j;
	float		frac, f, d;
	vec3_t		delta;
	float		bobjrotate;
	vec3_t		oldorg;
	dlight_t	*dl;

// determine partial update time	
	frac = CL_LerpPoint ();

	cl_numvisedicts = 0;

//
// interpolate player info
//
	for (i=0 ; i<3 ; i++)
		quake::cl.velocity[i] = quake::cl.mvelocity[1][i] + 
			frac * (quake::cl.mvelocity[0][i] - quake::cl.mvelocity[1][i]);

	if (quake::cls.demoplayback)
	{
	// interpolate the angles	
		for (j=0 ; j<3 ; j++)
		{
			d = quake::cl.mviewangles[0][j] - quake::cl.mviewangles[1][j];
			if (d > 180)
				d -= 360;
			else if (d < -180)
				d += 360;
			quake::cl.viewangles[j] = quake::cl.mviewangles[1][j] + frac*d;
		}
	}
	
	bobjrotate = anglemod(100.0f*idCast<float>(quake::cl.time));
	
// start on the entity after the world
	for (i=1,ent=cl_entities+1 ; i<quake::cl.num_entities ; i++,ent++)
	{
		if (!ent->model)
		{	// empty slot
			if (ent->forcelink)
				R_RemoveEfrags (ent);	// just became empty
			continue;
		}

// if the object wasn't included in the last packet, remove it
		if (ent->msgtime != quake::cl.mtime[0])
		{
			ent->model = NULL;
			continue;
		}

		VectorCopy (ent->origin, oldorg);

		if (ent->forcelink)
		{	// the entity was not updated in the last message
			// so move to the final spot
			VectorCopy (ent->msg_origins[0], ent->origin);
			VectorCopy (ent->msg_angles[0], ent->angles);
		}
		else
		{	// if the delta is large, assume a teleport and don't lerp
			f = frac;
			for (j=0 ; j<3 ; j++)
			{
				delta[j] = ent->msg_origins[0][j] - ent->msg_origins[1][j];
				if (delta[j] > 100 || delta[j] < -100)
					f = 1;		// assume a teleportation, not a motion
			}

		// interpolate the origin and angles
			for (j=0 ; j<3 ; j++)
			{
				ent->origin[j] = ent->msg_origins[1][j] + f*delta[j];

				d = ent->msg_angles[0][j] - ent->msg_angles[1][j];
				if (d > 180)
					d -= 360;
				else if (d < -180)
					d += 360;
				ent->angles[j] = ent->msg_angles[1][j] + f*d;
			}
			
		}

// rotate binary objects locally
		if (ent->model->flags & EF_ROTATE)
			ent->angles[1] = bobjrotate;

		if (ent->effects & EF_BRIGHTFIELD)
			R_EntityParticles (ent);
#ifdef QUAKE2
		if (ent->effects & EF_DARKFIELD)
			R_DarkFieldParticles (ent);
#endif
		if (ent->effects & EF_MUZZLEFLASH)
		{
			vec3_t		fv, rv, uv;

			dl = CL_AllocDlight (i);
			VectorCopy (ent->origin,  dl->origin);
			dl->origin[2] += 16;
			AngleVectors (ent->angles, fv, rv, uv);
			 
			VectorMA (dl->origin, 18, fv, dl->origin);
			dl->radius = 200.0f + static_cast<float>((rand()&31));
			dl->minlight = 32;
			dl->die = quake::cl.time + 100ms;
		}
		if (ent->effects & EF_BRIGHTLIGHT)
		{			
			dl = CL_AllocDlight (i);
			VectorCopy (ent->origin,  dl->origin);
			dl->origin[2] += 16;
			dl->radius = 400.0f + static_cast<float>((rand() & 31));
			dl->die = quake::cl.time + 1ms;
		}
		if (ent->effects & EF_DIMLIGHT)
		{			
			dl = CL_AllocDlight (i);
			VectorCopy (ent->origin,  dl->origin);
			dl->radius = 200.0f + static_cast<float>((rand() & 31));
			dl->die = quake::cl.time + 1ms;
		}
#ifdef QUAKE2
		if (ent->effects & EF_DARKLIGHT)
		{			
			dl = CL_AllocDlight (i);
			VectorCopy (ent->origin,  dl->origin);
			dl->radius = 200.0 + (rand()&31);
			dl->die = quake::cl.time + 0.001;
			dl->dark = true;
		}
		if (ent->effects & EF_LIGHT)
		{			
			dl = CL_AllocDlight (i);
			VectorCopy (ent->origin,  dl->origin);
			dl->radius = 200;
			dl->die = quake::cl.time + 0.001;
		}
#endif

		if (ent->model->flags & EF_GIB)
			R_RocketTrail (oldorg, ent->origin, 2);
		else if (ent->model->flags & EF_ZOMGIB)
			R_RocketTrail (oldorg, ent->origin, 4);
		else if (ent->model->flags & EF_TRACER)
			R_RocketTrail (oldorg, ent->origin, 3);
		else if (ent->model->flags & EF_TRACER2)
			R_RocketTrail (oldorg, ent->origin, 5);
		else if (ent->model->flags & EF_ROCKET)
		{
			R_RocketTrail (oldorg, ent->origin, 0);
			dl = CL_AllocDlight (i);
			VectorCopy (ent->origin, dl->origin);
			dl->radius = 200;
			dl->die = quake::cl.time + 10ms;
		}
		else if (ent->model->flags & EF_GRENADE)
			R_RocketTrail (oldorg, ent->origin, 1);
		else if (ent->model->flags & EF_TRACER3)
			R_RocketTrail (oldorg, ent->origin, 6);

		ent->forcelink = false;

		if (i == quake::cl.viewentity && !chase_active.value)
			continue;

#ifdef QUAKE2
		if ( ent->effects & EF_NODRAW )
			continue;
#endif
		if (cl_numvisedicts < MAX_VISEDICTS)
		{
			cl_visedicts[cl_numvisedicts] = ent;
			cl_numvisedicts++;
		}
	}

}


/*
===============
CL_ReadFromServer

Read all incoming data from the server
===============
*/
int CL_ReadFromServer (void)
{
	int		ret;

	quake::cl.oldtime = quake::cl.time;
	quake::cl.time += host_frametime;
	
	do
	{
		ret = CL_GetMessage ();
		if (ret == -1)
			Host_Error ("CL_ReadFromServer: lost server connection");
		if (!ret)
			break;
		
		quake::cl.last_received_message = realtime;
		CL_ParseServerMessage ();
	} while (ret && quake::cls.state == ca_connected);
	
//	if (cl_shownet.value)
//		quake::con << std::endl;


	CL_RelinkEntities ();
	CL_UpdateTEnts ();

//
// bring the links up to date
//
	return 0;
}

/*
=================
CL_SendCmd
=================
*/
void CL_SendCmd (void)
{
	usercmd_t		cmd;

	if (quake::cls.state != ca_connected)
		return;

	if (quake::cls.signon == SIGNONS)
	{
	// get basic movement from keyboard
		CL_BaseMove (&cmd);
	
	// allow mice or other external controllers to add to the move
		IN_Move (&cmd);
	
	// send the unreliable message
		CL_SendMove (&cmd);
	
	}

	if (quake::cls.demoplayback)
	{
		quake::cls.message.Clear();
		return;
	}
	
// send the reliable message
	if (!quake::cls.message.size())
		return;		// no message at all
	
	if (!NET_CanSendMessage (quake::cls.netcon))
	{
		Con_DPrintf ("CL_WriteToServer: can't send\n");
		return;
	}

	if (NET_SendMessage (quake::cls.netcon, &quake::cls.message) == -1)
		Host_Error ("CL_WriteToServer: lost server connection");

	quake::cls.message.Clear();;
}

/*
=================
CL_Init
=================
*/
void CL_Init (void)
{	
	quake::cls.message.Alloc( 1024);

	CL_InitInput ();
	CL_InitTEnts ();
	
//
// register our commands
//
	Cvar_RegisterVariable("cl_name",cl_name);
	Cvar_RegisterVariable("cl_color",cl_color);
	Cvar_RegisterVariable("cl_upspeed",cl_upspeed);
	Cvar_RegisterVariable("cl_forwardspeed",cl_forwardspeed);
	Cvar_RegisterVariable("cl_backspeed",cl_backspeed);
	Cvar_RegisterVariable("cl_sidespeed",cl_sidespeed);
	Cvar_RegisterVariable("cl_movespeedkey",cl_movespeedkey);
	Cvar_RegisterVariable("cl_yawspeed",cl_yawspeed);
	Cvar_RegisterVariable("cl_pitchspeed",cl_pitchspeed);
	Cvar_RegisterVariable("cl_anglespeedkey",cl_anglespeedkey);
	Cvar_RegisterVariable("cl_shownet",cl_shownet);
	Cvar_RegisterVariable("cl_nolerp",cl_nolerp);
	Cvar_RegisterVariable("lookspring",lookspring);
	Cvar_RegisterVariable("lookstrafe",lookstrafe);
	Cvar_RegisterVariable("sensitivity",sensitivity);

	Cvar_RegisterVariable("m_pitch",m_pitch);
	Cvar_RegisterVariable("m_yaw",m_yaw);
	Cvar_RegisterVariable("m_forward",m_forward);
	Cvar_RegisterVariable("m_side",m_side);

//	Cvar_RegisterVariable("cl_autofire",cl_autofire);
	
	Cmd_AddCommand ("entities", CL_PrintEntities_f);
	Cmd_AddCommand ("disconnect", CL_Disconnect_f);
	Cmd_AddCommand ("record", CL_Record_f);
	Cmd_AddCommand ("stop", CL_Stop_f);
	Cmd_AddCommand ("playdemo", CL_PlayDemo_f);
	Cmd_AddCommand ("timedemo", CL_TimeDemo_f);
}

