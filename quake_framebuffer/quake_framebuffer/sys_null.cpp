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
#include <io.h>  
#include <fcntl.h>  

GLFWwindow* glfw_window = NULL;
qboolean isDedicated = false;

void Sys_WriteConsole(const char* text) {
	std::cout << text;
	std::cout.flush();
	OutputDebugStringA(text);
}
void Sys_WriteConsole(const char* text, size_t size) {
	quake::fixed_string<128> str;
	str.append(text, size);
	Sys_WriteConsole(str.c_str());
}

void Sys_WriteError(const char* text) {
	std::cerr << text;
	std::cerr.flush();
	OutputDebugStringA(text);
}

/*
===============================================================================

FILE IO

===============================================================================
*/


struct sys_file_handle {
	int handle;
	//char filename[1];
};

sys_file::sys_file() : _handle((void*)-1) {}
sys_file::~sys_file() { if (_handle== (void*)-1) close(); }
sys_file::sys_file(sys_file&& move) : _handle(move._handle) {
	move._handle = (void*)-1;
}

bool sys_file::open(const char* filename, std::ios_base::openmode mode) {
	int imode = _O_RANDOM;
	assert((mode & (ios_base::in | ios_base::out)) != (ios_base::in | ios_base::out));
	if (mode & ios_base::binary) imode |= _O_BINARY ; else imode |= _O_TEXT;
	if (mode & ios_base::out) imode |= _O_WRONLY | _O_CREAT; else imode |= _O_RDONLY;
	if (mode & ios_base::ate) imode |= _O_APPEND;
	int handle = _open(filename, imode, _S_IREAD | _S_IWRITE);
	if (handle == -1) {
	//	setstate(ios_base::badbit);
		// check errno for why
		quake::con << "Could not open '" << filename << "', error " << strerror(errno) << std::endl;
		return false; 
	}
//	clear();
	_handle = (void*)handle;
	return true;
}

size_t sys_file::read(void* dest, size_t count) {
	int r = _read((int)_handle, dest, count);
	if (r == -1) {
	//	setstate(rdstate() | ios_base::failbit);
		Sys_Error("sys_file: read error");
		return 0;
	}
	else if (r == 0) {
	//	setstate(rdstate() | ios_base::eofbit);
	}
	return (size_t)r;
}
size_t sys_file::write(const void* src, size_t count) {
	int r = _write((int)_handle, src, count);
	if (r == -1) {
	//	setstate(rdstate() | ios_base::failbit);
		Sys_Error("sys_file: write error");
		return 0;
	}
	return (size_t)r;
}

void sys_file::close() {
	if ((int)_handle != -1) _close((int)_handle);
//	clear();
	_handle = (void*)-1;
}
size_t sys_file::seek(int32_t offset, ios_base::seekdir dir) {
	int r = _lseek((int)_handle, offset, dir == ios_base::end ? SEEK_END : dir == ios_base::cur ? SEEK_CUR : SEEK_SET);
	if (r == -1) {
	//	setstate(rdstate() | ios_base::failbit);
		Sys_Error("sys_file: seek error");
	}
	return r;
}
bool sys_file::is_open() const { return _handle != (void*)-1; }
size_t sys_file::file_size() const {
	struct stat s;
	int r = fstat((int)_handle, &s);
	if(r == -1) Sys_Error("sys_file: file_size error");
	return s.st_size;
}
bool sys_file::FileExist(const char* path) {
	struct stat s;
	return stat(path, &s) != -1;
}


time_t sys_file::FileTime(const char* path) {
	struct stat s;
	int r = stat(path, &s);
		if (r == -1) Sys_Error("sys_file: FileTime error");
	
	return s.st_mtime;
}
void sys_file::MakeDir(const char* path) {
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
void Sys_DebugLog(const char *filename, const char *fmt, ...)
{
	va_list argptr;
	static char data[1024];

	va_start(argptr, fmt);
	int len = vsprintf(data, fmt, argptr);
	va_end(argptr);
	assert(len >0 && len < 1024 - 1);
	if (filename == NULL) {
		Sys_Printf("DEBUG: %s", data);
	}
	else {
		//assert(fwrite(data, 1, len, fd) == len); 
		sys_file fd(filename,std::ios_base::out | std::ios_base::ate);
		fd.write(data, len);
		fd.close();
	}
};



namespace quake {
	quake_console_buffer::quake_console_buffer() { setp(_buffer.data(), _buffer.data() + _buffer.size() - 2); }
	quake_console_buffer::int_type quake_console_buffer::sync() {
		std::streamsize n = static_cast<std::streamsize>(pptr() - pbase());
		if (n > 0) {
			*pptr() = '\0';
			text_out(pbase(), n);

			pbump(-n);
			setp(_buffer.data(), _buffer.data() + _buffer.size() - 2);
		}
		return 0;// n != 0 ? EOF : 0;
	}
	quake_console_buffer::int_type quake_console_buffer::overflow(int_type ch)
	{
		if (ch != EOF)
		{
			std::streamsize n = static_cast<std::streamsize>(pptr() - pbase());
			*pptr() = char_traits::to_char_type(ch);
			pbump(1);
			sync();
		}
		return char_traits::not_eof(ch);
	}
	class console_buffer_t : public quake_console_buffer {
	public:
		console_buffer_t() : quake_console_buffer() {}
	protected:
		void text_out(const char* text, size_t size) override final {
			Sys_WriteConsole(text);
			Con_Print(text);
		}
	};
	class debug_buffer_t : public quake_console_buffer {
	public:
		debug_buffer_t() : quake_console_buffer() {}
	protected:
		void text_out(const char* text, size_t size) override final {
			Sys_WriteConsole(text);
		}
	};
	static console_buffer_t console_buffer;
	static debug_buffer_t debug_buffer;
	std::ostream con(&console_buffer); // this is the main console out

	extern std::ostream debug(&debug_buffer); // this is the console out
}


#include <iostream>
void Sys_Printf(const char *fmt, ...)
{
	va_list		argptr;
	char		text[2048];
	unsigned char		*p;

	va_start(argptr, fmt);
	int len = vsprintf(text, fmt, argptr);
	va_end(argptr);

	if (len <= 0 || len > sizeof(text))
		Sys_Error("memory overwrite in Sys_Printf");
	Sys_WriteConsole(text);
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
void Hunk_Print(bool all);

void Sys_Error(const char *error, ...)
{
	va_list     argptr;
	char        string[1024];

	// change stdin to non blocking

	va_start(argptr, error);
	int len = Q_vsprintf(string, error, argptr);
	va_end(argptr);
	Sys_Printf("Error: %s\n", string);

	Hunk_Print(false);

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

void main (int argc, const char **argv)
{
	idTime		time, oldtime, newtime;
	quakeparms_t    parms;

	parms.memsize = 8*1024*1024;
	parms.membase = malloc (parms.memsize);
	parms.basedir = ".";
	
	for (int i = 0; i < argc; i++)
		parms.COM_AddArg(argv[i]);


	//COM_GArgs.InitArgv(argc, argv);


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


