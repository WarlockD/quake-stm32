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
// sys_null.h -- null system driver to aid porting efforts

#include "quakedef.h"
#include "errno.h"

#include <assert.h>
#include <direct.h>

#include <limits.h>
#include <Windows.h>
#include <GLFW\glfw3.h>
#include <assert.h>
#include <sys\stat.h>

GLFWwindow* glfw_window = NULL;
qboolean isDedicated = false;

/*
===============================================================================

FILE IO

===============================================================================
*/

#define MAX_HANDLES             10
FILE    *sys_handles[MAX_HANDLES];

int             findhandle (void)
{
	int             i;
	
	for (i=1 ; i<MAX_HANDLES ; i++)
		if (!sys_handles[i])
			return i;
	Sys_Error ("out of handles");
	return -1;
}

/*
================
filelength
================
*/
int filelength (FILE *f)
{
	int             pos;
	int             end;

	pos = ftell (f);
	fseek (f, 0, SEEK_END);
	end = ftell (f);
	fseek (f, pos, SEEK_SET);

	return end;
}

int Sys_FileOpenRead (const char * path, idFileHandle *hndl)
{
	FILE    *f;
	int             i;
	
	i = findhandle ();

	f = fopen(path, "rb");
	if (!f)
	{
		*hndl = -1;
		return -1;
	}
	sys_handles[i] = f;
	*hndl = i;
	
	return filelength(f);
}
int Sys_FilevPrintF(idFileHandle handle, const char* fmt, va_list va)
{
	return vfprintf(sys_handles[handle], fmt, va);
}
int Sys_FilePrintF(idFileHandle handle, const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	int r = Sys_FilevPrintF(handle, fmt, va);
	va_end(va);
	return r;
}
idFileHandle Sys_FileOpenWrite (const char * path)
{
	FILE    *f;
	int             i;
	
	i = findhandle ();

	f = fopen(path, "wb");
	if (!f)
		Sys_Error ("Error opening %s: %s", path,strerror(errno));
	sys_handles[i] = f;
	
	return i;
}

void Sys_FileClose (idFileHandle handle)
{
	fclose (sys_handles[handle]);
	sys_handles[handle] = NULL;
}

void Sys_FileSeek (idFileHandle handle, int position)
{
	fseek (sys_handles[handle], position, SEEK_SET);
}

int Sys_FileRead (idFileHandle handle, void *dest, int count)
{
	return fread (dest, 1, count, sys_handles[handle]);
}

int Sys_FileWrite (idFileHandle handle, const void * data, int count)
{
	return fwrite (data, 1, count, sys_handles[handle]);
}

int     Sys_FileTime (const char * path)
{
	struct stat s;
	FILE    *f;
	if (stat(path, &s) < 0)  return -1;
	return static_cast<int>(s.st_mtime);
}

void Sys_mkdir (char *path)
{
	_mkdir(path);
}


/*
===============================================================================

SYSTEM IO

===============================================================================
*/

void Sys_DebugNumber(int y, int val)
{
}
void Sys_DebugLog(const char *file, const char *fmt, ...)
{
	va_list argptr;
	static char data[1024];
	int fd;

	va_start(argptr, fmt);
	int len = vsprintf(data, fmt, argptr);
	va_end(argptr);
	assert(len >0 && len < 1024 - 1);
	if (file == NULL) {
		Sys_Printf("DEBUG: %s", data);
	}
	else {
		//assert(fwrite(data, 1, len, fd) == len);
		fd = Sys_FileOpenWrite(file);  
		assert(Sys_FileWrite(fd, data, len) == len);
		Sys_FileClose(fd);
	}
};



void Sys_Printf(char *fmt, ...)
{
	va_list		argptr;
	char		text[2048];
	unsigned char		*p;

	va_start(argptr, fmt);
	int len = vsprintf(text, fmt, argptr);
	va_end(argptr);

	if (len <= 0 || len > sizeof(text))
		Sys_Error("memory overwrite in Sys_Printf");
	//Con_Print(text);
	OutputDebugStringA(text);
}

void Sys_MakeCodeWriteable (unsigned long startaddr, unsigned long length)
{
}





static void gfl3_error_callback(int error, const char* description)
{
	Sys_Error("Error: %s\n", description);
}

void Sys_Quit(void)
{

	Host_Shutdown();

	if (glfw_window) {
		glfwDestroyWindow(glfw_window); glfw_window = NULL;
	}
	//gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	glfwTerminate();
	exit(0);
}
void Sys_Init(void)
{
	glfwSetErrorCallback(gfl3_error_callback);
	//if (!glfwInit()) Sys_Error("glfwInit: failed!");
	assert(glfwInit());
	glfw_window = glfwCreateWindow(800, 600, "My Title", NULL, NULL);
	assert(glfw_window);


	//timeBeginPeriod(1);
}

void Sys_Error(char *error, ...)
{
	va_list     argptr;
	char        string[1024];

	// change stdin to non blocking

	va_start(argptr, error);
	int len = Q_vsprintf(string, error, argptr);
	va_end(argptr);
	Sys_Printf("Error: %s\n", string);

	Host_Shutdown();
	exit(1);

}

void Sys_Warn(char *warning, ...)
{
	va_list     argptr;
	char        string[1024];

	va_start(argptr, warning);
	Q_vsprintf(string, warning, argptr);
	va_end(argptr);
	Sys_Printf("Warning: %s", string);
}

idTime Sys_FloatTime (void)
{
	float ftime = (float)glfwGetTime();
	idTime time = idCast<idTime>(ftime);
	return time;
}

char *Sys_ConsoleInput (void)
{
	return NULL;
}

void Sys_Sleep (void)
{
	glfwWaitEvents();
}

void Sys_SendKeyEvents (void)
{
	glfwPollEvents();
	if (glfwWindowShouldClose(glfw_window)) 	Sys_Quit();
#if 0
	MSG        msg;
	while (!glfwWindowShouldClose(glfw_window))
		while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			// we always update if there are any event, even if we're paused
			scr_skipupdate = 0;

			if (!GetMessage(&msg, NULL, 0, 0))
				Sys_Quit();
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
#endif
}

void Sys_HighFPPrecision (void)
{
}

void Sys_LowFPPrecision (void)
{
}

//=============================================================================

void main (int argc, char **argv)
{
	idTime		time, oldtime, newtime;
	static quakeparms_t    parms;

	parms.memsize = 8*1024*1024;
	parms.membase = malloc (parms.memsize);
	parms.basedir = ".";

	COM_InitArgv (argc, argv);

	parms.argc = com_argc;
	parms.argv = com_argv;

	printf ("Host_Init\n");
	Sys_Init();
	Host_Init (&parms);
	oldtime = Sys_FloatTime();
	while (1)
	{
		// find time spent rendering last frame
		newtime = Sys_FloatTime();
		time = newtime - oldtime;

		Host_Frame(time);
		oldtime = newtime;
	}
}


