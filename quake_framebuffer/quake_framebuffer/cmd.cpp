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
// cmd.c -- Quake script command processing module


#include "icommon.h"

void Cmd_ForwardToServer(cmd_source_t source, size_t argc, const quake::string_view args[]);



static	cmd_function_t	*cmd_functions;		// possible commands to execute

cmdalias_t	*cmd_alias;

int trashtest;
int *trashspot;

qboolean	cmd_wait;

//=============================================================================

/*
============
Cmd_Wait_f

Causes execution of the remainder of the command buffer to be delayed until
next frame.  This allows commands like:
bind g "impulse 5 ; +attack ; wait ; -attack ; impulse 2"
============
*/
void Cmd_Wait_f(cmd_source_t source, size_t argc, const quake::string_view args[])
{
	cmd_wait = true;
}

/*
=============================================================================

						COMMAND BUFFER

=============================================================================
*/

std::default_delete<char> meh;

using StringPtr = std::unique_ptr<char, z_delete<char>>;
using StringViewList = UVector<quake::string_view>;



//sizebuf_t	cmd_text;
class SafeArgs {
	ZUniquePtr<char> _text;
	UList<StringViewList> _list;
public:
	static UList<StringViewList> ParseArgs(const quake::string_view& text);
	SafeArgs(ZUniquePtr<char>&& text) :_text(std::move(text)), _list(std::move(ParseArgs(_text.get()))) {}
	SafeArgs(const quake::string_view& text) :_text((char*)Z_Malloc(text.size()+1)) {
		text.copy(_text.get(), text.size());
		_text.get()[text.size()] = 0; // make a copy
		_list = ParseArgs(_text.get());
	}
	SafeArgs(const SafeArgs& copy) = delete;
	SafeArgs(SafeArgs&& move) :_text(std::move(move._text)),  _list(std::move(move._list)) {}
	UList<StringViewList>& list() { return _list; }
	const UList<StringViewList>& list() const { return _list; }
	UList<StringViewList>* operator->() { return &_list; }
	const UList<StringViewList>* operator->() const { return &_list; }
};
UList<StringViewList> SafeArgs::ParseArgs(const quake::string_view& text) {
	COM_Parser parser(text);
	quake::string_view token;
	StringViewList args;
	UList<StringViewList> list;
	while (parser.Next(token, true)) {
		if (token[0] == '\n') {
			if (!args.empty()) {
				list.push_back(args);
				args.clear();
			}
		}
		else args.emplace_back(token.data(), token.size());
	};
	if (!args.empty())list.push_back(args);
	return std::move(list);
}
static UList<SafeArgs> _parsed_commands;
//static UList<StringList> _parsed_commands;
/*
============
Cbuf_Init
============
*/
void Cbuf_Init (void)
{
//	cmd_text.Alloc(8192);		// space for commands and script files
	_parsed_commands.clear();
}



void ParseCommands(const quake::string_view& text, bool front = false) {

	auto start = front ? _parsed_commands.begin() : _parsed_commands.end();
	_parsed_commands.insert(start, SafeArgs(text));
}
/*
============
Cbuf_AddText

Adds command text at the end of the buffer
============
*/
void Cbuf_AddText (const quake::string_view& text){
	ParseCommands(text, false);
}


/*
============
Cbuf_InsertText

Adds command text immediately after the current command
Adds a \n to the text
FIXME: actually change the command buffer to do less copying
============
*/
void Cbuf_InsertText (const quake::string_view& text) {
	ParseCommands(text, true);
}

/*
============
Cbuf_Execute
============
*/
void Cbuf_Execute (void)
{
	while (!_parsed_commands.empty()) {
		auto it = _parsed_commands.begin();
		auto& list = it->list();

		while (!list.empty()) {
			execute_args(list.front(), src_command);
			list.pop_front();
			if (cmd_wait)
			{	// skip out while text still remains in buffer, leaving it
				// for next frame
				cmd_wait = false;
				return;
			}
		}
		_parsed_commands.erase(it);
	}
}

/*
==============================================================================

						SCRIPT COMMANDS

==============================================================================
*/

/*
===============
Cmd_StuffCmds_f

Adds command line parameters as script statements
Commands lead with a +, and continue until a - or another +
quake +prog jctest.qp +cmd amlev1
quake -nosound +cmd amlev1
===============
*/
void Cmd_StuffCmds_f (cmd_source_t source, size_t argc, const quake::string_view args[])
{
	if (argc != 1)
	{
		Con_Printf ("stuffcmds : execute command line parameters\n");
		return;
	}
// build the combined string to parse from
	quake::fixed_string<128> build;
	for (size_t i = 1; i<argc; i++)
	{
		if (args[i].empty()) continue;		// NEXTSTEP nulls out -NXHost
		const auto& arg = args[i];
		for (size_t j = 0; j < arg.size(); j++) {
			if (arg[j] == '+')
			{
				size_t k;
				for(k = ++i; (arg[k] != '+') && (arg[k] != '-') &&  k < arg.size(); k++);
				build += arg.substr(j + 1, k - j);
				build += '\n';
				i = k - 1;
			}
			else build += arg[j];
		}
		if (i != (argc - 1))
			build += ' ';
	}
	if(!build.empty())
		Cbuf_InsertText (build.c_str());
}

byte *COM_LoadZFile(const quake::string_view& path);
/*
===============
Cmd_Exec_f
===============
*/
void Cmd_Exec_f (cmd_source_t source, size_t argc, const quake::string_view args[])
{
	char	*f;
	int		mark;

	if (argc != 2)
	{
		Con_Printf ("exec <filename> : execute a script file\n");
		return;
	}
	// because we arn't saving the memory in a seperate buffer lets just allocate the file 
	// at the start
	f = (char*)COM_LoadZFile(args[1]);
	if (f) {
		quake::con << "execing " << args[1] << std::endl;
		_parsed_commands.emplace_front(ZUniquePtr<char>(f));
	}
	else 
		quake::con << "couldn't exec " << args[1] << std::endl;

#if 0
	mark = Hunk_LowMark ();
	f = (char *)COM_LoadHunkFile (args[1]);
	if (!f)
	{
		quake::con << "couldn't exec " << args[1] << std::endl;
		return;
	}
	quake::con << "execing " << args[1] << std::endl;


	
	Cbuf_InsertText (f);
	Hunk_FreeToLowMark (mark);
#endif
}


/*
===============
Cmd_Echo_f

Just prints the rest of the line to the console
===============
*/
void Cmd_Echo_f (cmd_source_t source, size_t argc, const quake::string_view args[])
{
	for (size_t i = 1; i < argc; i++) {
		quake::con << args[i] << ' ';
	}
	quake::con << std::endl;
}

/*
===============
Cmd_Alias_f

Creates a new command that executes a command string (possibly ; seperated)
===============
*/

void cmdalias_t::operator delete(void *ptr) { Z_Free(ptr); }
cmdalias_t* cmdalias_t::create(const quake::string_view&  name, const quake::string_view&  value) {
	char* ptr = (char*)Z_Malloc(value.size() + name.size() + sizeof(cmdalias_t) + 2);
	char* n_name = ptr + sizeof(cmdalias_t);
	char* n_value = n_name + name.size() + 1;
	::memcpy(n_name, name.data(), name.size()); n_name[name.size()] = 0;
	::memcpy(n_value, value.data(), value.size()); n_value[value.size()] = 0;
	return new(ptr) cmdalias_t(n_name, n_value);
}


void Cmd_Alias_f (cmd_source_t source, size_t argc, const quake::string_view args[])
{
	
	cmdalias_t	*a,*p;


	if (argc == 1)
	{
		Con_Printf ("Current alias commands:\n");
		for (a = cmd_alias ; a ; a=a->next)
			Con_Printf ("%s : %s\n", a->name, a->value);
		return;
	}

	const quake::string_view& s = args[1];
	if (s.size() >= MAX_ALIAS_NAME)
	{
		Con_Printf ("Alias name is too long\n");
		return;
	}


	// if the alias allready exists, reuse it
	for (a = cmd_alias, p = nullptr ; a ; p =a, a=a->next)
	{
		if (s == a->name)
		{
			if (p) p->next = a->next;
			else cmd_alias = a->next;
			delete a;
			break;
		}
	}


	std::stringstream ss;
		ZStringStream zs;
// copy the rest of the command line
	for (size_t i=2 ; i< argc ; i++)
	{
		if (i != 2)zs << ' ';
		zs << args[i];

	}
	zs << std::endl;
	a = cmdalias_t::create(args[1], zs.str());
	a->next = cmd_alias;
	cmd_alias = a;
}

/*
=============================================================================

					COMMAND EXECUTION

=============================================================================
*/


void Z_Print_f(cmd_source_t source, size_t argc, const quake::string_view args[]);
void Z_Stats_f(cmd_source_t source, size_t argc, const quake::string_view args[]);


/*
============
Cmd_Init
============
*/
void Cmd_Init (void)
{
//
// register our commands
//
	Cmd_AddCommand ("stuffcmds",Cmd_StuffCmds_f);
	Cmd_AddCommand ("exec",Cmd_Exec_f);
	Cmd_AddCommand ("echo",Cmd_Echo_f);
	Cmd_AddCommand ("alias",Cmd_Alias_f);
	Cmd_AddCommand ("cmd", Cmd_ForwardToServer);
	Cmd_AddCommand ("wait", Cmd_Wait_f);
	Cmd_AddCommand("zstats", Z_Stats_f);
	Cmd_AddCommand("zprint", Z_Print_f);
}




/*
============
Cmd_TokenizeString

Parses the given string into command line tokens.
============
*/


/*
============
Cmd_AddCommand
============
*/
void	Cmd_AddCommand (const quake::string_view& cmd_name, xcommand_t function)
{
	cmd_function_t	*cmd;
	
	if (host_initialized)	// because hunk allocation would get stomped
		Sys_Error ("Cmd_AddCommand after host_initialized");
		
// fail if the command is a variable name
	if (!Cvar_VariableString(cmd_name).empty())
	{
		Con_Printf ("Cmd_AddCommand: %s already defined as a var\n", cmd_name);
		return;
	}
	
// fail if the command already exists
	for (cmd=cmd_functions ; cmd ; cmd=cmd->next)
	{
		if (cmd_name == static_cast<const char*>(cmd->name))
		{
			Con_Printf ("Cmd_AddCommand: %s already defined\n", cmd_name);
			return;
		}
	}

	cmd = (cmd_function_t*)Hunk_Alloc (sizeof(cmd_function_t) + cmd_name.size() +1);
	Q_memcpy((void*)cmd->name, cmd_name.data(), cmd_name.size());
	cmd->name[cmd_name.size()] = 0;
	cmd->function = function;
	cmd->next = cmd_functions;
	cmd_functions = cmd;
}

/*
============
Cmd_Exists
============
*/
qboolean	Cmd_Exists (const quake::string_view& cmd_name)
{
	cmd_function_t	*cmd;

	for (cmd=cmd_functions ; cmd ; cmd=cmd->next)
	{
		if (cmd_name == cmd->name)
			return true;
	}

	return false;
}



/*
============
Cmd_CompleteCommand
============
*/
quake::string_view Cmd_CompleteCommand (const quake::string_view& partial)
{
	cmd_function_t	*cmd;
	
	if (partial.empty())
		return nullptr;
		
// check functions
	for (cmd=cmd_functions ; cmd ; cmd=cmd->next)
		if (!Q_strncmp (partial.data(),cmd->name, partial.size()))
			return cmd->name;

	return NULL;
}
qboolean	Cvar_Command(size_t argc, const quake::string_view argv[]);



/*
===================
Cmd_ForwardToServer

Sends the entire command line over to the server
===================
*/
void Cmd_ForwardToServer(cmd_source_t source, size_t argc, const quake::string_view args[])
{
	if (quake::cls.state != ca_connected)
	{
		Con_Printf ("Can't \"%s\", not connected\n", args[0]);
		return;
	}
	
	if (quake::cls.demoplayback)
		return;		// not really connected

	quake::cls.message.WriteByte( clc_stringcmd);
	if (Q_strcasecmp(args[0], "cmd") != 0)
	{
		quake::cls.message.Print( args[0]);
		quake::cls.message.Print(' ');
	}
	if(argc > 1)
	for(size_t i=1; i < argc;i++)
		quake::cls.message.Print(args[i]);
	quake::cls.message.Print('\n');
}
#define DEBUG_ARGS


void execute_args(const UVector<quake::string_view>& args, cmd_source_t src) {
	cmd_function_t	*cmd;
	cmdalias_t		*a;

	// execute the command line
	if (args.size() == 0)
		return;		// no tokens
	quake::symbol lower = args[0];
	// FIX, replace with array

	// check functions
	for (cmd = cmd_functions; cmd; cmd = cmd->next)
	{
		if (lower == cmd->name)
		{
			cmd->function(src, args.size(), args.data());
#ifdef DEBUG_ARGS
			quake::debug << "CALL: " << lower << '(';
			for (size_t i = 1; i < args.size(); i++) {
				if (i != 1) quake::debug << ',';
				quake::debug << args[i];
			}
			quake::debug << ")" << std::endl;
#endif
			return;
		}
	}

	// check alias
	for (a = cmd_alias; a; a = a->next)
	{
		if (lower == a->name)
		{
			Cbuf_InsertText(a->value);
			return;
		}
	}

	// check cvars
	if (!Cvar_Command(args.size(), args.data())) {
		quake::con << "Unknown command \"" << lower << "\"" << std::endl;
	}
	else {
#ifdef DEBUG_ARGS
		if (args.size() > 1) {
			quake::debug << "SET: " << lower << " = " << args[1] << std::endl;
		}
#endif
	}
}
void execute_args(const  quake::string_view& text, cmd_source_t src) {
	auto list = SafeArgs::ParseArgs(text);
	for (const auto& args : list) {
		execute_args(args, src_command);
	}
}