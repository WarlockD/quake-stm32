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
// cl_parse.c  -- parse a message received from the server


#include "icommon.h"

using namespace std::chrono;

static const char *svc_strings[] =
{
	"svc_bad",
	"svc_nop",
	"svc_disconnect",
	"svc_updatestat",
	"svc_version",		// [long] server version
	"svc_setview",		// [short] entity number
	"svc_sound",			// <see code>
	"svc_time",			// [float] server time
	"svc_print",			// [string] null terminated string
	"svc_stufftext",		// [string] stuffed into client's console buffer
						// the string should be \n terminated
	"svc_setangle",		// [vec3] set the view angle to this absolute value
	
	"svc_serverinfo",		// [long] version
						// [string] signon string
						// [string]..[0]model cache [string]...[0]sounds cache
						// [string]..[0]item cache
	"svc_lightstyle",		// [byte] [string]
	"svc_updatename",		// [byte] [string]
	"svc_updatefrags",	// [byte] [short]
	"svc_clientdata",		// <shortbits + data>
	"svc_stopsound",		// <see code>
	"svc_updatecolors",	// [byte] [byte]
	"svc_particle",		// [vec3] <variable>
	"svc_damage",			// [byte] impact [byte] blood [vec3] from
	
	"svc_spawnstatic",
	"OBSOLETE svc_spawnbinary",
	"svc_spawnbaseline",
	
	"svc_temp_entity",		// <variable>
	"svc_setpause",
	"svc_signonnum",
	"svc_centerprint",
	"svc_killedmonster",
	"svc_foundsecret",
	"svc_spawnstaticsound",
	"svc_intermission",
	"svc_finale",			// [string] music [string] text
	"svc_cdtrack",			// [byte] track [byte] looptrack
	"svc_sellscreen",
	"svc_cutscene"
};

//=============================================================================

/*
===============
CL_EntityNum

This error checks and tracks the total number of entities
===============
*/
entity_t	*CL_EntityNum (int num)
{
	if (num >= quake::cl.num_entities)
	{
		if (num >= MAX_EDICTS)
			Host_Error ("CL_EntityNum: %i is an invalid number",num);
		while (quake::cl.num_entities<=num)
		{
			cl_entities[quake::cl.num_entities].colormap = vid.colormap;
			quake::cl.num_entities++;
		}
	}
		
	return &cl_entities[num];
}


/*
==================
CL_ParseStartSoundPacket
==================
*/
void CL_ParseStartSoundPacket(void)
{
    vec3_t  pos;
    int 	channel, ent;
    int 	sound_num;
    int 	volume;
    int 	field_mask;
    float 	attenuation;  
 	int		i;
	           
    field_mask = MSG_ReadByte(); 

    if (field_mask & SND_VOLUME)
		volume = MSG_ReadByte ();
	else
		volume = DEFAULT_SOUND_PACKET_VOLUME;
	
    if (field_mask & SND_ATTENUATION)
		attenuation = MSG_ReadByte () / 64.0;
	else
		attenuation = DEFAULT_SOUND_PACKET_ATTENUATION;
	
	channel = MSG_ReadShort ();
	sound_num = MSG_ReadByte ();

	ent = channel >> 3;
	channel &= 7;

	if (ent > MAX_EDICTS)
		Host_Error ("CL_ParseStartSoundPacket: ent = %i", ent);
	
	for (i=0 ; i<3 ; i++)
		pos[i] = MSG_ReadCoord ();
 
    S_StartSound (ent, channel, quake::cl.sound_precache[sound_num], pos, volume/255.0, attenuation);
}       

/*
==================
CL_KeepaliveMessage

When the client is taking a long time to load stuff, send keepalive messages
so the server doesn't disconnect.
==================
*/
void CL_KeepaliveMessage (void)
{
	
	static idTime lastmsg;
	int		ret;
	static_sizebuf_t<8192>	old;

	
	if (sv.active)
		return;		// no need if server is local
	if (quake::cls.demoplayback)
		return;

	old = net_message; // read messages from server, should just be nops

	do
	{
		ret = CL_GetMessage ();
		switch (ret)
		{
		default:
			Host_Error ("CL_KeepaliveMessage: CL_GetMessage failed");		
		case 0:
			break;	// nothing waiting
		case 1:
			Host_Error ("CL_KeepaliveMessage: received a message");
			break;
		case 2:
			if (MSG_ReadByte() != svc_nop)
				Host_Error ("CL_KeepaliveMessage: datagram wasn't a nop");
			break;
		}
	} while (ret);

	net_message = old;

// check time
	idTime time = Sys_FloatTime ();
	if (time - lastmsg < 5s)
		return;
	lastmsg = time;

// write out a nop
	Con_Printf ("--> client to server keepalive\n");

	quake::cls.message.WriteByte(clc_nop);
	NET_SendMessage (quake::cls.netcon, &quake::cls.message);
	quake::cls.message.Clear();
}

/*
==================
CL_ParseServerInfo
==================
*/
void CL_ParseServerInfo (void)
{
	char	*str;
	int		i;
	int		nummodels, numsounds;
	char	model_precache[MAX_MODELS][MAX_QPATH];
	char	sound_precache[MAX_SOUNDS][MAX_QPATH];
	
	Con_DPrintf ("Serverinfo packet received.\n");
//
// wipe the client_state_t struct
//
	CL_ClearState ();

// parse protocol version number
	i = MSG_ReadLong ();
	if (i != PROTOCOL_VERSION)
	{
		Con_Printf ("Server returned version %i, not %i", i, PROTOCOL_VERSION);
		return;
	}

// parse maxclients
	quake::cl.maxclients = MSG_ReadByte ();
	if (quake::cl.maxclients < 1 || quake::cl.maxclients > MAX_SCOREBOARD)
	{
		Con_Printf("Bad maxclients (%u) from server\n", quake::cl.maxclients);
		return;
	}
	quake::cl.scores = (scoreboard_t*)Hunk_AllocName (quake::cl.maxclients*sizeof(*quake::cl.scores), "scores");

// parse gametype
	quake::cl.gametype = MSG_ReadByte ();

// parse signon message
	str = MSG_ReadString ();
	strncpy (quake::cl.levelname, str, sizeof(quake::cl.levelname)-1);

// seperate the printfs so the server message can have a color
	Con_Printf("\n\n\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n\n");
	Con_Printf ("%c%s\n", 2, str);

//
// first we go through and touch all of the precache data that still
// happens to be in the cache, so precaching something else doesn't
// needlessly purge it
//

// precache models
	memset (quake::cl.model_precache, 0, sizeof(quake::cl.model_precache));
	for (nummodels=1 ; ; nummodels++)
	{
		str = MSG_ReadString ();
		if (!str[0])
			break;
		if (nummodels==MAX_MODELS)
		{
			Con_Printf ("Server sent too many model precaches\n");
			return;
		}
		strcpy (model_precache[nummodels], str);
		Mod_TouchModel (str);
	}

// precache sounds
	memset (quake::cl.sound_precache, 0, sizeof(quake::cl.sound_precache));
	for (numsounds=1 ; ; numsounds++)
	{
		str = MSG_ReadString ();
		if (!str[0])
			break;
		if (numsounds==MAX_SOUNDS)
		{
			Con_Printf ("Server sent too many sound precaches\n");
			return;
		}
		Q_strcpy (sound_precache[numsounds], str);
		S_TouchSound (str);
	}

//
// now we try to load everything else until a cache allocation fails
//

	for (i=1 ; i<nummodels ; i++)
	{
		quake::cl.model_precache[i] = Mod_ForName (model_precache[i], false);
		if (quake::cl.model_precache[i] == NULL)
		{
			Con_Printf("Model %s not found\n", model_precache[i]);
			return;
		}
		CL_KeepaliveMessage ();
	}

	S_BeginPrecaching ();
	for (i=1 ; i<numsounds ; i++)
	{
		quake::cl.sound_precache[i] = S_PrecacheSound (sound_precache[i]);
		CL_KeepaliveMessage ();
	}
	S_EndPrecaching ();


// local state
	cl_entities[0].model = quake::cl.worldmodel = quake::cl.model_precache[1];
	
	R_NewMap ();

	Hunk_Check ();		// make sure nothing is hurt
	
	noclip_anglehack = false;		// noclip is turned off at start	
}


/*
==================
CL_ParseUpdate

Parse an entity update message from the server
If an entities model or origin changes from frame to frame, it must be
relinked.  Other attributes can change without relinking.
==================
*/
#include <iomanip>
int	bitcounts[16];
void PrintFastUpdate(int bits) {
	bool comma = false;
#define PRINT_BIT(BIT,TEXT) if (bits & (BIT)) { if(comma) quake::con << ","; else  comma= true; quake::con << TEXT; }
	quake::con << "fastupdate(" << std::hex << std::setfill('0') << std::setw(4) << bits << ":";
	PRINT_BIT(U_MOREBITS, "MOREBITS");
	PRINT_BIT(U_LONGENTITY, "LONGENTITY");
	PRINT_BIT(U_MODEL, "MODEL");
	PRINT_BIT(U_FRAME, "FRAME");
	PRINT_BIT(U_SKIN, "SKIN");
	PRINT_BIT(U_EFFECTS, "EFFECTS");
	PRINT_BIT(U_ORIGIN1, "ORIGIN1");
	PRINT_BIT(U_ORIGIN2, "ORIGIN2");
	PRINT_BIT(U_ORIGIN3, "ORIGIN3");
	PRINT_BIT(U_ANGLE1, "ANGLE1");
	PRINT_BIT(U_ANGLE2, "ANGLE2");
	PRINT_BIT(U_ANGLE3, "ANGLE3"); 
	PRINT_BIT(U_NOLERP, "U_NOLERP");
	quake::con << ")" << std::endl;
}
void CL_ParseUpdate (int bits)
{
	int			i;
	model_t		*model;
	int			modnum;
	qboolean	forcelink;
	entity_t	*ent;
	int			num;
	int			skin;

	if (quake::cls.signon == SIGNONS - 1)
	{	// first update is the final signon stage
		quake::cls.signon = SIGNONS;
		quake::cls.signon_reply();
	}
	if (cl_shownet.value == 2) PrintFastUpdate(bits);
	if (bits & U_MOREBITS)
	{
		i = MSG_ReadByte ();
		bits |= (i<<8);
	}

	if (bits & U_LONGENTITY)	
		num = MSG_ReadShort ();
	else
		num = MSG_ReadByte ();

	ent = CL_EntityNum (num);

for (i=0 ; i<16 ; i++)
if (bits&(1<<i))
	bitcounts[i]++;

	if (ent->msgtime != quake::cl.mtime[1])
		forcelink = true;	// no previous frame to lerp from
	else
		forcelink = false;

	ent->msgtime = quake::cl.mtime[0];
	
	if (bits & U_MODEL)
	{
		modnum = MSG_ReadByte ();
		if (modnum >= MAX_MODELS)
			Host_Error ("CL_ParseModel: bad modnum");
	}
	else
		modnum = ent->baseline.modelindex;
		
	model = quake::cl.model_precache[modnum];
	if (model != ent->model)
	{
		ent->model = model;
	// automatic animation (torches, etc) can be either all together
	// or randomized
		if (model)
		{
			if (model->synctype == ST_RAND)
				ent->syncbase = (float)(rand()&0x7fff) / 0x7fff;
			else
				ent->syncbase = 0.0;
		}
		else
			forcelink = true;	// hack to make null model players work
#ifdef GLQUAKE
		if (num > 0 && num <= quake::cl.maxclients)
			R_TranslatePlayerSkin (num - 1);
#endif
	}
	
	if (bits & U_FRAME)
		ent->frame = MSG_ReadByte ();
	else
		ent->frame = ent->baseline.frame;

	if (bits & U_COLORMAP)
		i = MSG_ReadByte();
	else
		i = ent->baseline.colormap;
	if (!i)
		ent->colormap = vid.colormap;
	else
	{
		if (i > quake::cl.maxclients)
			Sys_Error ("i >= quake::cl.maxclients");
		ent->colormap = quake::cl.scores[i-1].translations;
	}

#ifdef GLQUAKE
	if (bits & U_SKIN)
		skin = MSG_ReadByte();
	else
		skin = ent->baseline.skin;
	if (skin != ent->skinnum) {
		ent->skinnum = skin;
		if (num > 0 && num <= quake::cl.maxclients)
			R_TranslatePlayerSkin (num - 1);
	}

#else

	if (bits & U_SKIN)
		ent->skinnum = MSG_ReadByte();
	else
		ent->skinnum = ent->baseline.skin;
#endif

	if (bits & U_EFFECTS)
		ent->effects = MSG_ReadByte();
	else
		ent->effects = ent->baseline.effects;

// shift the known values for interpolation
	VectorCopy (ent->msg_origins[0], ent->msg_origins[1]);
	VectorCopy (ent->msg_angles[0], ent->msg_angles[1]);

	if (bits & U_ORIGIN1)
		ent->msg_origins[0][0] = MSG_ReadCoord ();
	else
		ent->msg_origins[0][0] = ent->baseline.origin[0];
	if (bits & U_ANGLE1)
		ent->msg_angles[0][0] = MSG_ReadAngle();
	else
		ent->msg_angles[0][0] = ent->baseline.angles[0];

	if (bits & U_ORIGIN2)
		ent->msg_origins[0][1] = MSG_ReadCoord ();
	else
		ent->msg_origins[0][1] = ent->baseline.origin[1];
	if (bits & U_ANGLE2)
		ent->msg_angles[0][1] = MSG_ReadAngle();
	else
		ent->msg_angles[0][1] = ent->baseline.angles[1];

	if (bits & U_ORIGIN3)
		ent->msg_origins[0][2] = MSG_ReadCoord ();
	else
		ent->msg_origins[0][2] = ent->baseline.origin[2];
	if (bits & U_ANGLE3)
		ent->msg_angles[0][2] = MSG_ReadAngle();
	else
		ent->msg_angles[0][2] = ent->baseline.angles[2];

	if ( bits & U_NOLERP )
		ent->forcelink = true;

	if ( forcelink )
	{	// didn't have an update last message
		VectorCopy (ent->msg_origins[0], ent->msg_origins[1]);
		VectorCopy (ent->msg_origins[0], ent->origin);
		VectorCopy (ent->msg_angles[0], ent->msg_angles[1]);
		VectorCopy (ent->msg_angles[0], ent->angles);
		ent->forcelink = true;
	}
}

/*
==================
CL_ParseBaseline
==================
*/

void CL_ParseBaseline (entity_t *ent)
{
	int			i;

	ent->baseline.modelindex = MSG_ReadByte ();
	ent->baseline.frame = MSG_ReadByte ();
	ent->baseline.colormap = MSG_ReadByte();
	ent->baseline.skin = MSG_ReadByte();
	for (i=0 ; i<3 ; i++)
	{
		ent->baseline.origin[i] = MSG_ReadCoord ();
		ent->baseline.angles[i] = MSG_ReadAngle ();
	}
}


/*
==================
CL_ParseClientdata

Server information pertaining to this client only
==================
*/
void CL_ParseClientdata (int bits)
{
	int		i, j;
	
	if (bits & SU_VIEWHEIGHT)
		quake::cl.viewheight = MSG_ReadChar ();
	else
		quake::cl.viewheight = DEFAULT_VIEWHEIGHT;

	if (bits & SU_IDEALPITCH)
		quake::cl.idealpitch = MSG_ReadChar ();
	else
		quake::cl.idealpitch = 0;
	
	VectorCopy (quake::cl.mvelocity[0], quake::cl.mvelocity[1]);
	for (i=0 ; i<3 ; i++)
	{
		if (bits & (SU_PUNCH1<<i) )
			quake::cl.punchangle[i] = MSG_ReadChar();
		else
			quake::cl.punchangle[i] = 0;
		if (bits & (SU_VELOCITY1<<i) )
			quake::cl.mvelocity[0][i] = MSG_ReadChar()*16;
		else
			quake::cl.mvelocity[0][i] = 0;
	}

// [always sent]	if (bits & SU_ITEMS)
		i = MSG_ReadLong ();

	if (quake::cl.items != i)
	{	// set flash times
		Sbar_Changed ();
		for (j=0 ; j<32 ; j++)
			if ( (i & (1<<j)) && !(quake::cl.items & (1<<j)))
				quake::cl.item_gettime[j] = quake::cl.time;
		quake::cl.items = i;
	}
		
	quake::cl.onground = (bits & SU_ONGROUND) != 0;
	quake::cl.inwater = (bits & SU_INWATER) != 0;

	if (bits & SU_WEAPONFRAME)
		quake::cl.stats[STAT_WEAPONFRAME] = MSG_ReadByte ();
	else
		quake::cl.stats[STAT_WEAPONFRAME] = 0;

	if (bits & SU_ARMOR)
		i = MSG_ReadByte ();
	else
		i = 0;
	if (quake::cl.stats[STAT_ARMOR] != i)
	{
		quake::cl.stats[STAT_ARMOR] = i;
		Sbar_Changed ();
	}

	if (bits & SU_WEAPON)
		i = MSG_ReadByte ();
	else
		i = 0;
	if (quake::cl.stats[STAT_WEAPON] != i)
	{
		quake::cl.stats[STAT_WEAPON] = i;
		Sbar_Changed ();
	}
	
	i = MSG_ReadShort ();
	if (quake::cl.stats[STAT_HEALTH] != i)
	{
		quake::cl.stats[STAT_HEALTH] = i;
		Sbar_Changed ();
	}

	i = MSG_ReadByte ();
	if (quake::cl.stats[STAT_AMMO] != i)
	{
		quake::cl.stats[STAT_AMMO] = i;
		Sbar_Changed ();
	}

	for (i=0 ; i<4 ; i++)
	{
		j = MSG_ReadByte ();
		if (quake::cl.stats[STAT_SHELLS+i] != j)
		{
			quake::cl.stats[STAT_SHELLS+i] = j;
			Sbar_Changed ();
		}
	}

	i = MSG_ReadByte ();

	if (standard_quake)
	{
		if (quake::cl.stats[STAT_ACTIVEWEAPON] != i)
		{
			quake::cl.stats[STAT_ACTIVEWEAPON] = i;
			Sbar_Changed ();
		}
	}
	else
	{
		if (quake::cl.stats[STAT_ACTIVEWEAPON] != (1<<i))
		{
			quake::cl.stats[STAT_ACTIVEWEAPON] = (1<<i);
			Sbar_Changed ();
		}
	}
}

/*
=====================
CL_NewTranslation
=====================
*/
void CL_NewTranslation (int slot)
{
	int		i, j;
	int		top, bottom;
	byte	*dest, *source;
	
	if (slot > quake::cl.maxclients)
		Sys_Error ("CL_NewTranslation: slot > quake::cl.maxclients");
	dest = quake::cl.scores[slot].translations;
	source = vid.colormap;
	Q_memcpy (dest, vid.colormap, sizeof(quake::cl.scores[slot].translations));
	top = quake::cl.scores[slot].colors & 0xf0;
	bottom = (quake::cl.scores[slot].colors &15)<<4;
#ifdef GLQUAKE
	R_TranslatePlayerSkin (slot);
#endif

	for (i=0 ; i<VID_GRADES ; i++, dest += 256, source+=256)
	{
		if (top < 128)	// the artists made some backwards ranges.  sigh.
			Q_memcpy (dest + TOP_RANGE, source + top, 16);
		else
			for (j=0 ; j<16 ; j++)
				dest[TOP_RANGE+j] = source[top+15-j];
				
		if (bottom < 128)
			Q_memcpy (dest + BOTTOM_RANGE, source + bottom, 16);
		else
			for (j=0 ; j<16 ; j++)
				dest[BOTTOM_RANGE+j] = source[bottom+15-j];		
	}
}

/*
=====================
CL_ParseStatic
=====================
*/
void CL_ParseStatic (void)
{
	entity_t *ent;
	int		i;
		
	i = quake::cl.num_statics;
	if (i >= MAX_STATIC_ENTITIES)
		Host_Error ("Too many static entities");
	ent = &cl_static_entities[i];
	quake::cl.num_statics++;
	CL_ParseBaseline (ent);

// copy it to the current state
	ent->model = quake::cl.model_precache[ent->baseline.modelindex];
	ent->frame = ent->baseline.frame;
	ent->colormap = vid.colormap;
	ent->skinnum = ent->baseline.skin;
	ent->effects = ent->baseline.effects;

	VectorCopy (ent->baseline.origin, ent->origin);
	VectorCopy (ent->baseline.angles, ent->angles);	
	R_AddEfrags (ent);
}

/*
===================
CL_ParseStaticSound
===================
*/
void CL_ParseStaticSound (void)
{
	vec3_t		org;
	int			sound_num, vol, atten;
	int			i;
	
	for (i=0 ; i<3 ; i++)
		org[i] = MSG_ReadCoord ();
	sound_num = MSG_ReadByte ();
	vol = MSG_ReadByte ();
	atten = MSG_ReadByte ();
	
	S_StaticSound (quake::cl.sound_precache[sound_num], org, vol, atten);
}

static void SHOWNET(const char* msg,  int readcount) {
	if (cl_shownet.value == 2) { 
		quake::con << std::setw(3) << (readcount - 1) << ':' << msg << std::endl;
	}
}

/*
=====================
CL_ParseServerMessage
=====================
*/
void CL_ParseServerMessage(void)
{
	int			cmd;
	int			i;

	//
	// if recording demos, copy the message out
	//
	if (cl_shownet.value == 1)
		Con_Printf("%i ", net_message.size());
	else if (cl_shownet.value == 2)
		Con_Printf("------------------\n");

	quake::cl.onground = false;	// unless the server says otherwise	
//
// parse the message
//
	MSG_BeginReading();

	while (1)
	{
		if (msg_badread)
			Host_Error("CL_ParseServerMessage: Bad server message");

		cmd = MSG_ReadByte();

		if (cmd == -1)
		{
			SHOWNET("END OF MESSAGE", msg_readcount);
			return;		// end of message
		}

		// if the high bit of the command byte is set, it is a fast update
		if (cmd & 128)
		{
			SHOWNET("fast update", msg_readcount);
			CL_ParseUpdate(cmd & 127);
			continue;
		}

		SHOWNET(svc_strings[cmd], msg_readcount);

		// other commands
		switch (cmd)
		{
		default:
			Host_Error("CL_ParseServerMessage: Illegible server message\n");
			break;

		case svc_nop:
			//			Con_Printf ("svc_nop\n");
			break;

		case svc_time:
			quake::cl.mtime[1] = quake::cl.mtime[0];
			quake::cl.mtime[0] = idCast<idTime>(MSG_ReadFloat());
			break;

		case svc_clientdata:
			i = MSG_ReadShort();
			CL_ParseClientdata(i);
			break;

		case svc_version:
			i = MSG_ReadLong();
			if (i != PROTOCOL_VERSION)
				Host_Error("CL_ParseServerMessage: Server is protocol %i instead of %i\n", i, PROTOCOL_VERSION);
			break;

		case svc_disconnect:
			Host_EndGame("Server disconnected\n");

		case svc_print:
			Con_Printf("%s", MSG_ReadString());
			break;

		case svc_centerprint:
			SCR_CenterPrint(MSG_ReadString());
			break;

		case svc_stufftext:
			Cbuf_AddText(MSG_ReadString());
			break;

		case svc_damage:
			V_ParseDamage();
			break;

		case svc_serverinfo:
			CL_ParseServerInfo();
			vid.recalc_refdef = true;	// leave intermission full screen
			break;

		case svc_setangle:
			for (i = 0; i < 3; i++)
				quake::cl.viewangles[i] = MSG_ReadAngle();
			break;

		case svc_setview:
			quake::cl.viewentity = MSG_ReadShort();
			break;

		case svc_lightstyle:
			i = MSG_ReadByte();
			if (i >= MAX_LIGHTSTYLES)
				Sys_Error("svc_lightstyle > MAX_LIGHTSTYLES");
			Q_strcpy(cl_lightstyle[i].map, MSG_ReadString());
			cl_lightstyle[i].length = Q_strlen(cl_lightstyle[i].map);
			break;

		case svc_sound:
			CL_ParseStartSoundPacket();
			break;

		case svc_stopsound:
			i = MSG_ReadShort();
			S_StopSound(i >> 3, i & 7);
			break;

		case svc_updatename:
			Sbar_Changed();
			i = MSG_ReadByte();
			if (i >= quake::cl.maxclients)
				Host_Error("CL_ParseServerMessage: svc_updatename > MAX_SCOREBOARD");
			strcpy(quake::cl.scores[i].name, MSG_ReadString());
			break;

		case svc_updatefrags:
			Sbar_Changed();
			i = MSG_ReadByte();
			if (i >= quake::cl.maxclients)
				Host_Error("CL_ParseServerMessage: svc_updatefrags > MAX_SCOREBOARD");
			quake::cl.scores[i].frags = MSG_ReadShort();
			break;

		case svc_updatecolors:
			Sbar_Changed();
			i = MSG_ReadByte();
			if (i >= quake::cl.maxclients)
				Host_Error("CL_ParseServerMessage: svc_updatecolors > MAX_SCOREBOARD");
			quake::cl.scores[i].colors = MSG_ReadByte();
			CL_NewTranslation(i);
			break;

		case svc_particle:
			R_ParseParticleEffect();
			break;

		case svc_spawnbaseline:
			i = MSG_ReadShort();
			// must use CL_EntityNum() to force quake::cl.num_entities up
			CL_ParseBaseline(CL_EntityNum(i));
			break;
		case svc_spawnstatic:
			CL_ParseStatic();
			break;
		case svc_temp_entity:
			CL_ParseTEnt();
			break;

		case svc_setpause:
		{
			quake::cl.paused = MSG_ReadByte();

			if (quake::cl.paused)
			{
				CDAudio_Pause();
#ifdef _WIN32
				VID_HandlePause(true);
#endif
			}
			else
			{
				CDAudio_Resume();
#ifdef _WIN32
				VID_HandlePause(false);
#endif
			}
		}
		break;

		case svc_signonnum:
			i = MSG_ReadByte();
			if (i <= quake::cls.signon)
				Host_Error("Received signon %i when at %i", i, quake::cls.signon);
			quake::cls.signon = i;
			quake::cls.signon_reply();
			break;

		case svc_killedmonster:
			quake::cl.stats[STAT_MONSTERS]++;
			break;

		case svc_foundsecret:
			quake::cl.stats[STAT_SECRETS]++;
			break;

		case svc_updatestat:
			i = MSG_ReadByte();
			if (i < 0 || i >= MAX_CL_STATS)
				Sys_Error("svc_updatestat: %i is invalid", i);
			quake::cl.stats[i] = MSG_ReadLong();;
			break;

		case svc_spawnstaticsound:
			CL_ParseStaticSound();
			break;

		case svc_cdtrack:
			quake::cl.cdtrack = MSG_ReadByte();
			quake::cl.looptrack = MSG_ReadByte();
			if ((quake::cls.demoplayback || quake::cls.demorecording) && (quake::cls.forcetrack != -1))
				CDAudio_Play((byte)quake::cls.forcetrack, true);
			else
				CDAudio_Play((byte)quake::cl.cdtrack, true);
			break;

		case svc_intermission:
			quake::cl.intermission = 1;
			quake::cl.completed_time = idCast<int>(quake::cl.time);
			vid.recalc_refdef = true;	// go to full screen
			break;

		case svc_finale:
			quake::cl.intermission = 2;
			quake::cl.completed_time = idCast<int>(quake::cl.time);
			vid.recalc_refdef = true;	// go to full screen
			SCR_CenterPrint(MSG_ReadString());
			break;

		case svc_cutscene:
			quake::cl.intermission = 3;
			quake::cl.completed_time = idCast<int>(quake::cl.time);
			vid.recalc_refdef = true;	// go to full screen
			SCR_CenterPrint(MSG_ReadString());
			break;

		case svc_sellscreen: {
			execute_args("help", src_command);
			break;
		}
		}
	}
}