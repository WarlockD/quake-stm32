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
// we need the basic file functions
#ifdef min
#undef min
#endif

using idTime = std::chrono::milliseconds;
using idTimef = std::chrono::duration<float, std::milli>;
#if !defined BYTE_DEFINED
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
			return __alignof__(T);
		return 4;
	}

#define ALIGNOF(type,grain)	\
	namespace ustl {		\
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
		inline				cmemlink(void) : _data(nullptr), _size(0) { }
		inline				cmemlink(const void* p, size_type n) : _data(const_pointer(p)), _size(n) { assert(p || !n); }
		inline				cmemlink(const cmemlink& l) : _data(l._data), _size(l._size) {}
		inline virtual     ~cmemlink(void) noexcept {}
		void				link(const void* p, size_type n);
		inline void			link(const cmemlink& l) { link(l.begin(), l.size()); }
		inline void			link(const void* first, const void* last) { link(first, std::distance((char*)first, (char*)last)); }
		inline void			relink(const void* p, size_type n);
		virtual void		unlink(void) noexcept { _data = nullptr; _size = 0; }
		inline rcself_t		operator= (const cmemlink& l) { link(l); return *this; }
		bool				operator== (const cmemlink& l) const noexcept;
		inline void			swap(cmemlink& l) { std::swap(_data, l._data); std::swap(_size, l._size); }
		inline size_type	size(void) const { return _size; }
		inline size_type	max_size(void) const { return size(); }
		inline size_type	readable_size(void) const { return size(); }
		inline bool			empty(void) const { return !size(); }
		inline const_pointer	data(void) const { return _data; }
		inline const_pointer	cdata(void) const { return _data; }
		inline iterator		begin(void) const { return iterator(cdata()); }
		inline iterator		iat(size_type i) const { assert(i <= size()); return begin() + i; }
		inline iterator		end(void) const { return iat(size()); }
		inline void			resize(size_type n) { _size = n; }
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
		inline		memlink(void) : cmemlink() {}
		inline		memlink(void* p, size_type n) : cmemlink(p, n) {}
		inline		memlink(const void* p, size_type n) : cmemlink(p, n) {}
		inline		memlink(rcself_t l) : cmemlink(l) {}
		inline explicit	memlink(const cmemlink& l) : cmemlink(l) {}
		inline pointer	data(void) { return const_cast<pointer>(cmemlink::data()); }
		inline const_pointer	data(void) const { return cmemlink::data(); }
		inline iterator	begin(void) { return iterator(data()); }
		inline iterator	iat(size_type i) { assert(i <= size()); return begin() + i; }
		inline iterator	end(void) { return iat(size()); }
		inline const_iterator	begin(void) const { return cmemlink::begin(); }
		inline const_iterator	end(void) const { return cmemlink::end(); }
		inline const_iterator	iat(size_type i) const { return cmemlink::iat(i); }
		size_type		writable_size(void) const { return size(); }
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

	//----------------------------------------------------------------------

	/// Use with cmemlink-derived classes to link to a static array
#define static_link(v)	link (VectorBlock(v))
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
			std::memcpy(dist, data() + offset, rlen * sizeof(T));
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
		printf_buffer(char* str, size_t size) : memlink(str, size) {}
		const char* operator()(const char* fmt, ...);
		const char* c_str() const { return data(); }
		operator const char*() const { return data(); }
	};

	template<size_t SIZE = DEFAULT_SIMPLE_BUFFER_SIZE>
	class fixed_simple_string_buffer : public printf_buffer {
		std::array<char, SIZE> _buff;
	public:
		fixed_simple_string_buffer() : memlink(_buff.data(), _buff.size() - 1) {}
		fixed_simple_string_buffer(char* str, size_t size) : memlink(_buff.data(), _buff.size() - 1) {
			assert(size < _buff.size());
			char_traits::copy(_buff.data(), str, size);
			_buff[size] = '\0';
		}
	};

	// so I don't have to keep recompiling headers, alot of this is going to be in commom
	class string_view : public data_view<char> {
	public:
		using char_traits = std::char_traits<char>;
		static inline size_type hasher(const char* str, size_type size) {
			size_type h = 5381;
			while (size--) h = (h * 33) ^ (uint8_t)*str++;
			return h;
		}
		constexpr string_view() : data_view<char>("",0) {}
		constexpr string_view(const_pointer str, size_t size) : data_view<char>(str, size) {}
		constexpr string_view(const_pointer str) : string_view(str, char_traits::length(str)) {}
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


		template<size_t SIZE = DEFAULT_SIMPLE_BUFFER_SIZE>
		fixed_simple_string_buffer<SIZE> make_cstr() const {
			return simple_string_buffer<SIZE>(_data,size());
		}
	};
	// do we need all the comparison operations? humm
	inline bool operator==(const string_view& l, const string_view& r) { return l.compare(r) == 0; }
	inline bool operator==(const string_view& l, const string_view::const_pointer r) { return l.compare(r) == 0; }
	inline bool operator==(const string_view::const_pointer  l, const string_view& r) { return r.compare(l) == 0; }
	inline bool operator!=(const string_view& l, const string_view& r) { return l.compare(r) != 0; }
	inline bool operator!=(const string_view& l, const string_view::const_pointer r) { return l.compare(r) != 0; }
	inline bool operator!=(const string_view::const_pointer  l, const string_view& r) { return r.compare(l) != 0; }

	std::ostream& operator<<(std::ostream& os, const string_view& sv);

	// special cview that is 0 terminated
	class cstring_view : public string_view {
	public:
		constexpr cstring_view() : string_view("", 0) {}
		constexpr cstring_view(const_pointer str, size_t size) : string_view(str, size) { 
			if (size > 0 && str[size] != '\0') {
				Sys_Error("Bad cstring assignment"); // remove cause this is debuging
			}
		}
		constexpr cstring_view(const_pointer str) : string_view(str) {}
		constexpr const char* c_str() const { return data(); }
	};
	// a symbol is a string that is case insentive.  Its still a string_view, but the equal functions are diffrent
	// warning though, this does NOT override the compare functions.  I could make  the main comapre virtual, but 
	// I am trying to hold off on that
	// mabye rewrite this with static inhertenice? humm
	class symbol : public string_view {
	public:
		using char_traits = std::char_traits<char>;
		static inline size_type hasher(const char* str, size_type size) {
			size_type h = 5381;
			while (size--) h = (h * 33) ^ (uint8_t)tolower(*str++);
			return h;
		}
		constexpr symbol() : string_view() {}
		constexpr symbol(const_pointer str, size_t size) : string_view(str, size) {}
		constexpr symbol(const_pointer str) : string_view(str, std::strlen(str)) {}
		constexpr symbol(const string_view& str) : string_view(str) {}

		void swap(symbol& r)
		{	// swap with _Right
			string_view::swap(static_cast<string_view&>(r));
		}
	};

	bool operator==(const symbol& l, const string_view& r);
	inline bool operator==(const symbol& l, const string_view::const_pointer r) { return l == string_view(r); }
	inline bool operator!=(const symbol& l, const string_view& r) { return !(l == r); }
	inline bool operator!=(const symbol& l, const string_view::const_pointer r) { return !(l == r); }
	std::ostream& operator<<(std::ostream& os, const symbol& sv);

	template<size_t SIZE> class fixed_string_stream;
	// fixed buffer string
	template<size_t _SIZE>
	class fixed_string : public cstring_view {
	protected:
		std::array<value_type, _SIZE> _buffer;
		friend class fixed_string_stream<_SIZE>;
	public:
		static_assert(_SIZE > 1, "buffer size has to have space for 0");
		using reference = value_type&;
		using pointer = value_type*;
		using iterator = pointer;
		using reverse_iterator = std::reverse_iterator<iterator>;
		constexpr size_type capacity() const { return _buffer.size(); }
		void clear() { _size = 0; _buffer[0] = '\0'; }
		constexpr void assign(const data_view<char>&  str) {
			assert(str.size() < (capacity() - 1));
			_data = _buffer.data();
			_size = str.size();
			char_traits::copy(_buffer.data(), str.data(), _size);
			_buffer[_size] = '\0';
		}
		inline fixed_string& assign(const char* str) { assign(string_view(str)); return *this; }
		inline fixed_string& assign(const char* str, size_type len) { assign(string_view(str, len)); return *this;}

		fixed_string&  assign(size_t n, char c) { 
			assert(str.size() < (capacity() - 1));
			_data = _buffer.data();
			_size = n;
			std::fill_n(_buffer.begin(), n, c);
			_buffer[_size] = '\0';
			return *this;
		}
		pointer data() { return _buffer.data(); }
		constexpr fixed_string() : cstring_view() {  }
		constexpr fixed_string(const_pointer str, size_type size) : cstring_view(_buffer.data(), 0) { assign(str, size); }
		constexpr fixed_string(const_pointer str) : cstring_view(_buffer.data(), 0) { assign(str); }
		constexpr fixed_string(const data_view<char>& str) : cstring_view(_buffer.data(), 0) { assign(str); }

		// backwards compatablity with all the va around.  fix this or not?  change to streams?

		void push_back(char c) { assert((_size + 1) < (capacity() - 1)); _buffer[_size++] = c; _buffer[_size] = '\0'; }
		void pop_back() { if (_size > 0) --_size; }

		fixed_string& append(const data_view<char>&  str) {
			assert(str.size() < (capacity() - _size - 1));
			char_traits::copy(_buffer.data() + _size, str.data(), str.size());
			_size += str.size();
			_buffer[_size] = '\0';
			return *this;
		}
		fixed_string& fill(size_t count, char c = ' ') {
			size_t n = std::min(count, (capacity() - _size - 1));
			std::fill_n(_buffer.data() + _size, n, c);
			_size += n;
			_buffer[_size] = '\0';
			return *this;
		}
		fixed_string& erase(size_type pos = 0, size_type len = npos) {
			assert(pos < capacity());
			if (len == npos || (len+pos) >= capacity()) _buffer[_size=pos] = 0;
			else {
				assert(0);
				// not implmented..meh, I really wish i could use std::string here
			}
			return *this;
		}

		inline void append(const char* str) { assign(string_view(str)); }
		inline void append(const char* str, size_type len) { assign(string_view(str, len)); }
		fixed_string& operator+=(const char* str) { append(str); return *this; }
		fixed_string& operator+=(char c) { push_back(c); return *this; }
		fixed_string& operator+=(const data_view<char>& str) { append(str); return *this; }
		reference operator[](size_type i) { return _buffer[i]; }
		reference at(size_type i) { return _buffer.at(i); }
		iterator begin() { return _buffer.data(); }
		iterator end() { return _buffer.data() + _size; }
		reverse_iterator rbegin() { return reverse_iterator(begin()); }
		reverse_iterator rend() { return reverse_iterator(end()); }

		void swap(fixed_string& r)
		{	// swap with _Right
			if (this != std::addressof(r)) {
				std::swap(_size, r._size);
				std::swap(_buffer, r._buffer);
			}
		}
	};

	class fixed_string_buffer : public std::streambuf {
	public:
		void clear() {
			if (_buffer) {
				setp(_buffer, _buffer + _size - 1);  // always have room for the zero
				*pptr() = 0;
			} else setp(nullptr, nullptr);
		} 
		fixed_string_buffer() : _buffer(nullptr), _size(0) {}
		fixed_string_buffer(const fixed_string_buffer& copy) = delete;
		fixed_string_buffer(fixed_string_buffer && move) : _buffer(move._buffer), _size(move._size), std::streambuf(move) {}
		fixed_string_buffer(char* buffer, size_t size) : _buffer(buffer), _size(size) { clear(); }
		size_t capacity() const { return _size; }
		size_t size() const { return pbase() ? pptr() - pbase() : 0; }
		// fuck windows, just fuck them
		// I had to zero terminate here becuase windows would take the raw buffer pointer
		cstring_view str() const { 
			const_cast<char*>(_buffer)[size()] = '\0';
			return cstring_view(_buffer,size()); 
		}
		void swap(fixed_string_buffer& r)
		{	// swap with _Right
			if (this != std::addressof(r)) {
				std::swap(_buffer,r._buffer);
				std::swap(_size,r._size);
				std::streambuf::swap(r);
			}
		}
	protected:
		char* _buffer;
		size_t _size;


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
			_buffer = buffer;
			_size = (size_t)size;
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
		cstring_view str() const { return _sbuf.str(); }
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
		fixed_string_stream() : buffer_string_stream(_str.data(), _str.size()) { }
	protected:
		std::array<char, SIZE> _str;
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
			return quake::string_view::hasher(s.data(), s.size());
		}
	};
	template<>
	struct hash<quake::symbol> {
		size_t  operator()(const quake::symbol& s) const {
			return quake::symbol::hasher(s.data(), s.size());
		}
	};
}


#include "zone.h"

namespace quake {
	using string = std::basic_string<char_type, char_traits, ZAllocator<char>>;

	struct ZObject {
		static void* operator new(size_t size) { return (void*)Z_Malloc(size); }
		static void operator delete(void* ptr) { Z_Free(ptr); }
	};

}
//============================================================================

struct sizebuf_t
{
	qboolean	allowoverflow;	// if false, do a Sys_Error
	qboolean	overflowed;		// set to true if the buffer size failed
	byte	*data;
	int		maxsize;
	int		cursize;
} ;

void SZ_Alloc (sizebuf_t *buf, int startsize);
void SZ_Free (sizebuf_t *buf);
void SZ_Clear (sizebuf_t *buf);
void *SZ_GetSpace (sizebuf_t *buf, int length);
void SZ_Write (sizebuf_t *buf, const void *data, int length);
void SZ_Print (sizebuf_t *buf, const quake::string_view& data);	// strcats onto the sizebuf
void SZ_Print(sizebuf_t *buf, char c);
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
void MSG_WriteChar(sizebuf_t *sb, const T c) {
	//static_assert("Do you REALLY want to convert this to char?");
	MSG_WriteChar(sb, static_cast<int>(c));
}
template<typename T, typename = std::enable_if<!std::is_same<T, int>::value && std::is_arithmetic<T>::value>>
void MSG_WriteByte(sizebuf_t *sb, const T c) {
	//static_assert("Do you REALLY want to convert this to byte?");
	MSG_WriteByte(sb, static_cast<int>(c));
}


void MSG_WriteShort (sizebuf_t *sb, int c);
template<typename T, typename = std::enable_if<!std::is_same<T, int>::value && std::is_arithmetic<T>::value>>
void MSG_WriteShort(sizebuf_t *sb, const T c) {
	//static_assert("Do you REALLY want to convert this to short?");
	MSG_WriteShort(sb, static_cast<int>(c));
}


void MSG_WriteLong (sizebuf_t *sb, int c);
template<typename T, typename = std::enable_if<!std::is_same<T, int>::value && std::is_arithmetic<T>::value>>
void MSG_WriteLong(sizebuf_t *sb, const T c) {
	//static_assert("Do you REALLY want to convert this to long?");
	MSG_WriteLong(sb, static_cast<int>(c));
}


void MSG_WriteFloat (sizebuf_t *sb, float f);
void MSG_WriteString (sizebuf_t *sb, const char *s);
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







// parser
class COM_Parser {
public:
	enum Kind {
		LineFeed = '\n',
		Symbol = 'a',
		Number = '0',
		Eof = -1
	};
	class COM_Token {
	public:
		COM_Token() : COM_Token(Kind::Eof){}
		COM_Token(int token) : _token(token), _text() {}
		COM_Token(Kind token) : _token((int)token), _text() {}
		COM_Token(Kind token, const quake::string_view& text) : _token((int)token), _text(text) {}
		COM_Token(int token, const quake::string_view& text) : _token(token), _text(text) {}
		bool operator==(const COM_Token& r) const { return _token == r._token;  }
		bool operator==(int c) const { return _token == c; }
		bool operator==(Kind c) const { return _token == (int)c; }
		bool operator!=(const COM_Token& r) const { return !(*this == r);}
		bool operator!=(int r) const { return  !(*this == r);}
		bool operator!=(Kind r) const { return !(*this == r); }
		const quake::string_view& text() const { return _text; }
	private:
		int _token;
		quake::string_view _text;
	};
	enum ParserType {
		//WhiteSpaceTokens=1,
		IgnoreComments=1,
		IgnoreNewLine=2,
		AcceptAll = 0,
		Default = IgnoreComments | IgnoreNewLine,
	};

	COM_Parser(const quake::string_view& data, ParserType mode = Default) : _data(data), _mode(mode) {}
	COM_Parser(const char* str, ParserType mode = Default) : _data(str) , _mode(mode) {}
	COM_Parser(const COM_Parser& copy) = delete;
	COM_Parser(COM_Parser&& move) = delete;
	void setMode(ParserType mode) { _mode = mode; }
	COM_Parser() : _data() {}
	COM_Token Next();
	operator const quake::string_view&() const { return _data; }
protected:
	quake::string_view _data;
	ParserType _mode;
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
template<typename T> T Q_atan(T t) { return std::atan(t); }

template<typename T1,typename T2> T1 Q_pow(T1 a, T2 b) { return std::pow(a,b); }
template<typename T1, typename T2> T1 Q_atan2(T1 a, T2 b) { return std::atan2(a, b); }

extern qboolean		standard_quake, rogue, hipnotic;

#endif