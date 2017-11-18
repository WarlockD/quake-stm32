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

/*
	Some side notes on the original Quake.  I havn't looked at Quake2 yet but they do ALOT of string copies.  First its copyied to com_token, THEN to another buff
	THEN another buffer for vector_t for the Q_atof.  Thats just WAY to much because ware are directly moving it to the dictionary
	So I made a class c alled quake::string_view that contains the raw data from the file (poisiton size) for a token with inbuilt "to float" function so we don't have to do all
	that crazy copying.

	Just watch it as if the file is unloaded all the quake::string_view's suddenly become invalid
*/
#include "icommon.h"

using namespace std::chrono;

dprograms_t		*progs;
dfunction_t		*pr_functions;
char			*pr_strings;
ddef_t			*pr_fielddefs;
ddef_t			*pr_globaldefs;
dstatement_t	*pr_statements;
globalvars_t	*pr_global_struct;
float			*pr_globals;			// same as pr_global_struct
int				pr_edict_size;	// in bytes

unsigned short		pr_crc;



ddef_t *ED_FieldAtOfs (int ofs);
qboolean	ED_ParseEpair (void *base, ddef_t *key, const quake::string_view& value);

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
void edict_t::Clear ()
{
	memset (&v, 0, progs->entityfields * 4);
	free = false;
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
		if (e->free && ( e->freetime < 2s || sv.time - e->freetime > 500ms ) )
		{
			e->Clear();
			return e;
		}
	}
	
	if (i == MAX_EDICTS)
		Sys_Error ("ED_Alloc: no free edicts");
		
	sv.num_edicts++;
	e = EDICT_NUM(i);
	e->Clear();

	return e;
}

/*
=================
ED_Free

Marks the edict as free
FIXME: walk all entities and NULL out references to this entity
=================
*/
void edict_t::Free ()
{
	using namespace std::chrono;

	Unlink();		// unlink from world bsp

	free = true;
	v.model = 0;
	v.takedamage = 0;
	v.modelindex = 0;
	v.colormap = 0;
	v.skin = 0;
	v.frame = 0;
	VectorCopy (vec3_origin, v.origin);
	VectorCopy (vec3_origin, v.angles);
	v.nextthink = -1s;
	v.solid = 0;

	freetime = sv.time;
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
	for (int i =0 ; i<progs->numfielddefs ; i++)
	{
		def = &pr_fielddefs[i];
		const char* def_name = pr_strings + def->s_name;
		if (name == def_name) return def;
	}
	return nullptr;
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
		const char* def_name = pr_strings + def->s_name;
		if (name == def_name) return def;
	}
	return nullptr;
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
		const char* func_name = pr_strings + func->s_name;
		if (name == func_name) return func;
	}
	return nullptr;
}


eval_t *GetEdictFieldValue(edict_t *ed, const quake::string_view& field)
{
	ddef_t			*def = NULL;
	int				i;
	static int		rep = 0;

	for (i=0 ; i<GEFV_CACHESIZE ; i++)
	{
		if (field ==  gefvCache[i].field)
		{
			def = gefvCache[i].pcache;
			goto Done;
		}
	}

	def = ED_FindField (field);

	assert(field.size() < MAX_FIELD_LEN);
	if (field.size() < MAX_FIELD_LEN)
	{
		gefvCache[rep].pcache = def;
		field.copy(gefvCache[rep].field,sizeof(gefvCache[rep].field));
		rep ^= 1;
	}

Done:
	if (!def)
		return nullptr;

	return (eval_t *)((char *)&ed->v + def->ofs*4);
}



/*
============
PR_UglyValueString

Returns a string describing *data in a type specific manner
Easier to parse than PR_ValueString
=============
*/
class PR_UglyValueString : public quake::stream_output {
public:
	PR_UglyValueString(idType type, eval_t *val) : type(type), val(val) {}
	void text_output(std::ostream& os) const override final {
		ddef_t		*def;
		dfunction_t	*f;

		switch (type)
		{
		case etype_t::ev_string:
			os << (const char*)(pr_strings + val->string);
			break;
		case etype_t::ev_entity:
			os << (int)(NUM_FOR_EDICT(PROG_TO_EDICT(val->edict)));
			break;
		case etype_t::ev_function:
		{
			dfunction_t	*f = pr_functions + val->function;
			os << (const char*)(pr_strings + f->s_name);
		}
		break;
		case etype_t::ev_field:
		{
			ddef_t		*def = ED_FieldAtOfs(val->_int);
			os << (const char*)(pr_strings + def->s_name);
		}
		break;
		case etype_t::ev_void:
			os << "void";
			break;
		case etype_t::ev_float:
			os << (float)val->_float;
			break;
		case etype_t::ev_vector:
			os << (float)val->vector[0] << ' ' << (float)val->vector[1] << ' ' << (float)val->vector[2];
			break;
		default:
			os << "bad type " << (int)(etype_t)type;
			break;
		}
	}
private:
	idType type;
	eval_t *val;
};




/*
=============
ED_Print

For debugging
=============
*/

void edict_t::Print()
{
	ddef_t	*d;
	int		*v;
	char	*name;
	int j;
	auto ed = this;
	if (ed->free)
	{
		quake::con << "FREE" << std::endl;
		return;
	}
	quake::con << std::endl;
	quake::con << "EDICT " << NUM_FOR_EDICT(ed) << ':' << std::endl;
	for (int i=1 ; i<progs->numfielddefs ; i++)
	{
		d = &pr_fielddefs[i];
		name = pr_strings + d->s_name;
		if (name[strlen(name)-2] == '_')
			continue;	// skip _x, _y, _z vars
			
		v = (int *)((char *)&ed->v + d->ofs*4);
		eval_t* eval = (eval_t *)v;
	// if the value is still all 0, skip the field
		idType type = d->type;
		size_t type_size = type.size();

		for (j=0 ; j<type_size; j++)
			if (v[j])
				break;
		if (j == type_size)
			continue;
	
		quake::con << std::left << std::setw(15) << name << quake::PR_ValueString(d->type, (eval_t *)v) << std::endl;	
	}
}

/*
=============
ED_Write

For savegames
=============
*/
void ED_Write (std::ostream& f, edict_t *ed)
{
	ddef_t	*d;
	int		*v;
	size_t j;
	char	*name;
	idType	type;

	f << '{' << std::endl;
	if (!ed->free)
	{
		for (size_t i = 1; i < (size_t)progs->numfielddefs; i++)
		{
			d = &pr_fielddefs[i];
			name = pr_strings + d->s_name;
			if (name[strlen(name) - 2] == '_')
				continue;	// skip _x, _y, _z vars

			v = (int *)((char *)&ed->v + d->ofs * 4);

			// if the value is still all 0, skip the field

			type = d->type;
			for (j = 0; j < type.size(); j++)
				if (v[j])
					break;
			if (j == type.size())
				continue;
			f << '"' << name << "\" \"" << PR_UglyValueString(d->type, (eval_t *)v) << '"' << std::endl;
		}
	}
	f << ')' << std::endl;
}

void ED_PrintNum (int ent)
{
	EDICT_NUM(ent)->Print ();
}

/*
=============
ED_PrintEdicts

For debugging, prints all the entities in the current server
=============
*/
void ED_PrintEdicts()
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
void ED_PrintEdict_f(cmd_source_t source, size_t argc, const quake::string_view argv[])
{
	int		i;
	
	i = Q_atoi (argv[1]);
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
void ED_Count(cmd_source_t source, size_t argc, const quake::string_view argv[])
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
void ED_WriteGlobals (std::ostream& f)
{
	ddef_t		*def;
	idType		type;
	f << '{' << std::endl;
	for (int i=0 ; i<progs->numglobaldefs ; i++)
	{
		def = &pr_globaldefs[i];
		type = def->type;
		if ( !type.saveglobal())
			continue;
		switch (type) {
		case etype_t::ev_string:
		case etype_t::ev_float:
		case etype_t::ev_entity:
			const char* name = pr_strings + def->s_name;
			f << '"' << name << "\" \"" << PR_UglyValueString(type, (eval_t *)&pr_globals[def->ofs]) << '"' << std::endl;
		}
		break;
	}
	f << '}' << std::endl;
}

/*
=============
ED_ParseGlobals
=============
*/
void ED_ParseGlobals (COM_Parser& parser)
{
	while (1)
	{	
		auto keyname = parser.Next();
		if (keyname.empty())
			Sys_Error ("ED_ParseEntity: EOF without closing brace");

		if (keyname[0]  == '}')
			break;

		auto value = parser.Next();
		if (value.empty())
			Sys_Error("ED_ParseEntity: closing brace without data");

		if (value[0] == '}')
			Sys_Error("ED_ParseEntity: EOF without closing brace");


		ddef_t	* key = ED_FindGlobal (keyname);
		if (!key)
		{
			quake::con << '"' << keyname << "\" is not global" << std::endl;
			continue;
		}

		if (!ED_ParseEpair ((void *)pr_globals, key, value))
			Host_Error ("ED_ParseGlobals: parse error");
	}
}

//============================================================================


/*
=============
ED_NewString
=============
*/
char *ED_NewString (const quake::string_view& string)
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
	
	return new_ptr;
}


/*
=============
ED_ParseEval

Can parse either fields or globals
returns false if error
=============
*/


qboolean	ED_ParseEpair(void *base, ddef_t *key, const quake::string_view& value)
{
	float*  f;
	int		i;
	char	string[128];
	ddef_t	*def;
	char	*v, *w;
	void	*d;
	dfunction_t	*func;

	d = (void *)((int *)base + key->ofs);
	idType type = key->type;
	switch (type)
	{
	case etype_t::ev_string:
		*(string_t *)d = ED_NewString(value) - pr_strings;
		break;

	case etype_t::ev_float:
		f = (float*)d;
		assert(quake::to_number(value, *f));
		break;

	case etype_t::ev_vector:
	{
		COM_Parser parser(value);
		assert(quake::to_number(parser.Next(), f[0]));
		assert(quake::to_number(parser.Next(), f[1]));
		assert(quake::to_number(parser.Next(), f[2]));
	}
	break;

	case etype_t::ev_entity:
		assert(quake::to_number(value,i));
		*(int *)d = EDICT_TO_PROG(EDICT_NUM(i));
		break;

	case etype_t::ev_field:
		def = ED_FindField(value);
		if (!def)
		{
			quake::con << "Can't find field " << value << std::endl;
			return false;
		}
		*(int *)d = G_INT(def->ofs);
		break;

	case etype_t::ev_function:
		func = ED_FindFunction(value);
		if (!func)
		{
			quake::con << "Can't find function " << value << std::endl;
			return false;
		}
		*(func_t *)d = func - pr_functions;
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
void ED_ParseEdict(COM_Parser& parser, edict_t *ent)
{
	ddef_t		*key;
	qboolean	anglehack;
	qboolean	init;
	
	int			n;

	init = false;
	//COM_Parser parser(data);

	// clear it
	if (ent != sv.edicts)	// hack
		memset(&ent->v, 0, progs->entityfields * 4);

	// go through all the dictionary pairs
	while (1)
	{
		auto keyname = parser.Next();
		if (keyname.empty())
			Sys_Error("ED_ParseEntity: EOF without closing brace");

		if (keyname[0] == '}') break;
		// anglehack is to allow QuakeEd to write single scalar angles
		// and allow them to be turned into vectors. (FIXME...)
		anglehack = false;

		if (keyname == "angles")
		{
			keyname =  "angles";
			anglehack = true;
		}
		// FIXME: change light to _light to get rid of this hack
		else if (keyname == "light") {
			keyname = "light_lev"; // hack for single light def
		}
		// another hack to fix heynames with trailing spaces
		// this shouldn't happen with the parser
		assert(keyname.back() != ' ');

		auto value = parser.Next();
		// parse value	
		if (value.empty() || value[0] == '}')
			Sys_Error("ED_ParseEntity: EOF without closing brace");

		init = true;

		// keynames with a leading underscore are used for utility comments,
		// and are immediately discarded by quake
		if (keyname.front() == '_')
			continue;

		key = ED_FindField(keyname);
		if (key == nullptr) {
			quake::con << '\'' << keyname << "' is not a field" << std::endl;
			continue;
		}

		if (anglehack)
		{
			quake::fixed_string_stream<128> buf;

			buf << "0 " << keyname << " 0";
			if (!ED_ParseEpair((void *)&ent->v, key, buf.str()))
				Host_Error("ED_ParseEdict: parse error");
		}
		else {
			if (!ED_ParseEpair((void *)&ent->v, key, value))
				Host_Error("ED_ParseEdict: parse error");
		}


	}

	if (!init)
		ent->free = true;
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
void ED_LoadFromFile (const quake::string_view& data)
{	
	edict_t		*ent;
	int			inhibit;
	dfunction_t	*func;
	
	ent = NULL;
	inhibit = 0;
	pr_global_struct->time = sv.time;
	COM_Parser parser(data);

// parse ents
	while (1)
	{
// parse the opening brace	
		auto token = parser.Next();
		if (token.empty()) break;

		if (token[0] != '{') {
			Sys_Error("ED_LoadFromFile:  expecting {");
		}

		if (!ent)
			ent = EDICT_NUM(0);
		else
			ent = ED_Alloc ();
		ED_ParseEdict (parser, ent);

// remove things from different skill levels or deathmatch
		if (deathmatch.value)
		{
			if (((int)ent->v.spawnflags & SPAWNFLAG_NOT_DEATHMATCH))
			{
				ent->Free();
				inhibit++;
				continue;
			}
		}
		else if ((current_skill == 0 && ((int)ent->v.spawnflags & SPAWNFLAG_NOT_EASY))
				|| (current_skill == 1 && ((int)ent->v.spawnflags & SPAWNFLAG_NOT_MEDIUM))
				|| (current_skill >= 2 && ((int)ent->v.spawnflags & SPAWNFLAG_NOT_HARD)) )
		{
			ent->Free();
			inhibit++;
			continue;
		}

//
// immediately call spawn function
//
		if (!ent->v.classname)
		{
			Con_Printf ("No classname for:\n");
			ent->Print ();
			ent->Free ();
			continue;
		}

	// look for the spawn function
		quake::string_view ref(pr_strings + ent->v.classname);
		func = ED_FindFunction (ref);

		if (!func)
		{
			Con_Printf ("No spawn function for:\n");
			ent->Print ();
			ent->Free ();
			continue;
		}

		pr_global_struct->self = EDICT_TO_PROG(ent);
		PR_ExecuteProgram (func - pr_functions);
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

	progs = (dprograms_t *)COM_LoadHunkFile ("progs.dat");
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

	pr_functions = (dfunction_t *)((byte *)progs + progs->ofs_functions);
	pr_strings = (char *)progs + progs->ofs_strings;
	pr_globaldefs = (ddef_t *)((byte *)progs + progs->ofs_globaldefs);
	pr_fielddefs = (ddef_t *)((byte *)progs + progs->ofs_fielddefs);
	pr_statements = (dstatement_t *)((byte *)progs + progs->ofs_statements);

	pr_global_struct = (globalvars_t *)((byte *)progs + progs->ofs_globals);
	pr_globals = (float *)pr_global_struct;
	
	pr_edict_size = progs->entityfields * 4 + sizeof (edict_t) - sizeof(entvars_t);
	
// byte swap the lumps
	for (i=0 ; i<progs->numstatements ; i++)
	{
		pr_statements[i].op = LittleShort(pr_statements[i].op);
		pr_statements[i].a = LittleShort(pr_statements[i].a);
		pr_statements[i].b = LittleShort(pr_statements[i].b);
		pr_statements[i].c = LittleShort(pr_statements[i].c);
	}

	for (i=0 ; i<progs->numfunctions; i++)
	{
	pr_functions[i].first_statement = LittleLong (pr_functions[i].first_statement);
	pr_functions[i].parm_start = LittleLong (pr_functions[i].parm_start);
	pr_functions[i].s_name = LittleLong (pr_functions[i].s_name);
	pr_functions[i].s_file = LittleLong (pr_functions[i].s_file);
	pr_functions[i].numparms = LittleLong (pr_functions[i].numparms);
	pr_functions[i].locals = LittleLong (pr_functions[i].locals);
	}	

	for (i=0 ; i<progs->numglobaldefs ; i++)
	{
		pr_globaldefs[i].type = static_cast<etype_t>(LittleShort (static_cast<uint16_t>(pr_globaldefs[i].type)));
		pr_globaldefs[i].ofs = LittleShort (pr_globaldefs[i].ofs);
		pr_globaldefs[i].s_name = LittleLong (pr_globaldefs[i].s_name);
	}

	for (i=0 ; i<progs->numfielddefs ; i++)
	{
		pr_fielddefs[i].type = static_cast<etype_t>(LittleShort(static_cast<uint16_t>(pr_fielddefs[i].type)));
		if (idType(pr_fielddefs[i].type).saveglobal())   
			Sys_Error ("PR_LoadProgs: pr_fielddefs[i].type & DEF_SAVEGLOBAL");
		pr_fielddefs[i].ofs = LittleShort (pr_fielddefs[i].ofs);
		pr_fielddefs[i].s_name = LittleLong (pr_fielddefs[i].s_name);
	}

	for (i=0 ; i<progs->numglobals ; i++)
		((int *)pr_globals)[i] = LittleLong (((int *)pr_globals)[i]);
}


/*
===============
PR_Init
===============
*/
void PR_Init (void)
{
	Cmd_AddCommand ("edict", ED_PrintEdict_f);
	Cmd_AddCommand ("edicts", (xcommand_t)ED_PrintEdicts);
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
