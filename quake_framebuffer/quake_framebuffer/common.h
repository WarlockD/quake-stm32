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
#include <chrono>
#include <array>
#include <string_view>
#include <memory>
#include <fstream>
#include <variant>
#include <exception>
#include <unordered_map>
#include <unordered_set>

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
	constexpr idTime() :_time(time_t::zero()) {}
	//constexpr idTime(int f) : _time(f*1000) {} 	// seconds are int
	constexpr idTime(float f) : _time(std::chrono::duration_cast<time_t>(timef_t(f)*1000.0f)) {}
	//constexpr idTime(double f) : _time(std::chrono::duration_cast<time_t>(timef_t(static_cast<float>(f))*1000.0f)) {}
	template<typename T, typename R>
	constexpr idTime(const std::chrono::duration<T, R>& f) : _time(std::chrono::duration_cast<time_t>(f)) {}
	constexpr auto count() const { return _time.count(); }
	constexpr int seconds() const { return std::chrono::duration_cast<std::chrono::seconds>(_time).count(); }
	
//	explicit constexpr operator time_t&()  { return _time; }
//	explicit constexpr operator const time_t&() const { return _time; }
	constexpr operator float() const { return std::chrono::duration_cast<timef_t>(_time).count() / 1000.0f; }
	static constexpr idTime zero() { return idTime(); }
	idTime& operator+=(const idTime& other) { _time += other._time; return *this; }
	idTime& operator-=(const idTime& other) { _time -= other._time; return *this; }
	
	//template<typename T, typename R>
	//idTime& operator+=(const std::chrono::duration<T,R>& other) { _time += std::chrono::duration_cast<time_t>(other); return *this; }
	//template<typename T, typename R>
	//idTime& operator-=(const std::chrono::duration<T, R>& other) { _time -= std::chrono::duration_cast<time_t>(other); return *this; }
private:
	time_t _time;
};
constexpr static inline idTime operator+(const idTime& s1, const idTime& s2) { return idTime(idTime::time_t(s1.count() + s2.count())); }
constexpr static inline idTime operator+(float s1, const idTime& s2) { return idTime(idTime::time_t(idTime::time_t::rep(s1 * 1000.0f) + s2.count())); }
constexpr static inline idTime operator+(const idTime& s1, float s2) { return idTime(idTime::time_t(s1.count() + idTime::time_t::rep(s2 * 1000.0f))); }

constexpr static inline idTime operator-(const idTime& s1, const idTime& s2) { return idTime(idTime::time_t(s1.count() - s2.count())); }
constexpr static inline idTime operator-(float s1, const idTime& s2) { return idTime(idTime::time_t(idTime::time_t::rep(s1 * 1000.0f) - s2.count())); }
constexpr static inline idTime operator-(const idTime& s1, float s2) { return idTime(idTime::time_t(s1.count() - idTime::time_t::rep(s2 * 1000.0f))); }
#if 0
#define TIME_MATH_OPS(op) \
constexpr static inline idTime operator##op (const idTime& s1, const idTime& s2) {	return idTime( static_cast<idTime::time_t>(s1) op static_cast<idTime::time_t>(s2) ); } \
template<typename T> constexpr static inline idTime operator##op (const idTime& s1, const T& s2) {  return s1 op idTime(s2); } \
template<typename T> constexpr static inline idTime operator##op (const T& s1, const idTime& s2) { return idTime(s1) op s2; } 

TIME_MATH_OPS(+)
TIME_MATH_OPS(-)
//TIME_MATH_OPS(*)
//TIME_MATH_OPS(/)
#undef TIME_MATH_OPS
#endif

#define TIME_CMP_OPS(op)	\
constexpr static inline bool operator##op (const idTime& s1, const idTime& s2) { return s1.count() op s2.count(); } \
constexpr static inline bool operator##op(float s1, const idTime& s2) { return idTime::time_t::rep(s1 * 1000.0f) op s2.count(); } \
constexpr static inline bool operator##op(const idTime& s1, float s2) { return s1.count() op idTime::time_t::rep(s2 * 1000.0f); } 
TIME_CMP_OPS(==)
TIME_CMP_OPS(!= )
TIME_CMP_OPS(>= )
TIME_CMP_OPS(<= )
TIME_CMP_OPS(> )
TIME_CMP_OPS(<)
#undef TIME_CMP_OPS


#ifndef BYTE_DEFINED
typedef uint8_t 		byte;
#define BYTE_DEFINED 1
#endif


typedef bool qboolean;




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
using is_cstr_class = std::conditional_t<
	priv::is_detected<cstr_member_t, std::remove_cv_t<T>>::value,
	std::true_type, std::false_type>;

#if 0
template<typename C = char, typename CT = std::char_traits<C>, typename T>
inline typename std::enable_if_t<!std::is_pointer<T>::value && is_data_class<T>::value, std::basic_ostream<C, CT>&>
operator<<(std::basic_ostream<C, CT>& os, const T& s) {
	os.write(s.data(), s.size());
	return os;
}
#endif
#include "sys.h"

// does a varargs printf into a temp buffer


//============================================================================

//http://videocortex.io/2017/custom-stream-buffers/

namespace quake {
	static constexpr size_t DEFAULT_STREAM_BUFFER_SIZE = 1024;
	static constexpr size_t DEFAULT_SIMPLE_BUFFER_SIZE = 64;
	using hashvalue_t = size_t;
	using ssize_t = typename std::conditional<sizeof(size_t) == sizeof(int32_t), int32_t, int64_t>::type;
	using char_type = char;
	using char_traits = std::char_traits<char_type>;

	struct stream_output {
		virtual void text_output(std::ostream& os) const = 0;
		virtual ~stream_output() {}
	};
	template<typename T>
	inline typename std::enable_if<std::is_base_of<stream_output, T>::value, std::ostream&>::type
		operator<<(std::ostream& os, const T& vs) { vs.text_output(os); return os; }


	class stream_buffer : public std::basic_streambuf<char_type, char_traits> {

	public:
		using streambuf_t = std::basic_streambuf<char_type, char_traits>;
		stream_buffer() : _mode(0), _offset(0), _current(0), _length(0) {}
		sys_file& file() { return _file; }
		bool is_open() const { return _file.is_open(); }
		void close();
		bool open(const char* filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::binary);
		void set_offset(off_type offset, off_type length);
		size_t file_length() const { return (size_t)_length; }
	protected:
		pos_type seekpos(pos_type pos, std::ios_base::openmode which
			= std::ios_base::in | std::ios_base::out) override final;
		std::streampos seekoff(std::streamoff off, std::ios_base::seekdir way,
			std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override final;
		int_type overflow(int_type c) override final;
		int_type underflow() override final;
		int sync() override;
	private:
		std::array<char, DEFAULT_STREAM_BUFFER_SIZE> _buffer;
		sys_file _file;
		std::ios_base::openmode _mode; // we can only be reading or writing
		off_type _offset; // used to override file location in seeks;
		off_type _length; // used to override end of file
		off_type _current; // current file position
		pos_type _seekoff(off_type off, std::ios_base::seekdir dir,
			std::ios_base::openmode /* which */ = std::ios_base::in | std::ios_base::out);
	};

	class ifstream : public std::basic_istream<char_type, char_traits> {
	public:
		using stream_t = std::basic_istream<char_type, char_traits>;
		ifstream() : stream_t(&_buffer) {}
		ifstream(const char* str) : stream_t(&_buffer) { open(str); }
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
		using stream_sizer_t = typename std::conditional_t<numeric_limits<T>::is_integral,integral_object_stream_size<T>, object_stream_size<T>>; ;
		return stream_sizer_t()(v);
	}

	/// \brief Returns the recommended stream alignment for type \p T. Override with ALIGNOF.
	/// Because this is occasionally called with a null value, do not access the argument!
	template <typename T>
	inline size_t stream_align_of(const T&)
	{
		if (std::numeric_limits<T>::is_integral)
			return __alignof(T);
		return 4;
	}

#define ALIGNOF(type,grain)	\
	namespace quake {		\
		template <> inline size_t stream_align_of (const type&) { return grain; } }

	class cmemlink {
	public:
		typedef char		value_type;
		typedef const value_type*	pointer;
		typedef const value_type*	const_pointer;
		typedef value_type		reference;
		typedef value_type		const_reference;
		typedef size_t		size_type;
		typedef uint32_t		written_size_type;
		typedef ptrdiff_t		difference_type;
		typedef const_pointer	const_iterator;
		typedef const_iterator	iterator;
		typedef const cmemlink&	rcself_t;
	public:
		constexpr inline		cmemlink(nullptr_t p, size_type n) : _data(nullptr), _size(0) { assert(n == 0); }
		constexpr inline		cmemlink(void) : _data(nullptr), _size(0) { }
		constexpr inline		cmemlink(const void* p, size_type n) : _data(const_pointer(p)), _size(n) { assert(p || !n); }
		constexpr inline		cmemlink(const cmemlink& l) : _data(l._data), _size(l._size) {}
		inline virtual     ~cmemlink(void) noexcept {}
		void				link(const void* p, size_type n);
		inline void			link(const cmemlink& l) { link(l.begin(), l.size()); }
		inline void			link(const void* first, const void* last) { link(first, std::distance((char*)first, (char*)last)); }
		inline void			relink(const void* p, size_type n);
		virtual void		unlink(void) noexcept { _data = nullptr; _size = 0; }
		inline rcself_t		operator= (const cmemlink& l) { link(l); return *this; }
		constexpr bool				operator== (const cmemlink& l) const noexcept;
		inline void			swap(cmemlink& l) { std::swap(_data, l._data); std::swap(_size, l._size); }
		constexpr inline size_type	size(void) const { return _size; }
		constexpr inline size_type	max_size(void) const { return size(); }
		constexpr inline size_type	readable_size(void) const { return size(); }
		constexpr inline bool			empty(void) const { return !size(); }
		constexpr inline const_pointer	data(void) const { return _data; }
		constexpr inline const_pointer	cdata(void) const { return _data; }
		constexpr inline iterator		begin(void) const { return iterator(cdata()); }
		constexpr inline iterator		iat(size_type i) const {  return begin() + i; }
		constexpr inline iterator		end(void) const { return iat(size()); }
		constexpr inline void			resize(size_type n) { _size = n; }
		inline void			read(std::istream&) { assert(!"ustl::cmemlink is a read-only object."); }
	private:
		const_pointer	_data;		///< Pointer to the data block (const)
		size_type		_size;		///< size of the data block
	};
	/// \class memlink memlink.h ustl.h
	/// \ingroup MemoryManagement
	///
	/// \brief Wrapper for pointer to block with size.
	///
	/// Use this class the way you would a pointer to an allocated unstructured block.
	/// The pointer and block size are available through member functions and cast operator.
	///
	/// Example usage:
	/// \code
	///     void* p = malloc (46721);
	///     memlink a, b;
	///     a.link (p, 46721);
	///     assert (a.size() == 46721));
	///     b = a;
	///     assert (b.size() == 46721));
	///     assert (b.begin() + 34 == a.begin + 34);
	///     assert (0 == memcmp (a, b, 12));
	///     a.fill (673, b, 42, 67);
	///     b.erase (87, 12);
	/// \endcode
	///
	class memlink : public cmemlink {
	public:
		typedef value_type*			pointer;
		typedef cmemlink::pointer		const_pointer;
		typedef cmemlink::const_iterator	const_iterator;
		typedef pointer			iterator;
		typedef const memlink&		rcself_t;
	public:
		constexpr inline		memlink(void) : cmemlink() {}
		constexpr inline		memlink(void* p, size_type n) : cmemlink(p, n) {}
		constexpr inline		memlink(const void* p, size_type n) : cmemlink(p, n) {}
		constexpr inline		memlink(nullptr_t p, size_type n) : cmemlink(p, n) {}
		constexpr inline		memlink(rcself_t l) : cmemlink(l) {}
		constexpr inline explicit	memlink(const cmemlink& l) : cmemlink(l) {}
		inline pointer	data(void) { return const_cast<pointer>(cmemlink::data()); }
		constexpr inline const_pointer	data(void) const { return cmemlink::data(); }
		inline iterator	begin(void) { return iterator(data()); }
		inline iterator	iat(size_type i) { assert(i <= size()); return begin() + i; }
		inline iterator	end(void) { return iat(size()); }
		constexpr inline const_iterator	begin(void) const { return cmemlink::begin(); }
		constexpr inline const_iterator	end(void) const { return cmemlink::end(); }
		constexpr inline const_iterator	iat(size_type i) const { return cmemlink::iat(i); }
		constexpr size_type		writable_size(void) const { return size(); }
		inline rcself_t	operator= (const cmemlink& l) { cmemlink::operator= (l); return *this; }
		inline rcself_t	operator= (rcself_t l) { cmemlink::operator= (l); return *this; }
		inline void		link(const void* p, size_type n) { cmemlink::link(p, n); }
		inline void		link(void* p, size_type n) { cmemlink::link(p, n); }
		inline void		link(const cmemlink& l) { cmemlink::link(l); }
		inline void		link(memlink& l) { cmemlink::link(l); }
		inline void		link(const void* first, const void* last) { link(first, std::distance((char*)first, (char*)last)); }
		inline void		link(void* first, void* last) { link(first, std::distance((char*)first, (char*)last)); }
		inline void		relink(const void* p, size_type n) { cmemlink::relink(p, n); }
		inline void		relink(void* p, size_type n) { cmemlink::relink(p, n); }
		inline void		swap(memlink& l) { cmemlink::swap(l); }
		void			fill(const_iterator start, const void* p, size_type elsize, size_type elCount = 1) noexcept;
		inline void		insert(const_iterator start, size_type size);
		inline void		erase(const_iterator start, size_type size);
	};

	/// Shifts the data in the linked block from \p start to \p start + \p n.
	/// The contents of the uncovered bytes is undefined.
	inline void memlink::insert(const_iterator cstart, size_type n)
	{
		assert(data() || !n);
		assert(cmemlink::begin() || !n);
		assert(cstart >= begin() && cstart + n <= end());
		iterator start = const_cast<iterator>(cstart);
		std::rotate(start, end() - n, end());
	}

	/// Shifts the data in the linked block from \p start + \p n to \p start.
	/// The contents of the uncovered bytes is undefined.
	inline void memlink::erase(const_iterator cstart, size_type n)
	{
		assert(data() || !n);
		assert(cmemlink::begin() || !n);
		assert(cstart >= begin() && cstart + n <= end());
		iterator start = const_cast<iterator>(cstart);
		std::rotate(start, start + n, end());
	}

	/// Use with memlink-derived classes to allocate and link to stack space.
#define alloca_link(m,n)	(m).link (alloca (n), (n))
	//----------------------------------------------------------------------

	/// A fast alternative to link which can be used when relinking to the same block (i.e. when it is resized)
	inline void cmemlink::relink(const void* p, size_type n)
	{
		_data = reinterpret_cast<const_pointer>(p);
		_size = n;
	}


	/// Use with cmemlink-derived classes to link to a static array
#define static_link(v)	link (VectorBlock(v))
	/// \class memblock memblock.h ustl.h
	/// \ingroup MemoryManagement
	///
	/// \brief Allocated memory block.
	///
	/// Adds memory management capabilities to memlink. Uses malloc and realloc to
	/// maintain the internal pointer, but only if allocated using members of this class,
	/// or if linked to using the Manage() member function. Managed memory is automatically
	/// freed in the destructor.
	///
	class memblock : public memlink {
	public:
		memblock(void) noexcept;
		memblock(const void* p, size_type n);
		explicit			memblock(size_type n);
		explicit			memblock(const cmemlink& b);
		explicit			memblock(const memlink& b);
		memblock(const memblock& b);
		virtual			~memblock(void) noexcept;
		virtual void		unlink(void) noexcept override;
		inline void			assign(const cmemlink& l) { assign(l.cdata(), l.readable_size()); }
		inline const memblock&	operator= (const cmemlink& l) { assign(l); return *this; }
		inline const memblock&	operator= (const memlink& l) { assign(l); return *this; }
		inline const memblock&	operator= (const memblock& l) { assign(l); return *this; }
		inline void			swap(memblock& l) noexcept { memlink::swap(l); std::swap(_capacity, l._capacity); }
		void			assign(const void* p, size_type n);
		void			reserve(size_type newSize, bool bExact = false);
		void			resize(size_type newSize, bool bExact = true);
		iterator			insert(const_iterator start, size_type size);
		iterator			erase(const_iterator start, size_type size);
		inline void			clear(void) noexcept { resize(0); }
		inline size_type		capacity(void) const { return _capacity; }
		inline bool			is_linked(void) const { return !capacity(); }
		inline size_type		max_size(void) const { return is_linked() ? memlink::max_size() : SIZE_MAX; }
		inline void			manage(memlink& l) { manage(l.begin(), l.size()); }
		void			deallocate(void) noexcept;
		void			shrink_to_fit(void);
		void			manage(void* p, size_type n) noexcept;
		void			copy_link(void);
		void			read(std::istream& is);
		void			read_file(const char* filename);
		inline			memblock(memblock&& b) : memlink(), _capacity(0) { swap(b); }
		inline memblock&		operator= (memblock&& b) { swap(b); return *this; }
	protected:
		virtual size_type		minimumFreeCapacity(void) const noexcept;
	private:
		size_type			_capacity;	///< Number of bytes allocated by Resize.
	};

	//----------------------------------------------------------------------

	// base class for all arrays that are refrenced and passed around
	// Its basicly a pointer and the count of elements
	// soo..  While I LOVE quake::string_view, I hate std::string as it allocates on the heap
	// when constructing with nothing.  Because of this I have to build my own string 
	// that uses ZMalloc or HeapAlloc
	template<typename T>
	class data_view {
	public:
		// types
		using value_type = T;
		using const_pointer = const T*;
		using const_reference = const T&;
		//using reference = const_reference;
		//using pointer = const_pointer;
		using const_iterator = const_pointer;
		//using iterator = const_pointer;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
	//	using reverse_iterator = std::reverse_iterator<iterator>;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		static constexpr size_type npos = -1;

		constexpr data_view(const_pointer data, size_type size) noexcept : _data(data), _size(size) {}
		constexpr data_view() noexcept : _data(nullptr), _size(0) {}
		constexpr data_view(const data_view& copy) noexcept : _data(copy.data()), _size(copy.size()) {}
		constexpr data_view(data_view&& move) noexcept : _data(move.data()), _size(move.size()) { move._data = nullptr; move._size = 0; }
		inline constexpr const_pointer data() const noexcept { return _data; }
		inline constexpr size_t size() const noexcept { return _size; }

		constexpr const_iterator begin() const noexcept { return data(); }
		constexpr const_iterator end() const noexcept { return data() + size(); }
		constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(begin()); }
		constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator(end()); }

		constexpr const_reference operator[](size_type pos) const noexcept { return data()[pos]; }
		const_reference at(size_type pos) const {
			if (pos >= size()) throw std::out_of_range("data_helpers::at");
			return data()[pos];
		}
		constexpr bool empty() const { return size() == 0; }
		const_reference front() const { assert(!empty()); return  *data(); }
		const_reference back() const { assert(!empty()); return  *(data() + size() - 1); }
		size_type copy(T* dist, size_type count, size_type offset = 0) const
		{
			if (offset > size())
				throw std::out_of_range("data_helpers::at");
			size_type rlen = std::min(count, size() - offset);
			Q_memcpy(dist, data() + offset, rlen * sizeof(T));
			return rlen;
		}
		inline constexpr void remove_prefix(size_type count) noexcept { assert(count <= size()); _data += count; _size -= count; }
		inline constexpr void remove_suffix(size_type count) noexcept { assert(count <= size()); _size -= count; }

		void swap(data_view& r)
		{	// swap with _Right
			if (this != std::addressof(r)) {
				std::swap(_data, r._data);
				std::swap(_size, r._size);
			}
		}
		constexpr data_view split(size_type pos = 0, size_type count = npos) const {
			return (pos > size())
				? (throw std::out_of_range("data_helpers::split"), data_view())
				: data_view(data() + pos, std::min(count, size() - pos));
		}
	protected:
		const_pointer _data;
		size_t _size;
	};
	using char_view = data_view<char>;
	namespace priv {
		constexpr char_view::size_type find(const char* data, char_view::size_type count, char_view::value_type what) {
			if (count > 0) {
				const char* end_data = data + count;
				auto p = std::find(data, end_data, what);
				if (p != end_data) return  p - data;
			}
			return char_view::npos;
		}
		constexpr char_view::size_type find(const char_view& data, char_view::size_type data_offset, char_view::value_type what) {
			return (data_offset < data.size()) ?  find(data.data() + data_offset, data.size(),what) : char_view::npos;
		}
		constexpr char_view::size_type rfind(const char* data, char_view::size_type count, char_view::value_type what) {
			if (count > 0) {
				const char* end_data = data + count;
				auto p = std::find(data, end_data, what);
				if (p != end_data) return  p - data;
			}
			return char_view::npos;
		}
		// general routines that work with finding junk
		constexpr char_view::size_type find(const char_view& data, char_view::size_type data_offset, const char_view& what) {
			const auto begin_data = data.begin() + data_offset;
			if (begin_data < data.end()) {
				auto p = std::search(data.begin() + data_offset, data.end(), what.begin(), what.end());
				if (p != data.end()) return p - data.begin();
			}
			return char_view::npos;
		}
		// general routines that work with finding junk
		constexpr char_view::size_type rfind(const char_view& data, char_view::size_type data_offset, const char_view& what) {
			const auto begin_data = data.begin() + data_offset;
			if (begin_data < data.end()) {
				auto p = std::find_end(data.begin() + data_offset, data.end(), what.begin(), what.end());
				if (p != data.end()) return p - data.begin();
			}
			return char_view::npos;
		}
	}
	// just used for printf stuff
	class printf_buffer : public memlink {
	public:
		printf_buffer(char* str, size_t size) : memlink(str, size) { str[0] = '\0'; }
		const char* operator()(const char* fmt, ...);
		const char* c_str() const { return data(); }
		operator const char*() const { return data(); }
	};

	template<size_t SIZE = DEFAULT_SIMPLE_BUFFER_SIZE>
	class fixed_printf_buffer : public printf_buffer {
		std::array<char, SIZE> _buff;
	public:
		fixed_printf_buffer() : printf_buffer(_buff.data(), _buff.size() - 1) {}
		fixed_printf_buffer(const char* str, size_t size) : printf_buffer(_buff.data(), _buff.size() - 1) {
			assert(size < _buff.size());
			char_traits::copy(_buff.data(), str, size);
			_buff[size] = '\0';
		}
		fixed_printf_buffer(const char* str) : fixed_printf_buffer(str, std::strlen(str)) {}
	};
	using string_view = std::string_view;

	static inline size_t string_hasher(const char* str, size_t size) {
		size_t h = 5381;
		while (size--) h = (h * 33) ^ (uint8_t)*str++;
		return h;
	}
#if 0
	// so I don't have to keep recompiling headers, alot of this is going to be in commom
	class string_view : public data_view<char> {
		void relink(const_pointer p, size_t len) { _data = p; _size = len; }
	public:
		using char_traits = std::char_traits<char>;
		static inline size_type hasher(const char* str, size_type size) {
			size_type h = 5381;
			while (size--) h = (h * 33) ^ (uint8_t)*str++;
			return h;
		}
		inline const_iterator begin() const { return data_view<char>::begin(); }
		inline const_iterator end() const { return data_view<char>::end(); }

		constexpr string_view() : data_view<char>("",0) {}
		constexpr string_view(const_pointer str, size_t size) : data_view<char>(str==nullptr || size == 0 ? "" : str, str == nullptr || size == 0 ? 0 : size) {}
		constexpr string_view(const_pointer str) : string_view(str, char_traits::length(str)) {}
		constexpr string_view(const string_view& copy) noexcept : string_view(copy.data(),copy.size()) {}
		constexpr string_view(string_view&& move) noexcept : string_view(move.data(), move.size()) { move._data = ""; move._size = 0; }
		string_view& operator=(const string_view& copy) noexcept {  relink(copy.data(), copy.size());  return *this; }
		string_view& operator=(string_view&& move) noexcept { relink(move.data(), move.size()); move.relink("", 0); return *this; }

		constexpr string_view substr(size_type pos = 0, size_type count = npos) const {
			return (pos > size())
				? (throw std::out_of_range("data_helpers::split"), string_view())
				: string_view(data() + pos, std::min(count, size() - pos));
		}
		int compare(string_view sv) const noexcept {
			size_type rlen = std::min(size(), sv.size());
			int ret = char_traits::compare(data(), sv.data(), rlen);
			if (ret == 0) // first __rlen chars matched
				ret = size() == sv.size() ? 0 : (size() < sv.size() ? -1 : 1);
			return ret;
		}
		inline const char& operator[](size_t i) const { return i < size() ? data()[i] : data()[size()-1]; } // so we can return a 0 termated string

		inline int compare(size_type pos1, size_type count1, string_view sv) const { return substr(pos1, count1).compare(sv); }
		inline int compare(size_type pos1, size_type count1, string_view sv, size_type pos2, size_type count2) const { return substr(pos1, count1).compare(sv.substr(pos2, count2)); }
		inline int compare(const char*s) const noexcept { return compare(string_view(s)); }
		inline int compare(size_type pos1, size_type count1, const char* s) const { return substr(pos1, count1).compare(string_view(s)); }
		inline int compare(size_type pos1, size_type count1, const char* s, size_type count2) const { return substr(pos1, count1).compare(string_view(s, count2)); }

		void swap(string_view& r)
		{	// swap with _Right
			data_view<char>::swap(static_cast<data_view<char>&>(r));
		}
		inline constexpr size_type find(const string_view& what, size_type offset=0) const { return priv::find(*this, offset, what); }
		inline constexpr size_type find(char c, size_type offset=0)  const { return priv::find(*this, offset, c); }
		inline constexpr size_type find(const char* what, size_type offset = 0)  const { return priv::find(*this, offset, string_view(what)); }
		inline constexpr size_type find(const char* what, size_type offset, size_type count) const { return priv::find(*this, offset, string_view(what, count)); }
		inline constexpr size_type rfind(const string_view& what, size_type offset = 0)  const { return priv::find(*this, offset, what); }
		inline constexpr size_type rfind(char c, size_type offset = 0) const { return priv::find(*this, offset, c); }
		inline constexpr size_type rfind(const char* what, size_type offset = 0) const { return priv::find(*this, offset, string_view(what)); }
		inline constexpr size_type rfind(const char* what, size_type offset, size_type count)  const { return priv::find(*this, offset, string_view(what, count)); }

		size_type to_number(float& value, size_type offset=0) const;
		size_type to_number(int& value, size_type offset = 0) const;

		template<size_t SIZE = DEFAULT_SIMPLE_BUFFER_SIZE>
		fixed_printf_buffer<SIZE> make_cstr() const {
			return fixed_printf_buffer<SIZE>(_data,size());
		}
		inline const_iterator	iat(size_type i) const { assert(i <= size()); return begin() + i; }
	};
#endif

	/// \class string ustring.h ustl.h
	/// \ingroup Sequences
	///
	/// \brief STL basic_string&lt;char&gt; equivalent.
	///
	/// An STL container for text string manipulation.
	/// Differences from C++ standard:
	///	- string is a class, not a template. Wide characters are assumed to be
	///		encoded with utf8 at all times except when rendering or editing,
	///		where you would use a utf8 iterator.
	/// 	- format member function - you can, of course use an \ref ostringstream,
	///		which also have format functions, but most of the time this way
	///		is more convenient. Because uSTL does not implement locales,
	///		format is the only way to create localized strings.
	/// 	- const char* cast operator. It is much clearer to use this than having
	/// 		to type .c_str() every time.
	/// 	- length returns the number of _characters_, not bytes.
	///		This function is O(N), so use wisely.
	///
	/// An additional note is in order regarding the use of indexes. All indexes
	/// passed in as arguments or returned by find are byte offsets, not character
	/// offsets. Likewise, sizes are specified in bytes, not characters. The
	/// rationale is that there is no way for you to know what is in the string.
	/// There is no way for you to know how many characters are needed to express
	/// one thing or another. The only thing you can do to a localized string is
	/// search for delimiters and modify text between them as opaque blocks. If you
	/// do anything else, you are hardcoding yourself into a locale! So stop it!
	///
	class string : public memblock {
	public:
		typedef char		value_type;
		typedef unsigned char	uvalue_type;
		typedef value_type*		pointer;
		typedef const value_type*	const_pointer;
		//typedef wchar_t		wvalue_type;
		//typedef wvalue_type*	wpointer;
		//typedef const wvalue_type*	const_wpointer;
		typedef pointer		iterator;
		typedef const_pointer	const_iterator;
		typedef value_type&		reference;
		typedef value_type		const_reference;
		typedef std::reverse_iterator<iterator>		reverse_iterator;
		typedef std::reverse_iterator<const_iterator>	const_reverse_iterator;
	//	typedef utf8in_iterator<const_iterator>		utf8_iterator;
		typedef size_type		pos_type;
		static constexpr const pos_type npos = INT_MAX;	///< Value that means the end of string.
		// https://stackoverflow.com/questions/2041355/c-constructor-accepting-only-a-string-literal
		// nice idea to detect string literals
		struct Dummy {};
		template<typename T> struct IsCharPtr {};
		template<> struct IsCharPtr<const char *> { typedef Dummy* Type; };
		template<> struct IsCharPtr<char *> { typedef Dummy* Type; };
	public:
		// 
		inline			string(void) noexcept : memblock() { relink("", 0); }
		template<int N> string(const char(&v)[N]) { relink(v, std::strlen(v)); }
		template<int N> string(char(&)[N]) { relink(v, std::strlen(v)); }
		template<typename T> string(T v, typename IsCharPtr<T>::Type = 0) { assign((T)v); }
		string(const string& s);
		inline			string(const string& s, pos_type o, size_type n = npos);
		inline explicit		string(const cmemlink& l);
		string(const_pointer s) noexcept;
		inline			string(const_pointer s, size_type len);
		inline			string(const_pointer s1, const_pointer s2);
		string(size_type n, value_type c);
		inline			~string(void) noexcept { }
		inline pointer		data(void) { return string::pointer(memblock::data()); }
		inline const_pointer	data(void) const { return string::const_pointer(memblock::data()); }
		inline const_pointer	c_str(void) const { return string::const_pointer(memblock::cdata()); }
		inline size_type		max_size(void) const { size_type s(memblock::max_size()); return s - !!s; }
		inline size_type		capacity(void) const { size_type c(memblock::capacity()); return c - !!c; }
		void			resize(size_type n);
		inline void			resize(size_type n, value_type c);
		inline void			clear(void) { resize(0); }
		inline iterator		begin(void) { return iterator(memblock::begin()); }
		inline const_iterator	begin(void) const { return const_iterator(memblock::begin()); }
		inline const_iterator	cbegin(void) const { return begin(); }
		inline iterator		end(void) { return iterator(memblock::end()); }
		inline const_iterator	end(void) const { return const_iterator(memblock::end()); }
		inline const_iterator	cend(void) const { return end(); }
		inline reverse_iterator	rbegin(void) { return reverse_iterator(end()); }
		inline const_reverse_iterator	rbegin(void) const { return const_reverse_iterator(end()); }
		inline const_reverse_iterator	crbegin(void) const { return rbegin(); }
		inline reverse_iterator	rend(void) { return reverse_iterator(begin()); }
		inline const_reverse_iterator	rend(void) const { return const_reverse_iterator(begin()); }
		inline const_reverse_iterator	crend(void) const { return rend(); }
		//inline utf8_iterator	utf8_begin(void) const { return utf8_iterator(begin()); }
		//inline utf8_iterator	utf8_end(void) const { return utf8_iterator(end()); }
		inline const_reference	at(pos_type pos) const { assert(pos <= size() && begin()); return begin()[pos]; }
		inline reference		at(pos_type pos) { assert(pos <= size() && begin()); return begin()[pos]; }
		constexpr inline const_iterator	iat(pos_type pos) const { return cdata() + ((pos) && pos >= npos ? size() : std::min(size_type(pos), size())); }
		inline iterator		iat(pos_type pos) { return const_cast<iterator>(const_cast<const string*>(this)->iat(pos)); }
		const_iterator		wiat(pos_type i) const noexcept;
		inline iterator		wiat(pos_type i) { return const_cast<iterator>(const_cast<const string*>(this)->wiat(i)); }
		inline const_reference	front(void) const { return at(0); }
		inline reference		front(void) { return at(0); }
		inline const_reference	back(void) const { return at(size() - 1); }
		inline reference		back(void) { return at(size() - 1); }
	//	inline size_type		length(void) const { return distance(utf8_begin(), utf8_end()); }
		inline size_type		length(void) const { return size(); }
		inline string&		append(const_iterator i1, const_iterator i2) { return append(i1, std::distance(i1, i2)); }
		string&	   		append(const_pointer s, size_type len);
		string&	   		append(const_pointer s);
		string&			append(size_type n, value_type c);
		//inline string&		append(size_type n, wvalue_type c) { insert(size(), n, c); return *this; }
	//	inline string&		append(const_wpointer s1, const_wpointer s2) { insert(size(), s1, s2); return *this; }
		//inline string&		append(const_wpointer s) { const_wpointer se(s); for (; se&&*se; ++se) {} return append(s, se); }
		inline string&		append(const string& s) { return append(s.begin(), s.end()); }
		inline string&		append(const string& s, pos_type o, size_type n) { return append(s.iat(o), s.iat(o + n)); }
		inline void			push_back(value_type c) { resize(size() + 1); end()[-1] = c; }
	//	inline void			push_back(wvalue_type c) { append(1, c); }
		inline void			pop_back(void) { resize(size() - 1); }
		inline string&		assign(const_iterator i1, const_iterator i2) { return assign(i1, std::distance(i1, i2)); }
		string&	    		assign(const_pointer s, size_type len);
		string&	    		assign(const_pointer s);
		//inline string&		assign(const_wpointer s1, const_wpointer s2) { clear(); return append(s1, s2); }
		//inline string&		assign(const_wpointer s1) { clear(); return append(s1); }
		inline string&		assign(const string& s) { return assign(s.begin(), s.end()); }
		inline string&		assign(const string& s, pos_type o, size_type n) { return assign(s.iat(o), s.iat(o + n)); }
		inline string&		assign(size_type n, value_type c) { clear(); return append(n, c); }
		//inline string&		assign(size_type n, wvalue_type c) { clear(); return append(n, c); }
		size_type			copy(pointer p, size_type n, pos_type pos = 0) const noexcept;
		inline size_type		copyto(pointer p, size_type n, pos_type pos = 0) const noexcept { size_type bc = copy(p, n - 1, pos); p[bc] = 0; return bc; }
		inline int			compare(const string& s) const { return compare(begin(), end(), s.begin(), s.end()); }
		inline int			compare(pos_type start, size_type len, const string& s) const { return compare(iat(start), iat(start + len), s.begin(), s.end()); }
		inline int			compare(pos_type s1, size_type l1, const string& s, pos_type s2, size_type l2) const { return compare(iat(s1), iat(s1 + l1), s.iat(s2), s.iat(s2 + l2)); }
		inline int			compare(const_pointer s) const { return compare(begin(), end(), s, s + strlen(s)); }
		inline int			compare(pos_type s1, size_type l1, const_pointer s, size_type l2) const { return compare(iat(s1), iat(s1 + l1), s, s + l2); }
		inline int			compare(pos_type s1, size_type l1, const_pointer s) const { return compare(s1, l1, s, strlen(s)); }
		static int			compare(const_iterator first1, const_iterator last1, const_iterator first2, const_iterator last2) noexcept;
		inline			operator const value_type* (void) const;
		inline			operator value_type* (void);
		inline const string&	operator= (const string& s) { return assign(s.begin(), s.end()); }
		inline const string&	operator= (const_reference c) { return assign(&c, 1); }
		inline const string&	operator= (const_pointer s) { return assign(s); }
		//inline const string&	operator= (const_wpointer s) { return assign(s); }
		inline const string&	operator+= (const string& s) { return append(s.begin(), s.size()); }
		inline const string&	operator+= (value_type c) { push_back(c); return *this; }
		inline const string&	operator+= (const_pointer s) { return append(s); }
		//inline const string&	operator+= (wvalue_type c) { return append(1, c); }
		inline const string&	operator+= (uvalue_type c) { return operator+= (value_type(c)); }
	//	inline const string&	operator+= (const_wpointer s) { return append(s); }
		inline string		operator+ (const string& s) const;
		inline bool			operator== (const string& s) const {
			return s.size() == size() &&
				(s.data() == data() || 0 == std::memcmp(s.data(), data(), size()));
		}
		bool			operator== (const_pointer s) const noexcept;
		inline bool			operator== (value_type c) const { return size() == 1 && c == at(0); }
		inline bool			operator== (uvalue_type c) const { return operator== (value_type(c)); }
		inline bool			operator!= (const string& s) const { return !operator== (s); }
		inline bool			operator!= (const_pointer s) const { return !operator== (s); }
		inline bool			operator!= (value_type c) const { return !operator== (c); }
		inline bool			operator!= (uvalue_type c) const { return !operator== (c); }
		inline bool			operator< (const string& s) const { return 0 > compare(s); }
		inline bool			operator< (const_pointer s) const { return 0 > compare(s); }
		inline bool			operator< (value_type c) const { return 0 > compare(begin(), end(), &c, &c + 1); }
		inline bool			operator< (uvalue_type c) const { return operator< (value_type(c)); }
		inline bool			operator> (const_pointer s) const { return 0 < compare(s); }
		inline string&		insert(pos_type ip, size_type n, value_type c) { insert(iat(ip), n, c); return *this; }
		inline string&		insert(pos_type ip, const_pointer s) { insert(iat(ip), s, s + strlen(s)); return *this; }
		inline string&		insert(pos_type ip, const_pointer s, size_type nlen) { insert(iat(ip), s, s + nlen); return *this; }
		inline string&		insert(pos_type ip, const string& s) { insert(iat(ip), s.c_str(), s.size()); return *this; }
		inline string&		insert(pos_type ip, const string& s, size_type sp, size_type slen) { insert(iat(ip), s.iat(sp), s.iat(sp + slen)); return *this; }
		//string&			insert(pos_type ip, size_type n, wvalue_type c);
		//string&			insert(pos_type ip, const_wpointer first, const_wpointer last, size_type n = 1);
		inline string&		insert(int ip, size_type n, value_type c) { insert(pos_type(ip), n, c); return *this; }
		inline string&		insert(int ip, const_pointer s, size_type nlen) { insert(pos_type(ip), s, nlen); return *this; }
		iterator			insert(const_iterator start, size_type n, value_type c);
		inline iterator		insert(const_iterator start, value_type c) { return insert(start, 1u, c); }
		iterator			insert(const_iterator start, const_pointer s, size_type n);
		iterator			insert(const_iterator start, const_pointer first, const_iterator last, size_type n = 1);
		iterator			erase(const_iterator epo, size_type n = 1);
		string&			erase(pos_type epo = 0, size_type n = npos);
		inline string&		erase(int epo, size_type n = npos) { return erase(pos_type(epo), n); }
		inline iterator		erase(const_iterator first, const_iterator last) { return erase(first, size_type(std::distance(first, last))); }
		inline iterator		eraser(pos_type first, pos_type last) { return erase(iat(first), iat(last)); }
		string&			replace(const_iterator first, const_iterator last, const_pointer i1, const_pointer i2, size_type n);
		template <typename InputIt>
		string&			replace(const_iterator first, const_iterator last, InputIt first2, InputIt last2) { return replace(first, last, first2, last2, 1); }
		inline string&		replace(const_iterator first, const_iterator last, const string& s) { return replace(first, last, s.begin(), s.end()); }
		string&			replace(const_iterator first, const_iterator last, const_pointer s);
		inline string&		replace(const_iterator first, const_iterator last, const_pointer s, size_type slen) { return replace(first, last, s, s + slen); }
		inline string&		replace(const_iterator first, const_iterator last, size_type n, value_type c) { return replace(first, last, &c, &c + 1, n); }
		inline string&		replace(pos_type rp, size_type n, const string& s) { return replace(iat(rp), iat(rp + n), s); }
		inline string&		replace(pos_type rp, size_type n, const string& s, size_type sp, size_type slen) { return replace(iat(rp), iat(rp + n), s.iat(sp), s.iat(sp + slen)); }
		inline string&		replace(pos_type rp, size_type n, const_pointer s, size_type slen) { return replace(iat(rp), iat(rp + n), s, s + slen); }
		inline string&		replace(pos_type rp, size_type n, const_pointer s) { return replace(iat(rp), iat(rp + n), string(s)); }
		inline string&		replace(pos_type rp, size_type n, size_type count, value_type c) { return replace(iat(rp), iat(rp + n), count, c); }
		inline string		substr(pos_type o = 0, size_type n = npos) const { return string(*this, o, n); }
		inline void			swap(string& v) { memblock::swap(v); }
		pos_type			find(value_type c, pos_type pos = 0) const noexcept;
		pos_type			find(const string& s, pos_type pos = 0) const noexcept;
		inline pos_type		find(uvalue_type c, pos_type pos = 0) const noexcept { return find(value_type(c), pos); }
		inline pos_type		find(const_pointer p, pos_type pos, size_type count) const { string sp; sp.link(p, count); return find(sp, pos); }
		pos_type			rfind(value_type c, pos_type pos = npos) const noexcept;
		pos_type			rfind(const string& s, pos_type pos = npos) const noexcept;
		inline pos_type		rfind(uvalue_type c, pos_type pos = npos) const noexcept { return rfind(value_type(c), pos); }
		inline pos_type		rfind(const_pointer p, pos_type pos, size_type count) const { string sp; sp.link(p, count); return rfind(sp, pos); }
		pos_type			find_first_of(const string& s, pos_type pos = 0) const noexcept;
		inline pos_type		find_first_of(value_type c, pos_type pos = 0) const { string sp(1, c); return find_first_of(sp, pos); }
		inline pos_type		find_first_of(uvalue_type c, pos_type pos = 0) const { return find_first_of(value_type(c), pos); }
		inline pos_type		find_first_of(const_pointer p, pos_type pos, size_type count) const { string sp; sp.link(p, count); return find_first_of(sp, pos); }
		pos_type			find_first_not_of(const string& s, pos_type pos = 0) const noexcept;
		inline pos_type		find_first_not_of(value_type c, pos_type pos = 0) const { string sp(1, c); return find_first_not_of(sp, pos); }
		inline pos_type		find_first_not_of(uvalue_type c, pos_type pos = 0) const { return find_first_not_of(value_type(c), pos); }
		inline pos_type		find_first_not_of(const_pointer p, pos_type pos, size_type count) const { string sp; sp.link(p, count); return find_first_not_of(sp, pos); }
		pos_type			find_last_of(const string& s, pos_type pos = npos) const noexcept;
		inline pos_type		find_last_of(value_type c, pos_type pos = npos) const { string sp(1, c); return find_last_of(sp, pos); }
		inline pos_type		find_last_of(uvalue_type c, pos_type pos = npos) const { return find_last_of(value_type(c), pos); }
		inline pos_type		find_last_of(const_pointer p, pos_type pos, size_type count) const { string sp; sp.link(p, count); return find_last_of(sp, pos); }
		pos_type			find_last_not_of(const string& s, pos_type pos = npos) const noexcept;
		inline pos_type		find_last_not_of(value_type c, pos_type pos = npos) const { string sp(1, c); return find_last_not_of(sp, pos); }
		inline pos_type		find_last_not_of(uvalue_type c, pos_type pos = npos) const { return find_last_not_of(value_type(c), pos); }
		inline pos_type		find_last_not_of(const_pointer p, pos_type pos, size_type count) const { string sp; sp.link(p, count); return find_last_not_of(sp, pos); }
		int				vformat(const char* fmt, va_list args);
		int				format(const char* fmt, ...);// __attribute__((__format__(__printf__, 2, 3)));
		void			read(std::istream&);
		void			write(std::ostream& os) const;
		size_t			stream_size(void) const noexcept;
		static hashvalue_t		hash(const char* f1, const char* l1) noexcept;

		using initlist_t = std::initializer_list<value_type>;
		inline			string(string&& v) : memblock(std::move(v)) {}
		inline			string(initlist_t v) : memblock() { assign(v.begin(), v.size()); }
		inline string&		assign(string&& v) { swap(v); return *this; }
		inline string&		assign(initlist_t v) { return assign(v.begin(), v.size()); }
		inline string&		append(initlist_t v) { return append(v.begin(), v.size()); }
		inline string&		operator+= (initlist_t v) { return append(v.begin(), v.size()); }
		inline string&		operator= (string&& v) { return assign(std::move(v)); }
		inline string&		operator= (initlist_t v) { return assign(v.begin(), v.size()); }
		inline iterator		insert(const_iterator ip, initlist_t v) { return insert(ip, v.begin(), v.end()); }
		inline string&		replace(const_iterator first, const_iterator last, initlist_t v) { return replace(first, last, v.begin(), v.end()); }

	private:
		virtual size_type		minimumFreeCapacity(void) const noexcept final override;
	};

	//----------------------------------------------------------------------

	/// Assigns itself the value of string \p s
	inline string::string(const cmemlink& s)
		: memblock()
	{
		assign(const_iterator(s.begin()), s.size());
	}

	/// Assigns itself a [o,o+n) substring of \p s.
	inline string::string(const string& s, pos_type o, size_type n)
		: memblock()
	{
		assign(s, o, n);
	}

	/// Copies the value of \p s of length \p len into itself.
	inline string::string(const_pointer s, size_type len)
		: memblock()
	{
		assign(s, len);
	}

	/// Copies into itself the string data between \p s1 and \p s2
	inline string::string(const_pointer s1, const_pointer s2)
		: memblock()
	{
		assert(s1 <= s2 && "Negative ranges result in memory allocation errors.");
		assign(s1, s2);
	}

	/// Returns the pointer to the first character.
	inline string::operator const string::value_type* (void) const
	{
		assert((!end() || !*end()) && "This string is linked to data that is not 0-terminated. This may cause serious security problems. Please assign the data instead of linking.");
		return begin();
	}

	/// Returns the pointer to the first character.
	inline string::operator string::value_type* (void)
	{
		assert((end() && !*end()) && "This string is linked to data that is not 0-terminated. This may cause serious security problems. Please assign the data instead of linking.");
		return begin();
	}

	/// Concatenates itself with \p s
	inline string string::operator+ (const string& s) const
	{
		string result(*this);
		result += s;
		return result;
	}

	/// Resize to \p n and fill new entries with \p c
	inline void string::resize(size_type n, value_type c)
	{
		const size_type oldn = size();
		resize(n);
		const ssize_t m = std::max(ssize_t(n - oldn), ssize_t(0));
		std::fill_n(iat(oldn), m, c);
	}

	//----------------------------------------------------------------------
	// Operators needed to avoid comparing pointer to pointer

#define PTR_STRING_CMP(op, impl)	\
static inline bool op (const char* s1, const string& s2) { return impl; }
	PTR_STRING_CMP(operator==, (s2 == s1))
		PTR_STRING_CMP(operator!=, (s2 != s1))
		PTR_STRING_CMP(operator<, (s2 >  s1))
		PTR_STRING_CMP(operator<=, (s2 >= s1))
		PTR_STRING_CMP(operator>, (s2 <  s1))
		PTR_STRING_CMP(operator>=, (s2 <= s1))
#undef PTR_STRING_CMP

		inline string operator+ (const char* cs, const string& ss) { string r; r.reserve(strlen(cs) + ss.size()); r += cs; r += ss; return r; }

	//----------------------------------------------------------------------

	inline hashvalue_t hash_value(const char* first, const char* last)
	{
		return string::hash(first, last);
	}
	inline hashvalue_t hash_value(const char* v)
	{
		return hash_value(v, v + strlen(v));
	}

	//----------------------------------------------------------------------
	// String-number conversions

#define STRING_TO_INT_CONVERTER(name,type,func)	\
inline type name (const string& str, size_t* idx = nullptr, int base = 10) \
{					\
    const char* sp = str.c_str();	\
    char* endp = nullptr;		\
    type r = func (sp, idx ? &endp : nullptr, base);\
    if (idx)				\
	*idx = endp - sp;		\
    return r;				\
}
	STRING_TO_INT_CONVERTER(stoi, int, strtol)
		STRING_TO_INT_CONVERTER(stol, long, strtol)
		STRING_TO_INT_CONVERTER(stoul, unsigned long, strtoul)
#if HAVE_LONG_LONG
		STRING_TO_INT_CONVERTER(stoll, long long, strtoll)
		STRING_TO_INT_CONVERTER(stoull, unsigned long long, strtoull)
#endif
#undef STRING_TO_INT_CONVERTER

#define STRING_TO_FLOAT_CONVERTER(name,type,func) \
inline type name (const string& str, size_t* idx = nullptr) \
{					\
    const char* sp = str.c_str();	\
    char* endp = nullptr;		\
    type r = func (sp, idx ? &endp : nullptr);\
    if (idx)				\
	*idx = endp - sp;		\
    return r;				\
}
		STRING_TO_FLOAT_CONVERTER(stof, float, strtof)
		STRING_TO_FLOAT_CONVERTER(stod, double, strtod)
		STRING_TO_FLOAT_CONVERTER(stold, long double, strtold)
#undef STRING_TO_FLOAT_CONVERTER

#define NUMBER_TO_STRING_CONVERTER(type,fmts)\
    inline string to_string (type v) { string r; r.format(fmts,v); return r; }
		NUMBER_TO_STRING_CONVERTER(int, "%d")
		NUMBER_TO_STRING_CONVERTER(long, "%ld")
		NUMBER_TO_STRING_CONVERTER(unsigned long, "%lu")
#if HAVE_LONG_LONG
		NUMBER_TO_STRING_CONVERTER(long long, "%lld")
		NUMBER_TO_STRING_CONVERTER(unsigned long long, "%llu")
#endif
		NUMBER_TO_STRING_CONVERTER(float, "%f")
		NUMBER_TO_STRING_CONVERTER(double, "%lf")
		NUMBER_TO_STRING_CONVERTER(long double, "%Lf")
#undef NUMBER_TO_STRING_CONVERTER

	template<typename FUNC, typename T>
	inline size_t _to_number_int(FUNC f, const string_view& str, T& value, int base = 0) {
		const char* sp = str.data();
		char* endp = nullptr;
		value = f(sp, &endp, base);
		return endp - sp;
	}
	template<typename FUNC, typename T>
	inline size_t _to_number_float(FUNC f, const string_view& str, T& value) {
		const char* sp = str.data();
		char* endp = nullptr;
		value = f(sp, &endp);
		return endp - sp;
	}
	inline size_t to_number(const string_view& str, float& value) { return _to_number_float(::strtof,str, value); }
	inline size_t to_number(const string_view& str, double& value) { return _to_number_float(::strtod, str, value); }
	inline size_t to_number(const string_view& str, long double& value) { return _to_number_float(::strtold, str, value); }
	inline size_t to_number(const string_view& str, int& value, int base = 0) { return _to_number_int(::strtol, str, value,base); }
	inline size_t to_number(const string_view& str, long& value, int base = 0) { return _to_number_int(::strtol, str, value, base); }
	inline size_t to_number(const string_view& str, unsigned long& value, int base = 0) { return _to_number_int(::strtoul, str, value, base); }


	// do we need all the comparison operations? humm
	inline bool operator==(const string_view& l, const string_view& r) { return l.compare(r) == 0; }
	inline bool operator==(const string_view& l, const string_view::const_pointer r) { return l.compare(r) == 0; }
	inline bool operator==(const string_view::const_pointer  l, const string_view& r) { return r.compare(l) == 0; }
	inline bool operator!=(const string_view& l, const string_view& r) { return l.compare(r) != 0; }
	inline bool operator!=(const string_view& l, const string_view::const_pointer r) { return l.compare(r) != 0; }
	inline bool operator!=(const string_view::const_pointer  l, const string_view& r) { return r.compare(l) != 0; }

	std::ostream& operator<<(std::ostream& os, const string_view& sv);

	// special cview that is 0 terminated

	template<typename T>
	class csimple_list {
		const T* _data;
		size_t _size;
	public:
		constexpr csimple_list(const T* list,size_t count) :_data(list), _size(size) {}
		size_t size() const { return _size; }
		const T* data() const { return _data; }
		const T& operator[](size_t i) const { return _data[i]; }
	};
	// a symbol is a string that is case insentive.  Its still a string_view, but the equal functions are diffrent
	// warning though, this does NOT override the compare functions.  I could make  the main comapre virtual, but 
	// I am trying to hold off on that
	// mabye rewrite this with static inhertenice? humm
	class symbol  {
		string_view _view;
	public:
		using char_traits = std::char_traits<char>;
		constexpr symbol(const string_view& str) : _view(str) {}
		constexpr symbol() : _view() {}
		constexpr symbol(const char* str, size_t size) : symbol(string_view(str, size)) {}
		constexpr symbol(const char* str) : symbol(string_view(str)) {}
	
		auto data() const { return _view.data(); }
		auto size() const { return _view.size(); }
		auto operator[](size_t i) const { return _view[i]; }
		auto begin() const { return _view.begin(); }
		auto end() const { return _view.end(); }
		bool operator==(const string_view& r) const;
		inline bool operator==(const symbol& r) const { return *this == r._view; }
		inline bool operator==(string_view::const_pointer r) const { return *this == string_view(r); }
		inline bool operator!=(const string_view& r) const { return !(*this == r); }
		inline bool operator!=(const symbol& r) const { return *this != r._view; }
		inline bool operator!=( string_view::const_pointer r)const { return !(*this == r); }
	};


	std::ostream& operator<<(std::ostream& os, const symbol& sv);

	template<size_t SIZE> class fixed_string_stream;
	// fixed buffer string
	class string_buffer {
	public:
		using value_type = typename string_view::value_type;
		using size_type = typename string_view::size_type;
		using reference = typename string_view::reference;
		using pointer = typename string_view::pointer;
		using iterator = pointer;
		using const_reference = typename string_view::const_reference;
		using const_pointer = typename string_view::const_pointer;
		using const_iterator = const_pointer;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using reverse_iterator = std::reverse_iterator<iterator>;
		constexpr size_type capacity() const { return _capacity; }
		static constexpr size_t npos = size_t(-1);
		const_pointer data() const { return _buffer; }
		const_pointer c_str() const { return _buffer; }
		pointer data() { return _buffer; }
		size_type size() const { return _size; }
		void clear() { _size = 0; _buffer[0] = '\0'; }
		void assign(const string_view&  str) {
			assert(str.size() < (capacity() - 1));
			_size = str.size();
			char_traits::copy(_buffer, str.data(), _size);
			_buffer[_size] = '\0';
		}
		inline string_buffer& assign(const char* str) { assign(string_view(str)); return *this; }
		inline string_buffer& assign(const char* str, size_type len) { assign(string_view(str, len)); return *this;}

		string_buffer&  assign(size_t n, char c) {
			assert(size() < (capacity() - 1));
			_size = n;
			std::fill_n(_buffer, n, c);
			_buffer[_size] = '\0';
			return *this;
		}


		constexpr string_buffer() : _size(0), _capacity(0),_buffer(nullptr) {  }
		string_buffer(pointer buffer, size_type size) : _size(0), _capacity(size), _buffer(buffer) { _buffer[0] = 0; }
		string_buffer(pointer buffer, size_type capacity, size_type size) : _size(size), _capacity(capacity), _buffer(buffer) { _buffer[size] = 0; }
		operator string_view() const { return string_view(_buffer, _size); }
		// backwards compatablity with all the va around.  fix this or not?  change to streams?

		void push_back(char c) { assert((_size + 1) < (capacity() - 1)); _buffer[_size++] = c; _buffer[_size] = '\0'; }
		void pop_back() { if (_size > 0) --_size; }

		string_buffer& append(const string_view&  str) {
			assert(str.size() < (capacity() - _size - 1));
			char_traits::copy(_buffer + _size, str.data(), str.size());
			_size += str.size();
			_buffer[_size] = '\0';
			return *this;
		}
		string_buffer& fill(size_t count, char c = ' ') {
			size_t n = std::min(count, (capacity() - _size - 1));
			std::fill_n(_buffer + _size, n, c);
			_size += n;
			_buffer[_size] = '\0';
			return *this;
		}
		string_buffer& erase(size_type pos = 0, size_type len = npos) {
			assert(pos < capacity());
			if (len == npos || (len+pos) >= capacity()) _buffer[_size=pos] = 0;
			else {
				assert(0);
				// not implmented..meh, I really wish i could use std::string here
			}
			return *this;
		}
		bool empty() const { return _size == 0; }
		inline void append(const char* str) { assign(string_view(str)); }
		inline void append(const char* str, size_type len) { assign(string_view(str, len)); }
		string_buffer& operator+=(const char* str) { append(str); return *this; }
		string_buffer& operator+=(char c) { push_back(c); return *this; }
		string_buffer& operator+=(const string_view& str) { append(str); return *this; }
		reference operator[](size_type i) { return _buffer[i]; }
		reference at(size_type i) { return _buffer[i]; }
		const_reference operator[](size_type i) const{ return _buffer[i]; }
		const_reference at(size_type i) const { return _buffer[i]; }
		iterator begin() { return _buffer; }
		iterator end() { return _buffer + _size; }
		reverse_iterator rbegin() { return reverse_iterator(begin()); }
		reverse_iterator rend() { return reverse_iterator(end()); }
		const_iterator begin()const { return _buffer; }
		const_iterator end()const { return _buffer + _size; }
		const_reverse_iterator rbegin()const { return const_reverse_iterator(begin()); }
		const_reverse_iterator rend() const { return const_reverse_iterator(end()); }
		reference front() { return _buffer[0]; }
		reference back() { return _buffer[_size - 1]; }
		const_reference front() const { return _buffer[0]; }
		const_reference back() const { return _buffer[_size-1]; }
		void swap(string_buffer& r)
		{	// swap with _Right
			if (this != std::addressof(r)) {
				std::swap(_size, r._size);
				std::swap(_buffer, r._buffer);
				std::swap(_capacity, r._capacity);
			}
		}
		inline bool operator==(const char* s) const { return string_view(_buffer, _size).compare(s) == 0; }
		inline bool operator!=(const  char* s) const { return string_view(_buffer, _size).compare(s) != 0; }
		inline bool operator==(const string_view& s) const { return string_view(_buffer,_size).compare(s) == 0; }
		inline bool operator!=(const string_view& s) const { return string_view(_buffer, _size).compare(s) != 0; }
		inline bool operator==(const string_buffer& s) const { return string_view(_buffer, _size).compare(s) == 0; }
		inline bool operator!=(const string_buffer& s) const { return string_view(_buffer, _size).compare(s) != 0; }
	protected:
		pointer _buffer;
		size_t _capacity;
		size_t _size;
	};
	// fixed buffer string
	template<size_t _SIZE>
	class fixed_string : public string_buffer {
		std::array<string_buffer::value_type, _SIZE> _fbuffer;
	public:
		fixed_string() : string_buffer(_fbuffer.data(), _fbuffer.size()) {}
		fixed_string(const_pointer str) : string_buffer(_fbuffer.data(), _fbuffer.size()) { assign(str); }
		fixed_string(const_pointer str,size_t len) : string_buffer(_fbuffer.data(), _fbuffer.size()) { assign(str,len); }
		fixed_string(const string_view& s) : string_buffer(_fbuffer.data(), _fbuffer.size()) { assign(s); }
		operator string_buffer&() { return *this; }
		operator const string_buffer&() const{ return *this; }
		template<size_t SIZE>
		inline bool operator==(const fixed_string<SIZE>& s) const { return string_view(_buffer, _size).compare(s) == 0; }
		template<size_t SIZE>
		inline bool operator!=(const fixed_string<SIZE>& s) const { return string_view(_buffer, _size).compare(s) != 0; }
	};

	class fixed_string_buffer : public std::streambuf {
	public:
		void clear() {
			if (_str) {
				setp(_str, _str + _capacity - 1);  // always have room for the zero
				*pptr() = 0;
			} else setp(nullptr, nullptr);
		} 
		//fixed_string_buffer() : _str() { }
		fixed_string_buffer(const fixed_string_buffer& copy) = delete;
		fixed_string_buffer(fixed_string_buffer && move) : _str(move._str), std::streambuf(move) {}
		fixed_string_buffer(char* buffer, size_t size) : _str(buffer), _capacity(size) { clear(); }
		size_t capacity() const { return _capacity; }
		size_t size() const { return pbase() ? pptr() - pbase() : 0; }
		// fuck windows, just fuck them
		// I had to zero terminate here becuase windows would take the raw buffer pointer
		string_buffer str() {
			return string_buffer(pbase(), capacity(), size());
		}
		string_buffer str() const {
			return string_buffer(pbase(), capacity(),size());
		}
		void swap(fixed_string_buffer& r)
		{	// swap with _Right
			if (this != std::addressof(r)) {
				std::swap(_str,r._str);
				std::streambuf::swap(r);
			}
		}
	protected:
		size_t _capacity;
		char* _str;


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
			_str = buffer;
			_capacity = size_t(size);
			clear();
			return std::streambuf::setbuf(buffer, size);
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
};

#if 0
// it could be data_view<char> but mabye data could be binary data so lets not
template<typename T>
inline typename  std::enable_if<std::is_base_of<quake::string_view, T>::value, std::ostream&>::type
operator<<(std::ostream& os, const quake::string_view& s) {
	os.write(s.data(), s.size());
	return os;
}
#endif
namespace std {

	template<>
	struct hash<quake::string_view> {
		size_t  operator()(const quake::string_view& s) const {
			return quake::string_hasher(s.data(), s.size());
		}
	};
	template<>
	struct hash<quake::symbol> {
		size_t  operator()(const quake::symbol& s) const {
			return quake::string_hasher(s.data(), s.size());
		}
	};
}
// any dynamicly defined stuff needs to be created after this

#include "zone.h"


//============================================================================

class sizebuf_t { //: public quake::memblock {
	qboolean	_allowoverflow;	// if false, do a Sys_Error
	qboolean	_overflowed;	// set to true if the buffer size failed
	int			_hunk_low_used;
	size_t		_cursize;
	size_t		_capacity;
	byte*		_data;
public:
	using refrence = byte&;
	using const_refrence = const byte&;
	using iterator = byte*;
	using const_iterator = const byte*;
	using size_type = size_t;
	sizebuf_t() : _allowoverflow(false), _overflowed(false), _hunk_low_used(0U), _data(nullptr), _capacity(0U), _cursize(0U) {}
	sizebuf_t(byte* data, size_t size, bool allowoverflow = true) : _allowoverflow(allowoverflow), _overflowed(false), _hunk_low_used(0U), _data(data), _capacity(size), _cursize(0U) {}
	void* GetSpace(size_t length);
	sizebuf_t(sizebuf_t&& move) = default;
	sizebuf_t(const sizebuf_t& copy) = default;
	sizebuf_t& operator=(sizebuf_t&& move) = default;
	sizebuf_t& operator=(const sizebuf_t& copy) = default;
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
	void WriteString(const quake::string_view& data);
	void WriteCoord(float f);
	void WriteAngle(float f);

} ;
#if 0
void SZ_Alloc (sizebuf_t *buf, int startsize);
void SZ_Free (sizebuf_t *buf);
void SZ_Clear (sizebuf_t *buf);
void *SZ_GetSpace (sizebuf_t *buf, int length);
void SZ_Write (sizebuf_t *buf, const void *data, int length);
void SZ_Print (sizebuf_t *buf, const quake::string_view& data);	// strcats onto the sizebuf
void SZ_Print(sizebuf_t *buf, char c);
#endif
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

bool Q_strcmp(const quake::string_view& a, const quake::string_view& b);

int Q_strncasecmp (const char * s1, const char * s2, size_t n);
#if 0
bool Q_strncasecmp(const quake::string_view& a, const quake::string_view& b);

#endif

int	Q_atoi (const char * str);
float Q_atof (const char * str);
int	Q_atoi(const quake::string_view& str);
float Q_atof(const quake::string_view& str);

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

//============================================================================


void COM_Init (const char *path); // path not used?




inline  std::pair<quake::string_view::size_type, quake::string_view::size_type> COM_FindFileBasePos(const quake::string_view& in) {
	size_t slash =  in.size() - 1;
	for (; slash > 0U && (in.data()[slash] != '/' || in.data()[slash] != '\\'); --slash);
	size_t dot = slash;
	for (; dot < in.size() && in.data()[dot] != '.'; ++dot);
	return std::make_pair(slash, dot == in.size() ? quake::string_view::npos : dot);
#if 0
	auto slash = in.find_last_of("/\\");
	if (slash == T::npos) slash = 0U;
	auto dot = in.find_first_of('.', slash);
	return std::make_pair(slash, dot);
#endif
}
inline quake::string_view
 COM_SkipPath(const quake::string_view & in) {
	auto pos = COM_FindFileBasePos(in);
	return quake::string_view(in.data()+ pos.first, in.size()-pos.first);
}
inline  quake::string_view  COM_FileExtension(const quake::string_view& in) {
	auto pos = COM_FindFileBasePos(in);
	if (pos.second != quake::string_view::npos)
		return quake::string_view(in.data() + pos.second, in.size() - pos.second);
	else
		return quake::string_view();
}

inline quake::string_view  COM_StripExtension(const quake::string_view & in)
{
	auto pos = COM_FindFileBasePos(in);
	if (pos.second != quake::string_view::npos)
		return quake::string_view(in.data() + pos.first, in.size() - pos.first - pos.second);
	else
		return in;
}
inline quake::string_view  COM_FileBase(const quake::string_view & in)
{
	auto pos = COM_FindFileBasePos(in);
	quake::string_view out(in.data()+ pos.first+1, in.size()-pos.first);
	if (pos.second != quake::string_view::npos)
		out.remove_suffix(out.size() - pos.second);
	return out;
}

template<typename C, typename CT, typename CA>
inline void COM_DefaultExtension(std::basic_string<C,CT,CA>& path, const quake::string_view&  extension) {
	auto pos = COM_FindFileBasePos(path);
	if (pos.second == priv::npos) {
		path += '.';
		path += extension;
	}
}
template<size_t SIZE>
inline void COM_DefaultExtension(quake::fixed_string<SIZE>& path, const quake::string_view&  extension) {
	auto pos = COM_FindFileBasePos(path);
	if (pos.second == quake::string_view::npos) {
		path += '.';
		path += extension;
	}
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

extern int com_filesize;
struct cache_user_t;



void COM_WriteFile (const quake::string_view& filename, const void * data, int len);
std::istream* COM_OpenFile (const quake::string_view& filename, size_t& length);


byte *COM_LoadStackFile (const quake::string_view& path, void *buffer, int bufsize);
byte *COM_LoadTempFile (const quake::string_view& path);
byte *COM_LoadHunkFile (const quake::string_view& path);
void COM_LoadCacheFile (const quake::string_view& path, struct cache_user_t *cu);





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
	COM_Parser(const quake::string_view& data) : _data(data), _pos(0), _state(0) {}
	bool Next(quake::string_view& token,bool test_eol=false);
	operator const quake::string_view&() const { return _data; }
	size_t pos() const { return _pos; }
	quake::string_view remaining() const { return _data.substr(_pos); }
protected:
	int _state;
	size_t _pos;
	quake::string_view _data;
};



#if 0
class COM_File_Parser : public COM_Parser {
	COM_File_Parser() : _handle(idFileHandle()) , _size(0U) {}
	COM_File_Parser(const char* filename) : _handle(idFileHandle()) { Open(filename);  }
	~COM_File_Parser() { Close(); }
	bool Next(quake::string_view& token);
	bool Open(const char* filename);
	void Close();
	bool isOpen() const { return _handle != idFileHandle(); }
private:
	void fill_buffer();
	
	quake::ifstream _stream;
	size_t _size;
};
#endif


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