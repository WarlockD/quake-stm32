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

#include "quakedef.h"
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
	const char* printf_buffer::operator()(const char* fmt, ...) {
		va_list va;
		va_start(va, fmt);
		int len = Q_vsnprintf(data(), size(), fmt, va);
		va_end(va);
		if (len <= 0) return nullptr;
		return c_str();
	}
	/// \brief Attaches the object to pointer \p p of size \p n.
	///
	/// If \p p is nullptr and \p n is non-zero, bad_alloc is thrown and current
	/// state remains unchanged.
	///
	void cmemlink::link(const void* p, size_type n)
	{
		if (!p && n)
			throw std::exception();
		unlink();
		relink(p, n);
	}

	/// Compares to memory block pointed by l. Size is compared first.
	bool cmemlink::operator== (const cmemlink& l) const noexcept
	{
		return l._size == _size &&
			(l._data == _data || 0 == memcmp(l._data, _data, _size));
	}


	/// Fills the linked block with the given pattern.
	/// \arg start   Offset at which to start filling the linked block
	/// \arg p       Pointer to the pattern.
	/// \arg elSize  Size of the pattern.
	/// \arg elCount Number of times to write the pattern.
	/// Total number of bytes written is \p elSize * \p elCount.
	///
	void memlink::fill(const_iterator cstart, const void* p, size_type elSize, size_type elCount) noexcept
	{
		assert(data() || !elCount || !elSize);
		assert(cstart >= begin() && cstart + elSize * elCount <= end());
		iterator start = const_cast<iterator>(cstart);
		if (elSize == 1)
			std::fill_n(start, elCount, *reinterpret_cast<const uint8_t*>(p));
		else while (elCount--)
			start = std::copy_n(const_iterator(p), elSize, start);
	}



}
#ifdef USE_CUSTOM_IDTIME
//https://stackoverflow.com/questions/4548004/how-to-correctly-and-standardly-compare-floats
template<typename T>

idTime::idTime() : _time(0.0f) {}
idTime::idTime(float f) : _time(f) {}
idTime::idTime(double f) : _time(static_cast<float>(f)) {}
idTime::operator float() const { return _time; }
idTime& idTime::operator+=(const idTime& r) { _time += r._time; return *this; }
idTime& idTime::operator-=(const idTime& r) { _time -= r._time; return *this; }
bool idTime::operator==(const idTime& r) const { return AreEqual(_time, r._time); }
bool idTime::operator!=(const idTime& r) const { return !AreEqual(_time, r._time); }
bool idTime::operator<(const idTime& r) const { return _time < r._time; }
bool idTime::operator>(const idTime& r) const { return _time < r._time; }
bool idTime::operator<=(const idTime& r) const { return operator==(r) || _time < r._time; }
bool idTime::operator>=(const idTime& r) const { return operator==(r) || _time < r._time; }
idTime& idTime::operator+=(float r) { _time += r; return *this; }
idTime& idTime::operator-=(float r) { _time -= r; return *this; }
idTime idTime::Seconds(int sec) { return idTime(static_cast<float>(sec)); }
idTime idTime::MilliSeconds(int msec) { return idTime(static_cast<float>(msec) * 1000); }
idTime idTime::UilliSeconds(int msec) { return idTime(static_cast<float>(msec) * 1000000); }
#endif


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

/*
============================================================================

					LIBRARY REPLACEMENT FUNCTIONS

============================================================================
*/

void Q_memset (void *dest, int fill, size_t count)
{
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
}

void Q_memcpy (void *dest, const void * src, size_t count)
{
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
}

int Q_memcmp (const void * m1, const void * m2, size_t count)
{
	while(count)
	{
		count--;
		if (((byte *)m1)[count] != ((byte *)m2)[count])
			return -1;
	}
	return 0;
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
bool Q_strncasecmp(const quake::string_view& a, const quake::string_view& b) {
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
int Q_strcasecmp(const quake::string_view& s1, const char * s2) {
	int c2 = -1;
	for(auto c1 : s1) {
		int c2 = *s2++; ;
		if (c2 == '\0' || std::tolower(c1) != std::tolower(c2)) return -1;
	}
	return *s2 == '\0' ? 0 : -1;
}
int Q_strcasecmp (const char * s1, const char * s2)
{
	return Q_strncasecmp (s1, s2, 99999);
}
int Q_strcasecmp(const quake::string_view& s1, const quake::string_view& s2) {
	return std::equal(s1.begin(), s1.end(), s2.begin(), s2.end(), [](char c1, char c2) { return std::tolower(c1) == std::tolower(c2);  }) ? 0 : -1;
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

int	Q_atoi(const quake::string_view& str) {
	char* ptr = nullptr;
	int ret = strtol(str.data(), &ptr,0);
	assert(str.data() < ptr);
	return ret;
}
float Q_atof(const quake::string_view& str) {
	char* ptr;
	float ret = strtof(str.data(), &ptr);
	assert(str.data() < ptr);
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
quake::string_view COM_GameDir() {
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

//
// writing functions
//

void MSG_WriteChar (sizebuf_t *sb, int c)
{
	byte    *buf;
	
#ifdef PARANOID
	if (c < -128 || c > 127)
		Sys_Error ("MSG_WriteChar: range error");
#endif

	buf = (byte*)SZ_GetSpace (sb, 1);
	buf[0] = c;
}

void MSG_WriteByte (sizebuf_t *sb, int c)
{
	byte    *buf;
	
#ifdef PARANOID
	if (c < 0 || c > 255)
		Sys_Error ("MSG_WriteByte: range error");
#endif

	buf = (byte*)SZ_GetSpace (sb, 1);
	buf[0] = c;
}

void MSG_WriteShort (sizebuf_t *sb, int c)
{
	byte    *buf;
	
#ifdef PARANOID
	if (c < ((short)0x8000) || c > (short)0x7fff)
		Sys_Error ("MSG_WriteShort: range error");
#endif

	buf = (byte*)SZ_GetSpace (sb, 2);
	buf[0] = c&0xff;
	buf[1] = c>>8;
}

void MSG_WriteLong (sizebuf_t *sb, int c)
{
	byte    *buf;
	
	buf = (byte*)SZ_GetSpace (sb, 4);
	buf[0] = c&0xff;
	buf[1] = (c>>8)&0xff;
	buf[2] = (c>>16)&0xff;
	buf[3] = c>>24;
}

void MSG_WriteFloat (sizebuf_t *sb, float f)
{
	union
	{
		float   f;
		int     l;
	} dat;
	
	
	dat.f = f;
	dat.l = LittleLong (dat.l);
	
	SZ_Write (sb, &dat.l, 4);
}

void MSG_WriteString (sizebuf_t *sb, const char *s)
{
	if (!s)
		SZ_Write (sb, "", 1);
	else
		SZ_Write (sb, s, Q_strlen(s)+1);
}

void MSG_WriteCoord (sizebuf_t *sb, float f)
{
	MSG_WriteShort (sb, (int)(f*8));
}

void MSG_WriteAngle (sizebuf_t *sb, float f)
{
	MSG_WriteByte (sb, ((int)f*256/360) & 255);
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
	
	if (msg_readcount+1 > net_message.cursize)
	{
		msg_badread = true;
		return -1;
	}
		
	c = (signed char)net_message.data[msg_readcount];
	msg_readcount++;
	
	return c;
}

int MSG_ReadByte (void)
{
	int     c;
	
	if (msg_readcount+1 > net_message.cursize)
	{
		msg_badread = true;
		return -1;
	}
		
	c = (unsigned char)net_message.data[msg_readcount];
	msg_readcount++;
	
	return c;
}

int MSG_ReadShort (void)
{
	int     c;
	
	if (msg_readcount+2 > net_message.cursize)
	{
		msg_badread = true;
		return -1;
	}
		
	c = (short)(net_message.data[msg_readcount]
	+ (net_message.data[msg_readcount+1]<<8));
	
	msg_readcount += 2;
	
	return c;
}

int MSG_ReadLong (void)
{
	int     c;
	
	if (msg_readcount+4 > net_message.cursize)
	{
		msg_badread = true;
		return -1;
	}
		
	c = net_message.data[msg_readcount]
	+ (net_message.data[msg_readcount+1]<<8)
	+ (net_message.data[msg_readcount+2]<<16)
	+ (net_message.data[msg_readcount+3]<<24);
	
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
	
	dat.b[0] =      net_message.data[msg_readcount];
	dat.b[1] =      net_message.data[msg_readcount+1];
	dat.b[2] =      net_message.data[msg_readcount+2];
	dat.b[3] =      net_message.data[msg_readcount+3];
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

void SZ_Alloc (sizebuf_t *buf, int startsize)
{
	if (startsize < 256)
		startsize = 256;
	buf->data = (byte*)Hunk_AllocName (startsize, "sizebuf");
buf->maxsize = startsize;
buf->cursize = 0;
}


void SZ_Free(sizebuf_t *buf)
{
	//      Z_Free (buf->data);
	//      buf->data = NULL;
	//      buf->maxsize = 0;
	buf->cursize = 0;
}

void SZ_Clear(sizebuf_t *buf)
{
	buf->cursize = 0;
}

void *SZ_GetSpace(sizebuf_t *buf, int length)
{
	void    *data;

	if (buf->cursize + length > buf->maxsize)
	{
		if (!buf->allowoverflow)
			Sys_Error("SZ_GetSpace: overflow without allowoverflow set");

		if (length > buf->maxsize)
			Sys_Error("SZ_GetSpace: %i is > full buffer size", length);

		buf->overflowed = true;
		Con_Printf("SZ_GetSpace: overflow");
		SZ_Clear(buf);
	}

	data = buf->data + buf->cursize;
	buf->cursize += length;

	return data;
}

void SZ_Write(sizebuf_t *buf, const void *data, int length)
{
	Q_memcpy(SZ_GetSpace(buf, length), data, length);
}



void SZ_Print(sizebuf_t *buf, const  quake::string_view& data)
{
	size_t 	len = data.size() + 1;
	if (buf->data[buf->cursize - 1])
		Q_memcpy((byte *)SZ_GetSpace(buf, len), data.data(), len); // no trailing 0
	else
		Q_memcpy((byte *)SZ_GetSpace(buf, len - 1) - 1, data.data(), len); // write over trailing 0
}

void SZ_Print(sizebuf_t *buf, char c) {
	if (buf->data[buf->cursize - 1])
		*((byte *)SZ_GetSpace(buf, 2)) = c; // no trailing 0
	else
		*(((byte *)SZ_GetSpace(buf, 1)) - 1) = c; // no trailing 0
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
// switched to more state based
COM_Parser::COM_Token COM_Parser::Next() {
	COM_Token token;
	size_t len = 0;
	int c;
	while (!_data.empty()) {
		int c = _data[len++];
		switch (c) {
		case '"':
			while (len < _data.size() && _data[len] != '\"') len++;
			token = COM_Token(Kind::Symbol, _data.substr(1U, len - 2));
			len++; // skip the '\"'
			break;
		case '/':
			if (_mode & IgnoreComments) {
				while (len < _data.size() && _data[len] != '\n') len++;
				_data.remove_prefix(len);
				len = 0; // go back to check the line feed
				continue;
			} else
				token = COM_Token(c);
			break;
		case '\n':
			if (_mode & IgnoreNewLine) continue;
			token = COM_Token(Kind::LineFeed);
			break;
		case ' ': case '\t': case '\r': case '\f':
			while (len < _data.size() && _data[len] == ' ' && _data[len] == '\t') len++;
			_data.remove_prefix(len);
			len = 0; // go back to check the line feed
			continue;
		default:
			while (len < _data.size()  && _data[len] > 32) len++;
			c = std::isdigit(c) ? (int)Kind::Number : std::isalpha(c) ? (int)Kind::Symbol : c;
			token = COM_Token(c, _data.substr(0, len));
		};
		break;
	}
	if(len > 0) _data.remove_prefix(len);
	return token;
#if 0
	while (_data.size() < len && _data[len++] > 32);

	if (len > 0) {
		COM_Token token = COM_Token(Kind::Symbol, quake::string_view(_data.data(), len));
		_data.remove_prefix(len + 2);
		return token;
	}
	else return false;

#endif
	return token;
}


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

	auto h = COM_OpenFile("gfx/pop.lmp",i);
	static_registered = 0;

	if (!h)
	{
#if WINDED
	Sys_Error ("This dedicated server requires a full registered copy of Quake");
#endif
		Con_Printf ("Playing shareware version.\n");
		if (com_modified)
			Sys_Error ("You must have the registered version to use modified games");
		return;
	}
	h->read((char*)check, sizeof(check));

	for (i=0 ; i<128 ; i++)
		if (pop[i] != (unsigned short)BigShort (check[i]))
			Sys_Error ("Corrupted data file.");
	
	Cvar_Set ("cmdline", com_cmdline);
	Cvar_Set ("registered", "1");
	static_registered = 1;
	Con_Printf ("Playing registered version.\n");
}


void COM_Path_f(cmd_source_t source, size_t argc, const quake::string_view argv[]);



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
#if 0
char    *va(char *format, ...)
{
	va_list         argptr;
	static char             string[1024];
	
	va_start (argptr, format);
	Q_vsprintf (string, format,argptr);
	va_end (argptr);

	return string;  
}
#endif

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

namespace quake {

	bool operator==(const symbol& l, const string_view& r) {
		if (l.size() != r.size()) return false;
		for (size_t i = 0; i < l.size(); i++)
			if (std::tolower(l[i]) != std::tolower(r[i])) return false;
		return true;
	}
	std::ostream& operator<<(std::ostream& os, const string_view& sv) {
		os.write(sv.data(), sv.size());
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const symbol& sv) {
		// can't just write a name, have to make sure its lowercase
		for (auto c : sv) os << (char)std::tolower(c);
		return os;
	}

	static  std::streambuf::pos_type STREAM_ERROR_RETURN = std::streambuf::pos_type(std::streambuf::off_type(-1)); 
	stream_buffer::pos_type stream_buffer::_seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode) {
			if (!_file.is_open())
				return pos_type(off_type(-1));



			off_type new_off = 0;
			switch (dir) {
			case std::ios_base::beg:
				new_off = off;
				break;
			case std::ios_base::cur:
				new_off += off;
				break;
			case std::ios_base::end:
				new_off = _length + off;
				break;
			}

			if (new_off < 0 || new_off >= _length) return pos_type(off_type(-1));
			_current = _file.seek(new_off + _offset);
			if (_mode & std::ios_base::in) {
				setg(nullptr, nullptr, nullptr);
			}
			else if (_mode & std::ios_base::out) {
				sync();
				setp(nullptr, nullptr, nullptr);
			}
			else
				return STREAM_ERROR_RETURN;

			if (_offset !=   0) {
				if (_current >= (_offset + _length)) {
					Sys_Error("file_read_stream_buffer: past override");
					return STREAM_ERROR_RETURN;
				}
				_current -= _offset;
			}
			return pos_type(_current);
	}
	stream_buffer::pos_type stream_buffer::seekpos(pos_type pos, std::ios_base::openmode which)  { return _seekoff(pos, std::ios_base::beg, which); }
	std::streampos stream_buffer::seekoff(std::streamoff off, std::ios_base::seekdir way, std::ios_base::openmode which) { return _seekoff(off, way, which); }
	bool stream_buffer::open(const char* filename, std::ios_base::openmode mode) {
		_offset = 0; _length = 0; 
		_current = 0;
		setg(nullptr, nullptr, nullptr); // flush buffer
		setp(nullptr, nullptr, nullptr); // flush buffer
		if (_file.open(filename, mode)) {
			_length = _file.file_size();
			return true;
		} return false;
	}
	void stream_buffer::close() {
		sync();
		_file.close();
	}
	int stream_buffer::sync()  {
		if (_mode & std::ios_base::out && pptr() >  pbase()) {
			size_t size = pptr() - pbase();
			_current += _file.write(pbase(), size);
			setp(_buffer.data(), _buffer.data(), _buffer.data()+ _buffer.size()); // flush buffer
		}
		return 0;
	}
	void stream_buffer::set_offset(off_type offset, off_type length) {
		sync();
		_offset = offset; _length = length; _current = 0;
		_file.seek(_offset);
		setg(nullptr, nullptr, nullptr); // flush buffer
	}
	stream_buffer::int_type stream_buffer::underflow()  {
		// only reason to get here is if the buffer is empty
		if (_current >= _length) return -1;
		size_t size = std::min(_buffer.size(), size_t(_length - _current));
		_current+= _file.read(_buffer.data(), size);
		setg(_buffer.data(), _buffer.data(), _buffer.data() + size); // flush buffer
		return traits_type::to_int_type(_buffer[0]);
	}
	stream_buffer::int_type stream_buffer::overflow(int_type c) {
		sync();
		return this->sputc(c);
	}

};

int     com_filesize;


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
	pack_t* handler;
	int offset;
	int length;
	char filename[MAX_QPATH];
	quake::ifstream& getstream();
	operator quake::string_view() const { return (const char*)filename; }
};


struct pack_t {
	int file_count;
	quake::ifstream handler;
	char filename[MAX_OSPATH];
	search_value_t files[1];
	operator quake::string_view() const { return (const char*)filename; }
	static void operator delete(void*) {} // itsloaded as a hunk
};

quake::ifstream& search_value_t::getstream() {
	assert(handler);
	auto& stream = handler->handler;

	if (!stream.is_open()) {
		stream.open(handler->filename);// | std::ios_base::binary);
		if (!stream.is_open()) Sys_Error("Cannot open pack file!");
	}
	stream.set_offset(offset, length);
	return stream;
}
#include <unordered_set>


using pack_ptr_t = std::unique_ptr<pack_t>;
using filename_ptr_t = std::unique_ptr<std::string>;
using unordered_map_t = std::unordered_map<quake::string_view, std::reference_wrapper<search_value_t>>;
static unordered_map_t com_packfilesearch;
static std::unordered_set<std::string> com_searchpaths;


static std::unordered_map<quake::string_view, std::unique_ptr<pack_t>> com_packfiles;
static void AddSearchPath(const quake::string_view& dir) {
#if 0
	char* cptr = (char*)Hunk_Alloc(sizeof(searchpath_t) + dir.size());
	searchpath_t* ptr = new(cptr) searchpath_t;
	dir.copy(ptr->_filename, dir.size());
	ptr->_filename[dir.size()] = '\0';
	ptr->next = com_searchpaths;
	com_searchpaths = ptr;
	return ptr;
#else
	com_searchpaths.emplace(dir.data(),dir.size());

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
static bool COM_LoadPackFile(const quake::string_view& packfile)
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
	packfile.copy(pack->filename, sizeof(pack->filename));
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
		::strncpy(psearch.filename, pfile.name, sizeof(psearch.filename) - 1);
		psearch.length = pfile.filelen;
		psearch.offset = pfile.filepos;
		psearch.handler = pack;

		quake::string_view filename(psearch.filename);


		if (com_packfilesearch.find(filename) != com_packfilesearch.end()) {
			quake::con << "dup pack file: '" << filename << "'" << std::endl;
		}
		else
			com_packfilesearch.emplace(filename, std::ref(pack->files[i]));
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
void COM_Path_f(cmd_source_t source, size_t argc, const quake::string_view argv[])
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
void COM_WriteFile (const quake::string_view& filename, const void * data, int len)
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


static quake::ifstream* COM_FindFile(const quake::string_view& filename)
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
		quake::con << "PackFile:" << pfile.handler->filename << "(" << pfile.offset << "," << pfile.length << "): " << pfile.filename << std::endl;
		return &pfile.getstream();
	}
	else { // not a pack file
		// hack for now
		static quake::ifstream temp_file;
		temp_file.close();
		for (const auto& path : com_searchpaths) {
			netpath.clear();
			netpath << path << '/' << filename;
			temp_file.open(netpath.str().c_str());
			if (temp_file.is_open()) return &temp_file;
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
	quake::con << "FindFile: can't find " << filename;

	return nullptr;
}




/*
===========
COM_OpenFile

filename never has a leading slash, but may contain directory walks
returns a handle and a length
it may actually be inside a pak file
===========
*/
std::istream* COM_OpenFile(const quake::string_view& filename, size_t& length)
{
	auto ret = COM_FindFile (filename);
	if (ret == nullptr) return nullptr;
	length = ret->file_length();
	return ret;
}



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

static byte *COM_LoadFile (const quake::string_view& path, int usehunk)
{
	byte    *buf = nullptr;  // quiet compiler warning
	size_t len;
// look for it in the filesystem or pack files
	auto is = COM_FindFile(path);
	if (is == nullptr)
		return nullptr;
	len = is->file_length();
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
	is->read((char*)buf, len);
#endif
	//std::ios_base

	//assert(is->good());
//	is->close();
	Draw_EndDisc ();

	return buf;
}

byte *COM_LoadHunkFile (const quake::string_view& path)
{
	return COM_LoadFile (path, 1);
}

byte *COM_LoadTempFile (const quake::string_view& path)
{
	return COM_LoadFile (path, 2);
}

void COM_LoadCacheFile (const quake::string_view& path, struct cache_user_t *cu)
{
	loadcache = cu;
	COM_LoadFile (path, 3);
}

// uses temp hunk if larger than bufsize
byte *COM_LoadStackFile (const quake::string_view& path, void *buffer, int bufsize)
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
static void COM_AddGameDirectory(const quake::string_view& dir)
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
	quake::string_view value;
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
		basedir.erase(slash_point).append("rogue");
		COM_AddGameDirectory(basedir);
	}
	if (host_parms.COM_CheckParm("-hipnotic")) {
		basedir.erase(slash_point).append("hipnotic");
		COM_AddGameDirectory(basedir);
	}

	//
	// -game <gamedir>
	// Adds basedir/gamedir as an override game
	//
	if (host_parms.COM_CheckParmValue("-game", value) != 0) {
		{
			com_modified = true;
			basedir.erase(slash_point).append(value);
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
		quake::string_view file_name;
		for (size_t j = i + 1; i < host_parms.argc; i++) {
			auto it = host_parms.argv[j];
			if (!it.empty() || it.at(0) == '+' || it.at(0) == '-') {
				file_name = it;
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


