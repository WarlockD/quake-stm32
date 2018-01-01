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
// cvar.h

/*

cvar_t variables are used to hold scalar or string variables that can be changed or displayed at the console or prog code as well as accessed directly
in C code.

it is sufficient to initialize a cvar_t with just the first two fields, or
you can add a ,true flag for variables that you want saved to the configuration
file when the game is quit:

cvar_t	r_draworder = {"r_draworder","1"};
cvar_t	scr_screensize = {"screensize","1",true};

Cvars must be registered before use, or they will have a 0 value instead of the float interpretation of the string.  Generally, all cvar_t declarations should be registered in the apropriate init function before any console commands are executed:
Cvar_RegisterVariable (&host_framerate);


C code usually just references a cvar in place:
if ( r_draworder.value )

It could optionally ask for the value to be looked up for a string name:
if (Cvar_VariableValue ("r_draworder"))

Interpreted prog code can access cvars with the cvar(name) or
cvar_set (name, value) internal functions:
teamplay = cvar("teamplay");
cvar_set ("registered", "1");

The user can access cvars from the console in two ways:
r_draworder			prints the current value
r_draworder 0		sets the current value to 0
Cvars are restricted from having the same names as commands to keep this
interface from being ambiguous.
*/
template<typename T = void> struct cvar_t;
class cvar_t_base {
protected:
	cvar_t_base(bool archive = false, bool server = false) : archive(archive), server(server) {}
public:
	qboolean archive;		// set to true to cause it to be saved to vars.rc
	qboolean server;		// notifies players when changed
};

template<>
struct cvar_t<float> : public cvar_t_base {
	using value_type = float;
	using refrence_type = value_type&;
	using pointer_type = value_type* ;
	using const_refrence_type = const value_type &;
	using const_pointer_type = const value_type *;
	float value;
	cvar_t(float value, bool archive = false, bool server = false) :  value(value), cvar_t_base(archive, server) {}
};	
using cvar_float = cvar_t<float>;

template<>
struct cvar_t<quake::string> : public cvar_t_base {
	using value_type = cstring_t;
	using refrence_type = value_type & ;
	using pointer_type = value_type * ;
	using const_refrence_type = const value_type &;
	using const_pointer_type = const value_type *;
	cstring_t value;
	cvar_t(cstring_t value, bool archive = false, bool server = false) : value(value), cvar_t_base(archive, server) {}
};
using cvar_string = cvar_t<quake::string>;

void Cvar_RegisterVariable(const quake::string_view& name, cvar_float& variable);
void Cvar_RegisterVariable(const quake::string_view& name, cvar_string& variable);

// registers a cvar that allready has the name, string, and optionally the
// archive elements set.

void 	Cvar_Set (string_t var_name, const quake::string_view& value);
void 	Cvar_Set(string_t var_name, float value);
// equivelant to "<name> <variable>" typed at the console

// expands value to a string and calls Cvar_Set
// returns an empty string if not defined

quake::string_view Cvar_CompleteVariable (const quake::string_view& partial);
// attempts to match a partial variable name for command line completion
// returns NULL if nothing fits

//qboolean Cvar_Command (const StringArgs& args);
// called by Cmd_ExecuteString when Cmd_Argv(0) doesn't match a known
// command.  Returns true if the command was a variable reference that
// was handled. (print or change)

void 	Cvar_WriteVariables (std::ostream& f);
// Writes lines containing "set variable value" for all variables
// with the archive flag set to true.
template<typename T> T* Cvar_Get(string_t var_name);


extern	cvar_t<float>		registered;
//extern cvar_t	*cvar_vars;
