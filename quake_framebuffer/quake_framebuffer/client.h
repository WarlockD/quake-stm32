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
// client.h
#ifndef _QUAKE_CLIENT_H_
#define _QUAKE_CLIENT_H_

#include "common.h"
#include "mathlib.h"
struct usercmd_t
{
	vec3_t	viewangles;

// intended velocities
	float	forwardmove;
	float	sidemove;
	float	upmove;
	usercmd_t() : viewangles{ 0.0f, 0.0f, 0.0f }, forwardmove(0.0f), sidemove(0.0f), upmove(0.0f) {}
#ifdef QUAKE2
	byte	lightlevel;
#endif
} ;

typedef struct
{
	int		length;
	char	map[MAX_STYLESTRING];
} lightstyle_t;

typedef struct
{
	char	name[MAX_SCOREBOARDNAME];
	float	entertime;
	int		frags;
	int		colors;			// two 4 bit fields
	byte	translations[VID_GRADES*256];
} scoreboard_t;

typedef struct
{
	int		destcolor[3];
	int		percent;		// 0-256
} cshift_t;

#define	CSHIFT_CONTENTS	0
#define	CSHIFT_DAMAGE	1
#define	CSHIFT_BONUS	2
#define	CSHIFT_POWERUP	3
#define	NUM_CSHIFTS		4

#define	NAME_LENGTH	64


//
// client_state_t should hold all pieces of the client state
//

#define	SIGNONS		4			// signon messages to receive before connected

#define	MAX_DLIGHTS		32
 struct dlight_t
{
	vec3_t	origin;
	float	radius;
	idTime	die;				// stop lighting after this time
	float	decay;				// drop this each second
	float	minlight;			// don't add when contributing less
	int		key;
#ifdef QUAKE2
	qboolean	dark;			// subtracts light instead of adding
#endif
} ;


#define	MAX_BEAMS	24
struct model_t;
struct beam_t
{
	int		entity;
	model_t	*model;
	idTime	endtime;
	vec3_t	start, end;
} ;

#define	MAX_EFRAGS		640

#define	MAX_MAPSTRING	2048
#define	MAX_DEMOS		8
#define	MAX_DEMONAME	16

typedef enum {
ca_dedicated, 		// a dedicated server with no ability to start a client
ca_disconnected, 	// full screen console with no connection
ca_connected		// valid netcon, talking to a server
} cactive_t;
#include <fstream>

//
// the client_static_t structure is persistant through an arbitrary number
// of server connections
//
struct client_static_t
{
	//quake::debug_t<cactive_t>	state;
	cactive_t	state;
// personalization data sent to server	
	quake::zstring mapstring;
	quake::zstring spawnparms;
	//char		mapstring[MAX_QPATH];
	//char		spawnparms[MAX_MAPSTRING];	// to restart a level

// demo loop control
	int			demonum;		// -1 = don't play demos
	StringArgs demos; // when not playing

// demo recording info must be here, because record is started before
// entering a map (and clearing client_state_t)
	qboolean	demorecording;
	qboolean	demoplayback;
	qboolean	timedemo;
	int			forcetrack;			// -1 = use normal cd track
	std::fstream	demofile;
	int			td_lastframe;		// to meter out one message a frame
	int			td_startframe;		// host_framecount at start
	idTime		td_starttime;		// realtime at second frame of timedemo


// connection information
	int			signon;			// 0 to SIGNONS
	qsocket_t	*netcon;
	sizebuf_t	message;		// writing buffer to send to server
	client_static_t();
	void clear();		
	void stop_playback();
	void stop();
	void establish_connection(const quake::string_view& host);
	void disconnect();
	void signon_reply();
	void next_demo();
} ;



//
// the client_state_t structure is wiped completely at every
// server signon
//

struct model_t;
struct sfx_t;

struct client_state_t
{
	int			movemessages;	// since connecting to this server
								// throw out the first couple, so the player
								// doesn't accidentally do something the 
								// first frame
	usercmd_t	cmd;			// last command sent to the server

// information for local display
	int			stats[MAX_CL_STATS];	// health, etc
	int			items;			// inventory bit flags
	idTime	item_gettime[32];	// quake::cl.time of aquiring item, for blinking
	idTime		faceanimtime;	// use anim frame if quake::cl.time < this

	cshift_t	cshifts[NUM_CSHIFTS];	// color shifts for damage, powerups
	cshift_t	prev_cshifts[NUM_CSHIFTS];	// and content types

// the client maintains its own idea of view angles, which are
// sent to the server each frame.  The server sets punchangle when
// the view is temporarliy offset, and an angle reset commands at the start
// of each level and after teleporting.
	vec3_t		mviewangles[2];	// during demo playback viewangles is lerped
								// between these
	vec3_t		viewangles;
	
	vec3_t		mvelocity[2];	// update by server, used for lean+bob
								// (0 is newest)
	vec3_t		velocity;		// lerped between mvelocity[0] and [1]

	vec3_t		punchangle;		// temporary offset
	
// pitch drifting vars
	float		idealpitch;
	float		pitchvel;
	qboolean	nodrift;
	float		driftmove;
	idTime		laststop;

	float		viewheight;
	float		crouch;			// local amount for smoothing stepups

	qboolean	paused;			// send over by server
	qboolean	onground;
	qboolean	inwater;
	
	int			intermission;	// don't change view angle, full screen, etc
	int			completed_time;	// latched at intermission start
	
	idTime		mtime[2];		// the timestamp of last two messages	
	idTime		time;			// clients view of time, should be between
								// servertime and oldservertime to generate
								// a lerp point for other data
	idTime		oldtime;		// previous quake::cl.time, time-oldtime is used
								// to decay light values and smooth step ups
	

	idTime		last_received_message;	// (realtime) for net trouble icon

//
// information that is static for the entire time connected to a server
//
	model_t*	model_precache[MAX_MODELS];
	sfx_t*		sound_precache[MAX_SOUNDS];

	char		levelname[40];	// for display on solo scoreboard
	int			viewentity;		// cl_entitites[quake::cl.viewentity] = player
	int			maxclients;
	int			gametype;

// refresh related state
	model_t*  worldmodel;	// cl_entitites[0].model
	efrag_t	*free_efrags;
	int			num_entities;	// held in cl_entities array
	int			num_statics;	// held in cl_staticentities array
	entity_t	viewent;			// the gun model

	int			cdtrack, looptrack;	// cd audio

// frag scoreboard
	scoreboard_t	*scores;		// [quake::cl.maxclients]
	client_state_t();
	void clear();
#ifdef QUAKE2
// light level at player's position including dlights
// this is sent back to the server each frame
// architectually ugly but it works
	int			light_level;
#endif

} ;
namespace quake {
	extern client_state_t cl;
	extern client_static_t cls;
}

//
// cvars
//
extern	cvar_t<ustl::string>	cl_name;
extern	cvar_t<float>	cl_color;

extern	cvar_t<float>		cl_upspeed;
extern	cvar_t<float>		cl_forwardspeed;
extern	cvar_t<float>		cl_backspeed;
extern	cvar_t<float>		cl_sidespeed;

extern	cvar_t<float>		cl_movespeedkey;

extern	cvar_t<float>		cl_yawspeed;
extern	cvar_t<float>		cl_pitchspeed;

extern	cvar_t<float>		cl_anglespeedkey;

extern	cvar_t<float>		cl_autofire;

extern	cvar_t<float>		cl_shownet;
extern	cvar_t<float>		cl_nolerp;

extern	cvar_t<float>		cl_pitchdriftspeed;
extern	cvar_t<float>	lookspring;
extern	cvar_t<float>		lookstrafe;
extern	cvar_t<float>		sensitivity;

extern	cvar_t<float>		m_pitch;
extern	cvar_t<float>		m_yaw;
extern	cvar_t<float>		m_forward;
extern	cvar_t<float>		m_side;


#define	MAX_TEMP_ENTITIES	64			// lightning bolts, etc
#define	MAX_STATIC_ENTITIES	128			// torches, etc



// FIXME, allocate dynamically
extern	efrag_t			cl_efrags[MAX_EFRAGS];
extern	entity_t		cl_entities[MAX_EDICTS];
extern	entity_t		cl_static_entities[MAX_STATIC_ENTITIES];
extern	lightstyle_t	cl_lightstyle[MAX_LIGHTSTYLES];
extern	dlight_t		cl_dlights[MAX_DLIGHTS];
extern	entity_t		cl_temp_entities[MAX_TEMP_ENTITIES];
extern	beam_t			cl_beams[MAX_BEAMS];

//=============================================================================

//
// cl_main
//
dlight_t *CL_AllocDlight (int key);
void	CL_DecayLights (void);

void CL_Init (void);

//void CL_EstablishConnection (cstring_t host, cmd_source_t source, const StringArgs& args);
void CL_Signon1 (void);
void CL_Signon2 (void);
void CL_Signon3 (void);
void CL_Signon4 (void);

//void CL_Disconnect (void);
void CL_Disconnect_f(cmd_source_t source, const StringArgs& args);
//void CL_NextDemo (void);

#define			MAX_VISEDICTS	256
extern	int				cl_numvisedicts;
extern	entity_t		*cl_visedicts[MAX_VISEDICTS];

//
// cl_input
//
typedef struct
{
	int		down[2];		// key nums holding it down
	int		state;			// low bit is down state
} kbutton_t;

extern	kbutton_t	in_mlook, in_klook;
extern 	kbutton_t 	in_strafe;
extern 	kbutton_t 	in_speed;

void CL_InitInput (void);
void CL_SendCmd (void);
void CL_SendMove (usercmd_t *cmd);

void CL_ParseTEnt (void);
void CL_UpdateTEnts (void);

void CL_ClearState (void);


int  CL_ReadFromServer (void);
void CL_WriteToServer (usercmd_t *cmd);
void CL_BaseMove (usercmd_t *cmd);


float CL_KeyState (kbutton_t *key);
const char *Key_KeynumToString (int keynum);

//
// cl_demo.c
//

int CL_GetMessage (void);

void CL_Stop_f(cmd_source_t source, const StringArgs&args);
void CL_Record_f(cmd_source_t source, const StringArgs&args);
void CL_PlayDemo_f(cmd_source_t source, const StringArgs&args);
void CL_TimeDemo_f(cmd_source_t source, const StringArgs&args);

//
// cl_parse.c
//
void CL_ParseServerMessage (void);
void CL_NewTranslation (int slot);

//
// view
//
void V_StartPitchDrift();
void V_StopPitchDrift (void);

void V_RenderView (void);
void V_UpdatePalette (void);
void V_Register (void);
void V_ParseDamage (void);
void V_SetContentsColor (int contents);


//
// cl_tent
//
void CL_InitTEnts (void);
//void CL_SignonReply (void);

#endif