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


#include "icommon.h"

cvar_t	*cvar_vars;
char	*cvar_null_string = "";

static std::unordered_map<string_t, cvar_t*> c_vars;
/*
============
Cvar_FindVar
============
*/
cvar_t *Cvar_FindVar(string_t var_name)
{
	auto it = c_vars.find(var_name);
	return  (it != c_vars.end()) ? it->second : nullptr;
}

/*
============
Cvar_VariableValue
============
*/
float	Cvar_VariableValue (string_t var_name) {
	cvar_t	*var = Cvar_FindVar(var_name);
	if (!var)
		return std::numeric_limits<float>::quiet_NaN();
	return Q_atof (var->string);
}


/*
============
Cvar_VariableString
============
*/
std::string_view Cvar_VariableString (string_t var_name)
{
	cvar_t *var = Cvar_FindVar (var_name);
	if (!var)
		return std::string_view();
	return var->string;
}


/*
============
Cvar_CompleteVariable
============
*/
std::string_view  Cvar_CompleteVariable (const std::string_view&	partial)
{
	cvar_t		*cvar;
	int			len;
	
	if (partial.empty()) return nullptr;

		
// check functions
	for (cvar=cvar_vars ; cvar ; cvar=cvar->next)
		if (cvar->name == partial)
			return cvar->name;

	return std::string_view();
}


/*
============
Cvar_Set
============
*/
void Cvar_Set (string_t var_name, const std::string_view& value)
{
	cvar_t	*var = Cvar_FindVar(var_name);
	if (!var)
	{	// there is an error in C code if this happens
		quake::con << "Cvar_Set: variable " << var_name << " not found" << std::endl;
		return;
	}
	var->set(value);
}
void cvar_t::set(const std::string_view& value) {
	if (string != value) {
		_string = string_t::intern(value);
		string = _string;
		this->value = Q_atof(string.c_str());
		if (server)
		{
			if (sv.active)
				SV_BroadcastPrintf("\"%s\" changed to \"%s\"\n",this->name.c_str(),_string.c_str());
		}
	}
}
void cvar_t::set(float value) {
	quake::fixed_string_stream<32> val;
	val << value;
	set(val.str().c_str());
}
/*
============
Cvar_SetValue
============
*/
void Cvar_SetValue (string_t  var_name, float value) {
	quake::fixed_string_stream<32> val;
	val << value;
	auto test = val.str();
	Cvar_Set (var_name, val.str().c_str());
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
//	oldstr = variable->string;
//	variable->string = (char*)Z_Malloc (Q_strlen(variable->string)+1);
//	Q_strcpy (variable->string, oldstr);
	variable->value = Q_atof (variable->string);
	c_vars.emplace(variable->name, variable);
	variable->name = string_t::intern(variable->name); // fix it
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

qboolean	Cvar_Command (const StringArgs& args)
{
	
	cvar_t			*v;

// check variables
	v = Cvar_FindVar (args[0]);
	if (!v)
		return false;
		
// perform a variable print or set
	if (args.size() == 1)
	{
		quake::con << "\"" << v->name << "\" is \"" << v->string << "\"" << std::endl;
		return true;
	}


	Cvar_Set (v->name, args[1]);
	return true;
}


/*
============
Cvar_WriteVariables

Writes lines containing "set variable value" for all variables
with the archive flag set to true.
============
*/
void Cvar_WriteVariables (std::ostream& f)
{
	cvar_t	*var;
	
	for (var = cvar_vars; var; var = var->next)
		if (var->archive)
			f << var->name << '\"' << var->string << '\"' << std::endl;
}

