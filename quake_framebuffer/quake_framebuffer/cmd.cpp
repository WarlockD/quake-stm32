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

void Cmd_ForwardToServer (void);



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
void Cmd_Wait_f (void)
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
	if (cmd_text.cursize + text.size() >= cmd_text.maxsize)
	{
		Con_Printf ("Cbuf_AddText: overflow\n");
		return;
	}

	SZ_Write(&cmd_text, text.data(), text.size());
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
	char	*temp = nullptr;
// copy off any commands still remaining in the exec buffer
	int templen = cmd_text.cursize;
	if (templen) {
		// instead of zmalloc lets use _alloca
		//temp = (char*)Z_Malloc (templen);
		temp = (char*)_alloca(templen);
		assert(temp);
		Q_memcpy (temp, cmd_text.data, templen);
		SZ_Clear (&cmd_text);
	}

// add the entire text of the file
	Cbuf_AddText (text);
	
// add the copied off data
	if (templen && temp)
	{
		SZ_Write (&cmd_text, temp, templen);
		//Z_Free (temp);
	}
}

/*
============
Cbuf_Execute
============
*/
void Cbuf_Execute (void)
{
	int		i;
	char	*text;
	quake::stack_string<1024> line;
	int		quotes;
	
	while (cmd_text.cursize)
	{
// find a \n or ; line break
		text = (char *)cmd_text.data;

		quotes = 0;
		for (i=0 ; i< cmd_text.cursize ; i++)
		{
			if (text[i] == '"')
				quotes++;
			if ( !(quotes&1) &&  text[i] == ';')
				break;	// don't break if inside a quoted string
			if (text[i] == '\n')
				break;
		}
			
				
		line.assign(text, i);

		// delete the text from the command buffer and move remaining commands down
		// this is necessary because commands (exec, alias) can insert data at the
		// beginning of the text buffer

		if (i == cmd_text.cursize)
			cmd_text.cursize = 0;
		else
		{
			i++;
			cmd_text.cursize -= i;
			Q_memcpy (text, text+i, cmd_text.cursize);
		}

// execute the command line
		Cmd_ExecuteString (line, src_command);
		
		if (cmd_wait)
		{	// skip out while text still remains in buffer, leaving it
			// for next frame
			cmd_wait = false;
			break;
		}
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
void Cmd_StuffCmds_f (void)
{
	int		i, j;
	int		s;
	char	*text, *build, c;
		
	if (Cmd_Argc () != 1)
	{
		Con_Printf ("stuffcmds : execute command line parameters\n");
		return;
	}

// build the combined string to parse from
	s = 0;
	for (i=1 ; i<com_argc ; i++)
	{
		if (!com_argv[i])
			continue;		// NEXTSTEP nulls out -NXHost
		s += Q_strlen (com_argv[i]) + 1;
	}
	if (!s)
		return;
		
	text = (char*)Z_Malloc (s+1);
	text[0] = 0;
	for (i=1 ; i<com_argc ; i++)
	{
		if (!com_argv[i])
			continue;		// NEXTSTEP nulls out -NXHost
		Q_strcat (text,com_argv[i]);
		if (i != com_argc-1)
			Q_strcat (text, " ");
	}
	
// pull out the commands
	build = (char*)Z_Malloc (s+1);
	build[0] = 0;
	
	for (i=0 ; i<s-1 ; i++)
	{
		if (text[i] == '+')
		{
			i++;

			for (j=i ; (text[j] != '+') && (text[j] != '-') && (text[j] != 0) ; j++)
				;

			c = text[j];
			text[j] = 0;
			
			Q_strcat (build, text+i);
			Q_strcat (build, "\n");
			text[j] = c;
			i = j-1;
		}
	}
	
	if (build[0])
		Cbuf_InsertText (build);
	
	Z_Free (text);
	Z_Free (build);
}


/*
===============
Cmd_Exec_f
===============
*/
void Cmd_Exec_f (void)
{
	char	*f;
	int		mark;

	if (Cmd_Argc () != 2)
	{
		Con_Printf ("exec <filename> : execute a script file\n");
		return;
	}

	mark = Hunk_LowMark ();
	quake::stack_string<128> filename(Cmd_Argv(1));
	f = (char *)COM_LoadHunkFile (filename.c_str());
	if (!f)
	{
		Con_Printf ("couldn't exec %s\n", filename.c_str());
		return;
	}
	Con_Printf ("execing %s\n", filename.c_str());
	
	Cbuf_InsertText (f);
	Hunk_FreeToLowMark (mark);
}


/*
===============
Cmd_Echo_f

Just prints the rest of the line to the console
===============
*/
void Cmd_Echo_f (void)
{
	int		i;
	
	for (i=1 ; i<Cmd_Argc() ; i++)
		Con_Printf ("%s ",Cmd_Argv(i));
	Con_Printf ("\n");
}

/*
===============
Cmd_Alias_f

Creates a new command that executes a command string (possibly ; seperated)
===============
*/

char *CopyString (const char *in)
{
	char	*out;
	
	out = (char*)Z_Malloc (strlen(in)+1);
	strcpy (out, in);
	return out;
}

void Cmd_Alias_f (void)
{
	cmdalias_t	*a;
	quake::stack_string<1024>	cmd;;

	if (Cmd_Argc() == 1)
	{
		Con_Printf ("Current alias commands:\n");
		for (a = cmd_alias ; a ; a=a->next)
			Con_Printf ("%s : %s\n", a->name, a->value);
		return;
	}

	auto s = Cmd_Argv(1);
	if (s.size() >= MAX_ALIAS_NAME)
	{
		Con_Printf ("Alias name is too long\n");
		return;
	}

	// if the alias allready exists, reuse it
	for (a = cmd_alias ; a ; a=a->next)
	{
		if (s ==  a->name)
		{
			Z_Free (a->value);
			break;
		}
	}

	if (!a)
	{
		a = (cmdalias_t*)Z_Malloc (sizeof(cmdalias_t));
		a->next = cmd_alias;
		cmd_alias = a;
	}
	a->name[s.copy(a->name)] = '\0';

// copy the rest of the command line

	auto c = Cmd_Argc();
	for (int i=2 ; i< c ; i++)
	{
		cmd += Cmd_Argv(i);
		if (i != c) cmd += ' ';
	}
	cmd += '\n';
	
	a->value = CopyString (cmd.c_str());
}

/*
=============================================================================

					COMMAND EXECUTION

=============================================================================
*/



#define	MAX_ARGS		80

static	int			cmd_argc;
static	quake::string_view cmd_argv[MAX_ARGS];
static	char		cmd_null_string[1] = { 0 };
static	quake::string_view cmd_args;

cmd_source_t	cmd_source;


static	cmd_function_t	*cmd_functions;		// possible commands to execute

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

/*
============
Cmd_Argc
============
*/
int		Cmd_Argc (void)
{
	return cmd_argc;
}

/*
============
Cmd_Argv
============
*/
const quake::string_view& Cmd_Argv (int arg)
{
	if ( (unsigned)arg >= cmd_argc )
		return quake::string_view();
	return cmd_argv[arg];	
}

/*
============
Cmd_Args
============
*/
const quake::string_view&  Cmd_Args (void)
{
	return cmd_args;
}


/*
============
Cmd_TokenizeString

Parses the given string into command line tokens.
============
*/
void Cmd_TokenizeString(quake::string_view  text)
{
	COM_Parser parser(text);
	quake::string_view token;
	cmd_argc = 0;
	cmd_args = quake::string_view();

	while (parser.Next(token, true)) {
		if (token.empty()) break;
		if (token[0] == '\n')  break;
		if (cmd_argc == 0)
			cmd_args = parser.remaining();
		if (cmd_argc < MAX_ARGS)
			cmd_argv[cmd_argc++] = token;

	};
	if (cmd_argc > 0 && cmd_argv[0] == "name") {
		Sys_Printf("TOKEN (%s:%i) :", va(cmd_argv[0]).c_str(), cmd_argc);
		for (int i = 1; i < cmd_argc; i++) Sys_Printf("(%s)", va(cmd_argv[i]).c_str());
		Sys_Printf("\n");
	}

}

void Old_Cmd_TokenizeString(quake::string_view  text)
{
	

	// clear the args from the last string
	//for (i = 0; i < cmd_argc; i++)
	//	Z_Free(cmd_argv[i]);
	
	cmd_argc = 0;
	cmd_args = quake::string_view();

	while (1)
	{
		// skip whitespace up to a /n
		size_t		pos = 0;
		while (pos < text.size() && text[pos] <= ' ' && text[pos] != '\n') pos++;

		text.remove_prefix(pos); // works, optimize latter?

		if (text.front() == '\n')
		{	// a newline seperates commands in the buffer
			text.remove_prefix(1); 
			break; // how is this diffrent than below
		}

		if (text.empty())
			return;

		if (cmd_argc == 1)
			cmd_args = text;

		//text = COM_Parse(text);
		if (text.empty() && com_token.empty())
			return;

		if (cmd_argc < MAX_ARGS)
			cmd_argv[cmd_argc++] = com_token;
	}
}


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
		Con_Printf ("Cmd_AddCommand: %s already defined as a var\n", cmd_name.data()); // pray to god that this is what it is
		return;
	}
	
// fail if the command already exists
	for (cmd=cmd_functions ; cmd ; cmd=cmd->next)
	{
		if (cmd_name == cmd->name)
		{
			Con_Printf ("Cmd_AddCommand: %s already defined\n", cmd_name.data());
			return;
		}
	}

	cmd = (cmd_function_t*)Hunk_Alloc (sizeof(cmd_function_t));
	cmd->name = cmd_name;
	cmd->function = function;
	cmd->next = cmd_functions;
	cmd_functions = cmd;
}

/*
============
Cmd_Exists
============
*/
qboolean	Cmd_Exists (const quake::string_view&cmd_name)
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
quake::string_view Cmd_CompleteCommand(const quake::string_view &partial)
{
	cmd_function_t	*cmd;


	if (!partial.empty()) {
		for (cmd = cmd_functions; cmd; cmd = cmd->next)
			if (partial.size() <= cmd->name.size() && quake::detail::str_case_compare(partial.begin(), partial.size(), cmd->name.begin(), partial.size()) == 0)
				return cmd->name;
	}
	// check functions

	return quake::string_view();
}

/*
============
Cmd_ExecuteString

A complete command line has been parsed, so try to execute it
FIXME: lookupnoadd the token to speed search?
============
*/
void	Cmd_ExecuteString (const quake::string_view& text, cmd_source_t src)
{	
	cmd_function_t	*cmd;
	cmdalias_t		*a;

	cmd_source = src;
	Cmd_TokenizeString (text);
			
// execute the command line
	if (!Cmd_Argc())
		return;		// no tokens

// check functions
	for (cmd=cmd_functions ; cmd ; cmd=cmd->next)
	{
		if (!Q_strcasecmp (cmd_argv[0],cmd->name)) {
			cmd->function ();
			return;
		}
	}

// check alias
	for (a=cmd_alias ; a ; a=a->next)
	{
		if (!Q_strcasecmp (cmd_argv[0], a->name)) {
			Cbuf_InsertText (a->value);
			return;
		}
	}
	
// check cvars
	if (!Cvar_Command ())
		Con_Printf ("Unknown command \"%s\"\n", Cmd_Argv(0));
	
}


/*
===================
Cmd_ForwardToServer

Sends the entire command line over to the server
===================
*/
void Cmd_ForwardToServer (void)
{
	if (cls.state != ca_connected)
	{
		Con_Printf ("Can't \"%s\", not connected\n", Cmd_Argv(0));
		return;
	}
	
	if (cls.demoplayback)
		return;		// not really connected

	MSG_WriteByte (&cls.message, clc_stringcmd);
	if (Q_strcasecmp(Cmd_Argv(0), "cmd") != 0)
	{
		SZ_Print (&cls.message, Cmd_Argv(0));
		SZ_Print (&cls.message, " ");
	}
	if (Cmd_Argc() > 1)
		SZ_Print (&cls.message, Cmd_Args());
	else
		SZ_Print (&cls.message, "\n");
}


/*
================
Cmd_CheckParm

Returns the position (1 to argc-1) in the command's argument list
where the given parameter apears, or 0 if not present
================
*/

int Cmd_CheckParm (const quake::string_view& parm)
{
	int i;
	
	if (parm.empty())
		Sys_Error ("Cmd_CheckParm: NULL");

	for (i = 1; i < Cmd_Argc (); i++)
		if (! Q_strcasecmp (parm, Cmd_Argv (i)))
			return i;
			
	return 0;
}
