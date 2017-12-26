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
using namespace std::chrono;

//#define	RETURN_EDICT(e) (((int *)pr_globals)[OFS_RETURN] = vm.EDICT_TO_PROG(e))

/*
===============================================================================

						BUILT-IN FUNCTIONS

===============================================================================
*/

const char *PF_VarString (int	first)
{
	int		i;
	quake::va_stack<256> out;
	for (i=first ; i<pr_argc ; i++)
	{
		out+=vm.G_STRING(OFS_PARM0+i*3).c_str();
	}
	return out;
}


/*
=================
PF_errror

This is a TERMINAL error, which will kill off the entire server.
Dumps self.

error(value)
=================
*/
void PF_error (void)
{
	const char	*s;
	auto fname =  vm.pr_xfunction->name();

	s = PF_VarString(0);
	Con_Printf ("======SERVER ERROR in %s:\n%s\n", fname,s);
	edict_t	* ed = vm.pr_global_struct->self;
	ed->Print ();

	Host_Error ("Program error");
}

/*
=================
PF_objerror

Dumps out self, then an error message.  The program is aborted and self is
removed, but the level can continue.

objerror(value)
=================
*/
void PF_objerror (void) {
	auto fname = vm.pr_xfunction->name();
	const char* s = PF_VarString(0);
	Con_Printf ("======OBJECT ERROR in %s:\n%s\n",fname,s);
	auto ed = vm.pr_global_struct->self;
	ed->Print();
	vm.ED_Free(ed);
	Host_Error ("Program error");
}



/*
==============
PF_makevectors

Writes new values for v_forward, v_up, and v_right based on angles
makevectors(vector)
==============
*/
void PF_makevectors (void)
{
	AngleVectors (vm.G_VECTOR(OFS_PARM0), vm.pr_global_struct->v_forward, vm.pr_global_struct->v_right, vm.pr_global_struct->v_up);
}

/*
=================
PF_setorigin

This is the only valid way to move an object without using the physics of the world (setting velocity and waiting).  Directly changing origin will not set internal links correctly, so clipping would be messed up.  This should be called when an object is spawned, and then only if it is teleported.

setorigin (entity, origin)
=================
*/
void PF_setorigin (void)
{
	edict_t	*e;
	float	*org;
	
	e = vm.G_EDICT(OFS_PARM0);
	org = vm.G_VECTOR(OFS_PARM1);
	VectorCopy (org, e->v.origin);
	SV_LinkEdict (e, false);
}


void SetMinMaxSize (edict_t *e, float *min, float *max, qboolean rotate)
{
	float	*angles;
	vec3_t	rmin, rmax;
	float	bounds[2][3];
	float	xvector[2], yvector[2];
	float	a;
	vec3_t	base, transformed;
	int		i, j, k, l;
	
	for (i=0 ; i<3 ; i++)
		if (min[i] > max[i])
			PR_RunError ("backwards mins/maxs");

	rotate = false;		// FIXME: implement rotation properly again

	if (!rotate)
	{
		VectorCopy (min, rmin);
		VectorCopy (max, rmax);
	}
	else
	{
	// find min / max for rotations
		angles = e->v.angles;
		
		a = angles[1]/180.0f * static_cast<float>(M_PI);
		
		xvector[0] = cos(a);
		xvector[1] = sin(a);
		yvector[0] = -sin(a);
		yvector[1] = cos(a);
		
		VectorCopy (min, bounds[0]);
		VectorCopy (max, bounds[1]);
		
		rmin[0] = rmin[1] = rmin[2] = 9999;
		rmax[0] = rmax[1] = rmax[2] = -9999;
		
		for (i=0 ; i<= 1 ; i++)
		{
			base[0] = bounds[i][0];
			for (j=0 ; j<= 1 ; j++)
			{
				base[1] = bounds[j][1];
				for (k=0 ; k<= 1 ; k++)
				{
					base[2] = bounds[k][2];
					
				// transform the point
					transformed[0] = xvector[0]*base[0] + yvector[0]*base[1];
					transformed[1] = xvector[1]*base[0] + yvector[1]*base[1];
					transformed[2] = base[2];
					
					for (l=0 ; l<3 ; l++)
					{
						if (transformed[l] < rmin[l])
							rmin[l] = transformed[l];
						if (transformed[l] > rmax[l])
							rmax[l] = transformed[l];
					}
				}
			}
		}
	}
	
// set derived values
	VectorCopy (rmin, e->v.mins);
	VectorCopy (rmax, e->v.maxs);
	VectorSubtract (max, min, e->v.size);
	
	SV_LinkEdict (e, false);
}

/*
=================
PF_setsize

the size box is rotated by the current angle

setsize (entity, minvector, maxvector)
=================
*/
void PF_setsize (void)
{
	edict_t	*e;
	float	*min, *max;
	
	e = vm.G_EDICT(OFS_PARM0);
	min = vm.G_VECTOR(OFS_PARM1);
	max = vm.G_VECTOR(OFS_PARM2);
	SetMinMaxSize (e, min, max, false);
}


/*
=================
PF_setmodel

setmodel(entity, model)
=================
*/
void PF_setmodel (void)
{
	edict_t	*e = vm.G_EDICT(OFS_PARM0);
	cstring_t m = vm.G_STRING(OFS_PARM1);

	int i = 0;
// check to see if model was properly precached
	for (auto check : sv.model_precache) {
		if (check == m) {
			e->v.model = m;
			e->v.modelindex = i;
			model_t	*mod = sv.models[i];  // Mod_ForName (m, true);

			if (mod)
				SetMinMaxSize(e, mod->mins, mod->maxs, true);
			else
				SetMinMaxSize(e, vec3_origin, vec3_origin, true);
			return;
		}
		i++;
	}
	PR_RunError("no precache: "); // %s\n", m);
		

}

/*
=================
PF_bprint

broadcast print to everyone on server

bprint(value)
=================
*/
void PF_bprint (void)
{
	auto s = PF_VarString(0);
	SV_BroadcastPrintf ("%s", s);
}

/*
=================
PF_sprint

single print to a specific client

sprint(clientent, value)
=================
*/
void PF_sprint (void)
{
	client_t	*client;
	
	int entnum =vm.G_EDICTNUM(OFS_PARM0);
	auto s = PF_VarString(1);
	
	if (entnum < 1 || entnum > svs.maxclients)
	{
		Con_Printf ("tried to sprint to a non-client\n");
		return;
	}
		
	client = &svs.clients[entnum-1];
		
	client->message.WriteChar(svc_print);
	client->message.WriteString(s );
}


/*
=================
PF_centerprint

single print to a specific client

centerprint(clientent, value)
=================
*/
void PF_centerprint (void)
{

	client_t	*client;
	int			entnum;
	
	entnum =vm.G_EDICTNUM(OFS_PARM0);
	auto s = PF_VarString(1);
	
	if (entnum < 1 || entnum > svs.maxclients)
	{
		Con_Printf ("tried to sprint to a non-client\n");
		return;
	}
		
	client = &svs.clients[entnum-1];
		
	client->message.WriteChar(svc_centerprint);
	client->message.WriteString(s );
}


/*
=================
PF_normalize

vector normalize(vector)
=================
*/
void PF_normalize (void)
{
	float	*value1;
	vec3_t	newvalue;
	float	newf;
	
	value1 = vm.G_VECTOR(OFS_PARM0);

	newf = value1[0] * value1[0] + value1[1] * value1[1] + value1[2]*value1[2];
	newf = sqrt(newf);
	
	if (newf == 0)
		newvalue[0] = newvalue[1] = newvalue[2] = 0;
	else
	{
		newf = 1/ newf;
		newvalue[0] = value1[0] * newf;
		newvalue[1] = value1[1] * newf;
		newvalue[2] = value1[2] * newf;
	}
	
	VectorCopy (newvalue, vm.G_VECTOR(OFS_RETURN));	
}

/*
=================
PF_vlen

scalar vlen(vector)
=================
*/
void PF_vlen (void)
{
	float	*value1;
	float	newf;
	
	value1 = vm.G_VECTOR(OFS_PARM0);

	newf = value1[0] * value1[0] + value1[1] * value1[1] + value1[2]*value1[2];
	newf = sqrt(newf);
	
	vm.G_FLOAT(OFS_RETURN) = newf;
}

/*
=================
PF_vectoyaw

float vectoyaw(vector)
=================
*/
void PF_vectoyaw (void)
{
	float	*value1;
	float	yaw;
	
	value1 = vm.G_VECTOR(OFS_PARM0);

	if (value1[1] == 0 && value1[0] == 0)
		yaw = 0;
	else
	{
		yaw = (int) (Q_atan2(value1[1], value1[0]) * 180.0f / static_cast<float>(M_PI));
		if (yaw < 0)
			yaw += 360;
	}

	vm.G_FLOAT(OFS_RETURN) = yaw;
}


/*
=================
PF_vectoangles

vector vectoangles(vector)
=================
*/
void PF_vectoangles (void)
{
	float	*value1;
	float	forward;
	float	yaw, pitch;
	
	value1 = vm.G_VECTOR(OFS_PARM0);

	if (value1[1] == 0.0f && value1[0] == 0.0f)
	{
		yaw = 0;
		if (value1[2] > 0)
			pitch = 90;
		else
			pitch = 270;
	}
	else
	{
		yaw = (int) (Q_atan2(value1[1], value1[0]) * 180.0f / static_cast<float>(M_PI));
		if (yaw < 0.0f)
			yaw += 360.0f;

		forward = sqrt (value1[0]*value1[0] + value1[1]*value1[1]);
		pitch = (int) (Q_atan2(value1[2], forward) * 180.0f / static_cast<float>(M_PI));
		if (pitch < 0.0f)
			pitch += 360.0f;
	}

	vm.G_FLOAT(OFS_RETURN+0) = pitch;
	vm.G_FLOAT(OFS_RETURN+1) = yaw;
	vm.G_FLOAT(OFS_RETURN+2) = 0.0f;
}

/*
=================
PF_Random

Returns a number from 0<= num < 1

random()
=================
*/
void PF_random (void)
{
	float		num;
		
	num = (rand ()&0x7fff) / ((float)0x7fff);
	
	vm.G_FLOAT(OFS_RETURN) = num;
}

/*
=================
PF_particle

particle(origin, color, count)
=================
*/
void PF_particle (void)
{
	float		*org, *dir;
	float		color;
	float		count;
			
	org = vm.G_VECTOR(OFS_PARM0);
	dir = vm.G_VECTOR(OFS_PARM1);
	color = vm.G_FLOAT(OFS_PARM2);
	count = vm.G_FLOAT(OFS_PARM3);
	SV_StartParticle (org, dir, color, count);
}


/*
=================
PF_ambientsound

=================
*/
void PF_ambientsound (void){
	const vec3_t& pos = vm.G_VECTOR (OFS_PARM0);
	cstring_t samp = vm.G_STRING(OFS_PARM1);
	float vol = vm.G_FLOAT(OFS_PARM2);
	float attenuation = vm.G_FLOAT(OFS_PARM3);
	
// check to see if samp was properly precached
	int soundnum = 0;
	for (auto check : sv.sound_precache) {
		if (check == samp) {
			// add an svc_spawnambient command to the level signon packet

			sv.signon.WriteByte(svc_spawnstaticsound);
			for (int i = 0; i<3; i++)
				sv.signon.WriteCoord(pos[i]);

			sv.signon.WriteByte(soundnum);

			sv.signon.WriteByte(static_cast<int>(vol * 255.0f));
			sv.signon.WriteByte(static_cast<int>(attenuation * 64.0f));
			return;
		}
		soundnum++;
	}	
	Con_Printf ("no precache: %s\n", samp);
}

/*
=================
PF_sound

Each entity can have eight independant sound sources, like voice,
weapon, feet, etc.

Channel 0 is an auto-allocate channel, the others override anything
allready running on that entity/channel pair.

An attenuation of 0 will play full volume everywhere in the level.
Larger attenuations will drop off.

=================
*/
void PF_sound (void) {	
	auto entity = vm.G_EDICT(OFS_PARM0);
	auto channel = vm.G_FLOAT(OFS_PARM1);
	auto sample = vm.G_STRING(OFS_PARM2);
	auto volume = vm.G_FLOAT(OFS_PARM3) * 255;
	auto attenuation = vm.G_FLOAT(OFS_PARM4);
	
	if (volume < 0 || volume > 255)
		Sys_Error ("SV_StartSound: volume = %i", volume);

	if (attenuation < 0 || attenuation > 4)
		Sys_Error ("SV_StartSound: attenuation = %f", attenuation);

	if (channel < 0 || channel > 7)
		Sys_Error ("SV_StartSound: channel = %i", channel);

	SV_StartSound (entity, channel, sample, volume, attenuation);
}

/*
=================
PF_break

break()
=================
*/
void PF_break (void)
{
Con_Printf ("break statement\n");
*(int *)-4 = 0;	// dump to debugger
//	PR_RunError ("break statement");
}

/*
=================
PF_traceline

Used for use tracing and shot targeting
Traces are blocked by bbox and exact bsp entityes, and also slide box entities
if the tryents flag is set.

traceline (vector1, vector2, tryents)
=================
*/
void PF_traceline (void){
	auto v1 = vm.G_VECTOR(OFS_PARM0);
	auto v2 = vm.G_VECTOR(OFS_PARM1);
	auto nomonsters = vm.G_FLOAT(OFS_PARM2);
	auto ent = vm.G_EDICT(OFS_PARM3);

	trace_t trace = SV_Move (v1, vec3_origin, vec3_origin, v2, nomonsters, ent);

	vm.pr_global_struct->trace_allsolid = trace.allsolid;
	vm.pr_global_struct->trace_startsolid = trace.startsolid;
	vm.pr_global_struct->trace_fraction = trace.fraction;
	vm.pr_global_struct->trace_inwater = trace.inwater;
	vm.pr_global_struct->trace_inopen = trace.inopen;
	VectorCopy (trace.endpos, vm.pr_global_struct->trace_endpos);
	VectorCopy (trace.plane.normal, vm.pr_global_struct->trace_plane_normal);
	vm.pr_global_struct->trace_plane_dist =  trace.plane.dist;	
	if (trace.ent)
		vm.pr_global_struct->trace_ent = trace.ent;
	else
		vm.pr_global_struct->trace_ent = nullptr;
}


#ifdef QUAKE2
extern trace_t SV_Trace_Toss (edict_t *ent, edict_t *ignore);

void PF_TraceToss (void)
{
	trace_t	trace;
	edict_t	*ent;
	edict_t	*ignore;

	ent = vm.G_EDICT(OFS_PARM0);
	ignore = vm.G_EDICT(OFS_PARM1);

	trace = SV_Trace_Toss (ent, ignore);

	vm.pr_global_struct->trace_allsolid = trace.allsolid;
	vm.pr_global_struct->trace_startsolid = trace.startsolid;
	vm.pr_global_struct->trace_fraction = trace.fraction;
	vm.pr_global_struct->trace_inwater = trace.inwater;
	vm.pr_global_struct->trace_inopen = trace.inopen;
	VectorCopy (trace.endpos, vm.pr_global_struct->trace_endpos);
	VectorCopy (trace.plane.normal, vm.pr_global_struct->trace_plane_normal);
	vm.pr_global_struct->trace_plane_dist =  trace.plane.dist;	
	if (trace.ent)
		vm.pr_global_struct->trace_ent = vm.EDICT_TO_PROG(trace.ent);
	else
		vm.pr_global_struct->trace_ent = vm.EDICT_TO_PROG(sv.edicts);
}
#endif


/*
=================
PF_checkpos

Returns true if the given entity can move to the given position from it's
current position by walking or rolling.
FIXME: make work...
scalar checkpos (entity, vector)
=================
*/
void PF_checkpos (void)
{
}

//============================================================================

byte	checkpvs[MAX_MAP_LEAFS/8];

int PF_newcheckclient (int check)
{
	int		i;
	byte	*pvs;
	edict_t	*ent;
	mleaf_t	*leaf;
	vec3_t	org;

// cycle to the next one

	if (check < 1)
		check = 1;
	if (check > svs.maxclients)
		check = svs.maxclients;

	if (check == svs.maxclients)
		i = 1;
	else
		i = check + 1;

	for ( ;  ; i++)
	{
		if (i == svs.maxclients+1)
			i = 1;

		ent = vm.EDICT_NUM(i);

		if (i == check)
			break;	// didn't find anything else

		if(vm.is_edict_free(ent))
			continue;
		if (ent->v.health <= 0)
			continue;
		if ((int)ent->v.flags & FL_NOTARGET)
			continue;

	// anything that is a client, or has a client as an enemy
		break;
	}

// get the PVS for the entity
	VectorAdd (ent->v.origin, ent->v.view_ofs, org);
	leaf = Mod_PointInLeaf (org, sv.worldmodel);
	pvs = Mod_LeafPVS (leaf, sv.worldmodel);
	std::memcpy (checkpvs, pvs, (sv.worldmodel->numleafs+7)>>3 );

	return i;
}

/*
=================
PF_checkclient

Returns a client (or object that has a client enemy) that would be a
valid target.

If there are more than one valid options, they are cycled each frame

If (self.origin + self.viewofs) is not in the PVS of the current target,
it is not returned at all.

name checkclient ()
=================
*/
#define	MAX_CHECK	16
int c_invis, c_notvis;
void PF_checkclient (void)
{
	mleaf_t	*leaf;
	int		l;
	vec3_t	view;
	
// find a new check if on a new frame
	if (sv.time - sv.lastchecktime >= 100ms)
	{
		sv.lastcheck = PF_newcheckclient (sv.lastcheck);
		sv.lastchecktime = sv.time;
	}

// return check if it might be visible	
	edict_t	*ent = vm.EDICT_NUM(sv.lastcheck);
	if (vm.is_edict_free(ent) || ent->v.health <= 0)
	{
		vm.RETURN_EDICT(sv.worldedict);
		return;
	}

// if current entity can't possibly see the check entity, return 0
	edict_t	*self = vm.pr_global_struct->self;
	VectorAdd (self->v.origin, self->v.view_ofs, view);
	leaf = Mod_PointInLeaf (view, sv.worldmodel);
	l = (leaf - sv.worldmodel->leafs) - 1;
	if ( (l<0) || !(checkpvs[l>>3] & (1<<(l&7)) ) )
	{
c_notvis++;
		vm.RETURN_EDICT(sv.worldedict);
		return;
	}

// might be able to see it
c_invis++;
	vm.RETURN_EDICT(ent);
}

//============================================================================


/*
=================
PF_stuffcmd

Sends text over to the client's execution buffer

stuffcmd (clientent, value)
=================
*/
void PF_stuffcmd (void)
{
	int		entnum;

	
	entnum =vm.G_EDICTNUM(OFS_PARM0);
	if (entnum < 1 || entnum > svs.maxclients)
		PR_RunError ("Parm 0 not a client");
	cstring_t str = vm.G_STRING(OFS_PARM1);
	
	client_t	*old = host_client;
	host_client = &svs.clients[entnum-1];
	Host_ClientCommands ("%s", str);
	host_client = old;
}

/*
=================
PF_localcmd

Sends text over to the client's execution buffer

localcmd (string)
=================
*/
void PF_localcmd (void)
{
	cstring_t str = vm.G_STRING(OFS_PARM0);
	Cbuf_AddText (str);
}

/*
=================
PF_cvar

float cvar (string)
=================
*/
void PF_cvar (void)
{
	cstring_t str = vm.G_STRING(OFS_PARM0);
	
	vm.G_FLOAT(OFS_RETURN) = *Cvar_Get<float>(str);
}

/*
=================
PF_cvar_set

float cvar (string)
=================
*/
void PF_cvar_set (void)
{

	cstring_t  var = vm.G_STRING(OFS_PARM0);
	cstring_t  val = vm.G_STRING(OFS_PARM1);
	
	Cvar_Set (var, val);
}

/*
=================
PF_findradius

Returns a chain of entities that have origins within a spherical area

findradius (origin, radius)
=================
*/
void PF_findradius (void)
{
	edict_t	*chain, *ent;
	float	rad;
	float	*org;
	vec3_t	eorg;
	int		i, j;

	chain = (edict_t *)sv.worldedict;
	
	org = vm.G_VECTOR(OFS_PARM0);
	rad = vm.G_FLOAT(OFS_PARM1);

	for(auto& ee : vm){
		ent = ee;
		if (ent->v.solid == SOLID_NOT)
			continue;
		for (j=0 ; j<3 ; j++)
			eorg[j] = org[j] - (ent->v.origin[j] + (ent->v.mins[j] + ent->v.maxs[j])*0.5);
		if (Length(eorg) > rad)
			continue;
			
		ent->v.chain = chain;
		chain = ent;
	}

	vm.RETURN_EDICT(chain);
}


/*
=========
PF_dprint
=========
*/
void PF_dprint (void)
{
	Con_DPrintf ("%s",PF_VarString(0));
}

char	pr_string_temp[128];

void PF_ftos (void)
{
	float v = vm.G_FLOAT(OFS_PARM0);
	
	if (v == (int)v)
		Q_sprintf (pr_string_temp, "%d",(int)v);
	else
		Q_sprintf (pr_string_temp, "%5.1f",v);
	vm.G_STRING(OFS_RETURN) = pr_string_temp;
}

void PF_fabs (void)
{
	float	v;
	v = vm.G_FLOAT(OFS_PARM0);
	vm.G_FLOAT(OFS_RETURN) = fabs(v);
}

void PF_vtos (void)
{
	Q_sprintf (pr_string_temp, "'%5.1f %5.1f %5.1f'", vm.G_VECTOR(OFS_PARM0)[0], vm.G_VECTOR(OFS_PARM0)[1], vm.G_VECTOR(OFS_PARM0)[2]);
	vm.G_STRING(OFS_RETURN) = pr_string_temp;
}

#ifdef QUAKE2
void PF_etos (void)
{
	sprintf (pr_string_temp, "entity %i",vm.G_EDICTNUM(OFS_PARM0));
	vm.G_INT(OFS_RETURN) = pr_string_temp - pr_strings;
}
#endif

void PF_Spawn (void)
{
	edict_t	*ed;
	ed = vm.ED_Alloc();
	vm.RETURN_EDICT(ed);
}

void PF_Remove (void)
{
	edict_t	*ed;
	
	ed = vm.G_EDICT(OFS_PARM0);
	vm.ED_Free(ed);
}


// entity (entity start, .string field, string match) find = #5;
void PF_Find (void)
#ifdef QUAKE2
{
	int		e;	
	int		f;
	char	*s, *t;
	edict_t	*ed;
	edict_t	*first;
	edict_t	*second;
	edict_t	*last;

	first = second = last = (edict_t *)sv.edicts;
	e =vm.G_EDICTNUM(OFS_PARM0);
	f = vm.G_INT(OFS_PARM1);
	s = vm.G_STRING(OFS_PARM2);
	if (!s)
		PR_RunError ("PF_Find: bad search string");
		
	for (e++ ; e < sv.num_edicts ; e++)
	{
		ed = vm.EDICT_NUM(e);
		if (ed->free)
			continue;
		t = E_STRING(ed,f);
		if (!t)
			continue;
		if (!strcmp(t,s))
		{
			if (first == (edict_t *)sv.edicts)
				first = ed;
			else if (second == (edict_t *)sv.edicts)
				second = ed;
			ed->v.chain = vm.EDICT_TO_PROG(last);
			last = ed;
		}
	}

	if (first != last)
	{
		if (last != second)
			first->v.chain = last->v.chain;
		else
			first->v.chain = vm.EDICT_TO_PROG(last);
		last->v.chain = vm.EDICT_TO_PROG((edict_t *)sv.edicts);
		if (second && second != last)
			second->v.chain = vm.EDICT_TO_PROG(last);
	}
	RETURN_EDICT(first);
}
#else
{
	int		e;	
	int		f;
	edict_t	*ed;

	e = vm.G_EDICTNUM(OFS_PARM0);
	f = vm.G_INT(OFS_PARM1);
	cstring_t s = vm.G_STRING(OFS_PARM2);
	if (s.empty())
		PR_RunError ("PF_Find: bad search string");
		
	for (e++ ; e < vm.used_edicts(); e++)
	{
		ed = vm.EDICT_NUM(e);
		if (vm.is_edict_free(ed))
			continue;
		cstring_t t = ed->E_STRING(f);
		if (t.empty())
			continue;
		if (t==s)
		{
			vm.RETURN_EDICT(ed);
			return;
		}
	}

	vm.RETURN_EDICT(sv.worldedict);
}
#endif

void PR_CheckEmptyString (const char *s)
{
	if (s[0] <= ' ')
		PR_RunError ("Bad string");
}

void PF_precache_file (void)
{	// precache_file is only used to copy files with qcc, it does nothing
	vm.G_INT(OFS_RETURN) = vm.G_INT(OFS_PARM0);
}

void PF_precache_sound (void)
{
	int		i;
	
	if (sv.state != ss_loading)
		PR_RunError ("PF_Precache_*: Precache can only be done in spawn functions");
		
	cstring_t s = vm.G_STRING(OFS_PARM0);
	vm.G_INT(OFS_RETURN) = vm.G_INT(OFS_PARM0);
	PR_CheckEmptyString (s.c_str());
	
	for (i=0 ; i<MAX_SOUNDS ; i++)
	{
		if (sv.sound_precache[i].empty())
		{
			sv.sound_precache[i] = s;
			return;
		}
		if (s == sv.sound_precache[i])
			return;
	}
	PR_RunError ("PF_precache_sound: overflow");
}

void PF_precache_model (void)
{
	int		i;
	
	if (sv.state != ss_loading)
		PR_RunError ("PF_Precache_*: Precache can only be done in spawn functions");
		
	cstring_t s = vm.G_STRING(OFS_PARM0);
	vm.G_INT(OFS_RETURN) = vm.G_INT(OFS_PARM0);
	PR_CheckEmptyString (s.c_str());

	for (i=0 ; i<MAX_MODELS ; i++)
	{
		if (sv.model_precache[i].empty())
		{
			sv.model_precache[i] = s;
			sv.models[i] = Mod_ForName (s, true);
			return;
		}
		if (sv.model_precache[i]== s)
			return;
	}
	PR_RunError ("PF_precache_model: overflow");
}


void PF_coredump (void)
{
	ED_PrintEdicts();
}

void PF_traceon (void)
{
	//pr_trace = true;
}

void PF_traceoff (void)
{
	//pr_trace = false;
}

void PF_eprint (void)
{
	ED_PrintNum(vm.G_EDICTNUM(OFS_PARM0));
}

/*
===============
PF_walkmove

float(float yaw, float dist) walkmove
===============
*/
void PF_walkmove (void) {
	vec3_t	move;
	
	edict_t	* ent = vm.pr_global_struct->self;
	float yaw = vm.G_FLOAT(OFS_PARM0);
	float dist = vm.G_FLOAT(OFS_PARM1);
	
	if ( !( (int)ent->v.flags & (FL_ONGROUND|FL_FLY|FL_SWIM) ) )
	{
		vm.G_FLOAT(OFS_RETURN) = 0;
		return;
	}

	yaw = yaw*M_PI*2.0f / 360.0f;
	
	move[0] = Q_cos(yaw)*dist;
	move[1] = Q_sin(yaw)*dist;
	move[2] = 0;

// save program state, because SV_movestep may call other progs
	auto oldf = vm.pr_xfunction;
	auto oldself = vm.pr_global_struct->self;
	
	vm.G_FLOAT(OFS_RETURN) = SV_movestep(ent, move, true);

// restore program state
	vm.pr_xfunction = oldf;
	vm.pr_global_struct->self = oldself;
}

/*
===============
PF_droptofloor

void() droptofloor
===============
*/
void PF_droptofloor (void)
{
	vec3_t		end;

	auto ent = vm.pr_global_struct->self;

	VectorCopy (ent->v.origin, end);
	end[2] -= 256;
	
	trace_t trace = SV_Move (ent->v.origin, ent->v.mins, ent->v.maxs, end, false, ent);

	if (trace.fraction == 1 || trace.allsolid)
		vm.G_FLOAT(OFS_RETURN) = 0;
	else
	{
		VectorCopy (trace.endpos, ent->v.origin);
		SV_LinkEdict (ent, false);
		ent->v.flags = (int)ent->v.flags | FL_ONGROUND;
		ent->v.groundentity = vm.EDICT_TO_PROG(trace.ent);
		vm.G_FLOAT(OFS_RETURN) = 1;
	}
}

/*
===============
PF_lightstyle

void(float style, string value) lightstyle
===============
*/
void PF_lightstyle (void)
{

	
	int style = static_cast<int>(vm.G_FLOAT(OFS_PARM0));
	cstring_t val = vm.G_STRING(OFS_PARM1);

// change the string in sv
	sv.lightstyles[style] = val;
	
// send message to all clients on this server
	if (sv.state != ss_active)
		return;

	client_t	*client;
	int			j;
	for (j=0, client = svs.clients ; j<svs.maxclients ; j++, client++)
		if (client->active || client->spawned)
		{
			client->message.WriteChar(svc_lightstyle);
			client->message.WriteChar(style);
			client->message.WriteString(val);
		}
}

void PF_rint (void)
{
	float	f;
	f = vm.G_FLOAT(OFS_PARM0);
	if (f > 0)
		vm.G_FLOAT(OFS_RETURN) = (int)(f + 0.5f);
	else
		vm.G_FLOAT(OFS_RETURN) = (int)(f - 0.5f);
}
void PF_floor (void)
{
	vm.G_FLOAT(OFS_RETURN) = floor(vm.G_FLOAT(OFS_PARM0));
}
void PF_ceil (void)
{
	vm.G_FLOAT(OFS_RETURN) = ceil(vm.G_FLOAT(OFS_PARM0));
}


/*
=============
PF_checkbottom
=============
*/
void PF_checkbottom (void)
{
	edict_t	*ent = vm.G_EDICT(OFS_PARM0);

	vm.G_FLOAT(OFS_RETURN) = SV_CheckBottom (ent);
}

/*
=============
PF_pointcontents
=============
*/
void PF_pointcontents (void) {
	float	*v =vm.G_VECTOR(OFS_PARM0);
	vm.G_FLOAT(OFS_RETURN) = SV_PointContents (v);	
}

/*
=============
PF_nextent

entity nextent(entity)
=============
*/
void PF_nextent (void)
{
	int		i;
	edict_t	*ent;
	
	i =vm.G_EDICTNUM(OFS_PARM0);
	ent = vm.G_EDICT(OFS_PARM0);
	while (1)
	{
		i++;
		if (i == vm.used_edicts())
		{
			vm.RETURN_EDICT(nullptr);
			return;
		}
		ent = vm.EDICT_NUM(i);
		if (vm.is_edict_free(ent)) continue;
		vm.RETURN_EDICT(ent);
		return;
	}
}

/*
=============
PF_aim

Pick a vector for the player to shoot along
vector aim(entity, missilespeed)
=============
*/
cvar_t<float> sv_aim = { 0.93f} ;
void PF_aim (void)
{
	edict_t	*ent;
	const edict_t *bestent;
	vec3_t	start, dir, end, bestdir;
	int		i, j;
	trace_t	tr;
	float	dist, bestdist;
	float	speed;
	
	ent = vm.G_EDICT(OFS_PARM0);
	speed =vm.G_FLOAT(OFS_PARM1);

	VectorCopy (ent->v.origin, start);
	start[2] += 20;

// try sending a trace straight
	VectorCopy (vm.pr_global_struct->v_forward, dir);
	VectorMA (start, 2048, dir, end);
	tr = SV_Move (start, vec3_origin, vec3_origin, end, false, ent);
	if (tr.ent && tr.ent->v.takedamage == DAMAGE_AIM
	&& (!teamplay.value || ent->v.team <=0 || ent->v.team != tr.ent->v.team) )
	{
		VectorCopy (vm.pr_global_struct->v_forward,vm.G_VECTOR(OFS_RETURN));
		return;
	}


// try all possible entities
	VectorCopy (dir, bestdir);
	bestdist = sv_aim.value;
	bestent = NULL;
	for(auto& e : vm)
	{
		const edict_t* check = e;
		if (check->v.takedamage != DAMAGE_AIM)
			continue;
		if (check == ent)
			continue;
		if (teamplay.value && ent->v.team > 0 && ent->v.team == check->v.team)
			continue;	// don't aim at teammate
		for (j=0 ; j<3 ; j++)
			end[j] = check->v.origin[j]
			+ 0.5f*(check->v.mins[j] + check->v.maxs[j]);
		VectorSubtract (end, start, dir);
		VectorNormalize (dir);
		dist = DotProduct (dir, vm.pr_global_struct->v_forward);
		if (dist < bestdist)
			continue;	// to far to turn
		tr = SV_Move (start, vec3_origin, vec3_origin, end, false, ent);
		if (tr.ent == check)
		{	// can shoot at this one
			bestdist = dist;
			bestent = check;
		}
	}
	
	if (bestent)
	{
		VectorSubtract (bestent->v.origin, ent->v.origin, dir);
		dist = DotProduct (dir, vm.pr_global_struct->v_forward);
		VectorScale (vm.pr_global_struct->v_forward, dist, end);
		end[2] = dir[2];
		VectorNormalize (end);
		VectorCopy (end,vm.G_VECTOR(OFS_RETURN));	
	}
	else
	{
		VectorCopy (bestdir,vm.G_VECTOR(OFS_RETURN));
	}
}

/*
==============
PF_changeyaw

This was a major timewaster in progs, so it was converted to C
==============
*/
void PF_changeyaw (void)
{

	float		ideal, current, move, speed;
	
	edict_t		*ent = vm.pr_global_struct->self;
	current = anglemod( ent->v.angles[1] );
	ideal = ent->v.ideal_yaw;
	speed = ent->v.yaw_speed;
	
	if (current == ideal)
		return;
	move = ideal - current;
	if (ideal > current)
	{
		if (move >= 180)
			move = move - 360;
	}
	else
	{
		if (move <= -180)
			move = move + 360;
	}
	if (move > 0)
	{
		if (move > speed)
			move = speed;
	}
	else
	{
		if (move < -speed)
			move = -speed;
	}
	
	ent->v.angles[1] = anglemod (current + move);
}

#ifdef QUAKE2
/*
==============
PF_changepitch
==============
*/
void PF_changepitch (void)
{
	edict_t		*ent;
	float		ideal, current, move, speed;
	
	ent = vm.G_EDICT(OFS_PARM0);
	current = anglemod( ent->v.angles[0] );
	ideal = ent->v.idealpitch;
	speed = ent->v.pitch_speed;
	
	if (current == ideal)
		return;
	move = ideal - current;
	if (ideal > current)
	{
		if (move >= 180)
			move = move - 360;
	}
	else
	{
		if (move <= -180)
			move = move + 360;
	}
	if (move > 0)
	{
		if (move > speed)
			move = speed;
	}
	else
	{
		if (move < -speed)
			move = -speed;
	}
	
	ent->v.angles[0] = anglemod (current + move);
}
#endif

/*
===============================================================================

MESSAGE WRITING

===============================================================================
*/

#define	MSG_BROADCAST	0		// unreliable to all
#define	MSG_ONE			1		// reliable to one (msg_entity)
#define	MSG_ALL			2		// reliable to all
#define	MSG_INIT		3		// write to the init string

sizebuf_t *WriteDest (void)
{
	int		entnum;
	int		dest;
	edict_t	*ent;

	dest = static_cast<int>(vm.G_FLOAT(OFS_PARM0));
	switch (dest)
	{
	case MSG_BROADCAST:
		return &sv.datagram;
	
	case MSG_ONE:
		ent = vm.pr_global_struct->msg_entity;
		entnum = vm.NUM_FOR_EDICT(ent);
		if (entnum < 1 || entnum > svs.maxclients)
			PR_RunError ("WriteDest: not a client");
		return &svs.clients[entnum-1].message;
		
	case MSG_ALL:
		return &sv.reliable_datagram;
	
	case MSG_INIT:
		return &sv.signon;

	default:
		PR_RunError ("WriteDest: bad destination");
		break;
	}
	
	return NULL;
}

void PF_WriteByte (void)
{
	WriteDest()->WriteByte(vm.G_FLOAT(OFS_PARM1));
}

void PF_WriteChar (void)
{
	WriteDest()->WriteChar(vm.G_FLOAT(OFS_PARM1));
}

void PF_WriteShort (void)
{
	WriteDest()->WriteShort(vm.G_FLOAT(OFS_PARM1));
}

void PF_WriteLong (void)
{
	WriteDest()->WriteLong(vm.G_FLOAT(OFS_PARM1));
}

void PF_WriteAngle (void)
{
	WriteDest()->WriteAngle(vm.G_FLOAT(OFS_PARM1));
}

void PF_WriteCoord (void)
{
	WriteDest()->WriteCoord(vm.G_FLOAT(OFS_PARM1));
}

void PF_WriteString (void)
{
	WriteDest()->WriteString(vm.G_STRING(OFS_PARM1));
}


void PF_WriteEntity (void)
{
	WriteDest()->WriteShort(vm.G_EDICTNUM(OFS_PARM1));
}

//=============================================================================

int SV_ModelIndex (cstring_t name);

void PF_makestatic (void)
{
	edict_t	*ent;
	int		i;
	
	ent = vm.G_EDICT(OFS_PARM0);

	sv.signon.WriteByte(svc_spawnstatic);

	sv.signon.WriteByte(SV_ModelIndex(ent->v.model));

	sv.signon.WriteByte(ent->v.frame);
	sv.signon.WriteByte(ent->v.colormap);
	sv.signon.WriteByte(ent->v.skin);
	for (i=0 ; i<3 ; i++)
	{
		sv.signon.WriteCoord(ent->v.origin[i]);
		sv.signon.WriteAngle(ent->v.angles[i]);
	}

// throw the entity away now
	vm.ED_Free(ent);
}

//=============================================================================

/*
==============
PF_setspawnparms
==============
*/
void PF_setspawnparms (void)
{
	edict_t	*ent;
	int		i;
	client_t	*client;

	ent = vm.G_EDICT(OFS_PARM0);
	i =vm.NUM_FOR_EDICT(ent);
	if (i < 1 || i > svs.maxclients)
		PR_RunError ("Entity is not a client");

	// copy spawn parms out of the client_t
	client = svs.clients + (i-1);

	for (i=0 ; i< NUM_SPAWN_PARMS ; i++)
		(&vm.pr_global_struct->parm1)[i] = client->spawn_parms[i];
}

/*
==============
PF_changelevel
==============
*/
void PF_changelevel (void)
{
#ifdef QUAKE2
	char	*s1, *s2;

	if (svs.changelevel_issued)
		return;
	svs.changelevel_issued = true;

	s1 = vm.G_STRING(OFS_PARM0);
	s2 = vm.G_STRING(OFS_PARM1);

	if ((int)vm.pr_global_struct->serverflags & (SFL_NEW_UNIT | SFL_NEW_EPISODE))
		Cbuf_AddText (va("changelevel %s %s\n",s1, s2));
	else
		Cbuf_AddText (va("changelevel2 %s %s\n",s1, s2));
#else

	quake::va_stack<128> va;
// make sure we don't issue two changelevels
	if (svs.changelevel_issued)
		return;
	svs.changelevel_issued = true;
	
	cstring_t s = vm.G_STRING(OFS_PARM0);
	Cbuf_AddText (va("changelevel %s\n",s));
#endif
}


#ifdef QUAKE2

#define	CONTENT_WATER	-3
#define CONTENT_SLIME	-4
#define CONTENT_LAVA	-5

#define FL_IMMUNE_WATER	131072
#define	FL_IMMUNE_SLIME	262144
#define FL_IMMUNE_LAVA	524288

#define	CHAN_VOICE	2
#define	CHAN_BODY	4

#define	ATTN_NORM	1

void PF_WaterMove (void)
{
	edict_t		*self;
	int			flags;
	int			waterlevel;
	int			watertype;
	float		drownlevel;
	float		damage = 0.0;

	self = vm.PROG_TO_EDICT(vm.pr_global_struct->self);

	if (self->v.movetype == MOVETYPE_NOCLIP)
	{
		self->v.air_finished = sv.time + 12;
		vm.G_FLOAT(OFS_RETURN) = damage;
		return;
	}

	if (self->v.health < 0)
	{
		vm.G_FLOAT(OFS_RETURN) = damage;
		return;
	}

	if (self->v.deadflag == DEAD_NO)
		drownlevel = 3;
	else
		drownlevel = 1;

	flags = (int)self->v.flags;
	waterlevel = (int)self->v.waterlevel;
	watertype = (int)self->v.watertype;

	if (!(flags & (FL_IMMUNE_WATER + FL_GODMODE)))
		if (((flags & FL_SWIM) && (waterlevel < drownlevel)) || (waterlevel >= drownlevel))
		{
			if (self->v.air_finished < sv.time)
				if (self->v.pain_finished < sv.time)
				{
					self->v.dmg = self->v.dmg + 2;
					if (self->v.dmg > 15)
						self->v.dmg = 10;
//					T_Damage (self, world, world, self.dmg, 0, FALSE);
					damage = self->v.dmg;
					self->v.pain_finished = sv.time + 1.0;
				}
		}
		else
		{
			if (self->v.air_finished < sv.time)
//				sound (self, CHAN_VOICE, "player/gasp2.wav", 1, ATTN_NORM);
				SV_StartSound (self, CHAN_VOICE, "player/gasp2.wav", 255, ATTN_NORM);
			else if (self->v.air_finished < sv.time + 9)
//				sound (self, CHAN_VOICE, "player/gasp1.wav", 1, ATTN_NORM);
				SV_StartSound (self, CHAN_VOICE, "player/gasp1.wav", 255, ATTN_NORM);
			self->v.air_finished = sv.time + 12.0;
			self->v.dmg = 2;
		}
	
	if (!waterlevel)
	{
		if (flags & FL_INWATER)
		{	
			// play leave water sound
//			sound (self, CHAN_BODY, "misc/outwater.wav", 1, ATTN_NORM);
			SV_StartSound (self, CHAN_BODY, "misc/outwater.wav", 255, ATTN_NORM);
			self->v.flags = (float)(flags &~FL_INWATER);
		}
		self->v.air_finished = sv.time + 12.0;
		vm.G_FLOAT(OFS_RETURN) = damage;
		return;
	}

	if (watertype == CONTENT_LAVA)
	{	// do damage
		if (!(flags & (FL_IMMUNE_LAVA + FL_GODMODE)))
			if (self->v.dmgtime < sv.time)
			{
				if (self->v.radsuit_finished < sv.time)
					self->v.dmgtime = sv.time + 0.2;
				else
					self->v.dmgtime = sv.time + 1.0;
//				T_Damage (self, world, world, 10*self.waterlevel, 0, TRUE);
				damage = (float)(10*waterlevel);
			}
	}
	else if (watertype == CONTENT_SLIME)
	{	// do damage
		if (!(flags & (FL_IMMUNE_SLIME + FL_GODMODE)))
			if (self->v.dmgtime < sv.time && self->v.radsuit_finished < sv.time)
			{
				self->v.dmgtime = sv.time + 1.0;
//				T_Damage (self, world, world, 4*self.waterlevel, 0, TRUE);
				damage = (float)(4*waterlevel);
			}
	}
	
	if ( !(flags & FL_INWATER) )
	{	

// player enter water sound
		if (watertype == CONTENT_LAVA)
//			sound (self, CHAN_BODY, "player/inlava.wav", 1, ATTN_NORM);
			SV_StartSound (self, CHAN_BODY, "player/inlava.wav", 255, ATTN_NORM);
		if (watertype == CONTENT_WATER)
//			sound (self, CHAN_BODY, "player/inh2o.wav", 1, ATTN_NORM);
			SV_StartSound (self, CHAN_BODY, "player/inh2o.wav", 255, ATTN_NORM);
		if (watertype == CONTENT_SLIME)
//			sound (self, CHAN_BODY, "player/slimbrn2.wav", 1, ATTN_NORM);
			SV_StartSound (self, CHAN_BODY, "player/slimbrn2.wav", 255, ATTN_NORM);

		self->v.flags = (float)(flags | FL_INWATER);
		self->v.dmgtime = 0;
	}
	
	if (! (flags & FL_WATERJUMP) )
	{
//		self.velocity = self.velocity - 0.8*self.waterlevel*frametime*self.velocity;
		VectorMA (self->v.velocity, -0.8 * self->v.waterlevel * host_frametime, self->v.velocity, self->v.velocity);
	}

	vm.G_FLOAT(OFS_RETURN) = damage;
}


void PF_sin (void)
{
	vm.G_FLOAT(OFS_RETURN) = sin(vm.G_FLOAT(OFS_PARM0));
}

void PF_cos (void)
{
	vm.G_FLOAT(OFS_RETURN) = cos(vm.G_FLOAT(OFS_PARM0));
}

void PF_sqrt (void)
{
	vm.G_FLOAT(OFS_RETURN) = sqrt(vm.G_FLOAT(OFS_PARM0));
}
#endif

void PF_Fixme (void)
{
	PR_RunError ("unimplemented bulitin");
}



builtin_t pr_builtin[] =
{
PF_Fixme,
PF_makevectors,	// void(entity e)	makevectors 		= #1;
PF_setorigin,	// void(entity e, vector o) setorigin	= #2;
PF_setmodel,	// void(entity e, string m) setmodel	= #3;
PF_setsize,	// void(entity e, vector min, vector max) setsize = #4;
PF_Fixme,	// void(entity e, vector min, vector max) setabssize = #5;
PF_break,	// void() break						= #6;
PF_random,	// float() random						= #7;
PF_sound,	// void(entity e, float chan, string samp) sound = #8;
PF_normalize,	// vector(vector v) normalize			= #9;
PF_error,	// void(string e) error				= #10;
PF_objerror,	// void(string e) objerror				= #11;
PF_vlen,	// float(vector v) vlen				= #12;
PF_vectoyaw,	// float(vector v) vectoyaw		= #13;
PF_Spawn,	// entity() spawn						= #14;
PF_Remove,	// void(entity e) remove				= #15;
PF_traceline,	// float(vector v1, vector v2, float tryents) traceline = #16;
PF_checkclient,	// entity() clientlist					= #17;
PF_Find,	// entity(entity start, .string fld, string match) find = #18;
PF_precache_sound,	// void(string s) precache_sound		= #19;
PF_precache_model,	// void(string s) precache_model		= #20;
PF_stuffcmd,	// void(entity client, string s)stuffcmd = #21;
PF_findradius,	// entity(vector org, float rad) findradius = #22;
PF_bprint,	// void(string s) bprint				= #23;
PF_sprint,	// void(entity client, string s) sprint = #24;
PF_dprint,	// void(string s) dprint				= #25;
PF_ftos,	// void(string s) ftos				= #26;
PF_vtos,	// void(string s) vtos				= #27;
PF_coredump,
PF_traceon,
PF_traceoff,
PF_eprint,	// void(entity e) debug print an entire entity
PF_walkmove, // float(float yaw, float dist) walkmove
PF_Fixme, // float(float yaw, float dist) walkmove
PF_droptofloor,
PF_lightstyle,
PF_rint,
PF_floor,
PF_ceil,
PF_Fixme,
PF_checkbottom,
PF_pointcontents,
PF_Fixme,
PF_fabs,
PF_aim,
PF_cvar,
PF_localcmd,
PF_nextent,
PF_particle,
PF_changeyaw,
PF_Fixme,
PF_vectoangles,

PF_WriteByte,
PF_WriteChar,
PF_WriteShort,
PF_WriteLong,
PF_WriteCoord,
PF_WriteAngle,
PF_WriteString,
PF_WriteEntity,

#ifdef QUAKE2
PF_sin,
PF_cos,
PF_sqrt,
PF_changepitch,
PF_TraceToss,
PF_etos,
PF_WaterMove,
#else
PF_Fixme,
PF_Fixme,
PF_Fixme,
PF_Fixme,
PF_Fixme,
PF_Fixme,
PF_Fixme,
#endif

SV_MoveToGoal,
PF_precache_file,
PF_makestatic,

PF_changelevel,
PF_Fixme,

PF_cvar_set,
PF_centerprint,

PF_ambientsound,

PF_precache_model,
PF_precache_sound,		// precache_sound2 is different only for qcc
PF_precache_file,

PF_setspawnparms
};

builtin_t *pr_builtins = pr_builtin;
int pr_numbuiltins = sizeof(pr_builtin)/sizeof(pr_builtin[0]);

