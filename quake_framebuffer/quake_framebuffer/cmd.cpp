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

#include "quakedef.h"

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

sizebuf_t	cmd_text;

/*
============
Cbuf_Init
============
*/
void Cbuf_Init (void)
{
	SZ_Alloc (&cmd_text, 8192);		// space for commands and script files
}


/*
============
Cbuf_AddText

Adds command text at the end of the buffer
============
*/
void Cbuf_AddText (const quake::string_view& text)
{
	if ((cmd_text.cursize + (int)text.size()) >= cmd_text.maxsize)
	{
		Con_Printf ("Cbuf_AddText: overflow\n");
		return;
	}

	SZ_Write (&cmd_text, text.data(), text.size());
}


/*
============
Cbuf_InsertText

Adds command text immediately after the current command
Adds a \n to the text
FIXME: actually change the command buffer to do less copying
============
*/
void Cbuf_InsertText (const quake::string_view& text)
{
	char	*temp = nullptr;// shut up compiler
	int		templen;

// copy off any commands still remaining in the exec buffer
	templen = cmd_text.cursize;
	if (templen)
	{
		temp = (char*)Z_Malloc (templen);
		std::memcpy (temp, cmd_text.data, templen);
		SZ_Clear (&cmd_text);
	}
		
// add the entire text of the file
	Cbuf_AddText (text);
	
// add the copied off data
	if (templen)
	{
		SZ_Write (&cmd_text, temp, templen);
		Z_Free (temp);
	}
}
void execute_args(const quake::string_view* args, size_t count, cmd_source_t src);
/*
============
Cbuf_Execute
============
*/
void Cbuf_Execute (void)
{
	if (cmd_text.cursize <= 0) return; // nothing?
	quake::string_view text((const char*)cmd_text.data, cmd_text.cursize);
	quake::string_view token;
	//UVector<quake::string_view> args;
	std::vector<quake::string_view> args;
	COM_Parser parser(text);
	do {
		token = parser.Next(true);
		if (token.empty() || token[0] == '\n') {
			if (!args.empty()) {
				execute_args(args.data(),args.size(), src_command);
				if (cmd_wait)
				{	// skip out while text still remains in buffer, leaving it
					// for next frame
					cmd_wait = false;
					if (parser.pos() == cmd_text.cursize)
						cmd_text.cursize = 0;
					else
					{
						cmd_text.cursize -= parser.pos();
						auto r = parser.remaining();
						r.copy((char*)cmd_text.data, cmd_text.cursize);
					}
					break;
				}
				args.clear();
			}
			continue;
		}
		args.emplace_back(token);
	} while (true);
#if 0
	int		i;
	char	*text;
	ZString line;
	line.reserve(1024);

	int		quotes;
	int		spaces = 0;
	while (cmd_text.cursize)
	{
// find a \n or ; line break
		text = (char *)cmd_text.data;
		spaces = 0;
		quotes = 0;
		for (i=0 ; i< cmd_text.cursize ; i++) {
			
			if (text[i] == '"')
				quotes++;
			if ( !(quotes&1) &&  text[i] == ';')
				break;	// don't break if inside a quoted string
			if (text[i] == '\n')
				break;
			if (!spaces && !::isspace(text[i])) ++spaces;
			if(text[i] == '/' && text[i+1] == '/')
		}

		
// delete the text from the command buffer and move remaining commands down
// this is necessary because commands (exec, alias) can insert data at the
// beginning of the text buffer



// execute the command line
		if (spaces) {
			assert(i < 1024);
			line.append(text, i);
			tokenizer.execute(line.data(), src_command);
			line.clear();
		}

		if (cmd_wait)
		{	// skip out while text still remains in buffer, leaving it
			// for next frame
			cmd_wait = false;
			break;
		}
	}
#endif
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

	mark = Hunk_LowMark ();
	f = (char *)COM_LoadHunkFile (args[1]);
	if (!f)
	{
		Con_Printf ("couldn't exec %s\n",args[1]);
		return;
	}
	Con_Printf ("execing %s\n",args[1]);
	
	Cbuf_InsertText (f);
	Hunk_FreeToLowMark (mark);
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


char *CopyString(const quake::string_view& in)
{
	char	*out;

	out = (char*)Z_Malloc(in.size() +1);
	std::memcpy(out, in.data(), in.size());
	return out;
}
void Cmd_Alias_f (cmd_source_t source, size_t argc, const quake::string_view args[])
{
	
	cmdalias_t	*a;
	quake::data_view<quake::string_view> view(args, argc);

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
	for (a = cmd_alias ; a ; a=a->next)
	{
		if (s == a->name)
		{
			a->value.clear();
			break;
		}
	}

	if (!a)
	{
		a = (cmdalias_t*)Z_Malloc (sizeof(cmdalias_t));
		a->next = cmd_alias;
		cmd_alias = a;
	}
	s.copy(a->name, sizeof(a->name));

// copy the rest of the command line
	for (size_t i=2 ; i< argc ; i++)
	{
		a->value += args[i];
		if (i != argc) a->value += ' ';
	}
	a->value += '\n';
}

/*
=============================================================================

					COMMAND EXECUTION

=============================================================================
*/






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
}



 char *CopyString(const quake::string_view& in); // from cmd.c
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
	std::memcpy(cmd->name, cmd_name.data(), cmd_name.size());
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
	if (cls.state != ca_connected)
	{
		Con_Printf ("Can't \"%s\", not connected\n", args[0]);
		return;
	}
	
	if (cls.demoplayback)
		return;		// not really connected

	MSG_WriteByte (&cls.message, clc_stringcmd);
	if (Q_strcasecmp(args[0], "cmd") != 0)
	{
		SZ_Print (&cls.message, args[0]);
		SZ_Print (&cls.message, ' ');
	}
	if(argc > 1)
	for(size_t i=1; i < argc;i++)
		SZ_Print(&cls.message, args[i]);
	SZ_Print (&cls.message, '\n');
}

void execute_args(const quake::string_view* args, size_t count, cmd_source_t src) {
	cmd_function_t	*cmd;
	cmdalias_t		*a;

	// execute the command line
	if (count == 0)
		return;		// no tokens
	quake::symbol lower = args[0];
	// FIX, replace with array

	// check functions
	for (cmd = cmd_functions; cmd; cmd = cmd->next)
	{
		if (lower == cmd->name)
		{
			cmd->function(src, count, args);
			return;
		}
	}

	// check alias
	for (a = cmd_alias; a; a = a->next)
	{
		if (lower == a->name)
		{
			Cbuf_InsertText(a->value.c_str());
			return;
		}
	}

	// check cvars
	if (!Cvar_Command(count, args)) {
		quake::con << "Unknown command \"" << args[0] << "\"" << std::endl;
	}
}

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
			return;
		}
	}

	// check alias
	for (a = cmd_alias; a; a = a->next)
	{
		if (lower == a->name)
		{
			Cbuf_InsertText(a->value.c_str());
			return;
		}
	}

	// check cvars
	if (!Cvar_Command(args.size(), args.data())) {
		quake::con << "Unknown command \"" << args[0] << "\"" << std::endl;
	}
}
void execute_args(const  quake::string_view& text, cmd_source_t src) {
	quake::string_view token;
	UVector<quake::string_view> args;
	COM_Parser parser(text);
	do {
		token = parser.Next(true);
		if (!token.empty() || token[0] == '\n') {
			if (!args.empty()) {
				execute_args(args, src_command);
				args.clear();
			}
			continue;
		}
		args.emplace_back(token);
	} while (true);
}