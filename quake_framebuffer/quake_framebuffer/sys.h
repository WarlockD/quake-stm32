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
// sys.h -- non-portable functions
#ifndef _QUAKE_SYS_H_
#define _QUAKE_SYS_H_

#include "quakedef.h"
#include <iostream>
#include <fstream>

//
// file IO
//

using idFileHandle = int;
int Sys_FileOpenRead (const char * path, idFileHandle *hndl);
int Sys_FilevPrintF(idFileHandle handle, const char* fmt, va_list va);
int Sys_FilePrintF(idFileHandle handle, const char* fmt, ...);
idFileHandle Sys_FileOpenWrite (const char * path);
void Sys_FileClose (idFileHandle handle);
void Sys_FileSeek (idFileHandle handle, int position);
int Sys_FileRead (idFileHandle handle, void *dest, int count);
int Sys_FileWrite (idFileHandle handle, const void * data, int count);
int	Sys_FileTime (const char * path);
void Sys_mkdir (char *path);



//
// memory protection
//
void Sys_MakeCodeWriteable (unsigned long startaddr, unsigned long length);

//
// system IO
//
void Sys_DebugLog(char *file, char *fmt, ...);

void Sys_Error (char *error, ...);
// an error will cause the entire program to exit

void Sys_Printf (char *fmt, ...);
// send text to the console

void Sys_Quit (void);

idTime Sys_FloatTime (void);

char *Sys_ConsoleInput (void);

void Sys_Sleep (void);
// called to yield for a little bit so as
// not to hog cpu when paused or debugging

void Sys_SendKeyEvents (void);
// Perform Key_Event () callbacks until the input que is empty

void Sys_LowFPPrecision (void);
void Sys_HighFPPrecision (void);
void Sys_SetFPCW (void);

#endif