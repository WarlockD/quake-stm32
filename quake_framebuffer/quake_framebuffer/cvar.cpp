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
using cvar_var_t = std::variant<float*, cstring_t*>;
struct var_ref {
	cvar_t_base& base;
	cvar_var_t value;
	template<typename T>
	var_ref(cvar_t_base& base, T* v) : base(base), value(v) {}
	template<typename T>
	bool is() const { return std::holds_alternative<T*>(value); }
	template<typename T>
	T* get() const { return is<T>() ? std::get<T*>(value) : nullptr; }
};

// get the value to stream
static inline std::ostream& operator<<(std::ostream& os, const var_ref& v) {
	std::visit([&os](auto&& arg) {std::cout << arg; }, v.value);
	return os;
}
static std::unordered_map<string_t, var_ref> c_vars;

 template<typename T> 
 static T* _Cvar_Get(string_t var_name) {
	auto it = c_vars.find(var_name);
	if (it != c_vars.end()) return it->second.get<T>();
	return nullptr;
}

/*
============
Cvar_FindVar
============
*/
template<> float* Cvar_Get<float>(string_t var_name) { return _Cvar_Get<float>(var_name); }
template<> cstring_t* Cvar_Get<cstring_t>(string_t var_name) { return _Cvar_Get<cstring_t>(var_name); }


void Cvar_Set(string_t var_name, const std::string_view& v) {
	cstring_t* old_value = Cvar_Get<cstring_t>(var_name);
	if (!old_value)
	{	// there is an error in C code if this happens
		quake::con << "Cvar_Set: variable " << var_name << " not found" << std::endl;
		return;
	}
	if (*old_value != v) {
		if (sv.active) {
			quake::fixed_string_stream<128> ss;
			ss << '"' << *old_value << "\" changed to \"" << v << '"' << std::endl;
			SV_BroadcastPrintf(ss.str().c_str());
		}
		*old_value = string_t::intern(v);
	}
}
void 	Cvar_Set(string_t var_name, float v) {
	float* old_value = Cvar_Get<float>(var_name);
	if (!old_value)
	{	// there is an error in C code if this happens
		quake::con << "Cvar_Set: variable " << var_name << " not found" << std::endl;
		return;
	}
	if (*old_value != v) {
		if (sv.active) {
			quake::fixed_string_stream<128> ss;
			ss << '"' << *old_value << "\" changed to \"" << v << '"' << std::endl;
			SV_BroadcastPrintf(ss.str().c_str());
		}
		*old_value = v;
	}
}



/*
============
Cvar_CompleteVariable
============
*/
std::string_view  Cvar_CompleteVariable (const std::string_view& partial)
{
	// not supported yet
#if 0
	if (partial.empty()) return nullptr;
// check functions
	for (auto& cvar : c_vars) {
		if(Q_)
		if (cvar->name == partial)
			return cvar->name;
	}
#endif


	return std::string_view();
}

static void Cvar_RegisterVariable(const std::string_view& name, var_ref v) {
	string_t qname = string_t::intern(name);
	// first check to see if it has allready been defined
	// check for overlap with a command
	if (Cmd_Exists(name))
	{
		quake::con << "Cvar_RegisterVariable: " << name << " is a command" << std::endl;
		return;
	}
	string_t var_name = string_t::intern(name);
	auto it = c_vars.find(var_name);
	if (it != c_vars.end())
	{
		quake::con << "Can't register variable " << name << ", allready defined" << std::endl;
		return;
	}
	c_vars.emplace(var_name, v);
}

void Cvar_RegisterVariable(const std::string_view& name, cvar_t<float>& variable) {
	Cvar_RegisterVariable(name, var_ref(variable, &variable.value));
}
void Cvar_RegisterVariable(const std::string_view& name, cvar_t<cstring_t>& variable) {
	Cvar_RegisterVariable(name, var_ref(variable, &variable.value));
}

/*
============
Cvar_Command

Handles variable inspection and changing from the console
============
*/

qboolean Cvar_Command (const StringArgs& args)
{
	string_t var_name = string_t::intern(args[0]);
	auto it = c_vars.find(var_name);
	if (it == c_vars.end()) return false;
	auto& var = it->second;
// perform a variable print or set
	if (args.size() == 1)
	{
		quake::con << "\"" << var_name << "\" is \"" << var << "\"" << std::endl;
		return true;
	}
	char* end;
	float f = strtof(args[1].data(), &end);
	if (end == args[1].data()) { // its a string value
		*var.get<cstring_t>() = string_t::intern(args[1]);
	}
	else {
		*var.get<float>() = f;
	}
	return true;
}


/*
============
Cvar_WriteVariables

Writes lines containing "set variable value" for all variables
with the archive flag set to true.
============
*/
void Cvar_WriteVariables(std::ostream& f)
{
	for (auto it : c_vars) {
		auto& var = it.second;
		if (var.base.archive)
			f << it.first << '\"' << var << '\"' << std::endl;
	}
}

