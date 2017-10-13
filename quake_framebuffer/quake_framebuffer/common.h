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
#include <chrono>
#include <iostream>
#include <sstream>
#include <string_view>
// we need the basic file functions
#include "sys.h"

#if !defined BYTE_DEFINED
typedef uint8_t 		byte;
#define BYTE_DEFINED 1
#endif


typedef bool qboolean;

//============================================================================

typedef struct sizebuf_s
{
	qboolean	allowoverflow;	// if false, do a Sys_Error
	qboolean	overflowed;		// set to true if the buffer size failed
	byte	*data;
	int		maxsize;
	int		cursize;
} sizebuf_t;

void SZ_Alloc (sizebuf_t *buf, int startsize);
void SZ_Free (sizebuf_t *buf);
void SZ_Clear (sizebuf_t *buf);
void *SZ_GetSpace (sizebuf_t *buf, int length);
void SZ_Write (sizebuf_t *buf, void *data, int length);
void SZ_Print (sizebuf_t *buf, char *data);	// strcats onto the sizebuf

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
template<typename T, typename = std::enable_if<!std::is_same<T,int>::value && std::is_arithmetic<T>::value>>
void MSG_WriteChar(sizebuf_t *sb, T c) {
	//static_assert("Do you REALLY want to convert this to char?");
	MSG_WriteChar(sb, static_cast<int>(c));
}
template<typename T, typename = std::enable_if<!std::is_same<T, int>::value && std::is_arithmetic<T>::value>>
void MSG_WriteByte(sizebuf_t *sb, T c) {
	//static_assert("Do you REALLY want to convert this to byte?");
	MSG_WriteByte(sb, static_cast<int>(c));
}


void MSG_WriteShort (sizebuf_t *sb, int c);
template<typename T, typename = std::enable_if<!std::is_same<T, int>::value && std::is_arithmetic<T>::value>>
void MSG_WriteShort(sizebuf_t *sb, T c) {
	//static_assert("Do you REALLY want to convert this to short?");
	MSG_WriteShort(sb, static_cast<int>(c));
}


void MSG_WriteLong (sizebuf_t *sb, int c);
template<typename T, typename = std::enable_if<!std::is_same<T, int>::value && std::is_arithmetic<T>::value>>
void MSG_WriteLong(sizebuf_t *sb, T c) {
	//static_assert("Do you REALLY want to convert this to long?");
	MSG_WriteLong(sb, static_cast<int>(c));
}


void MSG_WriteFloat (sizebuf_t *sb, float f);
void MSG_WriteString (sizebuf_t *sb, char *s);
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
char *MSG_ReadString (void);

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
int Q_strcpy(char(&dest)[N], const char * src) { Q_strncpy(dest, src, N);}

char *Q_strrchr (char *s, char c);
const char *Q_strrchr(const char *s, char c);

void Q_strcat (char *dest, const char * src);
int Q_strcmp (const char * s1, const char * s2);
int Q_strncmp (const char * s1, const char * s2, size_t count);

int Q_strcasecmp (const char * s1, const char * s2);
int Q_strncasecmp (const char * s1, const char * s2, size_t n);
int	Q_atoi (const char * str);
float Q_atof (const char * str);

int Q_vsprintf(char* buffer, const char* fmt, va_list va);
int Q_snprintf(char* buffer, size_t buffer_size, const char* fmt, ...);

int Q_vsnprintf(char* buffer, size_t buffer_size, const char* fmt, va_list va);
template<size_t N>
int Q_vsprintf(char(&buffer)[N], const char* fmt, va_list va) { return Q_vsnprintf(buffer, N, fmt, va); }

int Q_sprintf(char* buffer, const char* fmt, ...);
template<size_t N, typename ... Args>
int Q_sprintf(char(&buffer)[N], const char* fmt, Args&& ... args) { return Q_snprintf(buffer, N, fmt, std::forward<Args>(args)...); }



template<size_t N>
int Q_sprintf(char(&buffer)[N], const char* fmt, ...) { 
	va_list va;
	va_start(va, fmt);
	int ret = Q_vsnprintf(buffer, N, fmt, va);
	va_end(va);
	return ret;
}

//============================================================================




extern	int		com_argc;
extern	char	**com_argv;

int COM_CheckParm (const char * parm);
void COM_Init (char *path);
void COM_InitArgv (int argc, char **argv);
const char *COM_SkipPath(const char *pathname);
char *COM_SkipPath (char *pathname);
void COM_StripExtension (const char *in, char *out);
void COM_FileBase (const char *in, char *out,size_t out_size);
template<size_t N>
inline void COM_FileBase(const char *in, char(&out)[N]) { COM_FileBase(in, (char*)out, N); }

void COM_DefaultExtension (char *path, const char *extension);


// does a varargs printf into a temp buffer


//============================================================================

extern int com_filesize;
struct cache_user_s;

extern	char	com_gamedir[MAX_OSPATH];

void COM_WriteFile (const char * filename, const void * data, int len);
int COM_OpenFile (const char * filename, int *hndl);
int COM_FOpenFile (const char * filename, FILE **file);
void COM_CloseFile (int h);

byte *COM_LoadStackFile (const char * path, void *buffer, int bufsize);
byte *COM_LoadTempFile (const char * path);
byte *COM_LoadHunkFile (const char * path);
void COM_LoadCacheFile (const char * path, struct cache_user_s *cu);

#ifdef min
#undef min
#endif

#include <string_view>


//=============================================================================
class idStrRef {
public:
	using string_view = std::string_view;

	using char_traits = std::char_traits<char>;
	static constexpr size_t clength(const char* str) { return *str ? 1 + clength(str + 1) : 0; }
	static constexpr size_t chash(const char* str, size_t h, size_t i) { return i == 0 ? 0 : chash(str, (h * 33) ^ str[i], i - 1); }
	static constexpr size_t chash(const char* str, size_t i) { return chash(str, 5381, i); }
	constexpr idStrRef() : _str(""), _size(0) {}
	constexpr idStrRef(const char* s, size_t n) : _str(s), _size(n) {}
	template<size_t N>
	constexpr idStrRef(const char(&s)[N]) : idStrRef(s, N) {}
	constexpr idStrRef(const char* s) : idStrRef(s, clength(s)) {}
	constexpr size_t hash() const { return chash(_str, _size); }
	constexpr size_t size() const { return _size; }
	constexpr bool empty() const { return _size == 0; }
	constexpr const char& operator[](size_t i) const { assert(i < _size); return _str[i]; }
	const char& front() const { return _str[0]; }
	const char& back() const { return _str[_size-1]; }
	const char* begin() const { return &_str[0]; }
	const char* end() const { return &_str[_size - 1]; }
	constexpr char* data() const { return _str; }
	constexpr void remove_suffix(const size_t count) noexcept {	
		assert(_size >= count);
		_size -= count;
	}
	constexpr void remove_prefix(const size_t count) noexcept {	
		assert(_size >= count);
		_str += count;
		_size -= count;
	}

	// returns 0 if no conversion, or the size of the float
	// < 0 for some serious errors or bugs
	int to_float(float& f,size_t offset=0U) const;
	int to_int(int& i, size_t offset = 0U) const;
	bool equal(const char* str) const;
	bool equal(const idStrRef& str) const;
	bool operator==(const idStrRef& s) const { return equal(s); }
	bool operator!=(const idStrRef& s) const { return !(*this == s); }
	bool operator==(const char* s) const { return equal(s); }
	bool operator!=(const char* s) const { return !(*this == s); }

	void copy(char* str, size_t size) const {
		assert(size - 1 >= _size);
		Q_strncpy(str, _str, size);
	}
	template<size_t N>
	void copy(char(&s)[N])  const {
		assert(N - 1 >= _size);
		Q_strncpy(str, _str, size);
	}
public:
	const char* _str;
	size_t _size;
};


template<size_t SIZE = 1024U>
class buffer_t {
public:
	buffer_t() : _buffer{ 0 }, _size(0) {}
	buffer_t(const char* fmt, ...) {
		va_list va;
		va_start(va, fmt);
		_size = Q_sprintf(_buffer, fmt, va);
		va_end(va);
	}

	buffer_t& operator=(const char* s) const {
		Q_strcpy(_buffer, s);
		_size = Q_strlen(_buffer);
		return *this;
	}
	constexpr size_t size() const { return _size; }
	constexpr size_t capacity() const { return SIZE; }
	operator const char*() const { return _buffer; }
	const char* c_str() const { return _buffer; }
	operator idStrRef() const { return idStrRef(_buffer, _size); }
	const char* vprint(const char* fmt, va_list va) {
		_size = Q_sprintf(_buffer, fmt, va);
		return _buffer;
	}
	void push_back(int c) {
		assert((capacity() - 1) < _size);
		_buffer[_size++] = c;
		_buffer[_size] = 0;
	}
	void append(const char* s) {
		while (*s) _buffer[_size++] = *s++;;
		_buffer[_size] = 0;
	}
	void append(const char* s, size_t count) {
		assert((capacity() - 1 + count) < _size);
		while (count--) _buffer[_size++] = *s++;;
		_buffer[_size] = 0;
	}
	const char* print(const char* fmt, ...) {
		va_list va;
		va_start(va, fmt);
		_size = Q_sprintf(_buffer, fmt, va);
		va_end(va);
		return _buffer;
	}
	void clear() {
		_size = 0; _buffer[0] = 0;
	}
public:
	char _buffer[SIZE];
	size_t _size;
};


static inline std::ostream& operator<<(std::ostream& os, const idStrRef& s) {
	for (const auto a : s) os << a;
	return os;
}
//http://videocortex.io/2017/custom-stream-buffers/
// great info here, helps alot on this
class istreambuf_view : public std::streambuf
{
public:
	using byte = char;
	static_assert(1 == sizeof(byte), "sizeof buffer element type 1.");
	istreambuf_view(const byte* data, size_t len) :begin_(data), end_(data + len), current_(data) {} 	// ptr + size
	istreambuf_view(const byte* beg, const char* end) : begin_(beg), end_(end), current_(beg) {} // begin + end
protected:
	int_type underflow() override { return (current_ == end_ ? traits_type::eof() : traits_type::to_int_type(*current_)); }
	int_type uflow() override { return (current_ == end_ ? traits_type::eof() : traits_type::to_int_type(*current_++)); }
	std::streamsize showmanyc() override { return end_ - current_; }
	int_type pbackfail(int_type ch) override
	{
		if (current_ == begin_ || (ch != traits_type::eof() && ch != current_[-1]))
			return traits_type::eof();

		return traits_type::to_int_type(*--current_);
	}
	const byte* const begin_;
	const byte* const end_;
	const byte* current_;
};

class id_ifstream : public std::streambuf, public std::istream {
	static constexpr idFileHandle no_handle = idFileHandle();
	static constexpr size_t buffer_size = 128;
public:
	using buf_type = std::streambuf;
	using stream_type = std::istream;
	using char_type = typename stream_type::char_type;
	using traits_type = typename stream_type::traits_type;
	using int_type = typename stream_type::int_type;
	using pos_type = typename stream_type::pos_type;
	using off_type = typename stream_type::off_type;

	explicit id_ifstream() : stream_type(this), _handle{ no_handle }, _buffer{ traits_type::eof() } {  }
	explicit id_ifstream(const char *filename) : stream_type(this), _buffer{ traits_type::eof() } {
		open(filename);
		setg(_buffer, _buffer + buffer_size);
	}
	bool open(const char* filename) {
		if (Sys_FileOpenRead(filename, &_handle) != 0) return false;
		return true;
	}
	bool is_open() const { return _handle != no_handle; }
	void close() {
		if (_handle != no_handle) {
			Sys_FileClose(_handle);
			_handle = no_handle;
		}
	}
	void swap(id_ifstream& from) {
		std::swap(_handle, from._handle);
		buf_type::swap(from);
		stream_type::swap(from);
	}
protected:
	char_type _buffer[buffer_size];
	idFileHandle _handle; // file handle
	int_type underflow() override {
		int bytesRead = 0;
		bytesRead = Sys_FileRead(_handle, _buffer, buffer_size - 1);
		if (bytesRead <= 0) return traits_type::eof();
		setg(_buffer, _buffer, _buffer + bytesRead);
		return traits_type::to_int_type(_buffer[0]);
	}
};

class id_ifotream : public std::streambuf, public std::istream {
	static constexpr idFileHandle no_handle = idFileHandle();
	static constexpr size_t buffer_size = 128;
public:
	using buf_type = std::streambuf;
	using stream_type = std::istream;
	using char_type = typename stream_type::char_type;
	using traits_type = typename stream_type::traits_type;
	using int_type = typename stream_type::int_type;
	using pos_type = typename stream_type::pos_type;
	using off_type = typename stream_type::off_type;

	explicit id_ifotream() : stream_type(this), _handle{ no_handle }, _buffer{ traits_type::eof() } {  }
	explicit id_ifotream(const char *filename) : stream_type(this), _buffer{ traits_type::eof() } {
		open(filename);
		setg(_buffer, _buffer + buffer_size);
	}
	bool open(const char* filename) {
		_handle = Sys_FileOpenWrite(filename);
		return _handle != no_handle;
	}
	bool is_open() const { return _handle != no_handle; }
	void close() {
		if (_handle != no_handle) {
			Sys_FileClose(_handle);
			_handle = no_handle;
		}
	}
	void swap(id_ifotream& from) {
		std::swap(_handle, from._handle);
		buf_type::swap(from);
		stream_type::swap(from);
	}
protected:
	char_type _buffer[buffer_size];
	idFileHandle _handle; // file handle
	int_type overflow(int_type c) override {
		int bytesWritten = 0;
		if (pptr() - pbase() > 0) {
			bytesWritten = Sys_FileWrite(_handle, pbase(), (pptr() - pbase()));
			if (bytesWritten <= 0)  return traits_type::not_eof(c);
		}
		_buffer[0] = traits_type::to_char_type(c);
		setp(_buffer, _buffer + 1, _buffer + buffer_size);
		return traits_type::not_eof(c);
	}
};


// parser
class COM_Parser {
public:
	COM_Parser(const char* data,size_t size) : _data(data), _end(data+size) {}
	COM_Parser(const char* data) : COM_Parser(data, Q_strlen(data)) {}
	COM_Parser() : _data(nullptr) , _end(nullptr) {}
	bool Next(idStrRef& token);
	operator const char*() const { return _data; }
protected:
	void skip_whitespace();
	//char com_token[1024];
	const char* _data;
	const char* _end;
};

class COM_File_Parser : public COM_Parser {
	COM_File_Parser() : _handle(idFileHandle()) , _size(0U) {}
	COM_File_Parser(const char* filename) : _handle(idFileHandle()) { Open(filename);  }
	~COM_File_Parser() { Close(); }
	bool Next(idStrRef& token);
	bool Open(const char* filename);
	void Close();
	bool isOpen() const { return _handle != idFileHandle(); }
private:
	void fill_buffer();
	char _buffer[1024];
	idFileHandle _handle;
	size_t _size;
};

//extern	char		com_token[1024];


//const char *COM_Parse(const char * data);
#ifdef USE_CUSTOM_IDTIME
class idTime {
public:
	idTime();
	idTime(float f);
	idTime(double f);
	operator float() const;
	idTime& operator+=(const idTime& r);
	idTime& operator-=(const idTime& r);
	bool operator==(const idTime& r) const;
	bool operator!=(const idTime& r) const;
	bool operator<(const idTime& r) const;
	bool operator>(const idTime& r) const;
	bool operator<=(const idTime& r) const;
	bool operator>=(const idTime& r) const;
	idTime& operator+=(float r);
	idTime& operator-=(float r);
	static idTime Seconds(int sec);
	static idTime MilliSeconds(int msec);
	static idTime UilliSeconds(int msec);
public:
	float _time;
};
static inline idTime operator+(const idTime& l, const idTime& r) { idTime o(l); o += l; return o; }
static inline idTime operator-(const idTime& l, const idTime& r) { idTime o(l); o -= l; return o; }
#else
using idTimef = std::chrono::duration<float>;
using idTime = std::chrono::milliseconds;
#if 0
struct idTime : public std::chrono::milliseconds {
	using base_type = std::chrono::milliseconds;
	using base_type::base_type;
	//using base_type::operator=;
	template<typename Rep, typename Ratio>
	idTime(std::chrono::duration<Rep, Ratio>&& t) : base_type(t) {}
	template<typename Rep, typename Ratio>
	idTime(const std::chrono::duration<Rep, Ratio>& t) : base_type(t) {}
	using base_type::operator%=;
	using base_type::operator+=;
	using base_type::operator-=;
	using base_type::operator*=;
	using base_type::operator/=;
	using base_type::operator++;
	using base_type::operator--;
//	using base_type::operator++(int);
//	using base_type::operator--(int);
	using base_type::operator+;
	using base_type::operator-;
	constexpr idTime() : base_type{} {}
	constexpr idTime(float f) : base_type(std::chrono::duration_cast<base_type>((idTimef(f)))) {}
	constexpr operator base_type() const { return *this; }
	constexpr operator float() const { return idTimef(*this).count(); }
};
#endif
#if 0
class idTime { //: public  std::chrono::milliseconds {
public:
	using time_type = int64_t;
	using chrono_time_type = std::chrono::duration<time_type, std::milli>;
	constexpr idTime() : _value{ 0 } {}
	constexpr idTime(float f) : _value{ static_cast<time_type>(f * 1000.0f) } {}
	constexpr idTime(double f) : _value{ static_cast<time_type>(f * 1000.0) } {}

	template<typename Rep, typename Period>
	constexpr idTime(const std::chrono::duration<Rep, Period>& v) : _value{ std::chrono::duration_cast<chrono_time_type>(v).count() } {}

	constexpr explicit operator float() const { return static_cast<float>(_value) / 1000.0f; }

	bool operator==(const idTime& r) const { return _value == r._value; }
	bool operator!=(const idTime& r) const { return _value != r._value; }
	bool operator<(const idTime& r) const { return _value < r._value; }
	bool operator>(const idTime& r) const { return _value > r._value; }
	bool operator<=(const idTime& r) const { return _value <= r._value; }
	bool operator>=(const idTime& r) const { return _value >= r._value; }
	idTime& operator+=(const idTime&r) { _value += r._value; return *this; }
	idTime& operator-=(const idTime&r) { _value -= r._value; return *this; }
	idTime& operator*=(const idTime&r) { _value *= r._value; return *this; }
	idTime& operator/=(const idTime&r) { _value /= r._value; return *this; }
	time_type count() const { return _value; }
	time_type minutes() const { return _value; }
	time_type seconds() const { return _value/1000; }

	constexpr static idTime zero() { return idTime(); }
	constexpr static idTime fromMills(time_type mills) { return idTime(mills); }
	constexpr static idTime fromSec(time_type sec) { return idTime(sec*1000); }

private:
	friend idTime operator+(const idTime&, const idTime&);
	friend idTime operator-(const idTime&, const idTime&);
	friend idTime operator/(const idTime&, const idTime&);
	friend idTime operator*(const idTime&, const idTime&);
	constexpr idTime(time_type i) : _value{ i } {}
	time_type _value;
};
inline idTime operator+(const idTime&l, const idTime&r) { return idTime(l.count() + r.count()); }
inline idTime operator-(const idTime&l, const idTime&r) { return idTime(l.count() - r.count()); }
inline idTime operator*(const idTime&l, const idTime&r) { return idTime(l.count() * r.count()); }
inline idTime operator/(const idTime&l, const idTime&r) { return idTime(l.count() / r.count()); }
#endif
#endif

template<typename TO, typename FROM> constexpr TO idCast(FROM f);

template<>
constexpr inline int idCast<int,idTime>(idTime f) {
	return static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(f).count());
}
template<>
constexpr inline float idCast<float, idTime>(idTime f) {
	return std::chrono::duration_cast<idTimef>(f).count();
}
template<>
constexpr inline idTime idCast<idTime, float>(float f) {
	return std::chrono::duration_cast<idTime>(idTimef(f));
}
template<>
constexpr inline idTime idCast<idTime, double>(double f) {
	return idCast<idTime,float>(static_cast<float>(f));
}

template<typename T> T Q_sin(T t) { return std::sin(t); }
template<typename T> T Q_cos(T t) { return std::cos(t); }
template<typename T> T Q_tan(T t) { return std::tan(t); }
template<typename T> T Q_sqrt(T t) { return std::sqrt(t); }


template<typename T1,typename T2> T1 Q_pow(T1 a, T2 b) { return std::pow(a,b); }
template<typename T1, typename T2> T1 Q_atan2(T1 a, T2 b) { return std::atan2(a, b); }
extern qboolean		standard_quake, rogue, hipnotic;

#endif