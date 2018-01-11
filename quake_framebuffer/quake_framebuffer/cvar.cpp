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
// cvar.c -- dynamic variable tracking

#include "quakedef.h"

cvar_t	*cvar_vars;
char	*cvar_null_string = "";

/*
============
Cvar_FindVar
============
*/
cvar_t *Cvar_FindVar (const quake::string_view& var_name)
{
	cvar_t	*var;
	
	for (var=cvar_vars ; var ; var=var->next)
		if (var_name == var->name)
			return var;

	return NULL;
}

/*
============
Cvar_VariableValue
============
*/
float	Cvar_VariableValue (const quake::string_view& var_name)
{
	cvar_t	*var;
	
	var = Cvar_FindVar (var_name);
	if (!var)
		return 0;
	return Q_atof (var->string);
}


/*
============
Cvar_VariableString
============
*/
quake::string_view Cvar_VariableString (const quake::string_view& var_name)
{
	cvar_t *var;
	
	var = Cvar_FindVar (var_name);
	if (!var)
		return quake::string_view();
	return var->string;
}


/*
============
Cvar_CompleteVariable
============
*/
quake::string_view Cvar_CompleteVariable (const quake::string_view& partial)
{
	cvar_t		*cvar;
	size_t len = partial.size();
	if (partial.empty())
		return NULL;
		
// check functions
	for (cvar=cvar_vars ; cvar ; cvar=cvar->next)
		if (partial.size() <= cvar->name.size() && partial.compare(0, partial.size(),  cvar->name, 0, partial.size()) == 0)
			return cvar->name;

	return quake::string_view();
}


/*
============
Cvar_Set
============
*/
void Cvar_Set (const quake::string_view& var_name, const quake::string_view& value) {
	cvar_t	*var = Cvar_FindVar (var_name);
	if (!var)
	{	// there is an error in C code if this happens
		Con_Printf ("Cvar_Set: variable %s not found\n", var_name);
		return;
	}

	qboolean changed = value == var->string;
	
	var->string = value;
	var->value = quake::stof(var->string);

	if (var->server && changed)
	{
		if (sv.active)
			SV_BroadcastPrintf ("\"%s\" changed to \"%s\"\n", var->name, var->string);
	}
}

/*
============
Cvar_SetValue
============
*/
void Cvar_SetValue (const quake::string_view& var_name, float value) {
	cvar_t	*var = Cvar_FindVar(var_name);
	if (!var)
	{	// there is an error in C code if this happens
		Con_Printf("Cvar_Set: variable %s not found\n", var_name);
		return;
	}

	qboolean changed = value == var->value;

	var->value = value;
	var->string.clear();
	var->string << value;

	if (var->server && changed)
	{
		if (sv.active)
			SV_BroadcastPrintf("\"%s\" changed to \"%s\"\n", var->name, var->string);
	}
#if 0
	quake::stack_string<32> val;
	val << value;

	Cvar_Set (var_name, val);
#endif
}


/*
============
Cvar_RegisterVariable

Adds a freestanding variable to the variable list.
============
*/
void Cvar_RegisterVariable (cvar_t *variable)
{
	char	*oldstr;
	
// first check to see if it has allready been defined
	if (Cvar_FindVar (variable->name))
	{
		Con_Printf ("Can't register variable %s, allready defined\n", variable->name);
		return;
	}
	
// check for overlap with a command
	if (Cmd_Exists (variable->name))
	{
		Con_Printf ("Cvar_RegisterVariable: %s is a command\n", variable->name);
		return;
	}
		
	// copy the value off, because future sets will Z_Free it
	// we don't have to worry about this untill we get the interning system working
	if (variable->string.size() == 0U) {
		variable->string << variable->value; // get the value from string
	}
	else {
		variable->value = Q_atof(variable->string);
	}
	
// link the variable in
	variable->next = cvar_vars;
	cvar_vars = variable;
}

/*
============
Cvar_Command

Handles variable inspection and changing from the console
============
*/
qboolean	Cvar_Command (void)
{
	cvar_t			*v;

// check variables
	v = Cvar_FindVar (Cmd_Argv(0));
	if (!v)
		return false;
		
// perform a variable print or set
	if (Cmd_Argc() == 1)
	{
		Con_Printf ("\"%s\" is \"%s\"\n", v->name, v->string);
		return true;
	}

	Cvar_Set (v->name, Cmd_Argv(1));
	return true;
}


/*
============
Cvar_WriteVariables

Writes lines containing "set variable value" for all variables
with the archive flag set to true.
============
*/
void Cvar_WriteVariables (FILE *f)
{
	cvar_t	*var;
	
	for (var = cvar_vars ; var ; var = var->next)
		if (var->archive)
			fprintf (f, "%s \"%s\"\n", var->name, var->string);
}

