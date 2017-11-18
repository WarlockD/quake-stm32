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
#include "icommon.h"

//
// file IO
//
#if 0
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
void Sys_mkdir (const char *path);
#endif

class sys_file  {
public:
	using ios_base = std::ios_base;
	sys_file();
	sys_file(const char* filename, ios_base::openmode mode = ios_base::in | ios_base::binary) :sys_file() { open(filename, mode); }
	sys_file(const sys_file&) = delete; // no copy
	sys_file(sys_file&& move); // move is needed though
	~sys_file();
	// most common option in quake
	bool open(const char* filename, std::ios_base::openmode mode = ios_base::in | ios_base::binary);
	size_t read(void* dest, size_t count);
	size_t write(const void* src, size_t count);
	size_t seek(int32_t offset, ios_base::seekdir dir = ios_base::beg);
	void close();
	bool is_open() const;
	size_t file_size() const;
	static bool FileExist(const char* path); // used to detect if a file exisits
	static time_t FileTime(const char* path); 
	static void MakeDir(const char* path);
private:
	void* _handle;
};


namespace quake {
	static constexpr size_t CONSOLE_DEFAULT_BUFFER_SIZE = 1024;
	// its out here for custom error streams.  override sync on line endings
	class quake_console_buffer : public std::streambuf {
		std::array<char_type, CONSOLE_DEFAULT_BUFFER_SIZE> _buffer;
	public:
		quake_console_buffer();
	protected:
		virtual void text_out(const char* text, size_t size) = 0;
		int_type sync() override final;
		int_type overflow(int_type ch) override final;
	};
	extern std::ostream con; // this is the console out
	extern std::ostream debug; // this is the console out
	
}
//
// memory protection
//
void Sys_MakeCodeWriteable (unsigned long startaddr, unsigned long length);

//
// system IO
//
void Sys_DebugLog(const char *file, const char *fmt, ...);

void Sys_Error (const char *error, ...);
// an error will cause the entire program to exit

void Sys_Printf (const char *fmt, ...);
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