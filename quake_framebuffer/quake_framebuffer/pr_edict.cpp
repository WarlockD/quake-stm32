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
// sv_edict.c -- entity dictionary

#include "quakedef.h"

dprograms_t		*progs;
quake::array_view<dfunction_t>		pr_functions;
 char			*					pr_strings;
quake::array_view<ddef_t>			pr_globaldefs;
quake::array_view<ddef_t>			pr_fielddefs;
quake::array_view<dstatement_t>	pr_statements;
globalvars_t*	pr_global_struct;
quake::array_view<float>			pr_globals;			// same as pr_global_struct
#if 0
dfunction_t		*pr_functions;
char			*pr_strings;
ddef_t			*pr_fielddefs;
ddef_t			*pr_globaldefs;
dstatement_t	*pr_statements;
globalvars_t	*pr_global_struct;
float			*pr_globals;			// same as pr_global_struct
#endif
int				pr_edict_size;	// in bytes

unsigned short		pr_crc;

int		type_size[8] = {1,sizeof(string_t)/4,1,3,1,1,sizeof(func_t)/4,sizeof(void *)/4};

ddef_t *ED_FieldAtOfs (int ofs);
qboolean	ED_ParseEpair (void *base, ddef_t *key, const quake::string_view& s);

cvar_t	nomonsters = {"nomonsters", "0"};
cvar_t	gamecfg = {"gamecfg", "0"};
cvar_t	scratch1 = {"scratch1", "0"};
cvar_t	scratch2 = {"scratch2", "0"};
cvar_t	scratch3 = {"scratch3", "0"};
cvar_t	scratch4 = {"scratch4", "0"};
cvar_t	savedgamecfg = {"savedgamecfg", "0", true};
cvar_t	saved1 = {"saved1", "0", true};
cvar_t	saved2 = {"saved2", "0", true};
cvar_t	saved3 = {"saved3", "0", true};
cvar_t	saved4 = {"saved4", "0", true};



static gefv_cache	gefvCache[GEFV_CACHESIZE] = {{NULL, ""}, {NULL, ""}};

/*
=================
ED_ClearEdict

Sets everything to NULL
=================
*/
void ED_ClearEdict (edict_t *e)
{
	memset (&e->v, 0, progs->entityfields * 4);
	e->free = false;
}

/*
=================
ED_Alloc

Either finds a free edict, or allocates a new one.
Try to avoid reusing an entity that was recently freed, because it
can cause the client to think the entity morphed into something else
instead of being removed and recreated, which can cause interpolated
angles and bad trails.
=================
*/
edict_t *ED_Alloc (void)
{
	int			i;
	edict_t		*e;

	for ( i=svs.maxclients+1 ; i<sv.num_edicts ; i++)
	{
		e = EDICT_NUM(i);
		// the first couple seconds of server time can involve a lot of
		// freeing and allocating, so relax the replacement policy
		if (e->free && ( e->freetime < 2 || sv.time - e->freetime > 0.5 ) )
		{
			ED_ClearEdict (e);
			return e;
		}
	}
	
	if (i == MAX_EDICTS)
		Sys_Error ("ED_Alloc: no free edicts");
		
	sv.num_edicts++;
	e = EDICT_NUM(i);
	ED_ClearEdict (e);

	return e;
}

/*
=================
ED_Free

Marks the edict as free
FIXME: walk all entities and NULL out references to this entity
=================
*/
void ED_Free (edict_t *ed)
{
	SV_UnlinkEdict (ed);		// unlink from world bsp

	ed->free = true;
	ed->v.model = 0;
	ed->v.takedamage = 0;
	ed->v.modelindex = 0;
	ed->v.colormap = 0;
	ed->v.skin = 0;
	ed->v.frame = 0;
	VectorCopy (vec3_origin, ed->v.origin);
	VectorCopy (vec3_origin, ed->v.angles);
	ed->v.nextthink = -1;
	ed->v.solid = 0;
	
	ed->freetime = sv.time;
}

//===========================================================================

/*
============
ED_GlobalAtOfs
============
*/
ddef_t *ED_GlobalAtOfs (int ofs)
{
	ddef_t		*def;
	int			i;
	
	for (i=0 ; i<progs->numglobaldefs ; i++)
	{
		def = &pr_globaldefs[i];
		if (def->ofs == ofs)
			return def;
	}
	return NULL;
}

/*
============
ED_FieldAtOfs
============
*/
ddef_t *ED_FieldAtOfs (int ofs)
{
	ddef_t		*def;
	int			i;
	
	for (i=0 ; i<progs->numfielddefs ; i++)
	{
		def = &pr_fielddefs[i];
		if (def->ofs == ofs)
			return def;
	}
	return NULL;
}

/*
============
ED_FindField
============
*/
ddef_t *ED_FindField (const quake::string_view& name)
{
	ddef_t		*def;
	int			i;
	
	for (i=0 ; i<progs->numfielddefs ; i++)
	{
		def = &pr_fielddefs[i];
		if (name == (pr_strings + def->s_name))
			return def;
	}
	return NULL;
}


/*
============
ED_FindGlobal
============
*/
ddef_t *ED_FindGlobal (const quake::string_view& name)
{
	ddef_t		*def;
	int			i;
	
	for (i=0 ; i<progs->numglobaldefs ; i++)
	{
		def = &pr_globaldefs[i];
		if (name == (pr_strings + def->s_name) )
			return def;
	}
	return NULL;
}


/*
============
ED_FindFunction
============
*/
dfunction_t *ED_FindFunction (const quake::string_view& name)
{
	dfunction_t		*func;
	int				i;
	
	for (i=0 ; i<progs->numfunctions ; i++)
	{
		func = &pr_functions[i];
		if (name == (pr_strings + func->s_name) )
			return func;
	}
	return NULL;
}


eval_t *GetEdictFieldValue(edict_t *ed, char *field)
{
	ddef_t			*def = NULL;
	int				i;
	static int		rep = 0;

	for (i=0 ; i<GEFV_CACHESIZE ; i++)
	{
		if (!strcmp(field, gefvCache[i].field))
		{
			def = gefvCache[i].pcache;
			goto Done;
		}
	}

	def = ED_FindField (field);

	if (strlen(field) < MAX_FIELD_LEN)
	{
		gefvCache[rep].pcache = def;
		strcpy (gefvCache[rep].field, field);
		rep ^= 1;
	}

Done:
	if (!def)
		return NULL;

	return (eval_t *)((char *)&ed->v + def->ofs*4);
}


/*
============
PR_ValueString

Returns a string describing *data in a type specific manner
=============
*/
quake::cstring  PR_ValueString (etype_t type, eval_t *val)
{
	static quake::stack_string<256> line;
	ddef_t		*def;
	dfunction_t	*f;
	
	type =  static_cast<etype_t>(static_cast<unsigned int>(type) & ~DEF_SAVEGLOBAL);
	
	switch (type)
	{
	case ev_string:
		line = pr_strings + val->string;
		break;
	case ev_entity:	
		line.clear();
		line << "entity " << NUM_FOR_EDICT(PROG_TO_EDICT(val->edict));
		break;
	case ev_function:
		f = &pr_functions[val->function];
		line.clear();
		line << pr_strings + f->s_name << "()";
		break;
	case ev_field:
		def = ED_FieldAtOfs ( val->_int );
		line.clear();
		line << '.' << pr_strings + def->s_name;
		break;
	case ev_void:
		line = "void";
		break;
	case ev_float:
		line.assign_print( "%5.1f", val->_float);
		break;
	case ev_vector:
		line.assign_print("'%5.1f %5.1f %5.1f'", val->vector[0], val->vector[1], val->vector[2]);
		break;
	case ev_pointer:
		line="pointer";
		break;
	default:
		line= "bad type %i", type;
		break;
	}
	
	return line;
}

/*
============
PR_UglyValueString

Returns a string describing *data in a type specific manner
Easier to parse than PR_ValueString
=============
*/
quake::cstring  PR_UglyValueString (etype_t type, eval_t *val)
{
	static quake::stack_string<256> line;
	ddef_t		*def;
	dfunction_t	*f;
	
	type = static_cast<etype_t>(static_cast<unsigned int>(type) & ~DEF_SAVEGLOBAL);

	switch (type)
	{
	case ev_string:
		line = pr_strings + val->string;
		break;
	case ev_entity:	
		line.clear();
		line << NUM_FOR_EDICT(PROG_TO_EDICT(val->edict));
		break;
	case ev_function:
		f = &pr_functions[val->function]  ;
		line =  pr_strings + f->s_name;
		break;
	case ev_field:
		def = ED_FieldAtOfs ( val->_int );
		line =   pr_strings + def->s_name;
		break;
	case ev_void:
		line = "void";
		break;
	case ev_float:
		line.assign_print("%f", val->_float);
		break;
	case ev_vector:
		line.assign_print("%f %f %f", val->vector[0], val->vector[1], val->vector[2]);
		break;
	default:
		line.assign_print("bad type %i", type);
		break;
	}
	
	return line;
}

/*
============
PR_GlobalString

Returns a string with a description and the contents of a global,
padded to 20 field width
============
*/
quake::cstring  PR_GlobalString (int ofs)
{
	int		i;
	ddef_t	*def;
	void	*val;
	static quake::stack_string<128> line;
	
	val = (void *)&pr_globals[ofs];
	def = ED_GlobalAtOfs(ofs);
	if (!def)
		line << ofs << "(???)";
	else
	{
		auto s = PR_ValueString (def->type(), (eval_t*)val);
		line << ofs << '(' << pr_strings + def->s_name << ')' << s;
	}
	if (line.size() < 20) line.append(' ', 20 - line.size());
	line.push_back(' ');

		
	return line;
}

quake::cstring PR_GlobalStringNoContents (int ofs)
{
	int		i;
	ddef_t	*def;
	static quake::stack_string<128> line;
	
	def = ED_GlobalAtOfs(ofs);
	if (!def)
		line << ofs << "(???)";
	else

		line << ofs << '(' << pr_strings + def->s_name << ')';

	if (line.size() < 20) line.append(' ', 20 - line.size());
	line.push_back(' ');
		
	return line;
}


/*
=============
ED_Print

For debugging
=============
*/
void ED_Print (edict_t *ed)
{
	int		l;
	ddef_t	*d;
	int		*v;
	int		i, j;
	char	*name;
	int		type;

	if (ed->free)
	{
		Con_Printf ("FREE\n");
		return;
	}

	Con_Printf("\nEDICT %i:\n", NUM_FOR_EDICT(ed));
	for (i=1 ; i<progs->numfielddefs ; i++)
	{
		d = &pr_fielddefs[i];
		name = pr_strings + d->s_name;
		if (name[strlen(name)-2] == '_')
			continue;	// skip _x, _y, _z vars
			
		v = (int *)((char *)&ed->v + d->ofs*4);

	// if the value is still all 0, skip the field
		type = d->itype & ~DEF_SAVEGLOBAL;
		
		for (j=0 ; j<type_size[type] ; j++)
			if (v[j])
				break;
		if (j == type_size[type])
			continue;
	
		Con_Printf ("%s",name);
		l = strlen (name);
		while (l++ < 15)
			Con_Printf (" ");

		Con_Printf ("%s\n", PR_ValueString(d->type(), (eval_t *)v).c_str());		
	}
}

/*
=============
ED_Write

For savegames
=============
*/
void ED_Write (FILE *f, edict_t *ed)
{
	ddef_t	*d;
	int		*v;
	int		i, j;
	char	*name;
	int		type;

	fprintf (f, "{\n");

	if (ed->free)
	{
		fprintf (f, "}\n");
		return;
	}
	
	for (i=1 ; i<progs->numfielddefs ; i++)
	{
		d = &pr_fielddefs[i];
		name = pr_strings + d->s_name;
		if (name[strlen(name)-2] == '_')
			continue;	// skip _x, _y, _z vars
			
		v = (int *)((char *)&ed->v + d->ofs*4);

	// if the value is still all 0, skip the field
		type = d->itype & ~DEF_SAVEGLOBAL;
		for (j=0 ; j<type_size[type] ; j++)
			if (v[j])
				break;
		if (j == type_size[type])
			continue;
	
		fprintf (f,"\"%s\" ",name);
		fprintf (f,"\"%s\"\n", PR_UglyValueString(d->type(), (eval_t *)v).c_str());		
	}

	fprintf (f, "}\n");
}

void ED_PrintNum (int ent)
{
	ED_Print (EDICT_NUM(ent));
}

/*
=============
ED_PrintEdicts

For debugging, prints all the entities in the current server
=============
*/
void ED_PrintEdicts (void)
{
	int		i;
	
	Con_Printf ("%i entities\n", sv.num_edicts);
	for (i=0 ; i<sv.num_edicts ; i++)
		ED_PrintNum (i);
}

/*
=============
ED_PrintEdict_f

For debugging, prints a single edicy
=============
*/
void ED_PrintEdict_f (void)
{
	int		i;
	
	i = quake::stoi(Cmd_Argv(1));
	if (i >= sv.num_edicts)
	{
		Con_Printf("Bad edict number\n");
		return;
	}
	ED_PrintNum (i);
}

/*
=============
ED_Count

For debugging
=============
*/
void ED_Count (void)
{
	int		i;
	edict_t	*ent;
	int		active, models, solid, step;

	active = models = solid = step = 0;
	for (i=0 ; i<sv.num_edicts ; i++)
	{
		ent = EDICT_NUM(i);
		if (ent->free)
			continue;
		active++;
		if (ent->v.solid)
			solid++;
		if (ent->v.model)
			models++;
		if (ent->v.movetype == MOVETYPE_STEP)
			step++;
	}

	Con_Printf ("num_edicts:%3i\n", sv.num_edicts);
	Con_Printf ("active    :%3i\n", active);
	Con_Printf ("view      :%3i\n", models);
	Con_Printf ("touch     :%3i\n", solid);
	Con_Printf ("step      :%3i\n", step);

}

/*
==============================================================================

					ARCHIVING GLOBALS

FIXME: need to tag constants, doesn't really work
==============================================================================
*/

/*
=============
ED_WriteGlobals
=============
*/
void ED_WriteGlobals (FILE *f)
{
	ddef_t		*def;
	int			i;
	char		*name;
	int			type;

	fprintf (f,"{\n");
	for (i=0 ; i<progs->numglobaldefs ; i++)
	{
		def = &pr_globaldefs[i];
		type = def->type();
		if ( !(def->type() & DEF_SAVEGLOBAL) )
			continue;
		type &= ~DEF_SAVEGLOBAL;

		if (type != ev_string
		&& type != ev_float
		&& type != ev_entity)
			continue;

		name = pr_strings + def->s_name;		
		fprintf (f,"\"%s\" ", name);
		fprintf (f,"\"%s\"\n", PR_UglyValueString(static_cast<etype_t>(type), (eval_t *)&pr_globals[def->ofs]));		
	}
	fprintf (f,"}\n");
}

/*
=============
ED_ParseGlobals
=============
*/
void ED_ParseGlobals (const quake::string_view& text)
{
	quake::string_view data;
	ddef_t	*key;

	while (1)
	{	
	// parse key
		data = COM_Parse (data);
		if (com_token[0] == '}')
			break;
		if (data.empty())
			Sys_Error ("ED_ParseEntity: EOF without closing brace");

		auto keyname = com_token;;

	// parse value	
		data = COM_Parse (data);
		if (data.empty())
			Sys_Error ("ED_ParseEntity: EOF without closing brace");

		if (com_token[0] == '}')
			Sys_Error ("ED_ParseEntity: closing brace without data");

		key = ED_FindGlobal (keyname);
		if (!key)
		{
			Con_Printf ("'%s' is not a global\n", keyname);
			continue;
		}

		if (!ED_ParseEpair ((void *)&pr_globals[0], key, com_token))
			Host_Error ("ED_ParseGlobals: parse error");
	}
}

//============================================================================


/*
=============
ED_NewString
=============
*/
quake::cstring ED_NewString(const quake::string_view& string)
{
	char	*new_ptr, *new_p;
	int		i,l;
	
	l = string.size() + 1;
	new_ptr = (char*)Hunk_Alloc (l);
	new_p = new_ptr;

	for (i=0 ; i< l ; i++)
	{
		if (string[i] == '\\' && i < l-1)
		{
			i++;
			if (string[i] == 'n')
				*new_p++ = '\n';
			else
				*new_p++ = '\\';
		}
		else
			*new_p++ = string[i];
	}
	
	return quake::cstring(new_ptr);
}


/*
=============
ED_ParseEval

Can parse either fields or globals
returns false if error
=============
*/
qboolean	ED_ParseEpair (void *base, ddef_t *key, const quake::string_view& s)
{
	int		i;
	quake::stack_string<128> string;
	ddef_t	*def;
	const char	*v, *w;
	void	*d;
	dfunction_t	*func;
	
	d = (void *)((int *)base + key->ofs);
	
	switch (key->type() & ~DEF_SAVEGLOBAL)
	{
	case ev_string:
		*(string_t *)d = ED_NewString (s).c_str() - pr_strings;
		break;
		
	case ev_float:
		*(float *)d = Q_atof (s);
		break;
		
	case ev_vector:
		for (i = 0; i < 3; i++)
		{
			char* end;
			float f = ::strtof(s.data(), &end);
			if (end != s.data()) {
				((float *)d)[i] = f;

			}
			else {
				assert(0); // bad value
			}
		}
		break;
		
	case ev_entity:
		*(int *)d = EDICT_TO_PROG(EDICT_NUM(Q_atoi (s)));
		break;
		
	case ev_field:
		def = ED_FindField (s);
		if (!def)
		{
			Con_Printf ("Can't find field %s\n", s);
			return false;
		}
		*(int *)d = G_INT(def->ofs);
		break;
	
	case ev_function:
		func = ED_FindFunction (s);
		if (!func)
		{
			Con_Printf ("Can't find function %s\n", s);
			return false;
		}
		*(func_t *)d = pr_functions.indexof(func);
		break;
		
	default:
		break;
	}
	return true;
}

/*
====================
ED_ParseEdict

Parses an edict out of the given string, returning the new position
ed should be a properly initialized empty edict.
Used for initial level load and for savegames.
====================
*/
quake::string_view ED_ParseEdict(quake::string_view data, edict_t *ent)
{
	ddef_t		*key;
	qboolean	anglehack;
	qboolean	init;
	quake::stack_string<256> keyname;
	int			n;

	init = false;

	// clear it
	if (ent != sv.edicts)	// hack
		memset(&ent->v, 0, progs->entityfields * 4);

	// go through all the dictionary pairs
	while (1)
	{
		// parse key
		data = COM_Parse(data);
		if (com_token[0] == '}')
			break;
		if (data.empty())
			Sys_Error("ED_ParseEntity: EOF without closing brace");

		// anglehack is to allow QuakeEd to write single scalar angles
		// and allow them to be turned into vectors. (FIXME...)
		if (com_token == "angle")
		{
			com_token = "angles";
			anglehack = true;
		}
		else
			anglehack = false;

		// FIXME: change light to _light to get rid of this hack
		if (com_token == "light")
			com_token = "light_lev";	// hack for single light def
									// another hack to fix heynames with trailing spaces
		keyname = com_token;
		while (keyname.back() == ' ') keyname.pop_back();



		// parse value	
		data = COM_Parse(data);
		if (data.empty())
			Sys_Error("ED_ParseEntity: EOF without closing brace");

		if (com_token[0] == '}')
			Sys_Error("ED_ParseEntity: closing brace without data");

		init = true;

		// keynames with a leading underscore are used for utility comments,
		// and are immediately discarded by quake
		if (keyname[0] == '_')
			continue;

		key = ED_FindField(keyname);
		if (!key)
		{
			Con_Printf("'%s' is not a field\n", keyname.c_str());
			continue;
		}

		if (anglehack)
		{
			static quake::stack_string<32> temp;
			temp.clear();
			temp << "0 " << com_token << " 0";
			com_token = temp;
		}

		if (!ED_ParseEpair((void *)&ent->v, key, com_token))
			Host_Error("ED_ParseEdict: parse error");
	}

	if (!init)
		ent->free = true;

	return data;
}


/*
================
ED_LoadFromFile

The entities are directly placed in the array, rather than allocated with
ED_Alloc, because otherwise an error loading the map would have entity
number references out of order.

Creates a server's entity / program execution context by
parsing textual entity definitions out of an ent file.

Used for both fresh maps and savegame loads.  A fresh map would also need
to call ED_CallSpawnFunctions () to let the objects initialize themselves.
================
*/
void ED_LoadFromFile (quake::string_view data)
{	
	edict_t		*ent;
	int			inhibit;
	dfunction_t	*func;
	
	ent = NULL;
	inhibit = 0;
	pr_global_struct->time = sv.time;
	
// parse ents
	while (1)
	{
// parse the opening brace	
		data = COM_Parse (data);
		if (data.empty())
			break;
		if (com_token[0] != '{')
			Sys_Error ("ED_LoadFromFile: found %s when expecting {",com_token);

		if (!ent)
			ent = EDICT_NUM(0);
		else
			ent = ED_Alloc ();
		data = ED_ParseEdict (data, ent);

// remove things from different skill levels or deathmatch
		if (deathmatch.value)
		{
			if (((int)ent->v.spawnflags & SPAWNFLAG_NOT_DEATHMATCH))
			{
				ED_Free (ent);	
				inhibit++;
				continue;
			}
		}
		else if ((current_skill == 0 && ((int)ent->v.spawnflags & SPAWNFLAG_NOT_EASY))
				|| (current_skill == 1 && ((int)ent->v.spawnflags & SPAWNFLAG_NOT_MEDIUM))
				|| (current_skill >= 2 && ((int)ent->v.spawnflags & SPAWNFLAG_NOT_HARD)) )
		{
			ED_Free (ent);	
			inhibit++;
			continue;
		}

//
// immediately call spawn function
//
		if (!ent->v.classname)
		{
			Con_Printf ("No classname for:\n");
			ED_Print (ent);
			ED_Free (ent);
			continue;
		}

	// look for the spawn function
		func = ED_FindFunction ( pr_strings + ent->v.classname );

		if (!func)
		{
			Con_Printf ("No spawn function for:\n");
			ED_Print (ent);
			ED_Free (ent);
			continue;
		}

		pr_global_struct->self = EDICT_TO_PROG(ent);
		PR_ExecuteProgram ( pr_functions.indexof(func));
	}	

	Con_DPrintf ("%i entities inhibited\n", inhibit);
}


/*
===============
PR_LoadProgs
===============
*/
void PR_LoadProgs (void)
{
	int		i;

// flush the non-C variable lookup cache
	for (i=0 ; i<GEFV_CACHESIZE ; i++)
		gefvCache[i].field[0] = 0;

	CRC_Init (&pr_crc);
	char* raw_progs = (char*)COM_LoadHunkFile("progs.dat");
	progs = (dprograms_t *)raw_progs;
	if (!progs)
		Sys_Error ("PR_LoadProgs: couldn't load progs.dat");
	Con_DPrintf ("Programs occupy %iK.\n", com_filesize/1024);

	for (i=0 ; i<com_filesize ; i++)
		CRC_ProcessByte (&pr_crc, ((byte *)progs)[i]);

// byte swap the header
	for (i=0 ; i<sizeof(*progs)/4 ; i++)
		((int *)progs)[i] = LittleLong ( ((int *)progs)[i] );		

	if (progs->version != PROG_VERSION)
		Sys_Error ("progs.dat has wrong version number (%i should be %i)", progs->version, PROG_VERSION);
	if (progs->crc != PROGHEADER_CRC)
		Sys_Error ("progs.dat system vars have been modified, progdefs.h is out of date");

	pr_functions.manage(raw_progs + progs->ofs_functions, progs->numfunctions);
	pr_strings = (char *)progs + progs->ofs_strings;
	pr_globaldefs.manage(raw_progs + progs->ofs_globaldefs, progs->numglobaldefs);
	pr_fielddefs.manage(raw_progs + progs->ofs_fielddefs, progs->numfielddefs);
	pr_statements.manage(raw_progs + progs->ofs_statements, progs->numstatements);

	pr_global_struct = (globalvars_t *)((byte *)progs + progs->ofs_globals);
	pr_globals.manage(pr_global_struct, progs->numglobals);
	
	pr_edict_size = progs->entityfields * 4 + sizeof (edict_t) - sizeof(entvars_t);
	
// byte swap the lumps
	for (i=0 ; i<progs->numstatements ; i++)
	{
		pr_statements[i].op = LittleShort(pr_statements[i].op);
		pr_statements[i].a = LittleShort(pr_statements[i].a);
		pr_statements[i].b = LittleShort(pr_statements[i].b);
		pr_statements[i].c = LittleShort(pr_statements[i].c);
	}

	for (i = 0; i < progs->numfunctions; i++)
	{
		pr_functions[i].first_statement = LittleLong(pr_functions[i].first_statement);
		pr_functions[i].parm_start = LittleLong(pr_functions[i].parm_start);
		pr_functions[i].s_name = LittleLong(pr_functions[i].s_name);
		pr_functions[i].s_file = LittleLong(pr_functions[i].s_file);
		pr_functions[i].numparms = LittleLong(pr_functions[i].numparms);
		pr_functions[i].locals = LittleLong(pr_functions[i].locals);
	}

	for (i=0 ; i<progs->numglobaldefs ; i++)
	{
		pr_globaldefs[i].itype = LittleShort (static_cast<short>(pr_globaldefs[i].itype));
		pr_globaldefs[i].ofs = LittleShort (pr_globaldefs[i].ofs);
		pr_globaldefs[i].s_name = LittleLong (pr_globaldefs[i].s_name);
	}

	for (i=0 ; i<progs->numfielddefs ; i++)
	{
		pr_fielddefs[i].itype = LittleShort(static_cast<short>(pr_fielddefs[i].itype)); 
		if (pr_fielddefs[i].itype & DEF_SAVEGLOBAL)
			Sys_Error ("PR_LoadProgs: pr_fielddefs[i].type & DEF_SAVEGLOBAL");
		pr_fielddefs[i].ofs = LittleShort (pr_fielddefs[i].ofs);
		pr_fielddefs[i].s_name = LittleLong (pr_fielddefs[i].s_name);
	}

	for (i = 0; i < progs->numglobals; i++) {
		int* p = (int *)&pr_globals[i];
		*p = LittleLong(*p);
	}

}


/*
===============
PR_Init
===============
*/
void PR_Init (void)
{
	Cmd_AddCommand ("edict", ED_PrintEdict_f);
	Cmd_AddCommand ("edicts", ED_PrintEdicts);
	Cmd_AddCommand ("edictcount", ED_Count);
	Cmd_AddCommand ("profile", PR_Profile_f);
	Cvar_RegisterVariable (&nomonsters);
	Cvar_RegisterVariable (&gamecfg);
	Cvar_RegisterVariable (&scratch1);
	Cvar_RegisterVariable (&scratch2);
	Cvar_RegisterVariable (&scratch3);
	Cvar_RegisterVariable (&scratch4);
	Cvar_RegisterVariable (&savedgamecfg);
	Cvar_RegisterVariable (&saved1);
	Cvar_RegisterVariable (&saved2);
	Cvar_RegisterVariable (&saved3);
	Cvar_RegisterVariable (&saved4);
}



edict_t *EDICT_NUM(int n)
{
	if (n < 0 || n >= sv.max_edicts)
		Sys_Error ("EDICT_NUM: bad number %i", n);
	return (edict_t *)((byte *)sv.edicts+ (n)*pr_edict_size);
}

int NUM_FOR_EDICT(edict_t *e)
{
	int		b;
	
	b = (byte *)e - (byte *)sv.edicts;
	b = b / pr_edict_size;
	
	if (b < 0 || b >= sv.num_edicts)
		Sys_Error ("NUM_FOR_EDICT: bad pointer");
	return b;
}
