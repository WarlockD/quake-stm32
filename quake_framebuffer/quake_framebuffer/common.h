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
#include <chrono>
#include <array>
#include <ostream>
#include <istream>
#include <string_view>
#include <memory>
#include <fstream>
#include <variant>
#include <exception>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include  <functional>

// my personal link list implmentation
// based of sys\queue.h  could always use that I suppose
#include "list.hpp"
#include "tailq.hpp"
#include <ustring.h>

#include "macros.h"
// cause I am having problems with list and tail I am putting a wrapper around it
// we need the basic file functions
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
//using idTime = std::chrono::milliseconds;
class idTime  {
public:
	using time_t = std::chrono::milliseconds;
	using timef_t = std::chrono::duration<float, std::milli>;
	static constexpr idTime zero() { return idTime(time_t::zero()); }
	constexpr inline static time_t cast_from_float(const float f) { return time_t(static_cast<time_t::rep>(f * 1000.0)); }
	constexpr inline static float cast_to_float(const time_t& t) { return static_cast<float>(t.count()) / 1000.0f; }
	constexpr idTime() :_time(time_t::zero()) {}
	constexpr idTime(float f) : _time(cast_from_float(f)) {}
	template<typename T, typename R>
	constexpr idTime(const std::chrono::duration<T, R>& f) : _time(std::chrono::duration_cast<time_t>(f)) {}
	constexpr const time_t& time() const { return _time; }
	constexpr time_t::rep  count() const { return _time.count(); }
	constexpr auto seconds() const { return std::chrono::duration_cast<std::chrono::seconds>(_time).count(); }


	explicit constexpr operator float() const { return cast_to_float(_time); }

	template<typename T>
	constexpr time_t::rep  compare(const T& other) const {
		static_assert(std::is_constructible<idTime, decltype(other)>::value, "Wrong type?");
		return _time.count() - idTime(other).count();
	}
	template<typename T>
	idTime& operator+=(const T& other) {
		static_assert(std::is_constructible<idTime, decltype(other)>::value, "Wrong type?");
		_time += idTime(other).time(); 
		return *this; 
	}
	template<typename T>
	idTime& operator-=(const T& other) {
		static_assert(std::is_constructible<idTime, decltype(other)>::value, "Wrong type?");
		_time -= idTime(other).time();
		return *this;
	}

private:
	time_t _time;
};

static inline std::ostream& operator<<(std::ostream& os, const idTime& s) {
	//os << std::right << (s.count() / 1000) << '.' << std::setfill('0') << std::setw(4) << std::abs(s.count() % 1000);
	auto pre = os.precision();
	os.precision(4);
	os << idTime::cast_to_float(s.time());
	os.precision(pre);
	return os;
}
constexpr static inline bool operator==(const idTime& s1, const idTime&  s2) { return s1.compare(s2) == 0; }
constexpr static inline bool operator!=(const idTime& s1, const idTime&  s2) { return s1.compare(s2) != 0; }
constexpr static inline bool operator<(const idTime& s1, const idTime&  s2) { return s1.compare(s2) < 0; }
constexpr static inline bool operator>(const idTime& s1, const idTime&  s2) { return s1.compare(s2) > 0; }
constexpr static inline bool operator<=(const idTime& s1, const idTime&  s2) { return s1.compare(s2) <= 0; }
constexpr static inline bool operator>=(const idTime& s1, const idTime&  s2) { return s1.compare(s2) >= 0; }

constexpr static inline idTime operator+(const idTime& s1, const idTime& s2) { return idTime(s1.time() + s2.time());  }
constexpr static inline idTime operator-(const idTime& s1, const idTime& s2) { return idTime(s1.time() - s2.time()); }
constexpr static inline idTime operator*(const idTime& s1, const idTime& s2) { return idTime(s1.time() * s2.time()); }



#ifndef BYTE_DEFINED
typedef uint8_t 		byte;
#define BYTE_DEFINED 1
#endif

using qboolean = bool;


struct string_t;

using cstring_t = ustl::cstring;
namespace quake {
	using string = ustl::string;

	using string_view = ustl::string_view;

}



struct string_t : public ustl::cstring {
public:
	static  const char* intern(const char* str);
	static  const char* intern(const char* str,size_t size);
	static  const char* intern(const quake::string_view&  str);
	inline string_t() : ustl::cstring() {}
	template<typename T>
	inline string_t(const ustl::string_helper<T>& s) : ustl::cstring(intern(s.data(),s.size())) {}
	inline string_t(const quake::string_view&  str) : ustl::cstring(intern(str)) { }
	inline string_t(const char*  str) : ustl::cstring(intern(str)) { }
 	friend class pr_system_t;
};

// swaps
#if 0

static inline std::ostream& operator<<(std::ostream& os, const string_t& s) {
	os << s.c_str();
	return os;
}
static inline std::ostream& operator<<(std::ostream& os, const cstring_t& s) {
	os << s.c_str();
	return os;
}
#endif
namespace std {
	template<>
	struct hash<cstring_t> {
		inline constexpr size_t operator()(const cstring_t& s) const { return ustl::util::str_hash(s.begin(), s.end()); }
	};
	template<>
	struct hash<string_t> {
		inline constexpr size_t operator()(const string_t& s) const { return ustl::util::str_hash(s.begin(), s.end()); }
	};
}

// https://stackoverflow.com/questions/41936763/type-traits-to-check-if-class-has-member-function
namespace priv {
	// cause MSVC dosn't have is_Detected yet
	template< class... >
	using void_t = void;

	struct nonesuch {
		nonesuch() = delete;
		~nonesuch() = delete;
		nonesuch(nonesuch const&) = delete;
		void operator=(nonesuch const&) = delete;
	};

	namespace detail {
		template <class Default, class AlwaysVoid,
			template<class...> class Op, class... Args>
		struct detector {
			using value_t = std::false_type;
			using type = Default;
		};

		template <class Default, template<class...> class Op, class... Args>
		struct detector<Default, void_t<Op<Args...>>, Op, Args...> {
			using value_t = std::true_type;
			using type = Op<Args...>;
		};

	} // namespace detail

	template <template<class...> class Op, class... Args>
	using is_detected = typename detail::detector<nonesuch, void, Op, Args...>::value_t;

	template <template<class...> class Op, class... Args>
	using detected_t = typename detail::detector<nonesuch, void, Op, Args...>::type;

	template <class Default, template<class...> class Op, class... Args>
	using detected_or = detail::detector<Default, void, Op, Args...>;

	template <class Expected, template<class...> class Op, class... Args>
	using is_detected_exact = std::is_same<Expected, detected_t<Op, Args...>>;
}
// is raw data trait.  All "data" is a const char* data() const member with a size_t size() const member
// so check for that
template<class T>
using data_member_t = decltype(std::declval<const T&>().data());
template<class T>
using size_member_t = decltype(std::declval<const T&>().size());
template<class T>
using is_data_class = std::conditional_t<
	priv::is_detected<data_member_t, std::remove_cv_t<T>>::value &&
	priv::is_detected<size_member_t, std::remove_cv_t<T>>::value,
	std::true_type, std::false_type>;
// if it has a zero termenated string, then we do this
template<class T>
using cstr_member_t = decltype(std::declval<const T&>().c_str());
template<class T>
using is_cstr_class = std::conditional_t<priv::is_detected<cstr_member_t, std::remove_cv_t<T>>::value,std::true_type, std::false_type>;

#include "sys.h"
// don't mind this, just me trying to reinvent the god damn wheel again


template<typename T> using link_t = list::entry<T>;

template<typename T, typename list::field<T> FIELD>
using link_list_t = list::head<T, FIELD>;

//============================================================================

//http://videocortex.io/2017/custom-stream-buffers/
int Q_strcasecmp(const quake::string_view& s1, const quake::string_view& s2);

namespace quake {
	static constexpr size_t DEFAULT_STREAM_BUFFER_SIZE = 1024;
	static constexpr size_t DEFAULT_SIMPLE_BUFFER_SIZE = 64;

	// ran range
	//template<uint16_t MIN=0, uint16_t MAX = RAND_MAX>
	//static inline uint16_t rand() { return ::rand() % (MAX + 1 - MIN) + MIN; }
	// issue with this is that we don't have the accurcy with rand for this to work well
	template<typename RET = int16_t>
	static inline RET rand(const RET min, const RET max) {
		static constexpr RET one = static_cast<RET>(1);
		static constexpr RET rand_max = static_cast<RET>(RAND_MAX);
		return max + (static_cast<RET>(::rand()) / (rand_max / (max - min + one) + one));
	}

	using hashvalue_t = size_t;
	using ssize_t = typename std::conditional<sizeof(size_t) == sizeof(int32_t), int32_t, int64_t>::type;
	using char_type = char;
	using char_traits = std::char_traits<char_type>;

	struct stream_output {
		virtual void text_output(std::ostream& os) const = 0;
		virtual std::string to_string() const { std::stringstream ss; text_output(ss); return ss.str(); }
		virtual ~stream_output() {}
	};
	template<typename T>
	inline typename std::enable_if<std::is_base_of<stream_output, T>::value, std::ostream&>::type
		operator<<(std::ostream& os, const T& vs) { vs.text_output(os); return os; }
	// for debuging data changes
	template<typename T>
	class debug_value_t {
	public:
		using value_type = T;
		void set(const T& v) {
			if (v != _value) {
				_value = v; // value changed
			}
		}
		T& get() { return _value; }
		const T& get() const { return _value; }
		debug_value_t() : _value{} {}
		debug_value_t(const T& value) : _value{ } { set(value); }
	private:
		T _value;
	};
	template<typename T, typename E = void> class debug_t;

	template<typename T>
	class debug_t <T, typename std::enable_if<std::is_arithmetic_v<T> && !std::is_pointer_v<T>>::type> {
		debug_value_t<T> _value;
	public:
		constexpr debug_t() : _value() {}
		constexpr debug_t(const T& value) : _value() { _value.set(value); }
		debug_t& operator=(const T& value) { _value.set(value); return *this; }
		constexpr operator T&() { return _value.get(); }
		constexpr operator const T&() const { return _value.get(); }
	};

	template<typename T>
	class debug_t <T, typename std::enable_if<!std::is_arithmetic_v<T> && std::is_pointer_v<T>>::type> {
		debug_value_t<T> _value;
	public:
		constexpr debug_t() : _value() {}
		constexpr debug_t(const T& value) : _value() { _value.set(value); }
		debug_t& operator=(const T& value) { _value.set(value); return *this; }
		T operator->() { return _value.get(); }
		const T operator->() const { return _value.get(); }
		constexpr operator T&() { return _value.get(); }
		constexpr operator const T&() const { return _value.get(); }
	};

#define DEBUG_OP(op)	\
template<typename T,typename E> static inline bool operator##op (const T& s1, const debug_t<T,E>& s2) { return s1 op static_cast<T>(s2); } \
template<typename T,typename E> static inline bool operator##op (const debug_t<T,E>& s1, const T& s2) { return static_cast<T>(s2) op s2; } 
	DEBUG_OP(== )
		DEBUG_OP(!= )
		DEBUG_OP(> )
		DEBUG_OP(< )
		DEBUG_OP(>= )
		DEBUG_OP(<= )
#undef DEBUG_OP
#define DEBUG_MATH(op)	\
template<typename T,typename E> static inline T operator##op (const T& s1, const debug_t<T,E>& s2) { return s1 op static_cast<T>(s2); } \
template<typename T,typename E> static inline T operator##op (const debug_t<T,E>& s1, const T& s2) { return static_cast<T>(s2) op s2; } 
		DEBUG_MATH(&)
		DEBUG_MATH(| )
		DEBUG_MATH(+)
		DEBUG_MATH(-)
		DEBUG_MATH(*)
		DEBUG_MATH(/ )
#undef DEBUG_MATH

		class file {
		size_t _length;
		size_t _offset;
		sys_file _file;
		public:
			using ios_base = std::ios_base;
			void set_offset(size_t offset, size_t length) {
				if (_offset != offset && _length != length) {
					_offset = offset;
					_length = length;
					if (is_open())  _file.seek(_offset);
				}
			}
			void swap(file& other) {
				if (std::addressof(other) != this) {
					_file.swap(other._file);
					std::swap(_length, other._length);
					std::swap(_offset, other._offset);
				}
			}
			file() : _file(), _offset(0U), _length(0) { }
			explicit operator sys_file&() { return _file; }
			explicit operator const sys_file&() const { return _file; }

			file(const char* filename, ios_base::openmode mode = ios_base::in | ios_base::binary) :_length(0U), _offset(0U), _file(filename, mode) {
				if (_file.is_open())  _length = _file.file_size();
			}
			file(const char* filename, size_t offset, size_t length, ios_base::openmode mode = ios_base::in | ios_base::binary) :_length(length), _offset(offset), _file(filename, mode) {
				if (_file.is_open())  _file.seek(offset);
			}
			file(const file&) = delete; // no copy
			file& operator=(const file&) = delete; // no copy
			file(file&& move) { swap(move); }
			file& operator=(file&& move) { swap(move); return *this; }
			bool open(const char* filename, std::ios_base::openmode mode = ios_base::in | ios_base::binary) {
				if (_file.open(filename, mode)) {
					if (_length == 0)
						_length = _file.file_size();
					else
						_file.seek(_offset);
					return true;
				}
				return false;
			}
			size_t read(void* dest, size_t count) { return _file.read(dest, count); }
			size_t write(const void* src, size_t count) { return _file.write(src, count); }
			size_t seek(int32_t offset) {
				assert(offset < (int32_t)_length);
				return _file.seek(_offset + offset);
			}
			void close() { _file.close(); }
			bool is_open() const { return _file.is_open(); }
			size_t file_size() const { return _length; }
			size_t offset() const { return _offset; }
	};
	class stream_buffer : public std::basic_streambuf<char_type, char_traits> {
	private:
		file _file;
		int_type _putback;
	public:
		using streambuf_t = std::basic_streambuf<char_type, char_traits>;
		void swap(stream_buffer& other) {
			if (std::addressof(other) != this) {
				_file.swap(other._file);
				std::swap(_putback, other._putback);
			}
		}
		stream_buffer() :_putback(-1) { setp(nullptr, nullptr); setg(nullptr, nullptr, nullptr); }
		stream_buffer(const stream_buffer&) = delete; // no copy
		stream_buffer& operator=(const stream_buffer&) = delete; // no copy
		stream_buffer(stream_buffer&& move) { swap(move); }
		stream_buffer& operator=(stream_buffer&& move) { swap(move); return *this; }

		~stream_buffer() { if (_file.is_open()) _file.close(); }

		file& file() { return _file; }
		bool is_open() const { return _file.is_open(); }
		void close() { _file.close(); }
		bool open(const char* filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::binary) {
			return _file.open(filename, mode);
		}
		void set_offset(off_type offset, off_type length) { _file.set_offset((size_t)offset, (size_t)length); }
		size_t file_length() const { return _file.file_size(); }
	protected:
		pos_type seekpos(pos_type pos, std::ios_base::openmode which
			= std::ios_base::in | std::ios_base::out) override final {
			return _file.seek((size_t)pos);
		}
		std::streampos seekoff(std::streamoff off, std::ios_base::seekdir way,
			std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override final {
			return static_cast<sys_file&>(_file).seek((size_t)off, way);
		}
		int_type overflow(int_type c) override final { assert(0); return EOF; }
		int_type underflow() override final {
			if (_putback < 0) {
				char c;
				size_t count = _file.read(&c, 1);
				if (count == 0)  return char_traits::eof();
				_putback = (uint8_t)c;
			}
			return _putback;
		}
		int_type uflow() override final {
			int_type ret = underflow();
			_putback = -1;
			return _putback;
		}
		int sync() override final { assert(0); return char_traits::eof(); }
		std::streamsize showmanyc()override final { return _file.file_size(); }
		std::streamsize xsgetn(char* s, std::streamsize n) override final {
			assert(n > 0 && s);
			std::streamsize ret = 0;
			if (_putback >= 0) {
				*s++ = char_traits::to_char_type(_putback);
				--n;
				_putback = -1;
				ret = 1;
			}
			ret += _file.read(s, (size_t)n);
			return ret;
		}
		std::streamsize xsputn(const char* s, std::streamsize n) override final { return _file.write(s, (size_t)n); }
	};

	class ifstream : public std::basic_istream<char_type, char_traits> {
	public:
		using stream_t = std::basic_istream<char_type, char_traits>;

		ifstream() : stream_t(&_buffer) {}
		ifstream(const char* str) : stream_t(&_buffer) { _buffer.open(str, std::ios_base::in | std::ios_base::binary); }
		ifstream(ifstream&& move) : _buffer(std::move(move._buffer)), stream_t(&_buffer) { this->set_rdbuf(&_buffer); }
		ifstream& operator=(ifstream&& move) {
			_buffer = std::move(move._buffer);
			this->set_rdbuf(&_buffer);
			return *this;
		}
		bool is_open() const { return _buffer.is_open(); }
		void close() { _buffer.close(); }
		void open(const char* filename) {
			if (_buffer.open(filename, std::ios_base::in | std::ios_base::binary))
				clear();
			else
				setstate(failbit);
		}
		void set_offset(off_type offset, off_type length) { _buffer.set_offset(offset, length); }
		size_t file_length() const { return _buffer.file_length(); }
		// if this goes in the heep, then we are using Zmalloc

	protected:
		stream_buffer _buffer;
	};
	class iofstream : public std::basic_iostream<char_type, char_traits> {
	public:
		using stream_t = std::basic_iostream<char_type, char_traits>;
		void swap(iofstream& other) {
			if (std::addressof(other) != this) {
				_buffer.swap(other._buffer);
				this->set_rdbuf(&_buffer);
				other.set_rdbuf(&other._buffer);
			}
		}
		iofstream() : stream_t(&_buffer) {}
		iofstream(const char* str, int mode = std::ios_base::in | std::ios_base::binary) : stream_t(&_buffer) { _buffer.open(str, mode); }
		iofstream(const iofstream&) = delete; // no copy
		iofstream& operator=(const iofstream&) = delete; // no copy
		iofstream(iofstream&& move) : stream_t(&_buffer) { swap(move); }
		iofstream& operator=(iofstream&& move) { swap(move); return *this; }

		bool is_open() const { return _buffer.is_open(); }
		void close() { _buffer.close(); }
		void open(const char* filename, int mode = std::ios_base::in | std::ios_base::binary) {
			if (_buffer.open(filename, mode))
				clear();
			else
				setstate(failbit);
		}
		void set_offset(off_type offset, off_type length) { _buffer.set_offset(offset, length); }
		size_t file_length() const { return _buffer.file_length(); }
		// if this goes in the heep, then we are using Zmalloc

	protected:
		stream_buffer _buffer;
	};

	class ofstream : public std::basic_ostream<char_type, char_traits> {
	public:
		using stream_t = std::basic_ostream<char_type, char_traits>;
		ofstream() : stream_t(&_buffer) {}
		ofstream(const char* str, int mode = std::ios_base::binary) : stream_t(&_buffer) { open(str, mode); }
		void close() { _buffer.close(); }
		bool is_open() const { return _buffer.is_open(); }
		void open(const char* filename, int mode = std::ios_base::binary) {
			if (_buffer.open(filename, mode | std::ios_base::out))
				clear();
			else
				setstate(failbit);
		}
	protected:
		stream_buffer _buffer;
	};
	// copyed from utl, good ideas


	/// For partial specialization of stream_size_of for objects
	template <typename T> struct object_stream_size {
		inline std::streamsize operator()(const T& v) const { return v.stream_size(); }
	};
	template <typename T> struct integral_object_stream_size {
		inline std::streamsize operator()(const T& v) const { return sizeof(v); }
	};
	/// Returns the size of the given object. Overloads for standard types are available.
	template <typename T>
	inline std::streamsize stream_size_of(const T& v) {
		using stream_sizer_t = typename std::conditional_t<numeric_limits<T>::is_integral, integral_object_stream_size<T>, object_stream_size<T>>; ;
		return stream_sizer_t()(v);
	}


	template<typename FUNC, typename T>
	inline size_t _to_number_int(FUNC f, const quake::string_view& str, T& value, int base = 0) {
		const char* sp = str.data();
		char* endp = nullptr;
		value = f(sp, &endp, base);
		return endp - sp;
	}
	template<typename FUNC, typename T>
	inline size_t _to_number_float(FUNC f, const quake::string_view& str, T& value) {
		const char* sp = str.data();
		char* endp = nullptr;
		value = f(sp, &endp);
		return endp - sp;
	}
	inline size_t to_number(const quake::string_view& str, float& value) { return _to_number_float(::strtof, str, value); }
	inline size_t to_number(const quake::string_view& str, double& value) { return _to_number_float(::strtod, str, value); }
	inline size_t to_number(const quake::string_view& str, long double& value) { return _to_number_float(::strtold, str, value); }
	inline size_t to_number(const quake::string_view& str, int& value, int base = 0) { return _to_number_int(::strtol, str, value, base); }
	inline size_t to_number(const quake::string_view& str, long& value, int base = 0) { return _to_number_int(::strtol, str, value, base); }
	inline size_t to_number(const quake::string_view& str, unsigned long& value, int base = 0) { return _to_number_int(::strtoul, str, value, base); }



	// fixed buffer string
	template<size_t SIZE> class fixed_string_stream;
#if 0

	using string = ustl::string;
	

	template<size_t _SIZE>
	using fixed_string = ustl::static_string<_SIZE>;
	using string_buffer = ustl::fixed_string;

#endif
	using zstring = std::basic_string<char, std::char_traits<char>, ZAllocator<char>>;
	using string = std::string;
	using  string_view = std::string_view;
	using string_buffer = std::string;
	template<size_t _SIZE>
	using fixed_string = std::basic_string<char, std::char_traits<char>, umm_stack_allocator<char,_SIZE>>;



	class fixed_string_buffer : public std::streambuf {
	public:
		void clear() {
			if (_buf.data()) {
				_buf.resize(0U);
				setp(_buf.data(), _buf.data() + capacity() - 1);  // always have room for the zero
				*pptr() = 0;
			} else setp(nullptr, nullptr);
		} 
		//fixed_string_buffer() : _str() { }
		fixed_string_buffer(const fixed_string_buffer& copy) = delete;
		fixed_string_buffer(fixed_string_buffer && move) : _buf(move._buf), std::streambuf(move) {}
		fixed_string_buffer(char* buffer, size_t size) : _buf(buffer,size) { clear(); }
		size_t capacity() const { return _buf.capacity(); }
		size_t size() const { return pbase() ? pptr() - pbase() : 0; }
		// fuck windows, just fuck them
		// I had to zero terminate here becuase windows would take the raw buffer pointer
		string_buffer str() {
			_buf.resize(size());
			return string_buffer(_buf.data(),_buf.size());
		}
		string_buffer str() const {
			const_cast<ustl::fixed_memblock&>(_buf).resize(size());
			return string_buffer(_buf);
		}
		void swap(fixed_string_buffer& r)
		{	// swap with _Right
			if (this != std::addressof(r)) {
				std::swap(_buf,r._buf);
				std::streambuf::swap(r);
			}
		}
	protected:
		ustl::fixed_memblock _buf;


		pos_type seekoff(off_type off, std::ios_base::seekdir way, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override final {
			return std::streambuf::seekoff(off, way, which);
		}
		pos_type seekpos(pos_type ptr, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out) override final {
			// change position to _Pos, according to _Mode
			return std::streambuf::seekpos(ptr, mode);
		}
#if 0
	// position within write buffer
					{	// change position by _Off, according to _Way, _Mode
						if (_Mysb::pptr() != 0 && _Seekhigh < _Mysb::pptr())
							_Seekhigh = _Mysb::pptr();	// update high-water pointer
					if (_Way == ios_base::end)
						_Off += (off_type)(_Seekhigh - _Mysb::eback());
					else if (_Way == ios_base::cur)
						_Off += (off_type)(_Mysb::pptr() - _Mysb::eback());
					else if (_Way != ios_base::beg)
						_Off = _BADOFF;

					if (0 <= _Off && _Off <= _Seekhigh - _Mysb::eback())
						_Mysb::pbump((int)(_Mysb::eback()
							- _Mysb::pptr() + _Off));	// change write position
					else
						_Off = _BADOFF;
				}



		else if (_Off != 0)
		}
				virtual pos_type seekpos(pos_type _Ptr,
					ios_base::openmode _Mode = ios_base::in | ios_base::out)
				{	// change position to _Pos, according to _Mode
					streamoff _Off = (streamoff)_Ptr;
					if (_Mysb::pptr() != 0 && _Seekhigh < _Mysb::pptr())
						_Seekhigh = _Mysb::pptr();	// update high-water pointer

					if (_Off == _BADOFF)
						;
					else if (_Mode & ios_base::in && _Mysb::gptr() != 0)
					{	// position within read buffer
						if (0 <= _Off && _Off <= _Seekhigh - _Mysb::eback())
						{	// change read position
							_Mysb::gbump((int)(_Mysb::eback() - _Mysb::gptr() + _Off));
							if (_Mode & ios_base::out && _Mysb::pptr() != 0)
								_Mysb::setp(_Mysb::pbase(), _Mysb::gptr(),
									_Mysb::epptr());	// change write position to match
						}
						else
							_Off = _BADOFF;
					}
					else if (_Mode & ios_base::out && _Mysb::pptr() != 0)
					{	// position within write buffer
						if (0 <= _Off && _Off <= _Seekhigh - _Mysb::eback())
							_Mysb::pbump((int)(_Mysb::eback()
								- _Mysb::pptr() + _Off));	// change write position
						else
							_Off = _BADOFF;
					}
					else
						_Off = _BADOFF;	// neither read nor write buffer selected, fail
					return (streampos(_Off));
				}
#endif
		std::streambuf* setbuf(char * buffer, std::streamsize size) override final {
			_buf.manage(buffer, size);
			clear();
			return std::streambuf::setbuf(buffer, size);
		}
		// ugh set underflow for get
		// we are using the p buffer for it
		// be careful with this as there isn't any checking on it
		int_type underflow() override final {
			if (pbase() == nullptr) return EOF;
			setg(_buf.data(), _buf.data() + 1, _buf.data() + size());
			setp(nullptr,nullptr);  // clear the output as we are reading
			return _buf.data()[0];
		}
		// good suggestion
		// https://stackoverflow.com/questions/5642063/inheriting-ostream-and-streambuf-problem-with-xsputn-and-overflow
		int_type overflow(int_type ch) override final {
			if (pbase() == nullptr) {
				// save one char for next overflow:
				clear();
				assert(pbase() != nullptr); // no buffer was assigned
				if (ch != traits_type::eof()) {
					*pptr() = traits_type::to_char_type(ch);
					pbump(1);

				}
				else ch = 0;
			}
			else {
				// we are at the end of the buffer,
				Sys_Error("fixedOutBuffer: Overflow, out of space!"); // drop and quit here
				return traits_type::to_int_type(traits_type::eof());
			}
			*pptr() = 0; // always zero terminate
			return traits_type::not_eof(ch);
		}
	};

	class buffer_string_stream : public std::ostream {
	public:
		buffer_string_stream(char* buffer, size_t size) :_sbuf(buffer,size), std::ostream(&_sbuf) {  }
		auto str() const { return _sbuf.str(); }
		auto str()  { return _sbuf.str(); }
		size_t size() const { return _sbuf.size(); }
		void clear() { _sbuf.clear(); }
		fixed_string_buffer* rdbuf() const   { return const_cast<fixed_string_buffer*>(&_sbuf); }
	protected:
	
		fixed_string_buffer _sbuf;
	};

	template<size_t SIZE>
	class fixed_string_stream : public buffer_string_stream {
	public:
		using string_type = fixed_string<SIZE>;
		fixed_string_stream() : buffer_string_stream(_sbuffer.data(), _sbuffer.size()) { }
	protected:
		std::array<char, SIZE> _sbuffer;
	};

	template<typename T>
	class ref_string_stream : public buffer_string_stream {
	public:
		ref_string_stream(T& str) : _str(str), buffer_string_stream(str.data(), str.capacity()) { str.clear(); }
		ref_string_stream(ref_string_stream&& move) : _str(move._str), _sbuf(_sbuf) {}
		void clear() { _sbuf.clear(); }
		T& str() {
			_str._size = _sbuf.size(); // got to do this before it gets returned
			return _sbuf.str();
		}
		const T& str() const {
			const_cast<T&>(_str)._size = _sbuf.size(); // got to do this before it gets returned
			return _sbuf.str();
		}
	protected:
		T& _str;
		fixed_string_buffer _sbuf;
	};
#if 0
	template<size_t SIZE>
	auto make_ref_string_stream(quake::fixed_string<SIZE> & ref) { return std::move(ref_string_stream<fixed_string<SIZE>>(ref)); }
#endif
	using path_string = fixed_string<MAX_OSPATH>;

	template<typename VT>
	class FixedVector {

		template <typename T>
		inline void construct_at(T* p) { new (p) T; }
		template <typename T>
		inline void construct_at(T* p, const T& value) { new (p) T(value); }
		template <typename T>
		inline void construct_at(T* p, T&& value) { new (p) T(std::move<T>(value)); }
		template <typename T> inline void construct(T* p) { construct_at(p); }
	public:
		using value_type = VT;
		using reference = value_type&;
		using const_reference = const value_type&;;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using iterator = pointer;
		using const_iterator = const_pointer;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		using size_type = size_t;


		FixedVector() : _data(nullptr), _size(0U), _capacity(0U) {}
		FixedVector(value_type* data, size_type capacity) : _data(data), _size(0U), _capacity(capacity) {}
		size_type capacity() const { return _capacity; }
		size_type size() const { return _size; }
		pointer data()  { return _data; }
		const_pointer data() const { return _data; }
		iterator begin() { return _map; }
		iterator eend() { return _map + _size; }
		const_iterator begin() const { return _map; }
		const_iterator end() const { return _map + _size; }
		inline bool	empty(void) const { return _size == 0U; }
		inline reference		at(size_type i) { assert(i < size()); return begin()[i]; }
		inline const_reference	at(size_type i) const { assert(i < size()); return begin()[i]; }
		inline reference		operator[] (size_type i) { return at(i); }
		inline const_reference	operator[] (size_type i) const { return at(i); }
		inline reference		front(void) { return at(0); }
		inline const_reference	front(void) const { return at(0); }
		inline reference		back(void) { assert(!empty()); return end()[-1]; }
		inline const_reference	back(void) const { assert(!empty()); return end()[-1]; }
		inline void			pop_back(void) { sif(_size > 0) { td::destroy(end() - 1); --_size; } }
		inline void			clear(void) { std::destroy(begin(), end()); _data.clear(); }

		void resize(size_type n) {
			std::destroy(begin() + n, end());
			assert(capacity() > nb);
			std::uninitialized_default_construct_n(end(), nb - _size);
			_size = nb;
		}
		inline void assign(const_iterator i1, const_iterator i2)
		{
			assert(i1 <= i2);
			resize(std::distance(i1, i2));
			std::copy(i1, i2, begin());
		}
		template<typename T>
		inline void assign(size_type n, const T& v)
		{
			resize(n);
			std::fill(begin(), end(), v);
		}

		inline iterator insert_hole(const_iterator ip, size_type n)
		{
			assert(data() || !n);
			assert(cmemlink::begin() || !n);
			assert(ip >= begin() && ip + n <= end());
			iterator start = const_cast<iterator>(ip);
			std::rotate(start, end() - n, end());
			return start;
		}
		inline iterator insert_space(const_iterator ip, size_type n)
		{
			iterator ih = insert_hole(ip, n);
			std::uninitialized_default_construct_n(ih, n);
			return ih;
		}
		inline iterator insert(const_iterator ip, size_type n, const_reference v)
		{
			iterator d = insert_hole(ip, n);
			std::uninitialized_fill_n(d, n, v);
			return d;
		}
		inline iterator insert(const_iterator ip, const_reference v)
		{
			iterator d = insert_hole(ip, 1);
			new(d)(v);
			std::construct_at(d, v);
			return d;
		}
		inline iterator insert(const_iterator ip, const_iterator i1, const_iterator i2)
		{
			assert(i1 <= i2);
			iterator d = insert_hole(ip, distance(i1, i2));
			std::uninitialized_copy(i1, i2, d);
			return d;
		}
//		inline iterator		insert(const_iterator, const_reference v) { return insert(v).first; }
		void			insert(const_iterator i1, const_iterator i2) { for (; i1 != i2; ++i1) insert(*i1); }

		inline iterator erase(const_iterator cstart, size_type n)
		{
			assert(data() || !n);
			assert(cmemlink::begin() || !n);
			assert(cstart >= begin() && cstart + n <= end());
			iterator d = const_cast<iterator>(cstart);
			std::destroy_n(d, n);
			iterator start = const_cast<iterator>(cstart);
			std::rotate(start, start + n, end());
			return start;
	
		}
		inline iterator erase(const_iterator ep1, const_iterator ep2)
		{
			assert(ep1 <= ep2);
			return erase(ep1, distance(ep1, ep2));
		}
		template<typename T>
		void push_back(T&& v)
		{
			assert(_size + 1 < capacity());
			_size++;
			new(end()) value_type(std::move(v));
		} 
		template<typename T>
		void push_back(const T& v = T())
		{
			assert(_size + 1 < capacity());
			_size++;
			new(end()) value_type(v);
		}
		template <typename... Args>
		iterator  emplace(const_iterator ip, Args&&... args)
		{
			assert(_size + 1 < capacity());
			return new (insert_hole(ip, 1)) T(std::forward<Args>(args)...);
		}

		/// Constructs value at the end of the vector.
		template <typename... Args>
		inline void emplace_back(Args&&... args)
		{
			assert(_size + 1 < capacity());
			_size++;
			new(end()) value_type(std::forward<Args>(args)...);
		}
	private:
		value_type* _data;
		size_type _size;
		size_type _capacity;
	};

	
	template <typename Pair, typename Comp>
	struct pair_compare_first { //:  std::binary_function<Pair, Pair, bool> {
		inline bool operator()(const Pair& a, const Pair& b) { return Comp()(a.first, b.first); }
	};
	template <typename K, typename V, typename Comp>
	struct pair_compare_first_key { //:  std::binary_function<std::pair<K, V>, K, bool> {
		inline bool operator()(const std::pair<K, V>& a, const K& b) { return Comp()(a.first, b); }
		inline bool operator()(const K& a, const std::pair<K, V>& b) { return Comp()(a, b.first); }
	};
	// quake hash map
	//
	// hash map tables are Zmallcoed always, but the individual leafs can be anything


	// special case string set that manages strings
	// so we can lookup and and save them

	class StringSet {
		static constexpr size_t inital_buckets = 16;
		struct string_bucket {
			link_t<string_bucket> chain;
			quake::string_view str;
			string_bucket* next;
		};
	public:
		//StringSet() : _buckets(16) {}

	private:
		//UVector<string_bucket*> _buckets;

	};
	// humm work on this more
#if 0
	template<typename K, typename V, typename H, typename E, quake::Alloc A> class unordered_map;
	// used mainly for strings and other hash items that only last per level
	// you cannot erase individual items without wipeing them all
	template<typename K, typename V, typename H=std::hash<K>, typename E= std::equal_to<K>>
	class unordered_map<K, V, H, E, Alloc::HulkLow> {
	public:
		static constexpr size_t inital_buckets = 16;
		using value_type = std::pair<K, V>;
		using key_value_type = std::pair<value_type, value_type*>;

		unordered_map() : _buckets((key_value_type*)Z_Malloc(inital_buckets * sizeof(key_value_type), 16)), _count(0) {}


		void clear() { _buckets.clear(); }

	private:
		FixedVector<key_value_type> _buckets;
		size_t _count;
	};
#endif
	/// \class map umap.h ustl.h
	/// \ingroup AssociativeContainers
	///
	/// \brief A sorted associative container of pair<K,V>
	///
	template <typename K, typename V, typename Comp = std::less<K> >
	class map : public FixedVector<std::pair<K, V> > {
	public:
		typedef K						key_type;
		typedef V						data_type;
		typedef const K&					const_key_ref;
		typedef const V&					const_data_ref;
		typedef const map<K, V, Comp>&			rcself_t;
		typedef FixedVector<std::pair<K, V> >				base_class;
		typedef typename base_class::value_type		value_type;
		typedef typename base_class::size_type		size_type;
		typedef typename base_class::pointer		pointer;
		typedef typename base_class::const_pointer		const_pointer;
		typedef typename base_class::reference		reference;
		typedef typename base_class::const_reference	const_reference;
		typedef typename base_class::const_iterator		const_iterator;
		typedef typename base_class::iterator		iterator;
		typedef typename base_class::reverse_iterator	reverse_iterator;
		typedef typename base_class::const_reverse_iterator	const_reverse_iterator;
		typedef std::pair<const_iterator, const_iterator>		const_range_t;
		typedef std::pair<iterator, iterator>			range_t;
		typedef std::pair<iterator, bool>				insertrv_t;
		typedef Comp					key_compare;
		typedef pair_compare_first<value_type, Comp>		value_compare;
		typedef pair_compare_first_key<K, V, Comp>		value_key_compare;
	public:
		inline			map(void) : base_class() {}
		map(value_type* data, size_type capacity) : base_class(data, capacity) {}
#if 0
		explicit inline		map(size_type n) : base_class(n) {}
		inline			map(rcself_t v) : base_class(v) {}
		inline			map(const_iterator i1, const_iterator i2) : base_class() { insert(i1, i2); }
#endif
		inline rcself_t		operator= (rcself_t v) { base_class::operator= (v); return *this; }
		inline const_data_ref	at(const_key_ref k) const { assert(find(k) != end()); return find(k)->second; }
		inline data_type&		at(const_key_ref k) { assert(find(k) != end()); return find(k)->second; }
		inline const_data_ref	operator[] (const_key_ref i) const { return at(i); }
		data_type&			operator[] (const_key_ref i);
		inline key_compare		key_comp(void) const { return key_compare(); }
		inline value_compare	value_comp(void) const { return value_compare(); }
		inline size_type		size(void) const { return base_class::size(); }
		inline iterator		begin(void) { return base_class::begin(); }
		inline const_iterator	begin(void) const { return base_class::begin(); }
		inline iterator		end(void) { return base_class::end(); }
		inline const_iterator	end(void) const { return base_class::end(); }
		inline void			assign(const_iterator i1, const_iterator i2) { clear(); insert(i1, i2); }
		inline void			push_back(const_reference v) { insert(v); }
		inline const_iterator	find(const_key_ref k) const;
		inline iterator		find(const_key_ref k) { return const_cast<iterator> (const_cast<rcself_t>(*this).find(k)); }
		inline const_iterator	find_data(const_data_ref v, const_iterator first = nullptr, const_iterator last = nullptr) const;
		inline iterator		find_data(const_data_ref v, iterator first = nullptr, iterator last = nullptr) { return const_cast<iterator> (find_data(v, const_cast<const_iterator>(first), const_cast<const_iterator>(last))); }
		const_iterator		lower_bound(const_key_ref k) const { return std::lower_bound(begin(), end(), k, value_key_compare()); }
		inline iterator		lower_bound(const_key_ref k) { return const_cast<iterator>(const_cast<rcself_t>(*this).lower_bound(k)); }
		const_iterator		upper_bound(const_key_ref k) const { return std::upper_bound(begin(), end(), k, value_key_compare()); }
		inline iterator		upper_bound(const_key_ref k) { return const_cast<iterator>(const_cast<rcself_t>(*this).upper_bound(k)); }
		const_range_t		equal_range(const_key_ref k) const { return std::equal_range(begin(), end(), k, value_key_compare()); }
		inline range_t		equal_range(const_key_ref k) { return std::equal_range(begin(), end(), k, value_key_compare()); }
		inline size_type		count(const_key_ref v) const { const_range_t r = equal_range(v); return std::distance(r.first, r.second); }
		insertrv_t			insert(const_reference v);
		inline iterator		insert(const_iterator, const_reference v) { return insert(v).first; }
		void			insert(const_iterator i1, const_iterator i2) { for (; i1 != i2; ++i1) insert(*i1); }
		inline void			erase(const_key_ref k);
		inline iterator		erase(iterator ep) { return base_class::erase(ep); }
		inline iterator		erase(iterator ep1, iterator ep2) { return base_class::erase(ep1, ep2); }
		inline void			clear(void) { base_class::clear(); }
		inline void			swap(map& v) { base_class::swap(v); }
#if HAVE_CPP11
		using initlist_t = std::initializer_list<value_type>;
		inline			map(map&& v) : base_class(move(v)) {}
		inline			map(initlist_t v) : base_class() { insert(v.begin(), v.end()); }
		inline map&			operator= (map&& v) { base_class::operator= (move(v)); return *this; }
		insertrv_t			insert(value_type&& v);
		inline iterator		insert(const_iterator, value_type&& v) { return insert(move(v)).first; }
		inline void			insert(initlist_t v) { insert(v.begin(), v.end()); }
		template <typename... Args>
		inline insertrv_t		emplace(Args&&... args) { return insert(value_type(forward<Args>(args)...)); }
		template <typename... Args>
		inline iterator		emplace_hint(const_iterator h, Args&&... args) { return insert(h, value_type(forward<Args>(args)...)); }
		template <typename... Args>
		inline insertrv_t		emplace_back(Args&&... args) { return insert(value_type(forward<Args>(args)...)); }
#endif
	};

	/// Returns the pair<K,V> where K = \p k.
	template <typename K, typename V, typename Comp>
	inline typename map<K, V, Comp>::const_iterator map<K, V, Comp>::find(const_key_ref k) const
	{
		const_iterator i = lower_bound(k);
		return (i < end() && Comp()(k, i->first)) ? end() : i;
	}

	/// Returns the pair<K,V> where V = \p v, occuring in range [first,last).
	template <typename K, typename V, typename Comp>
	inline typename map<K, V, Comp>::const_iterator map<K, V, Comp>::find_data(const_data_ref v, const_iterator first, const_iterator last) const
	{
		if (!first) first = begin();
		if (!last) last = end();
		for (; first != last && first->second != v; ++first);
		return first;
	}

	/// Returns data associated with key \p k.
	template <typename K, typename V, typename Comp>
	typename map<K, V, Comp>::data_type& map<K, V, Comp>::operator[] (const_key_ref k)
	{
		iterator ip = lower_bound(k);
		if (ip == end() || Comp()(k, ip->first))
			ip = base_class::insert(ip, make_pair(k, V()));
		return ip->second;
	}

	/// Inserts the pair into the container.
	template <typename K, typename V, typename Comp>
	typename map<K, V, Comp>::insertrv_t map<K, V, Comp>::insert(const_reference v)
	{
		iterator ip = lower_bound(v.first);
		bool bInserted = ip == end() || Comp()(v.first, ip->first);
		if (bInserted)
			ip = base_class::insert(ip, v);
		return make_pair(ip, bInserted);
	}

#if HAVE_CPP11
	/// Inserts the pair into the container.
	template <typename K, typename V, typename Comp>
	typename map<K, V, Comp>::insertrv_t map<K, V, Comp>::insert(value_type&& v)
	{
		iterator ip = lower_bound(v.first);
		bool bInserted = ip == end() || Comp()(v.first, ip->first);
		if (bInserted)
			ip = base_class::insert(ip, move(v));
		return make_pair(ip, bInserted);
	}
#endif

	/// Erases the element with key value \p k.
	template <typename K, typename V, typename Comp>
	inline void map<K, V, Comp>::erase(const_key_ref k)
	{
		iterator ip = find(k);
		if (ip != end())
			erase(ip);
	}

	// container that has a precreated map
	template<typename K, typename V, typename Comp = std::less<K> >
	class FixedMap {
		
	public:
		using value_type = std::pair<K, V>;
		using refrence = value_type&;
		using const_refrence = const refrence;
		using pointer = value_type*;
		using const_pointer = const pointer;
		using const_iterator = const_pointer;
		using iterator = pointer;

		using key_compare = Comp;
		using key_type = K;
		using data_type = V;
		using const_key_ref = const K&;
		using const_data_ref = const V&;;

		using self_t = FixedMap<K, V, Comp>;
		using rcself_t = const self_t&;

		using const_key_ref = const K&;

		struct value_key_compare {
			Comp cmp;
			constexpr bool operator()(const value_type& l, const value_type& r) const { return cmp(l.first, r.first); }
			constexpr bool operator()(const value_type& l, const key_type& r) const { return cmp(l.first, r); }
			constexpr bool operator()(const key_type& l, const value_type& r) const { return cmp(l, r.first); }
		};
	
		static value_type* AllocHulk(size_t count) { return (value_type*)Hunk_Alloc(count * sizeof(value_type)); }
		static value_type* AllocHulk(size_t count, cstring_t v) { return (value_type*)Hunk_AllocName(count * sizeof(value_type), v); }
		FixedMap() : _map(nullptr), _size(0) {}
		FixedMap(value_type* ptr, size_t size) : _map(ptr), _size(0) {}
		FixedMap(size_t count) : _map(AllocHulk(count)), _size(size) {}
		FixedMap(size_t count, cstring_t v) : _map(AllocHulk(count, v)), _size(size) {}
		void alloc_hulk(size_t count) { _map = AllocHulk(count); _size = count; }
		void alloc_hulk(size_t count, cstring_t v) { _map = AllocHulk(count, v); _size = count; }
		iterator begin() { return _map; }
		iterator eend() { return _map + _size; }
		const_iterator begin() const { return _map; }
		const_iterator end() const { return _map + _size; }
		size_t size() const { return _size; }
		value_type* data() { return _map; }
		const value_type* data() const { return _map; }
		void sort() { 

			std::sort(_map, _map + _size, value_key_compare());
#if 0
			std::sort(_map, _map+ _size,[](const value_type& l, const value_type& r) {
				return l.first < r.first;
			});
#endif
		}

		const_iterator		lower_bound(const_key_ref k) const { return std::lower_bound(begin(), end(), k, value_key_compare()); }
		inline iterator		lower_bound(const_key_ref k) { return const_cast<iterator>(const_cast<rcself_t>(*this).lower_bound(k)); }
		const_iterator		upper_bound(const_key_ref k) const { return std::upper_bound(begin(), end(), k, value_key_compare()); }
		inline iterator		upper_bound(const_key_ref k) { return const_cast<iterator>(const_cast<rcself_t>(*this).upper_bound(k)); }
		inline value_type* find(const_key_ref k) const {
			const_iterator i = lower_bound(k);
			return (i < end() && Comp()(k, i->first)) ? end() : i;
		}
		
		inline iterator		find(const_key_ref k) { return const_cast<iterator> (const_cast<rcself_t>(*this).find(k)); }

	private:
		value_type* _map;
		size_t _size;
	};


	template <typename Key, typename T, typename Compare = std::less<Key>, typename ContainerType = std::vector<std::pair<Key, T>>>
		class flat_map : private ContainerType {
		public:
			using key_type = Key;
			using mapped_type = T;
			using value_type = std::pair<Key, T>;
			using container_type = ContainerType;
			using key_compare = Compare;

			struct value_compare {
			protected:
				key_compare keyComp;
			public:
				value_compare(key_compare kc) : keyComp(kc) {}
				bool operator()(const value_type &lhs, const value_type &rhs) const {
					return keyComp(lhs.first, rhs.first);
				}
			};

		private:
			key_compare keyComp;
			value_compare valComp{ keyComp };
		public:
			using typename container_type::size_type;
			using typename container_type::difference_type;
			using typename container_type::allocator_type;
			using typename container_type::reference;
			using typename container_type::const_reference;
			using typename container_type::pointer;
			using typename container_type::const_pointer;
			using typename container_type::iterator;
			using typename container_type::const_iterator;
			using typename container_type::reverse_iterator;
			using typename container_type::const_reverse_iterator;

			using container_type::begin;
			using container_type::cbegin;

			using container_type::rbegin;
			using container_type::crbegin;

			using container_type::end;
			using container_type::cend;

			using container_type::rend;
			using container_type::crend;

			using container_type::empty;
			using container_type::size;
			using container_type::max_size;
			using container_type::reserve;
			using container_type::clear;

			template <typename... Args>
			std::pair<iterator, bool> emplace(Args&&... args) {
				container_type::emplace_back(std::forward<Args>(args)...);
				auto lower = std::lower_bound(begin(), end(), container_type::back(), valComp);
				if (lower->first == container_type::back().first && lower != end() - 1) {
					container_type::pop_back();
					return{ lower, false };
				}
				if (lower == end() - 1) {
					return{ end() - 1, true };
				}
				return{ std::rotate(rbegin(), rbegin() + 1, reverse_iterator{ lower }).base(), true };

			}

			iterator find(const key_type &key) {
				auto lower = std::lower_bound(begin(), end(), key,
					[&](const value_type &val, const key_type &key) {
					return keyComp(val.first, key);
				});
				if (lower != end() && lower->first == key) return lower;
				return end();
			}
			const_iterator find(const key_type &key) const {
				auto lower = std::lower_bound(begin(), end(), key,
					[&](const value_type &val, const key_type &key) {
					return keyComp(val.first, key);
				});
				if (lower != end() && lower->first == key) return lower;
				return end();
			}

			iterator erase(const_iterator pos) {
				return container_type::erase(pos);
			}
			size_type erase(const key_type &key) {
				auto itr = find(key);
				if (itr == end()) return 0;
				erase(itr);
				return 1;
			}
};
};

#if 0
// it could be data_view<char> but mabye data could be binary data so lets not
template<typename T>
inline typename  std::enable_if<std::is_base_of<quake::string_view, T>::value, std::ostream&>::type
operator<<(std::ostream& os, cstring_t s) {
	os.write(s.data(), s.size());
	return os;
}
#endif

// any dynamicly defined stuff needs to be created after this

#include "zone.h"
#include "mathlib.h"

//============================================================================


class sizebuf_t { //: public quake::memblock {
	qboolean	_allowoverflow;	// if false, do a Sys_Error
	qboolean	_overflowed;	// set to true if the buffer size failed
	int			_hunk_low_used;
	size_t		_cursize;
	size_t		_capacity;
	size_t		_read_count;
	byte*		_data;
protected:
	sizebuf_t(const sizebuf_t& copy) :
		_allowoverflow(copy._allowoverflow),
		_overflowed(copy._overflowed),
		_hunk_low_used(copy._hunk_low_used),
		_data(copy._data),
		_capacity(copy._capacity),
		_cursize(copy._cursize),
		_read_count(copy._read_count) { }
	// copy is only used for static_size_buf
public:
	using refrence = byte&;
	using const_refrence = const byte&;
	using iterator = byte*;
	using const_iterator = const byte*;
	using size_type = size_t;
	sizebuf_t() : _allowoverflow(false), _overflowed(false), _hunk_low_used(0U), _data(nullptr), _capacity(0U), _cursize(0U) , _read_count(0U) {}
	sizebuf_t(byte* data, size_t size, bool allowoverflow = true) : _allowoverflow(allowoverflow), _overflowed(false), _hunk_low_used(0U), _data(data), _capacity(size), _cursize(0U), _read_count(0U) {}
	template<size_t N>
	sizebuf_t(byte (&data)[N], bool allowoverflow = true) : _allowoverflow(allowoverflow), _overflowed(false), _hunk_low_used(0U), _data(data), _capacity(N), _cursize(0U), _read_count(0U) {}
	template<size_t N>
	sizebuf_t(char(&data)[N], bool allowoverflow = true) : _allowoverflow(allowoverflow), _overflowed(false), _hunk_low_used(0U), _data((byte*)data), _capacity(N), _cursize(0U), _read_count(0U) {}

	void* GetSpace(size_t length);
	

	void swap(sizebuf_t& other) {
		if (std::addressof(other) != this) {
			std::swap(_allowoverflow, other._allowoverflow);
			std::swap(_overflowed, other._overflowed);
			std::swap(_hunk_low_used, other._hunk_low_used);
			std::swap(_data, other._data);
			std::swap(_capacity, other._capacity);
			std::swap(_cursize, other._cursize);
			std::swap(_read_count, other._read_count);
		}
	}
	sizebuf_t& operator=(sizebuf_t&& move) {
		if (std::addressof(move) != this) {
			swap(move);
			move.Free(); // clear
		}
		return *this;
	}
	sizebuf_t(sizebuf_t&& move) { swap(move);  move.Free(); }
	sizebuf_t& operator=(const sizebuf_t& copy) {
		if (std::addressof(copy) != this && copy._data != _data) {
			assert(_data); // we got to have some kind of pointer
			if (copy._data == nullptr) Free(); // full clear and release
			else if (copy._cursize == 0) Clear(); // just clear the data
			else {
				assert(_capacity > copy._capacity);
				std::copy(copy.begin(), copy.end(), begin()); // copy it over
			}
			_cursize = copy._cursize;
			_read_count = copy._read_count;
			_allowoverflow = copy._allowoverflow;
			_overflowed = copy._overflowed;
		}
		return *this;
	}
	refrence operator[](size_t i) { return _data[i]; }
	const_refrence operator[](size_t i) const { return _data[i]; }
	const_refrence front() const { return _data[0]; }
	const_refrence back() const { return _data[_cursize-1]; }
	refrence front()  { return _data[0]; }
	refrence back()  { return _data[_cursize - 1]; }
	inline size_t size() const { return _cursize; }
	inline size_t maxsize() const { return _capacity; }
	inline void resize(size_t size) { assert((size + 1) < maxsize()); _data[size] = 0; _cursize = size; }
	inline const byte* data() const { return _data; }
	inline byte* data() { return _data; }
	inline bool overflowed() const { return _overflowed; }
	inline bool allow_overflow() const { return _allowoverflow;  }
	inline void overflowed(bool value) { _overflowed = value; }
	inline iterator begin() { return data(); }
	inline const_iterator begin() const { return data(); }
	inline iterator end() { return data() + _cursize; }
	inline const_iterator end() const { return data() + _cursize; }

	/// Shifts the data in the linked block from \p start to \p start + \p n.
	/// The contents of the uncovered bytes is undefined.
	void InsertArea(const_iterator cstart, size_type n);

	/// Shifts the data in the linked block from \p start + \p n to \p start.
	/// The contents of the uncovered bytes is undefined.
	void EraseArea(const_iterator cstart, size_type n);

	void Clear();
	void Free();
	void Alloc(size_t startsize);
	void Insert(const void* data, size_t length, size_t pos=0);
	void Write(const void* data, size_t length);
	void Print(const quake::string_view& data);
	void Print(char c);
	void WriteByte(int c);
	void WriteChar(int c);
	void WriteShort(int c);
	void WriteLong(int c);
	void WriteFloat(float f);
	void WriteString(cstring_t data);
	void WriteCoord(float f);
	void WriteAngle(float f);
	//void WriteAngle16(float f);
	//void WriteDeltaUsercmd( struct usercmd_s *from, struct usercmd_s *cmd);
	//void WriteDeltaEntity(struct entity_state_s *from, struct entity_state_s *to,  qboolean force, qboolean newentity);
	//void WriteDir(vec3_t vector);

	void	BeginReading() { _read_count = 0; }


	template<typename T>
	typename std::enable_if_t<std::is_integral_v<T>,bool> Read(T& value) {
		if (_read_count + sizeof(T) > _cursize) return false;
		// this looks ineffechent but its unrolled on compile
		value = T{};
		for (size_t i = 0; i < (sizeof(T) * 8); i = 8)
			value += _data[_read_count++] << i;
		return true;
	}

	template<typename T>
	typename std::enable_if_t<std::is_integral_v<T>, T> Read() {
		T value;
		return Read(value) ? value : -1;
	}
	int		ReadChar() { return Read<int8_t>(); }
	int		ReadByte() { return Read<uint8_t>(); }
	int		ReadShort() { return Read<int16_t>(); }
	int		ReadLong() { return Read<int32_t>(); }
	float	ReadFloat();
	void    ReadString(char* data, size_t size);
	void    ReadStringLine(char* data, size_t size);

	template<typename T,typename B>
	void ReadString(ustl::string_builder<T,B>& str) {
		str.clear();
		str.reserve(_read_count - _cursize);
		while (_cursize < _read_count) {
			int c = _data[_read_count++];
			if (c == 0 || c == -1) break;
			if (str.size() + 1 >= str.capacity()) {
				str.reserve(str.capacity() + 256);
			}
			str.push_back(c);
		}
	}
	template<typename T, typename B>
	void ReadStringLine(ustl::string_builder<T, B>& str) {
		str.clear();
		str.reserve(_read_count - _cursize);
		while (_cursize < _read_count) {
			int c = _data[_read_count++];
			if (c == 0 || c == -1 || c == '\n') break;
			if (str.size() + 1 >= str.capacity()) {
				str.reserve(str.capacity() + 256);
			}
			str.push_back(c);
		}
	}

	inline float	ReadCoord() { return ReadShort() * (1.0f / 8.0f); }
	void ReadPos(vec3_t& pos) { 
		pos[0] = ReadCoord(); pos[1] = ReadCoord();  pos[2] = ReadCoord();
	}

	float	ReadAngle() { return ReadChar() * (360.0f / 256.0f); }
	float	ReadAngle16() { SHORT2ANGLE(ReadShort()); }
	//void	MSG_ReadDeltaUsercmd( struct usercmd_s *from, struct usercmd_s *cmd);

	void	ReadDir(vec3_t& vector);

	size_t	ReadData(void *buffer, int len);


} ;
template<size_t BYTE_SIZE>
class static_sizebuf_t : public sizebuf_t {
private:
	uint8_t _buffer[BYTE_SIZE];
public:
	using self_t = static_sizebuf_t<BYTE_SIZE>;
	static constexpr size_t byte_size = BYTE_SIZE;
	static_sizebuf_t(bool allow_overflow = true) : sizebuf_t(_buffer, BYTE_SIZE, allow_overflow) {}
	static_sizebuf_t& operator=(const sizebuf_t& copy) {
		if (std::addressof(copy) != this) 
			static_cast<sizebuf_t&>(*this) = static_cast<const sizebuf_t&>(copy);
		return *this;
	}
	template<size_t N>
	static_sizebuf_t& operator=(const static_sizebuf_t<N>& copy) {
		if (std::addressof(copy) != this) *this = static_cast<const sizebuf_t&>(copy);
		return *this;
	}
	template<size_t N>
	static_sizebuf_t(const static_sizebuf_t<N>& copy) : sizebuf_t(_buffer, BYTE_SIZE, copy.allow_overflow()) { *this = copy; }
	static_sizebuf_t(const sizebuf_t& copy) : sizebuf_t(_buffer, BYTE_SIZE, copy.allow_overflow()) { *this = copy; }
	template<size_t N>
	static_sizebuf_t(static_sizebuf_t<N>&& move) : sizebuf_t(_buffer, BYTE_SIZE, move.allow_overflow()) { *this = move; move.Free(); }
	static_sizebuf_t(sizebuf_t&& move) : sizebuf_t(_buffer, BYTE_SIZE, move.allow_overflow()) { *this = move; move.Free(); }
	void swap(sizebuf_t& other) {
		// can't really swap, so we copy
		if (std::addressof(other) != this) {
			*this = other;
		}
	}



};

#if 0
void SZ_Alloc (sizebuf_t *buf, int startsize);
void SZ_Free (sizebuf_t *buf);
void SZ_Clear (sizebuf_t *buf);
void *SZ_GetSpace (sizebuf_t *buf, int length);
void SZ_Write (sizebuf_t *buf, const void *data, int length);
void SZ_Print (sizebuf_t *buf, cstring_t data);	// strcats onto the sizebuf
void SZ_Print(sizebuf_t *buf, char c);
#endif
//============================================================================


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
#if 0
void sizebuf_t *sb->WriteChar(int c);
void sizebuf_t *sb->WriteByte(int c);
// I put some templates here so we can catch some odd balls
// alot of stuff in sv_main gets converted to bytes.  We are talking floats?  There
// are enough warnings for me to think about changing the df file
template<typename T, typename = std::enable_if<!std::is_same<T,int>::value && std::is_arithmetic<T>::value>>
void sizebuf_t *sb->WriteChar(const T c) {
	//static_assert("Do you REALLY want to convert this to char?");
	sb->WriteChar(static_cast<int>(c));
}
template<typename T, typename = std::enable_if<!std::is_same<T, int>::value && std::is_arithmetic<T>::value>>
void sizebuf_t *sb->WriteByte(const T c) {
	//static_assert("Do you REALLY want to convert this to byte?");
	sb->WriteByte(static_cast<int>(c));
}


void sizebuf_t *sb->WriteShort(int c);
template<typename T, typename = std::enable_if<!std::is_same<T, int>::value && std::is_arithmetic<T>::value>>
void sizebuf_t *sb->WriteShort(const T c) {
	//static_assert("Do you REALLY want to convert this to short?");
	sb->WriteShort(static_cast<int>(c));
}


void sizebuf_t *sb->WriteLong(int c);
template<typename T, typename = std::enable_if<!std::is_same<T, int>::value && std::is_arithmetic<T>::value>>
void sizebuf_t *sb->WriteLong(const T c) {
	//static_assert("Do you REALLY want to convert this to long?");
	sb->WriteLong(static_cast<int>(c));
}


void sizebuf_t *sb->WriteFloat(float f);
void sizebuf_t *sb->WriteString(const char *s);
void sizebuf_t *sb->WriteCoord(float f);
void sizebuf_t *sb->WriteAngle(float f);
#endif
extern	int			msg_readcount;
extern	qboolean	msg_badread;		// set if a read goes beyond end of message

void MSG_BeginReading();
int MSG_ReadChar (void);
int MSG_ReadByte (void);
int MSG_ReadShort (void);
int MSG_ReadLong (void);
float MSG_ReadFloat (void);
char *MSG_ReadString (void);

float MSG_ReadCoord (void);
float MSG_ReadAngle (void);

//============================================================================

void Q_memset (void *dest,int fill, size_t count);
void Q_memcpy (void *dest, const void * src, size_t count);
int Q_memcmp (const void * m1, const void * m2, size_t count);

template<typename V,size_t N>
inline void Q_memcpy(V(&dest)[N], const void * src, size_t count) { assert(count < (sizeof(V)*N)); ::memcpy_s(dest, sizeof(V)*N, src, count); }
template<typename V, size_t N>
inline void Q_memset(V(&dest)[N], int fill, size_t count) { assert(count < (sizeof(V)*N)); ::memset(dest, fill, count); }
template<typename V, size_t N>
inline void Q_memcmp(const V(&dest)[N], const void * m2, size_t count) { assert(count < (sizeof(V)*N)); return ::memcmp(dest, src, count); }

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
int Q_strcasecmp(const quake::string_view& s1, const char * s2);
int Q_strcasecmp(const quake::string_view& s1, const quake::string_view& s2);
inline int Q_strcmp(const quake::string_view& a, const quake::string_view& b) { return a.compare(b); }

int Q_strncasecmp (const char * s1, const char * s2, size_t n);
#if 0
bool Q_strncasecmp(cstring_t a, cstring_t b);

#endif

int	Q_atoi (const char * str);
float Q_atof (const char * str);
int	Q_atoi(cstring_t str);
int	Q_atoi(const quake::string_view& str);
float Q_atof(cstring_t str);
float	Q_atof(const quake::string_view& str);

int Q_vsprintf(char* buffer, const char* fmt, va_list va);
int Q_snprintf(char* buffer, size_t buffer_size, const char* fmt, ...);

int Q_vsnprintf(char* buffer, size_t buffer_size, const char* fmt, va_list va);
template<size_t N>
int Q_vsprintf(char(&buffer)[N], const char* fmt, va_list va) { return Q_vsnprintf(buffer, N, fmt, va); }
template<size_t N>
int Q_vsprintf(std::array<char, N>& buffer, const char* fmt, va_list va) { return Q_vsnprintf(buffer.data(), N, fmt, va);  }

int Q_sprintf(char* buffer, const char* fmt, ...);
template<size_t N, typename ... Args>
int Q_sprintf(char(&buffer)[N], const char* fmt, Args&& ... args) { return Q_snprintf(buffer, N, fmt, std::forward<Args>(args)...); }
template<size_t N, typename ... Args>
int Q_sprintf(std::array<char,N>& buffer, const char* fmt, Args&& ... args) { return Q_snprintf(buffer.data(), N, fmt, std::forward<Args>(args)...); }


template<size_t N>
int Q_sprintf(char(&buffer)[N], const char* fmt, ...) { 
	va_list va;
	va_start(va, fmt);
	int ret = Q_vsnprintf(buffer, N, fmt, va);
	va_end(va);
	return ret;
}
namespace quake {


	template<size_t N = 256>
	class va_stack {
		char _buffer[N + 1];
		size_t _size;
	public:
		using char_traits = std::char_traits<char>;
		constexpr size_t size() const { return _size; }
		void clear() { _size = 0;  _buffer[0] = '\0'; }
		constexpr const char* c_str() const { return _buffer; }
		constexpr operator const char*() const { return c_str(); }
		constexpr operator quake::string_view() const { return quake::string_view(c_str(),size()); }
		constexpr size_t capacity() const { return N; }

		void assign(const char* str) {
			_size = 0;
			while (*str) _buffer[_size++] = *str++;
			_buffer[_size] = 0;
		}
		void assign(const char* str, size_t size) {
			_size = size;
			char_traits::copy(_buffer, str, size);
			_buffer[_size] = 0;
		}
		void assign(cstring_t str) { assign(str.data(), str.size()); }
		va_stack& append(const char* str) {
			while (*str) _buffer[_size++] = *str++;
			_buffer[_size] = 0;
			return *this;
		}
		va_stack& append(const char* str, size_t size) {
			char_traits::copy(_buffer + size, str, size);
			_size += size;
			_buffer[_size] = 0;
			return *this;
		}
		va_stack& append(char c) {
			_buffer[_size++] = c;
			_buffer[_size] = 0;
			return *this;
		}
		va_stack& append(char c, size_t count) {
			while (count--) _buffer[_size++] = c;
			_buffer[_size] = 0;
			return *this;
		}
		va_stack& append(cstring_t str) { append(str.data(), str.size()); return *this; }
		template<typename ... Args>
		va_stack& print(const char* fmt, Args&&...args) {
			int ret = Q_sprintf(_buffer, fmt, std::forward<Args>(args)...);
			_size = static_cast<size_t>(ret);
			return *this;
		}
		int printv(const char* fmt, va_list va) {
			int ret = Q_vsnprintf(_buffer, va);
			_size = static_cast<size_t>(ret);
			return ret;
		}
		va_stack() : _size(0U) { _buffer[0] = '\0'; }
		template<typename ... Args>
		va_stack(const char* str, Args&&...args) { print(str, std::forward<Args>(args)...); }
		template<typename ... Args>
		va_stack& operator()(const char* str, Args&&...args) { return print(str, std::forward<Args>(args)...); }
		//va_stack(const char* str) { assign(str); }
		va_stack(cstring_t str) { assign(str); }

		va_stack& operator+=(const char* str) { return append(str); }
		va_stack& operator+=(cstring_t str) { return append(str); }

		template<size_t NN>
		va_stack(const va_stack<NN>& copy) : _size(copy._size) { char_traits::copy(_buffer, copy._buffer, copy._size); }
	};

	template<size_t N>
	static inline std::ostream& operator<<(std::ostream& os, const va_stack<N>& v) {
		os << v.c_str();
		return os;
	};
};

//============================================================================


void COM_Init (const char *path); // path not used?


inline quake::string_view COM_SkipPath(const quake::string_view & in) {
	auto slash = in.find_last_of("/\\");
	return slash != quake::string_view::npos ? in.substr(slash) : in;
}
inline quake::string_view  COM_StripExtension(const quake::string_view & in) {
	auto dot = in.find_last_of('.');
	return in.find_first_of("\\/", dot) != quake::string_view::npos ? in : in.substr(0, dot);
}

inline  quake::string_view  COM_FileExtension(const quake::string_view in) {
	auto dot= in.find_last_of('.');
	return in.find_first_of("\\/", dot) != quake::string_view::npos ? in.substr(dot + 1) : quake::string_view();
}


inline quake::string_view  COM_FileBase(const quake::string_view & in) {
	auto slash = in.find_last_of("/\\");
	auto strip = in.substr(slash == quake::string_view::npos ? 0 : slash + 1);
	return strip.substr(0, strip.find_last_of('.'));
}

template<typename STRING_T>
inline void COM_DefaultExtension(STRING_T& path, const quake::string_view &extension) {
	auto dot = path.find_last_of('.');
	if (dot == STRING_T::npos)
		path.append('.').append(extension);
}

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

class istream_view : public std::istream {
	istreambuf_view _buf;
public:
	using byte = char;
	istream_view(const byte* data, size_t len) :_buf(data, len), std::istream(&_buf) {} 	// ptr + size
	istream_view(const byte* beg, const char* end) : _buf(beg, end), std::istream(&_buf) {} // begin + end
};

class id_little_binary_reader {
	std::istream& _ss;
public:
	id_little_binary_reader(std::istream& s) : _ss(s) {}
	template<typename T>
	typename std::enable_if<std::is_arithmetic<T>::value, id_little_binary_reader&>::type
		operator>>(T& t) { _ss.read((char*)&t, sizeof(T)); return *this; }
	template<typename T>
	void read(T* p, size_t n) { return _ss.read((char*)p, n * sizeof(T)); }
	void read(uint8_t* p, size_t n) {  _ss.read((char*)p, n); }
	void read(char* p, size_t n) {  _ss.read((char*)p, n); }
};

class id_little_binary_writer {
	std::ostream& _ss;
public:
	id_little_binary_writer(std::ostream& s) : _ss(s) {}
	template<typename T>
	typename std::enable_if<!std::is_pointer<T>::value && std::is_arithmetic<T>::value, id_little_binary_writer&>::type
		operator<<(const T& t) { _ss.write((char*)&t, sizeof(T)); return *this; }
	template<typename T>
	void write(const T* p, size_t n) {  _ss.write((char*)p, n * sizeof(T)); }
	void write(const uint8_t* p, size_t n) { _ss.write((char*)p, n); }
	void write(const char* p, size_t n) { _ss.write((char*)p, n); }
};

//template<typename T>
//static inline id_little_binary_writer& operator<<(id_little_binary_writer& bw, const T& v) { bw << v; return *this; }




template<typename T>
class idHunkArray  {
	T* _array;
	size_t _size;
public:
	using value_type = T;
	using pointer = T*;
	using refrence = T&;
	using const_pointer = T*;
	using const_refrence = const T&;
	using iterator = pointer;
	using const_iterator = const_pointer;
	idHunkArray() : _array(nullptr), _size(0) {}
	idHunkArray(uint8_t* array, size_t array_size) : _array((T*)array), _size(array_size) {}
	refrence operator[](size_t i) { return _array[i]; }
	const_refrence operator[](size_t i) const { return _array[i]; }
	refrence at(size_t i) { return _array[i]; }
	const_refrence at(size_t i) const { return _array[i]; }
	iterator begin() { return _array; }
	iterator end() { return _array + _size; }
	const_pointer begin() const { return _array; }
	const_pointer end() const { return _array + _size; }
	size_t size() const { return _size; }
	const uint8_t* data() const { return _array; }
	uint8_t* data()  { return _array; }
};


struct cache_user_t;



void COM_WriteFile (const quake::string_view& filename, const void * data, int len);
//std::istream* COM_OpenFile (cstring_t filename, size_t& length);


byte *COM_LoadStackFile (const quake::string_view& path, void *buffer, int bufsize);
byte *COM_LoadTempFile (const quake::string_view& path);
byte *COM_LoadHunkFile (const quake::string_view& path, size_t* file_size = nullptr);
void COM_LoadCacheFile (const quake::string_view& path, struct cache_user_t *cu);
 size_t COM_FindFile(const quake::string_view& filename, std::fstream& file);




class COM_Value {
public:
	enum Kind {
		LineFeed = '\n',
		Symbol = 'a',
		Eof = -1
	};
	COM_Value() : _type((int)Kind::Eof) {}
	COM_Value(const quake::string_view& text, Kind token=Kind::Symbol) : _type((int)token), _text(text) {}
	bool operator==(const COM_Value& r) const { return _type == r._type; }
	Kind type() const { return (Kind)_type; }
	const quake::string_view& text() const {  return _text; }
	bool operator==(int c) const { return _type == c; }
	bool operator==(Kind c) const { return _type == (int)c; }
	bool operator!=(const COM_Value& r) const { return !(*this == r); }
	bool operator!=(int r) const { return  !(*this == r); }
	bool operator!=(Kind r) const { return !(*this == r); }
private:
	int _type;
	quake::string_view _text;
};

// parser
class COM_Parser {
public:
	COM_Parser() : _data(""), _state(0) {}
	COM_Parser(const quake::string_view & data) : _data(data), _pos(0), _state(0) {}
	bool Next(quake::string_view& token,bool test_eol=false);
	operator const quake::string_view&() const { return _data; }
	size_t pos() const { return _pos; }
	quake::string_view remaining() const { return _data.substr(_pos); }
protected:
	int _state;
	size_t _pos;
	quake::string_view _data;
};




quake::string_view  COM_GameDir();

template<typename TO, typename FROM> constexpr TO idCast(FROM f);

template<>
constexpr inline int idCast<int,idTime>(idTime f) {
	return f.seconds();
}
template<>
constexpr inline float idCast<float, idTime>(idTime f) {
	return static_cast<float>(f);
}
template<>
constexpr inline idTime idCast<idTime, float>(float f) {
	return idTime(f);
}
template<>
constexpr inline idTime idCast<idTime, double>(double f) {
	return  idTime(static_cast<float>(f));
}

template<typename T> T Q_sin(T t) { return std::sin(t); }
template<typename T> T Q_cos(T t) { return std::cos(t); }
template<typename T> T Q_tan(T t) { return std::tan(t); }
template<typename T> T Q_sqrt(T t) { return std::sqrt(t); }
template<typename T> T Q_atan(T t) { return std::atan(t); }

template<typename T1,typename T2> T1 Q_pow(T1 a, T2 b) { return std::pow(a,b); }
template<typename T1, typename T2> T1 Q_atan2(T1 a, T2 b) { return std::atan2(a, b); }

extern qboolean		standard_quake, rogue, hipnotic;

#endif