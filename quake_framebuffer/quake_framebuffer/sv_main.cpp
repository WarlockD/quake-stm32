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
// sv_main.c -- server main program
#include "icommon.h"
#include "cmd.h"

using namespace std::chrono;
server_t		sv;
server_static_t	svs;
#include <map>

void client_t::reset() {
	active = false;
	spawned = false;
	privileged = false;
	sendsignon = false;
	last_message = idTime::zero();
	netconnection = nullptr;
	cmd = usercmd_t();
	std::memset(wishdir, 0, sizeof(wishdir));
	message.Clear();
	if (edict) vm.ED_Free(edict);
	edict = nullptr;
	name = string_t();
	colors = 0;
	for (auto& s : ping_times) s = idTime::zero();
	num_pings = 0;
	for (auto& s : spawn_parms) s = 0.0f;
	old_frags = 0;
}

void server_t::reset() {
	datagram.Clear();
	reliable_datagram.Clear();
	signon.Clear();
	worldedict = nullptr;
	active = false;
	paused = false;
	loadgame = false;
	time = idTime::zero();
	lastcheck = 0;
	lastchecktime = 0;
	name = string_t();
	modelname = string_t();		// maps/<name>.bsp, for model_precache[0]
	worldmodel = nullptr;
	for (auto & s : model_precache) s = string_t();
	for (auto & s : models) s = nullptr;
	for (auto & s : sound_precache) s = string_t();
	for (auto & s : lightstyles) s = string_t();
	state = ss_loading;			// some actions are only valid during load
}
server_t::server_t() { reset(); }
	
#if 0
edict_t *vm_system_t::vm.EDICT_NUM(int n) {
	auto it = edicts.find(n);
	assert(it != edicts.end());
	return second;
}
int vm_system_t:vm.NUM_FOR_EDICT(edict_t *e) { return e->num; }
edict_t* vm_system_t::AllocEdict(int num) {
	edict_t* e = new edict_t(num);
	edicts.emplace(num, e);
	return e;
}
void vm_system_t::Unlink(edict_t *e) {
	edicts.remove(e);
	delete e;
}
#endif

//============================================================================

/*
===============
SV_Init
===============
*/
void SV_Init (void)
{
	int		i;
	extern	cvar_t<float>	sv_maxvelocity;
	extern	cvar_t<float>	sv_gravity;
	extern	cvar_t<float>	sv_nostep;
	extern	cvar_t<float>	sv_friction;
	extern	cvar_t<float>	sv_edgefriction;
	extern	cvar_t<float>	sv_stopspeed;
	extern	cvar_t<float>	sv_maxspeed;
	extern	cvar_t<float>	sv_accelerate;
	extern	cvar_t<float>	sv_idealpitchscale;
	extern	cvar_t<float>	sv_aim;

	Cvar_RegisterVariable ("sv_maxvelocity", sv_maxvelocity);
	Cvar_RegisterVariable ("sv_gravity", sv_gravity);
	Cvar_RegisterVariable ("sv_friction", sv_friction);
	Cvar_RegisterVariable ("sv_edgefriction", sv_edgefriction);
	Cvar_RegisterVariable ("sv_stopspeed", sv_stopspeed);
	Cvar_RegisterVariable ("sv_maxspeed", sv_maxspeed);
	Cvar_RegisterVariable ("sv_accelerate", sv_accelerate);
	Cvar_RegisterVariable ("sv_idealpitchscale", sv_idealpitchscale);
	Cvar_RegisterVariable ("sv_aim", sv_aim);
	Cvar_RegisterVariable ("sv_nostep", sv_nostep);

}

/*
=============================================================================

EVENT MESSAGES

=============================================================================
*/

/*  
==================
SV_StartParticle

Make sure the event gets sent to all clients
==================
*/
void SV_StartParticle (vec3_t org, vec3_t dir, int color, int count)
{
	int		i, v;

	if (sv.datagram.size() > MAX_DATAGRAM-16)
		return;	
	sv.datagram.WriteByte(svc_particle);
	sv.datagram.WriteCoord(org[0]);
	sv.datagram.WriteCoord(org[1]);
	sv.datagram.WriteCoord(org[2]);
	for (i=0 ; i<3 ; i++)
	{
		v = static_cast<int>(dir[i]*16);
		if (v > 127)
			v = 127;
		else if (v < -128)
			v = -128;
		sv.datagram.WriteChar(v);
	}
	sv.datagram.WriteByte(count);
	sv.datagram.WriteByte(color);
}           

/*  
==================
SV_StartSound

Each entity can have eight independant sound sources, like voice,
weapon, feet, etc.

Channel 0 is an auto-allocate channel, the others override anything
allready running on that entity/channel pair.

An attenuation of 0 will play full volume everywhere in the level.
Larger attenuations will drop off.  (max 4 attenuation)

==================
*/  
void SV_StartSound (edict_t *entity, int channel, cstring_t sample, int volume,
    float attenuation)
{       
    int         sound_num;
    int field_mask;
    int			i;
	int			ent;
	
	if (volume < 0 || volume > 255)
		Sys_Error ("SV_StartSound: volume = %i", volume);

	if (attenuation < 0 || attenuation > 4)
		Sys_Error ("SV_StartSound: attenuation = %f", attenuation);

	if (channel < 0 || channel > 7)
		Sys_Error ("SV_StartSound: channel = %i", channel);

	if (sv.datagram.size() > MAX_DATAGRAM-16)
		return;	

// find precache number for sound
    for (sound_num=1 ; sound_num<MAX_SOUNDS
        && sv.sound_precache[sound_num].empty() ; sound_num++)
        if (sample == sv.sound_precache[sound_num])
            break;
    
    if ( sound_num == MAX_SOUNDS || sv.sound_precache[sound_num].empty() )
    {
        Con_Printf ("SV_StartSound: %s not precacheed\n", sample.c_str());
        return;
    }
    
	ent = vm.NUM_FOR_EDICT(entity);

	channel = (ent<<3) | channel;

	field_mask = 0;
	if (volume != DEFAULT_SOUND_PACKET_VOLUME)
		field_mask |= SND_VOLUME;
	if (attenuation != DEFAULT_SOUND_PACKET_ATTENUATION)
		field_mask |= SND_ATTENUATION;

// directed messages go only to the entity the are targeted on
	sv.datagram.WriteByte(svc_sound);
	sv.datagram.WriteByte(field_mask);
	if (field_mask & SND_VOLUME)
		sv.datagram.WriteByte(volume);
	if (field_mask & SND_ATTENUATION)
		sv.datagram.WriteByte(static_cast<int>(attenuation*64));
	sv.datagram.WriteShort(channel);
	sv.datagram.WriteByte(sound_num);
	for (i=0 ; i<3 ; i++)
		sv.datagram.WriteCoord(entity->v.origin[i]+0.5f*(entity->v.mins[i]+entity->v.maxs[i]));
}           

/*
==============================================================================

CLIENT SPAWNING

==============================================================================
*/

/*
================
SV_SendServerinfo

Sends the first message from the server to a connected client.
This will be sent on the initial connection and upon each server load.
================
*/
void SV_SendServerinfo (client_t *client)
{
	char			message[2048];

	client->message.WriteByte(svc_print);
	Q_sprintf (message, "%c\nVERSION %4.2f SERVER (%i CRC)", 2, VERSION, vm.pr_crc);
	client->message.WriteString(message);

	client->message.WriteByte(svc_serverinfo);
	client->message.WriteLong(PROTOCOL_VERSION);
	client->message.WriteByte(svs.maxclients);

	if (!coop.value && deathmatch.value)
		client->message.WriteByte(GAME_DEATHMATCH);
	else
		client->message.WriteByte(GAME_COOP);

	Q_sprintf (message, sv.worldedict->v.message.c_str());

	client->message.WriteString(message);

	for (auto it = std::begin(sv.model_precache)+1 ; 
		it != std::end(sv.model_precache); it++)
		client->message.WriteString(*it);
	client->message.WriteByte(0);

	for (auto it = std::begin(sv.sound_precache) + 1;
		it != std::end(sv.sound_precache); it++)
		client->message.WriteString(*it);

	client->message.WriteByte(0);

// send music
	client->message.WriteByte(svc_cdtrack);
	client->message.WriteByte(static_cast<int>(sv.worldedict->v.sounds));
	client->message.WriteByte(static_cast<int>(sv.worldedict->v.sounds));

// set view	
	client->message.WriteByte(svc_setview);
	client->message.WriteShort(vm.NUM_FOR_EDICT(client->edict));

	client->message.WriteByte(svc_signonnum);
	client->message.WriteByte(1);

	client->sendsignon = true;
	client->spawned = false;		// need prespawn, spawn, etc
}

/*
================
SV_ConnectClient

Initializes a client_t for a new net connection.  This will only be called
once for a player each game, not once for each level change.
================
*/
void SV_ConnectClient (int clientnum)
{
	edict_t			*ent;
	client_t		*client;
	int				edictnum;
	qsocket_t *netconnection;
	int				i;
	float			spawn_parms[NUM_SPAWN_PARMS];

	client = svs.clients + clientnum;

	Con_DPrintf ("Client %s connected\n", client->netconnection->address);

	edictnum = clientnum+1;

	ent = vm.EDICT_NUM(edictnum);
	
// set up the client_t
	netconnection = client->netconnection;
	
	if (sv.loadgame)
		memcpy (spawn_parms, client->spawn_parms, sizeof(spawn_parms));
	memset (client, 0, sizeof(*client));
	client->netconnection = netconnection;

	client->name = string_t::intern("unconnected");
	client->active = true;
	client->spawned = false;
	client->edict = ent;
	client->message.Clear();
		
#ifdef IDGODS
	client->privileged = IsID(&client->netconnection->addr);
#else	
	client->privileged = false;				
#endif

	if (sv.loadgame)
		memcpy (client->spawn_parms, spawn_parms, sizeof(spawn_parms));
	else
	{
	// call the progs to get default spawn parms for the new client
		vm.pr_global_struct->SetNewParms->call();
		for (i=0 ; i<NUM_SPAWN_PARMS ; i++)
			client->spawn_parms[i] = (&vm.pr_global_struct->parm1)[i];
	}

	SV_SendServerinfo (client);
}


/*
===================
SV_CheckForNewClients

===================
*/
void SV_CheckForNewClients (void)
{
	qsocket_t	*ret;
	int				i;
		
//
// check for new connections
//
	while (1)
	{
		ret = NET_CheckNewConnections ();
		if (!ret)
			break;

	// 
	// init a new client structure
	//	
		for (i=0 ; i<svs.maxclients ; i++)
			if (!svs.clients[i].active)
				break;
		if (i == svs.maxclients)
			Sys_Error ("Host_CheckForNewClients: no free clients");
		
		svs.clients[i].netconnection = ret;
		SV_ConnectClient (i);	
	
		net_activeconnections++;
	}
}



/*
===============================================================================

FRAME UPDATES

===============================================================================
*/

/*
==================
SV_ClearDatagram

==================
*/
void SV_ClearDatagram (void)
{
	sv.datagram.Clear();
}

/*
=============================================================================

The PVS must include a small area around the client to allow head bobbing
or other small motion on the client side.  Otherwise, a bob might cause an
entity that should be visible to not show up, especially when the bob
crosses a waterline.

=============================================================================
*/

int		fatbytes;
byte	fatpvs[MAX_MAP_LEAFS/8];

void SV_AddToFatPVS (vec3_t org, mnode_t *node)
{
	int		i;
	byte	*pvs;
	mplane_t	*plane;
	float	d;

	while (1)
	{
	// if this is a leaf, accumulate the pvs bits
		if (node->contents < 0)
		{
			if (node->contents != CONTENTS_SOLID)
			{
				pvs = Mod_LeafPVS ( (mleaf_t *)node, sv.worldmodel);
				for (i=0 ; i<fatbytes ; i++)
					fatpvs[i] |= pvs[i];
			}
			return;
		}
	
		plane = node->plane;
		d = DotProduct (org, plane->normal) - plane->dist;
		if (d > 8)
			node = node->children[0];
		else if (d < -8)
			node = node->children[1];
		else
		{	// go down both
			SV_AddToFatPVS (org, node->children[0]);
			node = node->children[1];
		}
	}
}

/*
=============
SV_FatPVS

Calculates a PVS that is the inclusive or of all leafs within 8 pixels of the
given point.
=============
*/
byte *SV_FatPVS (vec3_t org)
{
	fatbytes = (sv.worldmodel->numleafs+31)>>3;
	Q_memset (fatpvs, 0, fatbytes);
	SV_AddToFatPVS (org, sv.worldmodel->nodes);
	return fatpvs;
}

//=============================================================================


/*
=============
SV_WriteEntitiesToClient

=============
*/
void SV_WriteEntitiesToClient (edict_t	*clent, sizebuf_t *msg)
{
	int		 i;
	int		bits;
	byte	*pvs;
	vec3_t	org;
	float	miss;
	edict_t	*ent;

// find the client's PVS
	VectorAdd (clent->v.origin, clent->v.view_ofs, org);
	pvs = SV_FatPVS (org);

// send over all entities (excpet the client) that touch the pvs
		for(auto& it : vm) {
			ent = it;
			
#ifdef QUAKE2
		// don't send if flagged for NODRAW and there are no lighting effects
		if (ent->v.effects == EF_NODRAW)
			continue;
#endif

// ignore if not touching a PV leaf
		if (ent != clent)	// clent is ALLWAYS sent
		{
// ignore ents without visible models
			if (!ent->v.modelindex || !ent->v.model.empty())
				continue;

			for (i=0 ; i < ent->num_leafs ; i++)
				if (pvs[ent->leafnums[i] >> 3] & (1 << (ent->leafnums[i]&7) ))
					break;
				
			if (i == ent->num_leafs)
				continue;		// not visible
		}

		if (msg->maxsize() - msg->size() < 16)
		{
			Con_Printf ("packet overflow\n");
			return;
		}

// send an update
		bits = 0;
		
		for (i=0 ; i<3 ; i++)
		{
			miss = ent->v.origin[i] - ent->baseline.origin[i];
			if ( miss < -0.1 || miss > 0.1 )
				bits |= U_ORIGIN1<<i;
		}

		if ( ent->v.angles[0] != ent->baseline.angles[0] )
			bits |= U_ANGLE1;
			
		if ( ent->v.angles[1] != ent->baseline.angles[1] )
			bits |= U_ANGLE2;
			
		if ( ent->v.angles[2] != ent->baseline.angles[2] )
			bits |= U_ANGLE3;
			
		if (ent->v.movetype == MOVETYPE_STEP)
			bits |= U_NOLERP;	// don't mess up the step animation
	
		if (ent->baseline.colormap != ent->v.colormap)
			bits |= U_COLORMAP;
			
		if (ent->baseline.skin != ent->v.skin)
			bits |= U_SKIN;
			
		if (ent->baseline.frame != ent->v.frame)
			bits |= U_FRAME;
		
		if (ent->baseline.effects != ent->v.effects)
			bits |= U_EFFECTS;
		
		if (ent->baseline.modelindex != ent->v.modelindex)
			bits |= U_MODEL;

		if (vm.NUM_FOR_EDICT(ent) >= 256)
			bits |= U_LONGENTITY;
			
		if (bits >= 256)
			bits |= U_MOREBITS;

	//
	// write the message
	//
		msg->WriteByte(bits | U_SIGNAL);
		
		if (bits & U_MOREBITS)
			msg->WriteByte(bits>>8);
		if (bits & U_LONGENTITY)
			msg->WriteShort(vm.NUM_FOR_EDICT(ent));
		else
			msg->WriteByte(vm.NUM_FOR_EDICT(ent));

		if (bits & U_MODEL)
			msg->WriteByte(static_cast<int>(ent->v.modelindex));
		if (bits & U_FRAME)
			msg->WriteByte(static_cast<int>(ent->v.frame));
		if (bits & U_COLORMAP)
			msg->WriteByte(static_cast<int>(ent->v.colormap));
		if (bits & U_SKIN)
			msg->WriteByte(static_cast<int>(ent->v.skin));
		if (bits & U_EFFECTS)
			msg->WriteByte(static_cast<int>(ent->v.effects));
		if (bits & U_ORIGIN1)
			msg->WriteCoord(ent->v.origin[0]);		
		if (bits & U_ANGLE1)
			msg->WriteAngle(ent->v.angles[0]);
		if (bits & U_ORIGIN2)
			msg->WriteCoord(ent->v.origin[1]);
		if (bits & U_ANGLE2)
			msg->WriteAngle(ent->v.angles[1]);
		if (bits & U_ORIGIN3)
			msg->WriteCoord(ent->v.origin[2]);
		if (bits & U_ANGLE3)
			msg->WriteAngle(ent->v.angles[2]);
	}
}

/*
=============
SV_CleanupEnts

=============
*/
void SV_CleanupEnts (void)
{
	edict_t	*ent;
	
	for (auto& it : vm) {
		ent = it;
		ent->v.effects = static_cast<float>(static_cast<int>(ent->v.effects) & ~EF_MUZZLEFLASH);
	}

}

/*
==================
SV_WriteClientdataToMessage

==================
*/
void SV_WriteClientdataToMessage (edict_t *ent, sizebuf_t *msg)
{
	int		bits;
	int		i;
	edict_t	*other;
	int		items;
#ifndef QUAKE2
	eval_t	*val;
#endif

//
// send a damage message
//
	if (ent->v.dmg_take || ent->v.dmg_save)
	{
		other = ent->v.dmg_inflictor;
		msg->WriteByte(svc_damage);
		msg->WriteByte(static_cast<int>(ent->v.dmg_save));
		msg->WriteByte(static_cast<int>(ent->v.dmg_take));
		for (i=0 ; i<3 ; i++)
			msg->WriteCoord(other->v.origin[i] + 0.5f*(other->v.mins[i] + other->v.maxs[i]));
	
		ent->v.dmg_take = 0;
		ent->v.dmg_save = 0;
	}

//
// send the current viewpos offset from the view entity
//
	SV_SetIdealPitch ();		// how much to look up / down ideally

// a fixangle might get lost in a dropped packet.  Oh well.
	if ( ent->v.fixangle )
	{
		msg->WriteByte(svc_setangle);
		for (i=0 ; i < 3 ; i++)
			msg->WriteAngle(ent->v.angles[i] );
		ent->v.fixangle = 0;
	}

	bits = 0;
	
	if (ent->v.view_ofs[2] != DEFAULT_VIEWHEIGHT)
		bits |= SU_VIEWHEIGHT;
		
	if (ent->v.idealpitch)
		bits |= SU_IDEALPITCH;

// stuff the sigil bits into the high bits of items for sbar, or else
// mix in items2
#ifdef QUAKE2
	items = (int)ent->v.items | ((int)ent->v.items2 << 23);
#else
	val = ent->E_EVAL("items2");

	if (val)
		items = (int)ent->v.items | ((int)val->_float << 23);
	else
		items = (int)ent->v.items | ((int)vm.pr_global_struct->serverflags << 28);
#endif

	bits |= SU_ITEMS;
	
	if ( (int)ent->v.flags & FL_ONGROUND)
		bits |= SU_ONGROUND;
	
	if ( ent->v.waterlevel >= 2)
		bits |= SU_INWATER;
	
	for (i=0 ; i<3 ; i++)
	{
		if (ent->v.punchangle[i])
			bits |= (SU_PUNCH1<<i);
		if (ent->v.velocity[i])
			bits |= (SU_VELOCITY1<<i);
	}
	
	if (ent->v.weaponframe)
		bits |= SU_WEAPONFRAME;

	if (ent->v.armorvalue)
		bits |= SU_ARMOR;

//	if (ent->v.weapon)
		bits |= SU_WEAPON;

// send the data

	msg->WriteByte(svc_clientdata);
	msg->WriteShort(bits);

	if (bits & SU_VIEWHEIGHT)
		msg->WriteChar(static_cast<int>(ent->v.view_ofs[2]));

	if (bits & SU_IDEALPITCH)
		msg->WriteChar(static_cast<int>(ent->v.idealpitch));

	for (i=0 ; i<3 ; i++)
	{
		if (bits & (SU_PUNCH1<<i))
			msg->WriteChar(static_cast<int>(ent->v.punchangle[i]));
		if (bits & (SU_VELOCITY1<<i))
			msg->WriteChar(static_cast<int>(ent->v.velocity[i]/16));
	}

// [always sent]	if (bits & SU_ITEMS)
	msg->WriteLong(items);

	if (bits & SU_WEAPONFRAME)
		msg->WriteByte(ent->v.weaponframe);
	if (bits & SU_ARMOR)
		msg->WriteByte(ent->v.armorvalue);
	if (bits & SU_WEAPON)
		msg->WriteByte(SV_ModelIndex(ent->v.weaponmodel));

	
	msg->WriteShort(ent->v.health);
	msg->WriteByte(ent->v.currentammo);
	msg->WriteByte(ent->v.ammo_shells);
	msg->WriteByte(ent->v.ammo_nails);
	msg->WriteByte(ent->v.ammo_rockets);
	msg->WriteByte(ent->v.ammo_cells);

	if (standard_quake)
	{
		msg->WriteByte(ent->v.weapon);
	}
	else
	{
		for(i=0;i<32;i++)
		{
			if ( ((int)ent->v.weapon) & (1<<i) )
			{
				msg->WriteByte(i);
				break;
			}
		}
	}
}

/*
=======================
SV_SendClientDatagram
=======================
*/
qboolean SV_SendClientDatagram (client_t *client)
{
	byte		buf[MAX_DATAGRAM];
	sizebuf_t	msg(buf, sizeof(buf));
	

	msg.WriteByte(svc_time);
	msg.WriteFloat(idCast<float>(sv.time));

// add the client specific data to the datagram
	SV_WriteClientdataToMessage (client->edict, &msg);

	SV_WriteEntitiesToClient (client->edict, &msg);

// copy the server datagram if there is space
	if (msg.size() + sv.datagram.size() < msg.maxsize())
		msg.Write(sv.datagram.data(), sv.datagram.size());

// send the datagram
	if (NET_SendUnreliableMessage (client->netconnection, &msg) == -1)
	{
		SV_DropClient (true);// if the message couldn't send, kick off
		return false;
	}
	
	return true;
}

/*
=======================
SV_UpdateToReliableMessages
=======================
*/
void SV_UpdateToReliableMessages (void)
{
	int			i, j;
	client_t *client;

// check for changes to be sent over the reliable streams
	for (i=0, host_client = svs.clients ; i<svs.maxclients ; i++, host_client++)
	{
		if (host_client->old_frags != host_client->edict->v.frags)
		{
			for (j=0, client = svs.clients ; j<svs.maxclients ; j++, client++)
			{
				if (!client->active)
					continue;
				client->message.WriteByte(svc_updatefrags);
				client->message.WriteByte(i);
				client->message.WriteShort(host_client->edict->v.frags);
			}

			host_client->old_frags = static_cast<int>(host_client->edict->v.frags);
		}
	}
	
	for (j=0, client = svs.clients ; j<svs.maxclients ; j++, client++)
	{
		if (!client->active)
			continue;
		client->message.Write(sv.reliable_datagram.data(), sv.reliable_datagram.size());
	}

	sv.reliable_datagram.Clear();
}


/*
=======================
SV_SendNop

Send a nop message without trashing or sending the accumulated client
message buffer
=======================
*/
void SV_SendNop (client_t *client)
{
	byte		buf[4];
	sizebuf_t	msg(buf, sizeof(buf));

	msg.WriteChar(svc_nop);

	if (NET_SendUnreliableMessage (client->netconnection, &msg) == -1)
		SV_DropClient (true);	// if the message couldn't send, kick off
	client->last_message = realtime;
}

/*
=======================
SV_SendClientMessages
=======================
*/
void SV_SendClientMessages (void)
{
	using namespace std::chrono;
	int			i;
	
// update frags, names, etc
	SV_UpdateToReliableMessages ();

// build individual updates
	for (i=0, host_client = svs.clients ; i<svs.maxclients ; i++, host_client++)
	{
		if (!host_client->active)
			continue;

		if (host_client->spawned)
		{
			if (!SV_SendClientDatagram (host_client))
				continue;
		}
		else
		{
		// the player isn't totally in the game yet
		// send small keepalive messages if too much time has passed
		// send a full message when the next signon stage has been requested
		// some other message data (name changes, etc) may accumulate 
		// between signon stages
			if (!host_client->sendsignon)
			{
				if (realtime - host_client->last_message > 5s)
					SV_SendNop (host_client);
				continue;	// don't send out non-signon messages
			}
		}

		// check for an overflowed message.  Should only happen
		// on a very fucked up connection that backs up a lot, then
		// changes level
		if (host_client->message.overflowed())
		{
			SV_DropClient (true);
			host_client->message.overflowed(false);
			continue;
		}
			
		if (host_client->message.size() || host_client->dropasap)
		{
			if (!NET_CanSendMessage (host_client->netconnection))
			{
//				I_Printf ("can't write\n");
				continue;
			}

			if (host_client->dropasap)
				SV_DropClient (false);	// went to another level
			else
			{
				if (NET_SendMessage (host_client->netconnection
				, &host_client->message) == -1)
					SV_DropClient (true);	// if the message couldn't send, kick off
				host_client->message.Clear();
				host_client->last_message = realtime;
				host_client->sendsignon = false;
			}
		}
	}
	
	
// clear muzzle flashes
	SV_CleanupEnts ();
}


/*
==============================================================================

SERVER SPAWNING

==============================================================================
*/

/*
================
SV_ModelIndex

================
*/
int SV_ModelIndex (cstring_t name)
{
	int		i;
	
	if (name.empty())
		return 0;

	for (i=0 ; i<MAX_MODELS && !sv.model_precache[i].empty() ; i++)
		if (name == sv.model_precache[i])
			return i;
	if (i==MAX_MODELS || !sv.model_precache[i].empty())
		Sys_Error ("SV_ModelIndex: model %s not precached", name);
	return i;
}

/*
================
SV_CreateBaseline

================
*/
void SV_CreateBaseline (void)
{
	int			i;
	edict_t			*svent;
	int				entnum;	
		
	for(auto& e : vm)
	{
	// get the current server version
		svent = e;

		if (entnum > svs.maxclients && !svent->v.modelindex)
			continue;

	//
	// create entity baseline
	//
		VectorCopy (svent->v.origin, svent->baseline.origin);
		VectorCopy (svent->v.angles, svent->baseline.angles);
		svent->baseline.frame = static_cast<int>(svent->v.frame);
		svent->baseline.skin = static_cast<int>(svent->v.skin);
		if (entnum > 0 && entnum <= svs.maxclients)
		{
			svent->baseline.colormap = entnum;
			svent->baseline.modelindex = SV_ModelIndex("progs/player.mdl");
		}
		else
		{
			svent->baseline.colormap = 0;
			svent->baseline.modelindex =
				SV_ModelIndex(svent->v.model);
		}
		
	//
	// add to the message
	//
		sv.signon.WriteByte(svc_spawnbaseline);		
		sv.signon.WriteShort(entnum);

		sv.signon.WriteByte(svent->baseline.modelindex);
		sv.signon.WriteByte(svent->baseline.frame);
		sv.signon.WriteByte(svent->baseline.colormap);
		sv.signon.WriteByte(svent->baseline.skin);
		for (i=0 ; i<3 ; i++)
		{
			sv.signon.WriteCoord(svent->baseline.origin[i]);
			sv.signon.WriteAngle(svent->baseline.angles[i]);
		}
	}
}


/*
================
SV_SendReconnect

Tell all the clients that the server is changing levels
================
*/
void SV_SendReconnect (void)
{
	byte	data[128];
	sizebuf_t	msg(data, sizeof(data));


	msg.WriteChar(svc_stufftext);
	msg.WriteString("reconnect\n");
	NET_SendToAll (&msg, 5s);
	
	if (quake::cls.state != ca_dedicated)
#ifdef QUAKE2
		Cbuf_InsertText("reconnect\n");
#else
	{
		execute_args("reconnect\n", src_command);
	}
#endif
}


/*
================
SV_SaveSpawnparms

Grabs the current state of each client for saving across the
transition to another level
================
*/
void SV_SaveSpawnparms (void)
{
	int		i, j;

	svs.serverflags = static_cast<int>(vm.pr_global_struct->serverflags);

	for (i=0, host_client = svs.clients ; i<svs.maxclients ; i++, host_client++)
	{
		if (!host_client->active)
			continue;

	// call the progs to get default spawn parms for the new client
		vm.pr_global_struct->SetChangeParms->call(host_client->edict);
		for (j=0 ; j<NUM_SPAWN_PARMS ; j++)
			host_client->spawn_parms[j] = (&vm.pr_global_struct->parm1)[j];
	}
}


/*
================
SV_SpawnServer

This is called at the start of each level
================
*/
extern idTime		scr_centertime_off;
std::vector<edict_t*> debug_edicts;


#ifdef QUAKE2
void SV_SpawnServer (char *server, char *startspot)
#else
void SV_SpawnServer (cstring_t server)
#endif
{
	debug_edicts.clear();

	int			i;

	// let's not have any servers with no name
	if (hostname.value.empty())
		Cvar_Set ("hostname", "UNNAMED");
	scr_centertime_off = idTime::zero();

	quake::dcon << "SpawnServer: " << server << std::endl;
	svs.changelevel_issued = false;		// now safe to issue another

//
// tell all connected clients that we are going to a new level
//
	if (sv.active) 		SV_SendReconnect();
//
// make cvars consistant
//
	if (coop.value)
		Cvar_Set ("deathmatch", 0);
	current_skill = (int)(skill.value + 0.5);
	if (current_skill < 0)
		current_skill = 0;
	if (current_skill > 3)
		current_skill = 3;

	Cvar_Set("skill", (float)current_skill);
	
//
// set up the new server
//
	Host_ClearMemory ();

	//Q_memset (&sv, 0, sizeof(sv));
	sv = server_t();
	sv.name = string_t::intern(server);
#ifdef QUAKE2
	if (startspot)
		strcpy(sv.startspot, startspot);
#endif

	// load progs to get entity field count
	vm.LoadProgs();

	// allocate server memory
#ifdef USE_OLD_EDICT_SYSTEM
	sv.max_edicts = MAX_EDICTS;

	sv.edicts = (decltype(sv.edicts))Hunk_AllocName(sv.max_edicts*pr_edict_size, "edicts");
	for (size_t i = 0; i < pr_edict_size; i++) {
		debug_edicts.push_back(new(sv.edicts + i) edict_t);
	}

	else


#endif
	vm.ED_HulkAllocEdicts(MAX_EDICTS);
// leave slots at start for clients only
	
	sv.worldedict = vm.ED_Alloc(true,vm.pr_edicts[0]);
	assert(vm.NUM_FOR_EDICT(sv.worldedict) == 0);// needs to be zero?
	for (i=0 ; i<svs.maxclients ; i++)
	{
		edict_t* ent = vm.ED_Alloc(true, vm.pr_edicts[i + 1]);
		assert(vm.NUM_FOR_EDICT(ent) == (i+1));// needs to be zero?
		svs.clients[i].edict = ent;
	}
	
	sv.state = ss_loading;
	sv.paused = false;

	sv.time = 1s;
	
	sv.name = string_t::intern(server);
	quake::fixed_string_stream<128> ss;
	ss << "maps/" << server << ".bsp";
	auto test = ss.str();
	sv.modelname = string_t::intern(ss.str());;
	sv.worldmodel = Mod_ForName (sv.modelname, false);
	if (!sv.worldmodel)
	{
		Con_Printf ("Couldn't spawn server %s\n", sv.modelname);
		sv.active = false;
		return;
	}
	sv.models[1] = sv.worldmodel;
	
//
// clear world interaction links
//
	SV_ClearWorld ();
	
	sv.sound_precache[0] = string_t();
	sv.model_precache[0] = string_t();
	sv.model_precache[1] = sv.modelname;
	for (i=1 ; i<sv.worldmodel->numsubmodels ; i++)
	{
		const char* localmodel = vm.ED_QuickToString(i);
		sv.model_precache[1+i] = string_t::intern(localmodel);
		sv.models[i+1] = Mod_ForName (localmodel, false);
	}
	
//
// load the rest of the entities
//	
	edict_t	*ent = sv.worldedict;
	//Q_memset (&ent->v, 0, vm.progs->entityfields * 4);

	ent->v.model = sv.worldmodel->name;
	ent->v.modelindex = 1;		// world model
	ent->v.solid = SOLID_BSP;
	ent->v.movetype = MOVETYPE_PUSH;

	if (coop.value)
		vm.pr_global_struct->coop = coop.value;
	else
		vm.pr_global_struct->deathmatch = deathmatch.value;

	vm.pr_global_struct->mapname = sv.name;
#ifdef QUAKE2
	vm.pr_global_struct->startspot = sv.startspot - pr_strings;
#endif

// serverflags are for cross level information (sigils)
	vm.pr_global_struct->serverflags = static_cast<float>(svs.serverflags);
	
	ED_LoadFromFile (sv.worldmodel->entities);

	sv.active = true;

// all setup is completed, any further precache statements are errors
	sv.state = ss_active;
	
// run two frames to allow everything to settle
	host_frametime = 100ms;
	SV_Physics ();
	SV_Physics ();

// create a baseline for more efficient communications
	SV_CreateBaseline ();

// send serverinfo to all connected clients
	for (i=0,host_client = svs.clients ; i<svs.maxclients ; i++, host_client++)
		if (host_client->active)
			SV_SendServerinfo (host_client);
	
	Con_DPrintf ("Server spawned.\n");
}

