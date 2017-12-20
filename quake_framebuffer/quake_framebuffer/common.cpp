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
// common.c -- misc functions used in client and server



#include "icommon.h"

#include <cctype>
#include <unordered_set>

// time commm
template<typename T>
static bool AreEqual(T f1, T f2, T fe = std::numeric_limits<T>::epsilon()) {
	return (std::fabs(f1 - f2) <= fe * std::fmax(fabs(f1), fabs(f2)));
}

static quake::fixed_string<MAX_OSPATH> com_cachedir;
static quake::fixed_string<MAX_OSPATH> com_gamedir;

// ustl stuff

namespace quake {

	void zstring::release() {
		if (_capacity) { _capacity = 0U; Z_Free(_buffer); }
		_buffer = "";
		_size = 0U;
	}
	void zstring::reserve(size_type n) { // don't have to do anything
		if (_capacity < n) {
			// special case
			if (_capacity == 0 && _size > 0U) {
				// we are on copy on write, so need to make a new buffer and copy first
				// we have an existing buffer
				char* p = (pointer)Z_Realloc(nullptr, n, "ztring");
				strncpy(p, _buffer, _size);
				_buffer = p;
			}
			else {
				_buffer = (pointer)Z_Realloc(_size == 0U ? nullptr : _buffer, n, "ztring");
			}

			_capacity = n;
			assert(_buffer);
		}
	}
}
#define NUM_SAFE_ARGVS  7


static char     *safeargvs[NUM_SAFE_ARGVS] =
	{"-stdvid", "-nolan", "-nosound", "-nocdaudio", "-nojoy", "-nomouse", "-dibonly"};

cvar_t  registered = {"registered","0"};
cvar_t  cmdline = {"cmdline","0", false, true};
static char _com_gamedir[MAX_QPATH];
qboolean        com_modified;   // set true if using non-id files

qboolean		proghack;

int             static_registered = 1;  // only for startup check, then set

qboolean		msg_suppress_1 = 0;

void COM_InitFilesystem (void);

// if a packfile directory differs from this, it is assumed to be hacked
#define PAK0_COUNT              339
#define PAK0_CRC                32981



#define CMDLINE_LENGTH	256
char	com_cmdline[CMDLINE_LENGTH];

qboolean		standard_quake = true, rogue, hipnotic;

// this graphic needs to be in the pak file to use registered features
unsigned short pop[] =
{
 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
,0x0000,0x0000,0x6600,0x0000,0x0000,0x0000,0x6600,0x0000
,0x0000,0x0066,0x0000,0x0000,0x0000,0x0000,0x0067,0x0000
,0x0000,0x6665,0x0000,0x0000,0x0000,0x0000,0x0065,0x6600
,0x0063,0x6561,0x0000,0x0000,0x0000,0x0000,0x0061,0x6563
,0x0064,0x6561,0x0000,0x0000,0x0000,0x0000,0x0061,0x6564
,0x0064,0x6564,0x0000,0x6469,0x6969,0x6400,0x0064,0x6564
,0x0063,0x6568,0x6200,0x0064,0x6864,0x0000,0x6268,0x6563
,0x0000,0x6567,0x6963,0x0064,0x6764,0x0063,0x6967,0x6500
,0x0000,0x6266,0x6769,0x6a68,0x6768,0x6a69,0x6766,0x6200
,0x0000,0x0062,0x6566,0x6666,0x6666,0x6666,0x6562,0x0000
,0x0000,0x0000,0x0062,0x6364,0x6664,0x6362,0x0000,0x0000
,0x0000,0x0000,0x0000,0x0062,0x6662,0x0000,0x0000,0x0000
,0x0000,0x0000,0x0000,0x0061,0x6661,0x0000,0x0000,0x0000
,0x0000,0x0000,0x0000,0x0000,0x6500,0x0000,0x0000,0x0000
,0x0000,0x0000,0x0000,0x0000,0x6400,0x0000,0x0000,0x0000
};

/*


All of Quake's data access is through a hierchal file system, but the contents of the file system can be transparently merged from several sources.

The "base directory" is the path to the directory holding the quake.exe and all game directories.  The sys_* files pass this to host_init in quakeparms_t->basedir.  This can be overridden with the "-basedir" command line parm to allow code debugging in a different directory.  The base directory is
only used during filesystem initialization.

The "game directory" is the first tree on the search path and directory that all generated files (savegames, screenshots, demos, config files) will be saved to.  This can be overridden with the "-game" command line parameter.  The game directory can never be changed while quake is executing.  This is a precacution against having a malicious server instruct clients to write files over areas they shouldn't.

The "cache directory" is only used during development to save network bandwidth, especially over ISDN / T1 lines.  If there is a cache directory
specified, when a file is found by the normal search path, it will be mirrored
into the cache directory, then opened there.



FIXME:
The file "parms.txt" will be read out of the game directory and appended to the current command line arguments to allow different games to initialize startup parms differently.  This could be used to add a "-sspeed 22050" for the high quality sound edition.  Because they are added at the end, they will not override an explicit setting on the original command line.
	
*/

//============================================================================

#if 0
// ClearLink is used for new headnodes
void ClearLink (link_t *l)
{
	l->prev = l->next = l;
}

void RemoveLink (link_t *l)
{
	l->next->prev = l->prev;
	l->prev->next = l->next;
}

void InsertLinkBefore (link_t *l, link_t *before)
{
	l->next = before;
	l->prev = before->prev;
	l->prev->next = l;
	l->next->prev = l;
}
void InsertLinkAfter (link_t *l, link_t *after)
{
	l->next = after->next;
	l->prev = after;
	l->prev->next = l;
	l->next->prev = l;
}
#endif
/*
============================================================================

					LIBRARY REPLACEMENT FUNCTIONS

============================================================================
*/

void Q_memset (void *dest, int fill, size_t count)
{

	::memset(dest, fill, count);
#if 0
	int             i;
	
	if ( (((long)dest | (int)count) & 3) == 0)
	{
		count >>= 2;
		fill = fill | (fill<<8) | (fill<<16) | (fill<<24);
		for (i=0 ; i<count ; i++)
			((int *)dest)[i] = fill;
	}
	else
		for (i=0 ; i<count ; i++)
			((byte *)dest)[i] = fill;
#endif
}

void Q_memcpy (void *dest, const void * src, size_t count)
{

	::memcpy(dest, src, count);
#if 0
	int             i;
	
	if (( ( (long)dest | (long)src | (int)count) & 3) == 0 )
	{
		count>>=2;
		for (i=0 ; i<count ; i++)
			((int *)dest)[i] = ((int *)src)[i];
	}
	else
		for (i=0 ; i<count ; i++)
			((byte *)dest)[i] = ((byte *)src)[i];
#endif
}

int Q_memcmp (const void * m1, const void * m2, size_t count)
{
	return ::memcmp(m1, m2, count) == 0 ? 0: -1;
#if 0
	while(count)
	{
		count--;
		if (((byte *)m1)[count] != ((byte *)m2)[count])
			return -1;
	}
	return 0;
#endif
}

void Q_strcpy (char *dest, const char * src)
{
	while (*src)
	{
		*dest++ = *src++;
	}
	*dest++ = 0;
}

void Q_strncpy (char *dest, const char * src, size_t count)
{
	while (*src && count--)
	{
		*dest++ = *src++;
	}
	if (count)
		*dest++ = 0;
}

size_t Q_strlen (const char * str)
{
	size_t count = 0;
	while (str[count])
		count++;

	return count;
}

int Q_vsprintf(char* buffer, const char* fmt, va_list va) {
	return vsprintf(buffer, fmt, va);
}
int Q_vsnprintf(char* buffer, size_t buffer_size, const char* fmt, va_list va) {
	return vsnprintf(buffer, buffer_size,  fmt, va);
}

int Q_sprintf(char* buffer, const char* fmt, ...) {
	va_list va;
	va_start(va, fmt);
	int ret=  Q_vsprintf(buffer, fmt, va);
	va_end(va);
	return ret;
}
int Q_snprintf(char* buffer, size_t buffer_size, const char* fmt, ...) {
	va_list va;
	va_start(va, fmt);
	int ret = Q_vsnprintf(buffer, buffer_size, fmt, va);
	va_end(va);
	return ret;
}

char *Q_strrchr(char *s, char c) {

	int len = Q_strlen(s);
	s += len;
	while (len--)
		if (*--s == c) return s;
	return 0;
}
const char *Q_strrchr(const char *s, char c) {

	int len = Q_strlen(s);
	s += len;
	while (len--)
		if (*--s == c) return s;
	return 0;
}
void Q_strcat (char *dest, const char * src)
{
	dest += Q_strlen(dest);
	Q_strcpy (dest, src);
}

int Q_strcmp (const char * s1, const char * s2)
{
	while (1)
	{
		if (*s1 != *s2)
			return -1;              // strings not equal    
		if (!*s1)
			return 0;               // strings are equal
		s1++;
		s2++;
	}
	
	return -1;
}

int Q_strncmp (const char * s1, const char * s2, size_t count)
{
	while (1)
	{
		if (!count--)
			return 0;
		if (*s1 != *s2)
			return -1;              // strings not equal    
		if (!*s1)
			return 0;               // strings are equal
		s1++;
		s2++;
	}
	
	return -1;
}
#if 0
bool Q_strncasecmp(cstring_t a, cstring_t b) {
		return a.size() == b.size() && std::equal(b.begin(), b.end(), a.begin(), [](char l, char r) { return ::tolower(l) == ::tolower(r); });
}
#endif
int Q_strncasecmp (const char * s1, const char * s2, size_t n)
{
	int             c1, c2;
	
	while (1)
	{
		c1 = *s1++;
		c2 = *s2++;

		if (!n--)
			return 0;               // strings are equal until end point
		
		if (c1 != c2)
		{
			if (c1 >= 'a' && c1 <= 'z')
				c1 -= ('a' - 'A');
			if (c2 >= 'a' && c2 <= 'z')
				c2 -= ('a' - 'A');
			if (c1 != c2)
				return -1;              // strings not equal
		}
		if (!c1)
			return 0;               // strings are equal
//              s1++;
//              s2++;
	}
	
	return -1;
}
int Q_strcasecmp(const std::string_view& s1, const char * s2) {
	return Q_strncasecmp(s1.data(), s2,  s1.size());
}
int Q_strcasecmp (const char * s1, const char * s2)
{
	return Q_strncasecmp(s1, s2, 99999);
}
int Q_strcasecmp(const std::string_view& s1, const std::string_view& s2) {
	return Q_strncasecmp(s1.data(), s2.data(), std::min(s1.size(), s2.size()));
}
static const char* SkipWhiteSpace(const char* data)  {
	while (1) {
		if (data[0] <= ' ') { data++; continue; }
		if (data[0] == '/' && data[1] == '/') {
			while (*data != '\n' && *data != '\0') data++;
			continue;
		}
		break;
	}
	return data;
}



static inline int toHex(int c) { 
	if (c >= '0' && c <= '9')
		return c - '0';
	else if (c >= 'a' && c <= 'f')
		return  c - 'a' + 10;
	else if (c >= 'A' && c <= 'F')
		return  c - 'A' + 10;
	else return -1; 
}

int	Q_atoi(cstring_t str) {
	char* ptr = nullptr;
	int ret = strtol(str.data(), &ptr,0);
	assert(ptr == (str.data() + str.size())); // debug
	return ret;
}
float Q_atof(cstring_t str) {
	char* ptr;
	float ret = strtof(str.data(), &ptr);
	if (ptr == str.data()) { // not a number
		return std::numeric_limits<float>::quiet_NaN();

	}
//	assert(ptr == (str.data() + str.size()) ); // debug
	return ret;
}
int Q_atoi (const char * str)
{
	int             val;
	int             sign;
	int             c;
	
	if (*str == '-')
	{
		sign = -1;
		str++;
	}
	else
		sign = 1;
		
	val = 0;

//
// check for hex
//
	if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X') )
	{
		str += 2;
		while (1)
		{
			c = *str++;
			if (c >= '0' && c <= '9')
				val = (val<<4) + c - '0';
			else if (c >= 'a' && c <= 'f')
				val = (val<<4) + c - 'a' + 10;
			else if (c >= 'A' && c <= 'F')
				val = (val<<4) + c - 'A' + 10;
			else
				return val*sign;
		}
	}
	
//
// check for character
//
	if (str[0] == '\'')
	{
		return sign * str[1];
	}
	
//
// assume decimal
//
	while (1)
	{
		c = *str++;
		if (c <'0' || c > '9')
			return val*sign;
		val = val*10 + c - '0';
	}
	
	return 0;
}
std::string_view COM_GameDir() {
	return com_gamedir;
}

float Q_atof (const char * str)
{
	float			val;
	int             sign;
	int             c;
	int             decimal, total;
	
	if (*str == '-')
	{
		sign = -1;
		str++;
	}
	else
		sign = 1;
		
	val = 0;

//
// check for hex
//
	if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X') )
	{
		str += 2;
		while (1)
		{
			c = *str++;
			if (c >= '0' && c <= '9')
				val = (val*16) + c - '0';
			else if (c >= 'a' && c <= 'f')
				val = (val*16) + c - 'a' + 10;
			else if (c >= 'A' && c <= 'F')
				val = (val*16) + c - 'A' + 10;
			else
				return val*sign;
		}
	}
	
//
// check for character
//
	if (str[0] == '\'')
	{
		return static_cast<float>(sign * str[1]);
	}
	
//
// assume decimal
//
	decimal = -1;
	total = 0;
	while (1)
	{
		c = *str++;
		if (c == '.')
		{
			decimal = total;
			continue;
		}
		if (c <'0' || c > '9')
			break;
		val = val*10 + c - '0';
		total++;
	}

	if (decimal == -1)
		return val*sign;
	while (total > decimal)
	{
		val /= 10;
		total--;
	}
	
	return val*sign;
}

/*
============================================================================

					BYTE ORDER FUNCTIONS

============================================================================
*/

qboolean        bigendien;

short   (*BigShort) (short l);
short   (*LittleShort) (short l);
int     (*BigLong) (int l);
int     (*LittleLong) (int l);
float   (*BigFloat) (float l);
float   (*LittleFloat) (float l);

short   ShortSwap (short l)
{
	byte    b1,b2;

	b1 = l&255;
	b2 = (l>>8)&255;

	return (b1<<8) + b2;
}

short   ShortNoSwap (short l)
{
	return l;
}

int    LongSwap (int l)
{
	byte    b1,b2,b3,b4;

	b1 = l&255;
	b2 = (l>>8)&255;
	b3 = (l>>16)&255;
	b4 = (l>>24)&255;

	return ((int)b1<<24) + ((int)b2<<16) + ((int)b3<<8) + b4;
}

int     LongNoSwap (int l)
{
	return l;
}

float FloatSwap (float f)
{
	union
	{
		float   f;
		byte    b[4];
	} dat1, dat2;
	
	
	dat1.f = f;
	dat2.b[0] = dat1.b[3];
	dat2.b[1] = dat1.b[2];
	dat2.b[2] = dat1.b[1];
	dat2.b[3] = dat1.b[0];
	return dat2.f;
}

float FloatNoSwap (float f)
{
	return f;
}

/*
==============================================================================

			MESSAGE IO FUNCTIONS

Handles byte ordering and avoids alignment errors
==============================================================================
*/
// reading functions



float	sizebuf_t::ReadFloat() {
	union {
		float	f;
		int	l;
	} dat;

	if (Read(dat.l)) {
		dat.l = LittleLong(dat.l);
		return dat.f;
	}
	return -1;
}


//void	MSG_ReadDeltaUsercmd( struct usercmd_s *from, struct usercmd_s *cmd);

void	sizebuf_t::ReadDir(vec3_t& vector) {
	assert(0);
#ifdef QUAKE2
	int		b;

	b = ReadByte();
	if (b >= NUMVERTEXNORMALS)
		quake::com << "MSF_ReadDir: out of range" << std::endl;
		//Com_Error(ERR_DROP, "MSF_ReadDir: out of range");
	VectorCopy(bytedirs[b], vector);
#endif
}

size_t	sizebuf_t::ReadData(void *buffer, int len) {
	uint8_t* data = reinterpret_cast<uint8_t*>(buffer);
	while (len--) {
		int c = ReadByte();
		if (c == -1) break;
		*data++ = c;
	}
	return (size_t)(data - reinterpret_cast<uint8_t*>(buffer));
}

//
// writing functions
//

void sizebuf_t::WriteChar (int c)
{
	byte    *buf;
	
#ifdef PARANOID
	if (c < -128 || c > 127)
		Sys_Error ("MSG_WriteChar: range error");
#endif

	buf = (byte*)GetSpace (1);
	buf[0] = c;
}

void sizebuf_t::WriteByte ( int c)
{
	byte    *buf;
	
#ifdef PARANOID
	if (c < 0 || c > 255)
		Sys_Error ("MSG_WriteByte: range error");
#endif

	buf = (byte*)GetSpace(1);
	buf[0] = c;
}

void sizebuf_t::WriteShort ( int c)
{
	byte    *buf;
	
#ifdef PARANOID
	if (c < ((short)0x8000) || c > (short)0x7fff)
		Sys_Error ("MSG_WriteShort: range error");
#endif

	buf = (byte*)GetSpace ( 2);
	buf[0] = c&0xff;
	buf[1] = c>>8;
}

void sizebuf_t::WriteLong (int c)
{
	byte    *buf;
	
	buf = (byte*)GetSpace ( 4);
	buf[0] = c&0xff;
	buf[1] = (c>>8)&0xff;
	buf[2] = (c>>16)&0xff;
	buf[3] = c>>24;
}

void sizebuf_t::WriteFloat ( float f)
{
	union
	{
		float   f;
		int     l;
	} dat;
	
	
	dat.f = f;
	dat.l = LittleLong (dat.l);
	
	Write (&dat.l, 4);
}

void sizebuf_t::WriteString ( cstring_t s)
{
	if (s.empty())
		Write ("", 1);
	else
		Write (s.data(),s.size());
}

void sizebuf_t::WriteCoord ( float f)
{
	WriteShort ( (int)(f*8));
}

void sizebuf_t::WriteAngle ( float f)
{
	WriteByte ( ((int)f*256/360) & 255);
}

//
// reading functions
//
int                     msg_readcount;
qboolean        msg_badread;

void MSG_BeginReading (void)
{
	msg_readcount = 0;
	msg_badread = false;
}

// returns -1 and sets msg_badread if no more characters are available
int MSG_ReadChar (void)
{
	int     c;
	
	if (msg_readcount+1 > net_message.size())
	{
		msg_badread = true;
		return -1;
	}
		
	c = (signed char)net_message.data()[msg_readcount];
	msg_readcount++;
	
	return c;
}

int MSG_ReadByte (void)
{
	int     c;
	
	if (msg_readcount+1 > net_message.size())
	{
		msg_badread = true;
		return -1;
	}
		
	c = (unsigned char)net_message.data()[msg_readcount];
	msg_readcount++;
	
	return c;
}

int MSG_ReadShort (void)
{
	int     c;
	
	if (msg_readcount+2 > net_message.size())
	{
		msg_badread = true;
		return -1;
	}
		
	c = (short)(net_message.data()[msg_readcount]
	+ (net_message.data()[msg_readcount+1]<<8));
	
	msg_readcount += 2;
	
	return c;
}

int MSG_ReadLong (void)
{
	int     c;
	
	if (msg_readcount+4 > net_message.size())
	{
		msg_badread = true;
		return -1;
	}
		
	c = net_message.data()[msg_readcount]
	+ (net_message.data()[msg_readcount+1]<<8)
	+ (net_message.data()[msg_readcount+2]<<16)
	+ (net_message.data()[msg_readcount+3]<<24);
	
	msg_readcount += 4;
	
	return c;
}

float MSG_ReadFloat (void)
{
	union
	{
		byte    b[4];
		float   f;
		int     l;
	} dat;
	
	dat.b[0] =      net_message.data()[msg_readcount];
	dat.b[1] =      net_message.data()[msg_readcount+1];
	dat.b[2] =      net_message.data()[msg_readcount+2];
	dat.b[3] =      net_message.data()[msg_readcount+3];
	msg_readcount += 4;
	
	dat.l = LittleLong (dat.l);

	return dat.f;   
}

char *MSG_ReadString (void)
{
	static char     string[2048];
	int             l,c;
	
	l = 0;
	do
	{
		c = MSG_ReadChar ();
		if (c == -1 || c == 0)
			break;
		string[l] = c;
		l++;
	} while (l < sizeof(string)-1);
	
	string[l] = 0;
	
	return string;
}

float MSG_ReadCoord (void)
{
	return MSG_ReadShort() * (1.0f/8.0f);
}

float MSG_ReadAngle (void)
{
	return MSG_ReadChar() * (360.0f/256.0f);
}



//===========================================================================



void sizebuf_t::Print(const std::string_view&  data) {
	size_t 	len = data.size() + 1;
	if (back())
		Q_memcpy(GetSpace(len), data.data(), len); // no trailing 0
	else
		Q_memcpy(GetSpace(len - 1), data.data(), len);  // write over trailing 0
}

void sizebuf_t::Print(char c) {
	if (back())
		*((byte *)GetSpace(2)) = c; // no trailing 0
	else
		*(((byte *)GetSpace(1)) - 1) = c; // no trailing 0
}


void sizebuf_t::Clear() { _cursize = 0; _read_count=0U; }
void sizebuf_t::Free() { 
	if (_hunk_low_used) {
		Hunk_FreeToLowMark(_hunk_low_used);
		_hunk_low_used = 0;
		_capacity = 0;
		_data = nullptr;
	}
	Clear();
}
void sizebuf_t::Alloc(size_t startsize) {
	assert(!_hunk_low_used);
	if (startsize < 256) startsize = 256;
	_hunk_low_used = Hunk_LowMark(); // debuging mainly
	_data = (byte*)Hunk_AllocName(startsize, "sizebuf");
	_capacity = startsize;
	Clear();
}

void sizebuf_t::Write(const void* data, size_t length) {
	assert((length + size()) < maxsize());
	std::copy((const char*)data,(const char*)data + length, (char*)GetSpace(length));
	_cursize += length;
}
/// Shifts the data in the linked block from \p start to \p start + \p n.
/// The contents of the uncovered bytes is undefined.
void sizebuf_t::InsertArea(const_iterator cstart, size_type n)
{
	assert(data() || !n);
	assert(begin() || !n);
	assert(cstart >= begin() && cstart + n <= end());
	iterator start = const_cast<iterator>(cstart);
	std::rotate(start, end() - n, end());
}

/// Shifts the data in the linked block from \p start + \p n to \p start.
/// The contents of the uncovered bytes is undefined.
void sizebuf_t::EraseArea(const_iterator cstart, size_type n)
{
	assert(data() || !n);
	assert(begin() || !n);
	assert(cstart >= begin() && cstart + n <= end());
	iterator start = const_cast<iterator>(cstart);
	std::rotate(start, start + n, end());
}
void sizebuf_t::Insert(const void* data, size_t n, size_t pos) {
	const_iterator start(begin() + pos);
	const size_t ipmi = std::distance(const_iterator(begin()), start);
	const ptrdiff_t ip = start - const_iterator(begin());
	assert(ip <= maxsize());
	InsertArea(begin() + ip, n);
	std::copy((const byte*)data, (const byte*)data+n, begin() + ip);
	_cursize += n;
}
void* sizebuf_t::GetSpace(size_t length) {
	void    *data;

	if (_cursize + length > maxsize())
	{
		if (!_allowoverflow)
			Sys_Error("SZ_GetSpace: overflow without allowoverflow set");

		if (length >maxsize())
			Sys_Error("SZ_GetSpace: %i is > full buffer size", length);

		_overflowed = true;
		Con_Printf("SZ_GetSpace: overflow");
		Clear();
	}

	data = _data + _cursize;
	_cursize += length;
	return data;
}


inline static bool isComPunc(int c) {
	return c == '{' || c == '}' || c == ')' || c == '(' || c == '\'' || c == ':';
}


/*
==============
COM_Parse

Parse a token out of a string
==============
*/
enum class  Action {
	Accept,		// accept the charater of the token
	Goto,		// hard goto.  mainly for skipping charaters
	Halt,		// finished with all the tokens
	Restart,	// discard last accepts
	Error
};
struct Entry { Action action; int next; int arg; };

// state tables

#define STATE_TABLE 0
#define ACTION_TABLE 1
#define LOOKUP_TABLE 2
#define START 0
#define RETURN -1
#define GOTO(N)		{ Action::Goto, (N), 0 }
#define ACCEPT(N) { Action::Accept, (N), 0  }
#define RESTART(N) { Action::Restart, (N), 0  }
#define HALT(N, C) { Action::Halt, N, ((int)(C))  }
#define HALT0(C) HALT(0,C)
#define ERROR(N, C) { Action::Error,  N, ((int)(C))  }
#define ERROR0(C) ERROR(0,C)
enum class Class {
	Error=0,
	Symbol,
	Eof,
	NewLine,
	Whitespace,
	String,
	Comment,
	Discard
};
//http://hackingoff.com/compilers/scanner-generator
static constexpr const Entry lex_table_accept_eol[][7] = {
	//   S				"						/						eof					'\n'				    Other
	{ ACCEPT(1),	GOTO(2),				ACCEPT(4),			HALT0(Class::Eof),			ACCEPT(7),		ACCEPT(6) },
	// symbol state
	{ ACCEPT(1),	HALT(0,Class::Symbol),	ACCEPT(1),			HALT0(Class::Symbol),	HALT0(Class::Symbol),	HALT0(Class::Symbol) },
	// string state
	{ ACCEPT(2),	GOTO(3),				ACCEPT(2), 			ERROR0('"'),			ACCEPT(2),		ACCEPT(2) },
	{ HALT0(Class::String), HALT0(Class::String), HALT0(Class::String), HALT0(Class::String), HALT0(Class::String), HALT0(Class::String) },
	// comment
	{ ACCEPT(1),	ERROR0('"'),			ACCEPT(5),			HALT0(Class::Symbol),	HALT0(Class::Symbol),	HALT0(Class::Symbol) },
	{ ACCEPT(5),	ACCEPT(5),				ACCEPT(5),			RESTART(0),				RESTART(0),		ACCEPT(5), },
	// whitespace
	{ RESTART(0),	RESTART(0),				RESTART(0),			 	RESTART(0),			RESTART(0),				ACCEPT(6), },
	// new line
	{ HALT0(Class::NewLine),	HALT0(Class::NewLine),	  HALT0(Class::NewLine),			HALT0(Class::NewLine),		HALT0(Class::NewLine),	HALT0(Class::NewLine) },
};
static constexpr const Entry lex_table_ignore_eol[][7] = {
	//   S				"						/						eof					'\n'				    Other
	{ ACCEPT(1),	GOTO(2),				ACCEPT(4),			HALT0(Class::Eof),			ACCEPT(7),				ACCEPT(6) },
	// symbol state
	{ ACCEPT(1),	HALT(0,Class::Symbol),	ACCEPT(1),			HALT0(Class::Symbol),	HALT0(Class::Symbol),	HALT0(Class::Symbol) },
	// string state
	{ ACCEPT(2),	GOTO(3),				ACCEPT(2), 			ERROR0('"'),			ACCEPT(2),		ACCEPT(2) },
	{ HALT0(Class::String), HALT0(Class::String), HALT0(Class::String), HALT0(Class::String), HALT0(Class::String), HALT0(Class::String) },
	// comment
	{ ACCEPT(1),	ERROR0('"'),			ACCEPT(5),			HALT0(Class::Symbol),	HALT0(Class::Symbol),	HALT0(Class::Symbol) },
	{ ACCEPT(5),	ACCEPT(5),				ACCEPT(5),			RESTART(0),				RESTART(0),		ACCEPT(5), },
	// whitespace
	{ RESTART(0),	RESTART(0),				RESTART(0),			RESTART(0),			RESTART(0),				ACCEPT(6), },
	// new line
	{ RESTART(0),	RESTART(0),				RESTART(0),			RESTART(0),			RESTART(0),				RESTART(0), },
};
static std::string_view eol_token("\n");


// switched to more state based
bool COM_Parser::Next(std::string_view& token, bool test_eol){
	token = std::string_view();
	while (_pos >= _data.size()) return false;
	size_t start = _pos;
	size_t len = 0;
	int current_read;
	bool buffered = false;
	const auto& lex_table = test_eol ? lex_table_accept_eol : lex_table_ignore_eol;
	while(1){
		//    @label_codes = {"a"=>0, "\""=>1, "/"=>2, "0"=>3, "c"=>4, "b"=>5, :other=>6}
		// I could built a 256 charater table for this, but seriously, for this few tokens?
		int ch = _pos < _data.size() ? _data[_pos] : -1;

		switch (ch) { // get the charater class
		case '"': current_read = 1; break;
		case '/': current_read = 2; break;
		case '\0':
		case -1: current_read = 3; break;
		case '\n': case ';': current_read = 4; break;
		default:
			current_read = ch > 32 ? 0 : 5;
			break;
		}
		const Entry& action = lex_table[_state][current_read];
		_state = action.next;

		switch (action.action) {
		case Action::Accept:
			if (len++ == 0) 
				start = _pos;
		case Action::Goto:
			++_pos;
			break;
		case Action::Restart:
			len = 0;
			break;
		case Action::Halt:
			switch ((Class)action.arg) {
			case Class::Eof: 
				return false;
			case Class::NewLine:
				token = eol_token;
				break;
			default:
				assert(_pos > start && len > 0);
				token = _data.substr(start, len);
				break;

			}
			return true;
		default:
			assert(0);
			break; // error ugh

		} 
	};
	return false; // compiler meh
}

#if 0
	
	size_t start = _pos;
	int c;
	char* ret;
	while (_pos < _data.size()) {
		int c = _data[_pos++];
		if (c == '\n' && test_eol) {
			text = _data.substr(_pos, _pos-start);
			break;
		}
		else if (c == '/' && _data[_pos] == '/') {
			while (_pos < _data.size() && _data[_pos] != '\n') _pos++;
			start = _pos;
		} 
		else if (std::isspace(c)) {
			while (_pos < _data.size() && std::isspace(_data[_pos]) &&  _data[_pos] != '\n') _pos++;
			start = _pos;
			len = 0;
		}
		else if (c == '"') {
			while (_pos < _data.size() && _data[_pos] != '"') len++;
			text = _data.substr(_pos, _pos-start);
			++_pos;
			break;
		}
		else if (c > ' ') {
			while (_pos < _data.size() && _data[_pos] > ' ') _pos++;
			text = _data.substr(_pos, _pos-start);
			break;
		}
	}
	return text;
#endif




/*
================
COM_CheckRegistered

Looks for the pop.txt file and verifies it.
Sets the "registered" cvar.
Immediately exits out if an alternate game was attempted to be started without
being registered.
================
*/
void COM_CheckRegistered (void)
{
	unsigned short  check[128];
	size_t                     i;
	std::fstream h;
	size_t length = COM_FindFile("gfx/pop.lmp", h);
	static_registered = 0;

	if (!length)
	{
#if WINDED
	Sys_Error ("This dedicated server requires a full registered copy of Quake");
#endif
		Con_Printf ("Playing shareware version.\n");
		if (com_modified)
			Sys_Error ("You must have the registered version to use modified games");
		return;
	}
	h.read((char*)check, sizeof(check));

	for (i=0 ; i<128 ; i++)
		if (pop[i] != (unsigned short)BigShort (check[i]))
			Sys_Error ("Corrupted data file.");
	
	Cvar_Set ("cmdline", com_cmdline);
	Cvar_Set ("registered", "1");
	static_registered = 1;
	Con_Printf ("Playing registered version.\n");
}


void COM_Path_f(cmd_source_t source, const StringArgs& args);



/*
================
COM_Init
================
*/
void COM_Init (const char *basedir)
{
	byte    swaptest[2] = {1,0};

// set the byte swapping variables in a portable manner 
	if ( *(short *)swaptest == 1)
	{
		bigendien = false;
		BigShort = ShortSwap;
		LittleShort = ShortNoSwap;
		BigLong = LongSwap;
		LittleLong = LongNoSwap;
		BigFloat = FloatSwap;
		LittleFloat = FloatNoSwap;
	}
	else
	{
		bigendien = true;
		BigShort = ShortNoSwap;
		LittleShort = ShortSwap;
		BigLong = LongNoSwap;
		LittleLong = LongSwap;
		BigFloat = FloatNoSwap;
		LittleFloat = FloatSwap;
	}

	Cvar_RegisterVariable (&registered);
	Cvar_RegisterVariable (&cmdline);
	Cmd_AddCommand ("path", COM_Path_f);

	COM_InitFilesystem ();
	COM_CheckRegistered ();
}


/*
============
va

does a varargs printf into a temp buffer, so I don't need to have
varargs versions of all text functions.
FIXME: make this buffer size safe someday
============
*/


/// just for debugging
int     memsearch (byte *start, int count, int search)
{
	int             i;
	
	for (i=0 ; i<count ; i++)
		if (start[i] == search)
			return i;
	return -1;
}

/*
=============================================================================

QUAKE FILESYSTEM

=============================================================================
*/
static 	umm_stack_allocator<0xFFFF> umm_memory;


//
// in memory
//
#if 0
struct packfile_t
{
	char    name[MAX_QPATH];
	int     filepos, filelen;
} ;

struct pack_t
{
	char    filename[MAX_OSPATH];
	int             handle;
	int             numfiles;
	packfile_t		files[1]; // ,make it one continuious load
	//packfile_t      *files;
} ;
#endif
//
// on disk
//
struct dpackfile_t
{
	char    name[56];
	int      filepos, filelen;
} ;

struct dpackheader_t
{
	char    id[4];
	int             dirofs;
	int             dirlen;
} ;

#define MAX_FILES_IN_PACK       2048



struct pack_t;
struct search_value_t {
	pack_t* pack; 
	string_t filename;
	int offset;
	int length;
	sys_file handler;
};


struct pack_t {
	int file_count;
	string_t filename;
	search_value_t files[1];
	static void operator delete(void*) {} // itsloaded as a hunk
};


#include <unordered_set>


using pack_ptr_t = std::unique_ptr<pack_t>;

using unordered_map_t = std::unordered_map<string_t, std::reference_wrapper<search_value_t>>;
static unordered_map_t com_packfilesearch;
static ZVector<string_t> com_searchpaths;


static std::unordered_map<std::string_view, std::unique_ptr<pack_t>> com_packfiles;
static void AddSearchPath(const std::string_view& dir) {
#if 0
	char* cptr = (char*)Hunk_Alloc(sizeof(searchpath_t) + dir.size());
	searchpath_t* ptr = new(cptr) searchpath_t;
	dir.copy(ptr->_filename, dir.size());
	ptr->_filename[dir.size()] = '\0';
	ptr->next = com_searchpaths;
	com_searchpaths = ptr;
	return ptr;
#else
	com_searchpaths.emplace_back(dir);

#endif
}
/*
=================
COM_LoadPackFile

Takes an explicit (not game tree related) path to a pak file.

Loads the header and directory, adding the files at the beginning
of the list so they override previous pack files.
=================
*/
static bool COM_LoadPackFile(const std::string_view& packfile)
{
	dpackheader_t   header;
	//dpackfile_t             info[MAX_FILES_IN_PACK];
	unsigned short          crc;
	quake::fixed_string_stream<MAX_OSPATH> c_helper;

	if (com_packfiles.find(packfile) != com_packfiles.end()) return true; // already loaded
	c_helper << packfile;
	sys_file file(c_helper.str().c_str());
	if (!file.is_open())
	{
		Con_Printf("Couldn't open %s\n", packfile);
		return false;
	}
	file.read((char*)&header, sizeof(header));

	if (header.id[0] != 'P' || header.id[1] != 'A'
		|| header.id[2] != 'C' || header.id[3] != 'K')
		Sys_Error("%s is not a packfile", packfile);
	header.dirofs = LittleLong(header.dirofs);
	header.dirlen = LittleLong(header.dirlen);

	size_t numpackfiles = header.dirlen / sizeof(dpackfile_t);

	if (numpackfiles > MAX_FILES_IN_PACK)
		Sys_Error("%s has %i files", packfile, numpackfiles);

	if (numpackfiles != PAK0_COUNT)
		com_modified = true;    // not the original file

	file.seek(header.dirofs);

	// crc the directory to check for modifications
	CRC_Init(&crc);
	pack_t* pack = new((pack_t*)Hunk_AllocName(sizeof(pack_t) + (numpackfiles - 1) * sizeof(search_value_t), "packfile")) pack_t;
	pack->filename = string_t::intern(packfile);
	pack->file_count = numpackfiles;

	com_packfiles.emplace(pack->filename, pack);
	for (int i = 0; i < numpackfiles; i++) {
		search_value_t& psearch = pack->files[i];
		dpackfile_t pfile;
		file.read((char*)&pfile, sizeof(dpackfile_t));
		CRC_ProcessStruct(&crc, &pfile);
		pfile.filepos = LittleLong(pfile.filepos);
		pfile.filelen = LittleLong(pfile.filelen);
		assert(pfile.name[55] == 0);
		psearch.filename = string_t::intern(pfile.name);
		psearch.length = pfile.filelen;
		psearch.offset = pfile.filepos;
		psearch.pack = pack;


		if (com_packfilesearch.find(psearch.filename) != com_packfilesearch.end()) {
			quake::con << "dup pack file: '" << psearch.filename << "'" << std::endl;
		}
		else
			com_packfilesearch.emplace(psearch.filename, std::ref(pack->files[i]));
	}
	file.close();
	quake::con << "Added packfile " << packfile << " (" << numpackfiles << " files)" << std::endl;
	return true;
}


/*
============
COM_Path_f

============
*/
void COM_Path_f(cmd_source_t source, const StringArgs& args)
{
	Con_Printf ("Current search path:\n");
	for (const auto& it : com_packfilesearch) {
		quake::con << it.second.get().filename << std::endl;
	}
}

/*
============
COM_WriteFile

The filename will be prefixed by the current game directory
============
*/
void COM_WriteFile (const std::string_view& filename, const void * data, int len)
{
	quake::fixed_string_stream<MAX_OSPATH> name;

	name << com_gamedir << '/' << filename;

	quake::ofstream file(name.str().c_str());
	if (file.bad())
	{
		quake::con << "COM_WriteFile: failed on " << name.str() << std::endl;
	}
	else {
		quake::con << "COM_WriteFile: " << name.str() << std::endl;
		file.write((const char*)data, len);
		file.close();
	}
}


/*
============
COM_CreatePath

Only used for CopyFile
============
*/
void    COM_CreatePath(const char *path)
{
	ZString ofs = path;
	for (size_t i = 1; i < ofs.size(); i++) {
		if (ofs[i] == '/')
		{       // create the directory
			ofs[i] = 0;
			sys_file::MakeDir(ofs.data());
			ofs[i] = '/';
		}
	}
}


/*
===========
COM_CopyFile

Copies a file over from the net to the local cache, creating any directories
needed.  This is for the convenience of developers using ISDN from home.
===========
*/
void COM_CopyFile (const char *netpath, const char *cachepath)
{
	quake::ifstream in(netpath);
	quake::ofstream out(cachepath);
	out << in.rdbuf();
	out.flush();
}

/*
===========
COM_FindFile

Finds the file in the search path.
Sets com_filesize and one of handle or file
===========
*/


size_t COM_FindFile(const std::string_view& filename, std::fstream& stream)
{
	quake::fixed_string_stream<MAX_OSPATH>   cachepath;
	quake::fixed_string_stream<MAX_OSPATH>   netpath;
#if 0
	if (file && handle)
		Sys_Error("COM_FindFile: both handle and file set");
	if (!file && !handle)
		Sys_Error("COM_FindFile: neither handle or file set");
#endif
	//
	// search through the path, one element at a time
	//
	auto& it = com_packfilesearch.find(filename);
	if (it != com_packfilesearch.end()) { // its a pack file
		auto& pfile = it->second.get();		
		
	//	quake::con << "PackFile:" << pfile.handler->filename << "(" << pfile.offset << "," << pfile.length << "): " << pfile.filename << std::endl;
		stream.open(pfile.pack->filename.c_str(), std::ifstream::binary | std::ifstream::in);
		stream.seekg(pfile.offset);
		return pfile.length;
	}
	else { // not a pack file
		// hack for now
		for (const auto& path : com_searchpaths) {
			netpath.clear();
			netpath << path << '/' << filename;
			auto test = netpath.str();
			stream.open(test.c_str(), std::ifstream::binary | std::ifstream::in | std::ifstream::ate);
			if (stream.is_open()) {
				size_t len = stream.tellg();
				stream.seekg(0);
				return len;
			}
#if 0
			findtime = Sys_FileTime(netpath.data());
			if (findtime == -1) continue;
			quake::con << "FindFile: " << netpath << std::endl;
			//int handle;
			//com_filesize = Sys_FileOpenRead(netpath.data(), &handle);
			return &id_ifstream(netpath.data());
#endif
		}
	}
	quake::con << "FindFile: can't find " << filename << std::endl;


	return 0;
}




/*
===========
COM_OpenFile

filename never has a leading slash, but may contain directory walks
returns a handle and a length
it may actually be inside a pak file
===========
*/


enum class AllocType {
	Hunk,
	Temp,
	Zmalloc,
	Cache,
	Stack
};
/*
============
COM_LoadFile

Filename are reletive to the quake directory.
Allways appends a 0 byte.
============
*/
cache_user_t *loadcache;
byte    *loadbuf;
int             loadsize;

static byte *COM_LoadFile (const std::string_view& path, int usehunk, size_t* file_size=nullptr)
{
	byte    *buf = nullptr;  // quiet compiler warning
// look for it in the filesystem or pack files
	std::fstream is;
	size_t len = COM_FindFile(path, is);
	if (!len) return nullptr;
	if (file_size) *file_size = len;
// extract the filename base name for hunk tag
	auto base = COM_FileBase (path);
	
	if (usehunk == 1)
		buf = (byte*)Hunk_AllocName (len+1, base);
	else if (usehunk == 2)
		buf = (byte*)Hunk_TempAlloc (len+1);
	else if (usehunk == 0)
		buf = (byte*)Z_Malloc (len+1);
	else if (usehunk == 3)
		buf = (byte*)Cache_Alloc (loadcache, len+1, base);
	else if (usehunk == 4)
	{
		if (len+1 > loadsize)
			buf = (byte*)Hunk_TempAlloc (len+1);
		else
			buf = loadbuf;
	}
	else
		Sys_Error ("COM_LoadFile: bad usehunk");

	if (!buf)
		Sys_Error ("COM_LoadFile: not enough space for %s", path);
		
	((byte *)buf)[len] = 0;

	Draw_BeginDisc ();
	// ok debug this thing
#if 0
	for (size_t i = 0; i < len; i++) {
		int c = is->get();
		assert(c >= 0);
		buf[i] = (uint8_t)c;
	}
#else
	is.read((char*)buf, len);
#endif
	//std::ios_base

	//assert(is->good());
	is.close();
	Draw_EndDisc ();

	return buf;
}
byte *COM_LoadZFile(const std::string_view& path, size_t* file_size )
{
	return COM_LoadFile(path, 0);
}

byte *COM_LoadHunkFile (const std::string_view& path, size_t* file_size)
{
	return COM_LoadFile (path, 1, file_size);
}

byte *COM_LoadTempFile (const std::string_view& path)
{
	return COM_LoadFile (path, 2);
}

void COM_LoadCacheFile (const std::string_view& path, struct cache_user_t *cu)
{
	loadcache = cu;
	COM_LoadFile (path, 3);
}

// uses temp hunk if larger than bufsize
byte *COM_LoadStackFile (const std::string_view& path, void *buffer, int bufsize)
{
	byte    *buf;
	
	loadbuf = (byte *)buffer;
	loadsize = bufsize;
	buf = COM_LoadFile (path, 4);
	
	return buf;
}


/*
================
COM_AddGameDirectory

Sets com_gamedir, adds the directory to the head of the path,
then loads and adds pak1.pak pak2.pak ... 
================
*/
static void COM_AddGameDirectory(const std::string_view& dir)
{
	AddSearchPath(dir);

	//
	// add any pak files in the format pak0.pak pak1.pak, ...
	//
	quake::fixed_string_stream<MAX_OSPATH> path;
	for (int i = 0; ; i++)
	{
		path.clear();
		path << dir << "/pak" << i << ".pak";
		if (!COM_LoadPackFile(path.str())) break;
	}

}


/*
================
COM_InitFilesystem
================
*/
void COM_InitFilesystem(void)
{
	size_t i;
	
	quake::fixed_string<MAX_OSPATH> basedir;
	cstring_t value;
	//
	// -basedir <path>
	// Overrides the system supplied base directory (under GAMENAME)
	//
	if (host_parms.COM_CheckParmValue("-basedir", value) != 0)
		basedir = value;
	else
		basedir = host_parms.basedir;

	if (!basedir.empty() && (basedir.back() == '\\' || basedir.back() == '/'))
		basedir.pop_back();

	//
	// -cachedir <path>
	// Overrides the system supplied cache directory (NULL or /qcache)
	// -cachedir - will disable caching.
	//
	com_cachedir = "";
	if (host_parms.COM_CheckParmValue("-cachedir", value) != 0) {
		if (!value.empty() && value[0] != '-')
			com_cachedir = value;
	}
	else if (host_parms.cachedir)
		com_cachedir = host_parms.cachedir;
	//
	// start up with GAMENAME by default (id1)
	//

	if (!basedir.empty() && basedir.back() != '/')
		basedir.push_back('/');
	size_t slash_point = basedir.size();
	basedir.append(GAMENAME);
	COM_AddGameDirectory(basedir);

	if (host_parms.COM_CheckParm("-rogue")) {
		basedir.erase(slash_point);
		basedir.append("rogue");
		COM_AddGameDirectory(basedir);
	}
	if (host_parms.COM_CheckParm("-hipnotic")) {
		basedir.erase(slash_point);
		basedir.append("hipnotic");
		COM_AddGameDirectory(basedir);
	}

	//
	// -game <gamedir>
	// Adds basedir/gamedir as an override game
	//
	if (host_parms.COM_CheckParmValue("-game", value) != 0) {
		{
			com_modified = true;
			basedir.erase(slash_point);
			basedir.append(value);
			COM_AddGameDirectory(basedir);
		}
	}
	//
	// -path <dir or packfile> [<dir or packfile>] ...
	// Fully specifies the exact serach path, overriding the generated one
	//
	if ((i = host_parms.COM_CheckParm("-path")) != 0)
	{
		com_modified = true;
		std::string_view file_name;
		for (size_t j = i + 1; i < host_parms.args.size(); i++) {
			auto it = host_parms.args[j];
			if (!it.empty() || it.at(0) == '+' || it.at(0) == '-') {
				file_name = std::string_view(it);
				break;
			}

		}
		if (COM_FileExtension(file_name) == "pak")
		{
			COM_LoadPackFile(file_name);
#if 0
			if (!search->pack)
				quake::con << "Couldn't load packfile: " << *it << std::endl;
#endif
		}
		else
			AddSearchPath(file_name);
	}

	if (host_parms.COM_CheckParm("-proghack"))
		proghack = true;
}


