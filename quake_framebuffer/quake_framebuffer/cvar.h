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
#if 0
class cvar_t
{
public:
	enum Type {
		Null =0,
		InitalString,
		String,
		Number,
		Bool
	};
	cvar_t(const char* name, bool archive=false, bool server=false) : _name(name), _type(Type::Null), _inital(nullptr) , _archive(archive), _server(server) {}
	cvar_t(const char* name, float n, bool archive = false, bool server = false) : _name(name), _type(Type::Number), _value(n), _archive(archive), _server(server) {}
	cvar_t(const char* name, bool n, bool archive = false, bool server = false) : _name(name), _type(Type::Bool), _bool(n), _archive(archive), _server(server) {}
	cvar_t(const char* name, const char* s, bool archive = false, bool server = false) : _name(name), _type(Type::InitalString), _inital(s), _archive(archive), _server(server) {}
	cvar_t(const cvar_t& copy) = delete;
	cvar_t& operator=(const cvar_t& copy) = delete;
	inline void clear() { if (_type == Type::String) _string.~basic_string(); _type = Type::Null; }
	cvar_t& operator=(nullptr_t) { if (_type == Type::String) _string.~basic_string(); _type = Type::Null; }
	cvar_t& operator=(const char* v) { 
		if (_type == Type::String)
			_string.assign(v);
		else {
			_type = Type::String;
			_string = std::move(ZString(v));
		}
	}
	template<typename C, typename CT, typename CA>
	cvar_t& operator=(const std::basic_string<C,CT,CA>& v) {
		if (_type == Type::String)
			_string.assign(v);
		else {
			_type = Type::String;
			_string = std::move(ZString(v));
		}
	}
	cvar_t& operator=(bool v) {
		clear();
		_bool = v;
		_type = Type::Bool;
		return *this;
	}
	cvar_t& operator=(float v) {
		clear();
		_value = v;
		_type = Type::Number;
		return *this;
	}
	const char* name() const { return _name; }
	bool archive() const { return _archive; }
	bool server() const { return _server; }

	~cvar_t() { clear(); }
private:
	static cvar_t	*& cvar_vars() { static cvar_t* root = nullptr; return root; }
	const char	*_name;
	Type _type;
	union {
		ZString _string;
		const char* _inital;
		float _value;
		bool _bool;
	};
	qboolean _archive;		// set to true to cause it to be saved to vars.rc
	qboolean _server;		// notifies players when changed
	cvar_t *next;
} ;


#else
struct cvar_t {
	const char* name;
	char* string;
	float value;
	qboolean archive;		// set to true to cause it to be saved to vars.rc
	qboolean server;		// notifies players when changed
	cvar_t *next;
};
#endif
void 	Cvar_RegisterVariable (cvar_t *variable);
// registers a cvar that allready has the name, string, and optionally the
// archive elements set.

void 	Cvar_Set (const quake::string_view& var_name, const quake::string_view& value);
// equivelant to "<name> <variable>" typed at the console

void	Cvar_SetValue (const quake::string_view& var_name, float value);
// expands value to a string and calls Cvar_Set

float	Cvar_VariableValue (const quake::string_view& var_name);
// returns 0 if not defined or non numeric

quake::string_view Cvar_VariableString (const quake::string_view &var_name);
// returns an empty string if not defined

quake::string_view Cvar_CompleteVariable (const quake::string_view & partial);
// attempts to match a partial variable name for command line completion
// returns NULL if nothing fits

//qboolean Cvar_Command (size_t argc, const quake::string_view argv[]);
// called by Cmd_ExecuteString when Cmd_Argv(0) doesn't match a known
// command.  Returns true if the command was a variable reference that
// was handled. (print or change)

void 	Cvar_WriteVariables (std::ostream& f);
// Writes lines containing "set variable value" for all variables
// with the archive flag set to true.

cvar_t *Cvar_FindVar (const quake::string_view& var_name);


extern	cvar_t	registered;
//extern cvar_t	*cvar_vars;
