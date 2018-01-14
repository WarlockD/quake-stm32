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
// comndef.h  -- general definitions
#ifndef _QUAKE_COMMON_H_
#define _QUAKE_COMMON_H_

#include <cstdint>
#include <cstdlib>
#include <type_traits>
#include <cassert>

#include "qvector.h"
#if !defined BYTE_DEFINED
typedef uint8_t 		byte;
#define BYTE_DEFINED 1
#endif


typedef bool qboolean;

//============================================================================

struct sizebuf_t
{
	qboolean	allowoverflow;	// if false, do a Sys_Error
	qboolean	overflowed;		// set to true if the buffer size failed
	byte * data;
	int		maxsize;
	int		cursize;
	void push_back(byte c) { *get_space(1) = c; }
	void alloc(size_t s);
	void free();
	void manage(void* d, size_t s) { data = reinterpret_cast<byte*>(d); maxsize = s; }
	void write(const void* data, size_t length);
	void write(const char* str);


	size_t size() const { return cursize; }
	void clear() { cursize = 0; }

private:
	byte * get_space(size_t s);
	
} ;


template <typename T, typename Container = vector<T> >
class queue {
public:
	typedef T			value_type;
	typedef Container		container_type;
	typedef typename container_type::size_type size_type;
	typedef typename container_type::difference_type difference_type;
	typedef value_type&		reference;
	typedef const value_type&	const_reference;
public:
	inline			queue(void) : _storage(), _front(0) { }
	explicit inline		queue(const container_type& s) : _storage(s), _front(0) { }
	explicit inline		queue(const queue& s) : _storage(s._storage), _front(0) { }
	inline size_type		size(void) const { return _storage.size() - _front; }
	inline bool			empty(void) const { return !size(); }
	inline reference		front(void) { return _storage[_front]; }
	inline const_reference	front(void) const { return _storage[_front]; }
	inline reference		back(void) { return _storage.back(); }
	inline const_reference	back(void) const { return _storage.back(); }
	inline void			push(const_reference v) { _storage.push_back(v); }
	void			pop(void) {
		if (++_front > _storage.size() / 2) {
			_storage.erase(_storage.begin(), _front);
			_front = 0;
		}
	}
	inline void			swap(queue& v) { _storage.swap(v); swap(_front, v._front); }
	inline bool			operator== (const queue& s) const { return _storage == s._storage && _front == s._front; }
	inline bool			operator< (const queue& s) const { return size() < s.size(); }
#if HAVE_CPP11
	inline			queue(queue&& v) : _storage(move(v._storage)), _front(v._front) { v._front = 0; }
	inline			queue(container_type&& s) : _storage(move(s)), _front(0) {}
	inline queue&		operator= (queue&& v) { swap(v); return *this; }
	template <typename... Args>
	inline void			emplace(Args&&... args) { _storage.emplace_back(forward<Args>(args)...); }
#endif
private:
	container_type		_storage;	///< Where the data actually is.
	size_type			_front;	///< Index of the element returned by next pop.
};


void SZ_Alloc (sizebuf_t *buf, int startsize);
void SZ_Free (sizebuf_t *buf);
void SZ_Clear (sizebuf_t *buf);
void *SZ_GetSpace (sizebuf_t *buf, int length);
void SZ_Write (sizebuf_t *buf, const void *data, int length);
void SZ_Print (sizebuf_t *buf, const char *data);	// strcats onto the sizebuf

template<typename U>
static inline void SZ_Print(sizebuf_t *buf, const quake::string_helper<U>& data) {
	size_t len = data.size() + 1;

	// byte * cast to keep VC++ happy
	if (buf->data[buf->cursize - 1])
		Q_memcpy((byte *)SZ_GetSpace(buf, len), data.data(), len); // no trailing 0
	else
		Q_memcpy((byte *)SZ_GetSpace(buf, len - 1) - 1, data.data(), len); // write over trailing 0
}

//============================================================================

typedef struct link_s
{
	struct link_s	*prev, *next;
} link_t;


void ClearLink (link_t *l);
void RemoveLink (link_t *l);
void InsertLinkBefore (link_t *l, link_t *before);
void InsertLinkAfter (link_t *l, link_t *after);

// (type *)STRUCT_FROM_LINK(link_t *link, type, member)
// ent = STRUCT_FROM_LINK(link,entity_t,order)
// FIXME: remove this mess!
#define	STRUCT_FROM_LINK(l,t,m) ((t *)((byte *)l - (int)&(((t *)0)->m)))

//============================================================================

#ifndef NULL
#define NULL ((void *)0)
#endif

#define Q_MAXCHAR ((char)0x7f)
#define Q_MAXSHORT ((short)0x7fff)
#define Q_MAXINT	((int)0x7fffffff)
#define Q_MAXLONG ((int)0x7fffffff)
#define Q_MAXFLOAT ((int)0x7fffffff)

#define Q_MINCHAR ((char)0x80)
#define Q_MINSHORT ((short)0x8000)
#define Q_MININT 	((int)0x80000000)
#define Q_MINLONG ((int)0x80000000)
#define Q_MINFLOAT ((int)0x7fffffff)

//============================================================================

extern	qboolean		bigendien;

extern	short	(*BigShort) (short l);
extern	short	(*LittleShort) (short l);
extern	int	(*BigLong) (int l);
extern	int	(*LittleLong) (int l);
extern	float	(*BigFloat) (float l);
extern	float	(*LittleFloat) (float l);

//============================================================================

void MSG_WriteChar (sizebuf_t *sb, int c);
void MSG_WriteByte (sizebuf_t *sb, int c);
// I put some templates here so we can catch some odd balls
// alot of stuff in sv_main gets converted to bytes.  We are talking floats?  There
// are enough warnings for me to think about changing the df file
template<typename T, typename = std::enable_if<!std::is_same<T,int>::value && std::is_arithmetic<T>::value>::type>
void MSG_WriteChar(sizebuf_t *sb, T c) {
	//static_assert("Do you REALLY want to convert this to char?");
	MSG_WriteChar(sb, static_cast<int>(c));
}
template<typename T, typename = std::enable_if<!std::is_same<T, int>::value && std::is_arithmetic<T>::value>::type>
void MSG_WriteByte(sizebuf_t *sb, T c) {
	//static_assert("Do you REALLY want to convert this to byte?");
	MSG_WriteByte(sb, static_cast<int>(c));
}


void MSG_WriteShort (sizebuf_t *sb, int c);
template<typename T, typename = std::enable_if<!std::is_same<T, int>::value && std::is_arithmetic<T>::value>::type>
void MSG_WriteShort(sizebuf_t *sb, T c) {
	//static_assert("Do you REALLY want to convert this to short?");
	MSG_WriteShort(sb, static_cast<int>(c));
}


void MSG_WriteLong (sizebuf_t *sb, int c);
template<typename T, typename = std::enable_if<!std::is_same<T, int>::value && std::is_arithmetic<T>::value>::type>
void MSG_WriteLong(sizebuf_t *sb, T c) {
	//static_assert("Do you REALLY want to convert this to long?");
	MSG_WriteLong(sb, static_cast<int>(c));
}


void MSG_WriteFloat (sizebuf_t *sb, float f);
void MSG_WriteString (sizebuf_t *sb, const char *s);
void MSG_WriteString(sizebuf_t *sb, const char *s, size_t length);
template<typename U>
inline void MSG_WriteString(sizebuf_t *sb, const quake::string_helper<U>& s) { MSG_WriteString(sb, s.data(),s.size()); }
void MSG_WriteCoord (sizebuf_t *sb, float f);
void MSG_WriteAngle (sizebuf_t *sb, float f);

extern	int			msg_readcount;
extern	qboolean	msg_badread;		// set if a read goes beyond end of message

void MSG_BeginReading (void);
int MSG_ReadChar (void);
int MSG_ReadByte (void);
int MSG_ReadShort (void);
int MSG_ReadLong (void);
float MSG_ReadFloat (void);
quake::cstring MSG_ReadString (void);

float MSG_ReadCoord (void);
float MSG_ReadAngle (void);

//============================================================================

void Q_memset (void *dest, int fill, size_t count);
void Q_memcpy (void *dest, const void * src, size_t count);
int Q_memcmp (const void * m1, const void * m2, size_t count);



size_t Q_strlen(const char * str);
void Q_strncpy(char *dest, const char * src, size_t count);
void Q_strcpy(char *dest, const char * src);
template<size_t N>
void Q_strcpy(char(&dest)[N], const char * src) { return Q_strncpy(dest, src, N);}
template<size_t N,typename U>
void Q_strcpy(char(&dest)[N], const quake::string_helper<U>& src) { return  Q_strncpy(dest, src.data(), std::min(src.size(),N-1)); }


char *Q_strrchr (char *s, char c);
const char *Q_strrchr(const char *s, char c);

void Q_strcat (char *dest, const char * src);
int Q_strcmp (const char * s1, const char * s2);
int Q_strncmp (const char * s1, const char * s2, size_t count);

int Q_strcasecmp (const char * s1, const char * s2);

template<typename U>
static inline int Q_strcasecmp(const quake::string_helper<U>& s1, const quake::string_view& s2) { return quake::detail::str_case_compare(s1.data(), s1.size(), s2.data(), s2.size()); }


int Q_strncasecmp (const char * s1, const char * s2, size_t n);
int	Q_atoi (const char * str);

// just a quick hack
template<typename U>
int	Q_atoi(const quake::string_helper<U>& str) { 
	char* end;
	int ret = ::strtol(str.data(), &end,0);
	//assert(end != str.data());
	return ret;
}

float Q_atof (const char * str);
template<typename U>
int	Q_atof(const quake::string_helper<U>& str) {
	char* end;
	float ret = ::strtof(str.data(), &end);
//	assert(end != str.data());
	return ret;
}

int Q_vsprintf(char* buffer, const char* fmt, va_list va);
int Q_vsnprintf(char* buffer, size_t buffer_size, const char* fmt, va_list va);
template<size_t N>
int Q_vsprintf(char(&buffer)[N], const char* fmt, va_list va) { return Q_vsnprintf(buffer, N, fmt, va); }

int Q_sprintf(char* buffer, const char* fmt, ...);
int Q_snprintf(char* buffer, size_t buffer_size, const char* fmt, ...);

template<size_t N>
int Q_sprintf(char(&buffer)[N], const char* fmt, ...) { 
	va_list va;
	va_start(va, fmt);
	int ret = Q_vsnprintf(buffer, N, fmt, va);
	va_end(va);
	return ret;
}

//============================================================================

extern	quake::string_view		com_token;
extern	qboolean	com_eof;

quake::string_view Old_COM_Parse (const quake::string_view& data);


extern	int		com_argc;
extern	char	**com_argv;

int COM_CheckParm (const quake::string_view& parm);
void COM_Init (const quake::string_view& basedir);
void COM_InitArgv (int argc, const char **argv);
quake::string_view COM_SkipPath(const quake::string_view& pathname);

quake::string_view  COM_StripExtension (const quake::string_view&in);
#if 0
void COM_FileBase (const char *in, char *out,size_t out_size);
template<size_t N>
inline void COM_FileBase(const char *in, char(&out)[N]) { COM_FileBase(in, (char*)out, N); }
#endif

inline quake::string_view COM_FileBase(quake::string_view in);
/*
==================
COM_DefaultExtension
==================
*/
template<typename U,typename UU>
inline void COM_DefaultExtension(quake::string_builder<U>& path, const quake::string_helper<UU>& extension) {
	auto dot = path.find_last_of('.');
	if (dot != quake::string_view::npos) {
		auto slash = path.find_last_of("/\\");
		if (slash != quake::string_view::npos && dot > slash) 
			dot = quake::string_view::npos;
	}
	if (dot == quake::string_view::npos) {
		if (extension.front() != '.') 
			path.push_back('.'); 
		path.append(extension);
	}
}
template<typename U>
inline void COM_DefaultExtension(quake::string_builder<U>& path, const char* extension) { COM_DefaultExtension(path, quake::string_view(extension)); }
/*
============
va

does a varargs printf into a temp buffer, so I don't need to have
varargs versions of all text functions.
FIXME: make this buffer size safe someday
============
*/
// silly hack till streams work
class va {
	constexpr static size_t va_size = 1024;
	char _string[va_size];
	size_t _size;
public:
	va() : _string{ 0 }, _size{ 0 } {}
	va(const char* fmt, ...);
	template<typename U>
	va(const quake::string_helper<U>& s) : _size(s.size()) { assert(s.size() < va_size   - 1); ::memcpy(_string,s.data(),s.size()); _string[s.size()] = '\0'; }
	operator const char*() const { return _string; }
	const char* c_str() const { return _string; }
	operator quake::string_view() const { return quake::string_view(_string, _size); }
	size_t size() const { return _size; }
	va& operator+=(const char* s) { strcat_s(_string, s); return *this; }
	va& operator+=(char c) { char v[2] = { c,0 }; operator+=(v); return *this; }
};

// does a varargs printf into a temp buffer


//============================================================================

extern int com_filesize;
struct cache_user_s;

extern	quake::stack_string<MAX_OSPATH> com_gamedir;

void COM_WriteFile (const char * filename, const void * data, int len);
int COM_OpenFile (const char * filename, int *hndl);
int COM_FOpenFile (const char * filename, FILE **file);
void COM_CloseFile (int h);

byte *COM_LoadStackFile (const char * path, void *buffer, int bufsize);
byte *COM_LoadTempFile (const char * path);
byte *COM_LoadHunkFile (const char * path);
void COM_LoadCacheFile (const char * path, struct cache_user_s *cu);



// parser
class COM_Parser {
public:
	COM_Parser() : _data(""), _state(0) {}
	COM_Parser(const quake::string_view & data) : _data(data), _pos(0), _state(0) {}
	bool Next(quake::string_view& token, bool test_eol = false);
	operator const quake::string_view&() const { return _data; }
	size_t pos() const { return _pos; }
	quake::string_view remaining() const { return _data.substr(_pos); }
protected:
	int _state;
	size_t _pos;
	quake::string_view _data;
};




extern	struct cvar_s	registered;

extern qboolean		standard_quake, rogue, hipnotic;

#endif