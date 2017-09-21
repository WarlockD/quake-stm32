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
// sys_win.h
// glad loader
#ifdef _DEBUG

#include "../glad/debug/src/glad.c"
#else
#include "../glad/release/src/glad.c"

#endif

#include "quakedef.h"
#include "winquake.h"
#include "resource.h"
#include "errno.h"
#include <fcntl.h>  
#include <sys/types.h>  
#include <sys/stat.h>  
#include <io.h>  
#include <stdio.h>  
#include <signal.h>

#include <limits.h>
#include <Windows.h>
#include <GLFW\glfw3.h>
#include <assert.h>

#define MINIMUM_WIN_MEMORY	0x0c00000
#define MAXIMUM_WIN_MEMORY	0x1000000

#define PAUSE_SLEEP		50				// sleep time on pause or minimization
#define NOT_FOCUS_SLEEP	20				// sleep time when not focus

int		starttime;
qboolean ActiveApp, Minimized;
qboolean	WinNT;
GLFWwindow* glfw_window = NULL;


static double		pfreq;
static double		curtime = 0.0;
static double		lastcurtime = 0.0;
static int			lowshift;
static HANDLE		hinput, houtput;

HANDLE		qwclsemaphore;

static HANDLE	tevent;

void Sys_InitFloatTime (void);
// =======================================================================
// General routines
// =======================================================================

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
		assert(fwrite(data, 1, len, fd) == len);
		fd = _open(file, O_WRONLY | O_CREAT | O_APPEND, 0666);
		assert(_write(fd, data, len) == len);
		_close(fd);
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

	if (len <=0 || len > sizeof(text))
		Sys_Error("memory overwrite in Sys_Printf");
	//Con_Print(text);
	OutputDebugStringA(text);
}


static void gfl3_error_callback(int error, const char* description)
{
	Sys_Error(stderr, "Error: %s\n", description);
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
	int len = vsprintf(string, error, argptr);
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
	vsprintf(string, warning, argptr);
	va_end(argptr);
	Sys_Printf("Warning: %s", string);
}
/*
===============================================================================

FILE IO

===============================================================================
*/



int	Sys_FileTime (const char *path)
{
	struct	stat	buf;

	if (_stat(path, &buf) == -1)
		return -1;

	return buf.st_mtime;
}

void Sys_mkdir (const char *path)
{
	_mkdir (path);
}

int Sys_FileOpenRead(const char *path, int *handle)
{
	int	h;
	struct stat	fileinfo;


	h = _open(path, O_RDONLY, 0666);
	*handle = h;
	if (h == -1)
		return -1;

	if (_fstat(h, &fileinfo) == -1)
		Sys_Error("Error fstating %s", path);

	return fileinfo.st_size;
}
int Sys_FileOpenWrite(const char *path)
{
	int     handle;

	handle = open(path, O_RDWR | O_CREAT | O_TRUNC, 0666);

	if (handle == -1)
		Sys_Error("Error opening %s: %s", path, strerror(errno));

	return handle;
}
int Sys_FileWrite(int handle,const void *src, int count)
{
	return _write(handle, src, count);
}

void Sys_FileClose(int handle)
{
	_close(handle);
}

void Sys_FileSeek(int handle, int position)
{
	_lseek(handle, position, SEEK_SET);
}

int Sys_FileRead(int handle, void *dest, int count)
{
	return _read(handle, dest, count);
}
/*
===============================================================================

SYSTEM IO

===============================================================================
*/



static volatile int oktogo;
void floating_point_exception_handler(int whatever);

void alarm_handler(int x)
{
	oktogo = 1;
}

void Sys_LineRefresh(void)
{
}

void floating_point_exception_handler(int whatever)
{
		Sys_Warn("floating point exception\n");
	signal(SIGFPE, &floating_point_exception_handler);
}

void Sys_HighFPPrecision(void)
{
}

void Sys_LowFPPrecision(void)
{
}

/*
================
Sys_MakeCodeWriteable
================
*/
void Sys_MakeCodeWriteable (unsigned long startaddr, unsigned long length)
{
#if 0
	DWORD  flOldProtect;

//@@@ copy on write or just read-write?
	if (!VirtualProtect((LPVOID)startaddr, length, PAGE_READWRITE, &flOldProtect))
   		Sys_Error("Protection change failed\n");
#endif
	// not sure we need this yet

}



	// make sure the timer is high precision, otherwise
	// NT gets 18ms resolution



#if 0
/*
================
Sys_DoubleTime
================
*/
double Sys_DoubleTime (void)
{
	static int			sametimecount;
	static unsigned int	oldtime;
	static int			first = 1;
	LARGE_INTEGER		PerformanceCount;
	unsigned int		temp, t2;
	double				time;

	Sys_PushFPCW_SetHigh ();

	QueryPerformanceCounter (&PerformanceCount);

	temp = ((unsigned int)PerformanceCount.LowPart >> lowshift) |
		   ((unsigned int)PerformanceCount.HighPart << (32 - lowshift));

	if (first)
	{
		oldtime = temp;
		first = 0;
	}
	else
	{
	// check for turnover or backward time
		if ((temp <= oldtime) && ((oldtime - temp) < 0x10000000))
		{
			oldtime = temp;	// so we can't get stuck
		}
		else
		{
			t2 = temp - oldtime;

			time = (double)t2 * pfreq;
			oldtime = temp;

			curtime += time;

			if (curtime == lastcurtime)
			{
				sametimecount++;

				if (sametimecount > 100000)
				{
					curtime += 1.0;
					sametimecount = 0;
				}
			}
			else
			{
				sametimecount = 0;
			}

			lastcurtime = curtime;
		}
	}

	Sys_PopFPCW ();

    return curtime;
}

/*
================
Sys_InitFloatTime
================
*/
void Sys_InitFloatTime (void)
{
	int		j;

	Sys_DoubleTime ();

	j = COM_CheckParm("-starttime");

	if (j)
	{
		curtime = (double) (Q_atof(com_argv[j+1]));
	}
	else
	{
		curtime = 0.0;
	}

	lastcurtime = curtime;
}

#endif

/*
================
Sys_DoubleTime
================
*/
double Sys_DoubleTime(void)
{
	return glfwGetTime();

}
char *Sys_ConsoleInput (void)
{
	static char	text[256];
	static int		len;
	INPUT_RECORD	recs[1024];
	int		count;
	int		i, dummy;
	int		ch, numread, numevents;
	HANDLE	th;
	char	*clipText, *textCopied;

	for ( ;; )
	{
		if (!GetNumberOfConsoleInputEvents (hinput, &numevents))
			Sys_Error ("Error getting # of console events");

		if (numevents <= 0)
			break;

		if (!ReadConsoleInput(hinput, recs, 1, &numread))
			Sys_Error ("Error reading console input");

		if (numread != 1)
			Sys_Error ("Couldn't read console input");

		if (recs[0].EventType == KEY_EVENT)
		{
			if (!recs[0].Event.KeyEvent.bKeyDown)
			{
				ch = recs[0].Event.KeyEvent.uChar.AsciiChar;

				switch (ch)
				{
					case '\r':
						WriteFile(houtput, "\r\n", 2, &dummy, NULL);	

						if (len)
						{
							text[len] = 0;
							len = 0;
							return text;
						}
						break;

					case '\b':
						WriteFile(houtput, "\b \b", 3, &dummy, NULL);	
						if (len)
						{
							len--;
							putch('\b');
						}
						break;

					default:
						Con_Printf("Stupid: %d\n", recs[0].Event.KeyEvent.dwControlKeyState);
						if (((ch=='V' || ch=='v') && (recs[0].Event.KeyEvent.dwControlKeyState & 
							(LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED))) || ((recs[0].Event.KeyEvent.dwControlKeyState 
							& SHIFT_PRESSED) && (recs[0].Event.KeyEvent.wVirtualKeyCode
							==VK_INSERT))) {
							if (OpenClipboard(NULL)) {
								th = GetClipboardData(CF_TEXT);
								if (th) {
									clipText = GlobalLock(th);
									if (clipText) {
										textCopied = malloc(GlobalSize(th)+1);
										strcpy(textCopied, clipText);
/* Substitutes a NULL for every token */strtok(textCopied, "\n\r\b");
										i = strlen(textCopied);
										if (i+len>=256)
											i=256-len;
										if (i>0) {
											textCopied[i]=0;
											text[len]=0;
											strcat(text, textCopied);
											len+=dummy;
											WriteFile(houtput, textCopied, i, &dummy, NULL);
										}
										free(textCopied);
									}
									GlobalUnlock(th);
								}
								CloseClipboard();
							}
						} else if (ch >= ' ')
						{
							WriteFile(houtput, &ch, 1, &dummy, NULL);	
							text[len] = ch;
							len = (len + 1) & 0xff;
						}

						break;

				}
			}
		}
	}

	return NULL;
}

void Sys_Sleep (void)
{
	glfwWaitEvents();
}


void Sys_SendKeyEvents (void)
{
	glfwPollEvents();
	if(glfwWindowShouldClose(glfw_window)) 	Sys_Quit();
#if 0
    MSG        msg;
	while (!glfwWindowShouldClose(glfw_window))
	while (PeekMessage (&msg, NULL, 0, 0, PM_NOREMOVE))
	{
	// we always update if there are any event, even if we're paused
		scr_skipupdate = 0;

		if (!GetMessage (&msg, NULL, 0, 0))
			Sys_Quit ();
      	TranslateMessage (&msg);
      	DispatchMessage (&msg);
	}
#endif
}



/*
==============================================================================

 WINDOWS CRAP

==============================================================================
*/

/*
==================
WinMain
==================
*/
#if 0
void SleepUntilInput (int time)
{

	MsgWaitForMultipleObjects(1, &tevent, FALSE, time, QS_ALLINPUT);
}
#endif

int		skipframes;

int main(int c, char **v)
{

	double		time, oldtime, newtime;
	quakeparms_t parms;
	int j;

	//	static char cwd[1024];

	//	signal(SIGFPE, floating_point_exception_handler);
	signal(SIGFPE, SIG_IGN);

	memset(&parms, 0, sizeof(parms));

	COM_InitArgv(c, v);
	parms.argc = com_argc;
	parms.argv = com_argv;

	parms.memsize = 16 * 1024 * 1024;

	j = COM_CheckParm("-mem");
	if (j)
		parms.memsize = (int)(Q_atof(com_argv[j + 1]) * 1024 * 1024);
	parms.membase = malloc(parms.memsize);

	parms.basedir = ".";
	// caching is disabled by default, use -cachedir to enable
	//	parms.cachedir = cachedir;

	//noconinput = COM_CheckParm("-noconinput");
#if 0
	if (!noconinput)
		fcntl(0, F_SETFL, fcntl(0, F_GETFL, 0) | FNDELAY);

	if (COM_CheckParm("-nostdout"))
		nostdout = 1;
#endif

	Sys_Init();

	Host_Init(&parms);

	oldtime = Sys_DoubleTime();
	while (1)
	{
		// find time spent rendering last frame
		newtime = Sys_DoubleTime();
		time = newtime - oldtime;

		Host_Frame(time);
		oldtime = newtime;
	}

}

