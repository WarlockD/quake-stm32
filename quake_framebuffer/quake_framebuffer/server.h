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
// server.h
#ifndef _QUAKE_SERVER_H_
#define _QUAKE_SERVER_H_

struct client_t;

 struct server_static_t
{
	int			maxclients;
	int			maxclientslimit;
	client_t	*clients;		// [maxclients]
	int			serverflags;		// episode completion information
	qboolean	changelevel_issued;	// cleared when at SV_SpawnServer
} ;

//=============================================================================

typedef enum {ss_loading, ss_active} server_state_t;

struct server_t
{
	server_t();
	void reset();
	edict_t*	worldedict;
	qboolean	active;				// false if only a net client

	qboolean	paused;
	qboolean	loadgame;			// handle connections specially

	idTime		time; // was double
	
	int			lastcheck;			// used by PF_checkclient
	idTime		lastchecktime;      // was doube
	
	string_t	name;			// map name
#ifdef QUAKE2
	char		startspot[64];
#endif
	string_t	modelname;		// maps/<name>.bsp, for model_precache[0]
	model_t 	*worldmodel;
	string_t	model_precache[MAX_MODELS];	// NULL terminated
	model_t		*models[MAX_MODELS];
	string_t	sound_precache[MAX_SOUNDS];	// NULL terminated
	string_t	lightstyles[MAX_LIGHTSTYLES];


#ifdef USE_OLD_EDICT_SYSTEM
	int			num_edicts;
	int			max_edicts;
	edict_t		*edicts;			// can NOT be array indexed, because
									// edict_t is variable sized, but can
									// be used to reference the world ent
#else
	// alright, the first major change to the edict system
	// new and delete work with it now from a pool created from load_progs

#endif
	server_state_t	state;			// some actions are only valid during load
	static_sizebuf_t<MAX_DATAGRAM> datagram;
	static_sizebuf_t<MAX_DATAGRAM> reliable_datagram; // copied to all clients at end of frame
	static_sizebuf_t<8192> signon;
} ;


#define	NUM_PING_TIMES		16
#define	NUM_SPAWN_PARMS		16
struct qsocket_t;

struct client_t
{
	void reset();
	quake::debug_t<qboolean> active;				// false = client is free
	qboolean		spawned;			// false = don't send datagrams
	qboolean		dropasap;			// has been told to go to another level
	qboolean		privileged;			// can execute any host command
	qboolean		sendsignon;			// only valid before spawned

	idTime			last_message;		// reliable messages must be sent
										// periodically

	qsocket_t *netconnection;	// communications handle

	usercmd_t		cmd;				// movement
	vec3_t			wishdir;			// intended motion calced from cmd


	static_sizebuf_t<MAX_MSGLEN>		message;			// can be added to at any time,
										// copied and clear once per frame
	edict_t			*edict;				// EDICT_NUM(clientnum+1)
	string_t		name;			// for printing to other people
	int				colors;
		
	idTime			ping_times[NUM_PING_TIMES];
	int				num_pings;			// ping_times[num_pings%NUM_PING_TIMES]

// spawn parms are carried from level to level
	float			spawn_parms[NUM_SPAWN_PARMS];

// client known data for deltas	
	int				old_frags;
} ;


//=============================================================================

// edict->movetype values
#define	MOVETYPE_NONE			0		// never moves
#define	MOVETYPE_ANGLENOCLIP	1
#define	MOVETYPE_ANGLECLIP		2
#define	MOVETYPE_WALK			3		// gravity
#define	MOVETYPE_STEP			4		// gravity, special edge handling
#define	MOVETYPE_FLY			5
#define	MOVETYPE_TOSS			6		// gravity
#define	MOVETYPE_PUSH			7		// no clip to world, push and crush
#define	MOVETYPE_NOCLIP			8
#define	MOVETYPE_FLYMISSILE		9		// extra size to monsters
#define	MOVETYPE_BOUNCE			10
#ifdef QUAKE2
#define MOVETYPE_BOUNCEMISSILE	11		// bounce w/o gravity
#define MOVETYPE_FOLLOW			12		// track movement of aiment
#endif

// edict->solid values
#define	SOLID_NOT				0		// no interaction with other objects
#define	SOLID_TRIGGER			1		// touch on edge, but not blocking
#define	SOLID_BBOX				2		// touch on edge, block
#define	SOLID_SLIDEBOX			3		// touch on edge, but not an onground
#define	SOLID_BSP				4		// bsp clip, touch on edge, block

// edict->deadflag values
#define	DEAD_NO					0
#define	DEAD_DYING				1
#define	DEAD_DEAD				2

#define	DAMAGE_NO				0
#define	DAMAGE_YES				1
#define	DAMAGE_AIM				2

// edict->flags
#define	FL_FLY					1
#define	FL_SWIM					2
//#define	FL_GLIMPSE				4
#define	FL_CONVEYOR				4
#define	FL_CLIENT				8
#define	FL_INWATER				16
#define	FL_MONSTER				32
#define	FL_GODMODE				64
#define	FL_NOTARGET				128
#define	FL_ITEM					256
#define	FL_ONGROUND				512
#define	FL_PARTIALGROUND		1024	// not all corners are valid
#define	FL_WATERJUMP			2048	// player jumping out of water
#define	FL_JUMPRELEASED			4096	// for jump debouncing
#ifdef QUAKE2
#define FL_FLASHLIGHT			8192
#define FL_ARCHIVE_OVERRIDE		1048576
#endif

// entity effects

#define	EF_BRIGHTFIELD			1
#define	EF_MUZZLEFLASH 			2
#define	EF_BRIGHTLIGHT 			4
#define	EF_DIMLIGHT 			8
#ifdef QUAKE2
#define EF_DARKLIGHT			16
#define EF_DARKFIELD			32
#define EF_LIGHT				64
#define EF_NODRAW				128
#endif

#define	SPAWNFLAG_NOT_EASY			256
#define	SPAWNFLAG_NOT_MEDIUM		512
#define	SPAWNFLAG_NOT_HARD			1024
#define	SPAWNFLAG_NOT_DEATHMATCH	2048

#ifdef QUAKE2
// server flags
#define	SFL_EPISODE_1		1
#define	SFL_EPISODE_2		2
#define	SFL_EPISODE_3		4
#define	SFL_EPISODE_4		8
#define	SFL_NEW_UNIT		16
#define	SFL_NEW_EPISODE		32
#define	SFL_CROSS_TRIGGERS	65280
#endif

//============================================================================

extern	cvar_t<float>		teamplay;
extern	cvar_t<float>		skill;
extern	cvar_t<float>		deathmatch;
extern	cvar_t<float>		coop;
extern	cvar_t<float>		fraglimit;
extern	cvar_t<float>		timelimit;

extern	server_static_t	svs;				// persistant server info
extern	server_t		sv;					// local server

extern	client_t	*host_client;

extern	jmp_buf 	host_abortserver;

extern	idTime		host_time;

extern	edict_t		*sv_player;

//===========================================================

void SV_Init (void);

void SV_StartParticle (vec3_t org, vec3_t dir, int color, int count);
void SV_StartSound (edict_t *entity, int channel, cstring_t sample, int volume,
    float attenuation);

void SV_DropClient (qboolean crash);

void SV_SendClientMessages (void);
void SV_ClearDatagram (void);

int SV_ModelIndex (cstring_t name);

void SV_SetIdealPitch (void);

void SV_AddUpdates (void);

void SV_ClientThink (void);
void SV_AddClientToServer (qsocket_t	*ret);

void SV_ClientPrintf (const char *fmt, ...);
void SV_BroadcastPrintf (const char *fmt, ...);

void SV_Physics (void);

qboolean SV_CheckBottom (edict_t *ent);
qboolean SV_movestep (edict_t *ent, vec3_t move, qboolean relink);

void SV_WriteClientdataToMessage (edict_t *ent, sizebuf_t *msg);

void SV_MoveToGoal (void);

void SV_CheckForNewClients (void);
void SV_RunClients (void);
void SV_SaveSpawnparms ();
#ifdef QUAKE2
void SV_SpawnServer (char *server, char *startspot);
#else
void SV_SpawnServer (cstring_t server);
#endif

#endif