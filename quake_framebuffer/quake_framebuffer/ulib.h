#ifndef ULIB_H_
#define ULIB_H_

#if 0
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
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

namespace ustl {
	typedef size_t		uoff_t;		///< A type for storing offsets into blocks measured by size_t.
	typedef uint32_t	hashvalue_t;	///< Value type returned by the hash functions.
	typedef size_t		streamsize;	///< Size of stream data
	typedef uoff_t		streamoff;	///< Offset into a stream

	typedef uint8_t utf8subchar_t;	///< Type for the encoding subcharacters.

	enum class Endian { Little, Big };
	constexpr Endian native_byte_order = Endian::Little;
	/// Returns the minimum of \p a and \p b
	template <typename T1, typename T2>
	inline constexpr T1 min(const T1& a, const T2& b) { return a < b ? a : b; }

	/// Returns the maximum of \p a and \p b
	template <typename T1, typename T2>
	inline constexpr T1 max(const T1& a, const T2& b) { return b < a ? a : b; }
	// Offsets a pointer
	template <typename T>
	inline T advance_ptr(T i, ptrdiff_t offset) { return i + offset; }
	// Offsets a void pointer
	template <> inline const void* advance_ptr(const void* p, ptrdiff_t offset) { assert(p || !offset); return reinterpret_cast<const uint8_t*>(p) + offset; }
	template <> inline void* advance_ptr(void* p, ptrdiff_t offset) { assert(p || !offset); return reinterpret_cast<uint8_t*>(p) + offset; }
	/// Offsets an iterator
	template <typename T, typename Distance>
	inline T advance(T i, Distance offset) { return advance_ptr(i, offset); }
	/// Returns the difference \p p1 - \p p2
	template <typename T1, typename T2>
	inline constexpr ptrdiff_t distance(T1 i1, T2 i2) { return i2 - i1; }


	template <typename T>
	T* addressof(T& v) { return reinterpret_cast<T*>(&const_cast<char&>(reinterpret_cast<const volatile char&>(v))); }

	/// Returns the number of bits in the given type

	//template<typename T> static constexpr size_t  BitsInType(T v) { return sizeof(v) * 8; }

	/// Indexes into a static array with bounds limit
	template <typename T, size_t N>
	inline constexpr T& VectorElement(T(&v)[N], size_t i) { return v[min(i, N - 1)]; }

	/// The alignment performed by default.
	constexpr static  size_t c_DefaultAlignment = sizeof(void*);

	/// \brief Rounds \p n up to be divisible by \p grain
	template <typename T> inline constexpr T AlignDown(T n, size_t grain = c_DefaultAlignment) { return n - n % grain; }
	template <typename T> inline constexpr T Align(T n, size_t grain = c_DefaultAlignment) { return AlignDown(n + grain - 1, grain); }


	template <typename T, bool IsSigned> struct __is_negative { inline constexpr bool operator()(const T& v) const { return v < 0; } };
	template <typename T> struct __is_negative<T, false> { inline constexpr bool operator()(const T&) const { return false; } };
	template <typename T> inline constexpr bool is_negative(const T& v) { return __is_negative<T, std::numeric_limits<T>::is_signed()>()(v); }
	template <typename T> inline constexpr T absv(T v) { return is_negative(v) ? -v : v; }
	template <typename T> inline constexpr T sign(T v) { return (0 < v) - is_negative(v); }
	template <typename T1, typename T2> inline constexpr size_t abs_distance(const T1& i1, const T2& i2) { return absv(distance(i1, i2)); }
	template <typename T> inline constexpr size_t size_of_elements(size_t n, const T*) { return n * sizeof(T); }

	template <typename T> constexpr T gcd(T a, T b) { return b ? gcd(b, a%b) : absv(a); }
	template <typename T> constexpr T lcm(T a, T b) { return a / gcd(a, b)*b; }
	constexpr inline uint16_t bswap_16(uint16_t v) { return v << 8 | v >> 8; }
	constexpr inline uint32_t bswap_32(uint32_t v) { return v << 24 | (v & 0xFF00) << 8 | ((v >> 8) & 0xFF00) | v >> 24; }
	constexpr inline uint64_t bswap_32(uint64_t v) { return (uint64_t(bswap_32(v)) << 32) | bswap_32(v >> 32); }
	/// \brief Swaps the byteorder of \p v.
	template <typename T>
	inline   T bswap(const T& v)
	{
		return T(BitsInType<T>() == 16 ?
			bswap_16(uint16_t(v)) : BitsInType<T>() == 32 ?
			bswap_32(uint32_t(v)) : BitsInType<T>() == 64 ?
			bswap_64(uint64_t(v)) : throw std::invalid_argument(), v); // not sure how this happened
	}
	template <typename T> constexpr inline T le_to_native(const T& v) { return native_byte_order == Endian::Little ? v : bswap(v); }
	template <typename T> constexpr inline T be_to_native(const T& v) { return native_byte_order == Endian::Big ? v : bswap(v); }
	template <typename T> constexpr inline T native_to_le(const T& v) { return native_byte_order == Endian::Little ? v : bswap(v); }
	template <typename T>constexpr  inline T native_to_be(const T& v) { return native_byte_order == Endian::Big ? v : bswap(v); }


	/// Template of making != from ! and ==
	template <typename T>
	inline constexpr bool operator!= (const T& x, const T& y)
	{
		return !(x == y);
	}

	/// Template of making > from <
	template <typename T>
	inline constexpr bool operator> (const T& x, const T& y)
	{
		return y < x;
	}

	/// Template of making <= from < and ==
	template <typename T>
	inline constexpr bool operator<= (const T& x, const T& y)
	{
		return !(y < x);
	}

	/// Template of making >= from < and ==
	template <typename T>
	inline constexpr bool operator>= (const T& x, const T& y)
	{
		return !(x < y);
	}


	/// Packs \p s multiple times into \p b. Useful for loop unrolling.
	template <typename TSmall, typename TBig>
	inline void pack_type(TSmall s, TBig& b)
	{
		b = s;
		for (unsigned h = BitsInType(TSmall); h < BitsInType(TBig); h *= 2)
			b = (b << h) | b;
	}
	/// \brief Divides \p n1 by \p n2 and rounds the result up.
	/// This is in contrast to regular division, which rounds down.
	template <typename T1, typename T2>
	inline T1 DivRU(T1 n1, T2 n2)
	{
		T2 adj = n2 - 1;
		if (is_negative(n1))
			adj = -adj;
		return (n1 + adj) / n2;
	}
	/// Sets the contents of \p pm to 1 and returns true if the previous value was 0.
	inline bool TestAndSet(int* pm)
	{
#if CPU_HAS_CMPXCHG8
		bool rv;
		int oldVal(1);
		asm volatile ( // cmpxchg compares to %eax and swaps if equal
			"cmpxchgl %3, %1\n\t"
			"sete %0"
			: "=a" (rv), "=m" (*pm), "=r" (oldVal)
			: "2" (oldVal), "a" (0)
			: "memory");
		return rv;
#elif __i386__ || __x86_64__
		int oldVal(1);
		asm volatile ("xchgl %0, %1" : "=r"(oldVal), "=m"(*pm) : "0"(oldVal), "m"(*pm) : "memory");
		return !oldVal;
#elif __sparc32__	// This has not been tested
		int rv;
		asm volatile ("ldstub %1, %0" : "=r"(rv), "=m"(*pm) : "m"(pm));
		return !rv;
#else
		const int oldVal(*pm);
		*pm = 1;
		return !oldVal;
#endif
	}

	/// Returns the index of the first set bit in \p v or \p nbv if none.
	inline uoff_t FirstBit(uint32_t v, uoff_t nbv)
	{
		uoff_t n = nbv;
#if __i386__ || __x86_64__
		if (!__builtin_constant_p(v)) asm("bsr\t%1, %k0":"+r,r"(n) : "r,m"(v)); else
#endif
#if __GNUC__
			if (v) n = 31 - __builtin_clz(v);
#else
			if (v) for (uint32_t m = uint32_t(1) << (n = 31); !(v & m); m >>= 1) --n;
#endif
		return n;
	}
	/// Returns the index of the first set bit in \p v or \p nbv if none.
	inline uoff_t FirstBit(uint64_t v, uoff_t nbv)
	{
		uoff_t n = nbv;
#if __x86_64__
		if (!__builtin_constant_p(v)) asm("bsr\t%1, %0":"+r,r"(n) : "r,m"(v)); else
#endif
#if __GNUC__ && SIZE_OF_LONG >= 8
			if (v) n = 63 - __builtin_clzl(v);
#elif __GNUC__ && HAVE_LONG_LONG && SIZE_OF_LONG_LONG >= 8
			if (v) n = 63 - __builtin_clzll(v);
#else
			if (v) for (uint64_t m = uint64_t(1) << (n = 63); !(v & m); m >>= 1) --n;
#endif
		return n;
	}

	/// Returns the next power of 2 >= \p v.
	/// Values larger than UINT32_MAX/2 will return 2^0
	inline uint32_t NextPow2(uint32_t v)
	{
		uint32_t r = v - 1;
#if __i386__ || __x86_64__
		if (!__builtin_constant_p(r)) asm("bsr\t%0, %0":"+r"(r)); else
#endif
		{
			r = FirstBit(r, r); if (r >= BitsInType(r) - 1) r = uint32_t(-1);
		}
		return 1 << (1 + r);
	}

	/// Bitwise rotate value left
	template <typename T>
	inline T Rol(T v, size_t n)
	{
#if __i386__ || __x86_64__
		if (!(__builtin_constant_p(v) && __builtin_constant_p(n))) asm("rol\t%b1, %0":"+r,r"(v) : "i,c"(n)); else
#endif
			v = (v << n) | (v >> (BitsInType(T) - n));
		return v;
	}

	/// Bitwise rotate value right
	template <typename T>
	inline T Ror(T v, size_t n)
	{
#if __i386__ || __x86_64__
		if (!(__builtin_constant_p(v) && __builtin_constant_p(n))) asm("ror\t%b1, %0":"+r,r"(v) : "i,c"(n)); else
#endif
			v = (v >> n) | (v << (BitsInType(T) - n));
		return v;
	}

	/// \brief This template is to be used for dereferencing a type-punned pointer without a warning.
	///
	/// When casting a local variable to an unrelated type through a pointer (for
	/// example, casting a float to a uint32_t without conversion), the resulting
	/// memory location can be accessed through either pointer, which violates the
	/// strict aliasing rule. While -fno-strict-aliasing option can be given to
	/// the compiler, eliminating this warning, inefficient code may result in
	/// some instances, because aliasing inhibits some optimizations. By using
	/// this template, and by ensuring the memory is accessed in one way only,
	/// efficient code can be produced without the warning. For gcc 4.1.0+.
	///
	template <typename DEST, typename SRC>
	inline DEST noalias(const DEST&, SRC* s)
	{
		asm("":"+g"(s)::"memory");
		union UPun { SRC s; DEST d; };
		return ((UPun*)(s))->d;
	}

	template <typename DEST, typename SRC>
	inline DEST noalias_cast(SRC s)
	{
		asm("":"+g"(s)::"memory");
		union { SRC s; DEST d; } u = { s };
		return u.d;
	}

	/// For partial specialization of stream_size_of for objects
	template <typename T> struct object_stream_size {
		inline streamsize operator()(const T& v) const { return v.stream_size(); }
	};
	template <typename T> struct integral_object_stream_size {
		inline streamsize operator()(const T& v) const { return sizeof(v); }
	};
	/// Returns the size of the given object. Overloads for standard types are available.
	template <typename T>
	inline streamsize stream_size_of(const T& v) {
		typedef typename tm::Select <numeric_limits<T>::is_integral,
			integral_object_stream_size<T>, object_stream_size<T> >::Result stream_sizer_t;
		return stream_sizer_t()(v);
	}

	/// \brief Returns the recommended stream alignment for type \p T. Override with ALIGNOF.
	/// Because this is occasionally called with a null value, do not access the argument!
	template <typename T>
	inline size_t stream_align_of(const T&)
	{
		if (numeric_limits<T>::is_integral)
			return __alignof__(T);
		return 4;
	}

#define ALIGNOF(type,grain)	\
namespace ustl {		\
    template <> inline size_t stream_align_of (const type&) { return grain; } }

} // namespace ustl

  //
  // Extra overloads in this macro are needed because it is the one used for
  // marshalling pointers. Passing a pointer to stream_size_of creates a
  // conversion ambiguity between converting to const pointer& and converting
  // to bool; the compiler always chooses the bool conversion (because it
  // requires 1 conversion instead of 2 for the other choice). There is little
  // point in adding the overloads to other macros, since they are never used
  // for pointers.
  //
  /// Declares that T is to be written as is into binary streams.
#define INTEGRAL_STREAMABLE(T)	\
    namespace ustl {		\
	inline istream& operator>> (istream& is, T& v)		{ is.iread(v);  return is; }	\
	inline ostream& operator<< (ostream& os, const T& v)	{ os.iwrite(v); return os; }	\
	inline ostream& operator<< (ostream& os, T& v)		{ os.iwrite(v); return os; }	\
	template<> inline streamsize stream_size_of(const T& v)	{ return sizeof(v); }		\
    }

  /// Declares that T contains read, write, and stream_size methods. This is no longer needed and is deprecated.
#define STD_STREAMABLE(T)

  /// Declares \p T to be writable to text streams. This is no longer needed and is deprecated.
#define TEXT_STREAMABLE(T)

  /// Declares that T is to be cast into TSUB for streaming.
#define CAST_STREAMABLE(T,TSUB)	\
    namespace ustl {		\
	inline istream& operator>> (istream& is, T& v)		{ TSUB sv; is >> sv; v = (T)(sv); return is; }	\
	inline ostream& operator<< (ostream& os, const T& v)	{ os << TSUB(v); return os; }			\
	template<> inline streamsize stream_size_of(const T& v)	{ return stream_size_of (TSUB(v)); }		\
    }

  /// Placed into a class it declares the methods required by STD_STREAMABLE. Syntactic sugar.
#define DECLARE_STD_STREAMABLE				\
    public:						\
	void		read (istream& is);		\
	void		write (ostream& os) const;	\
	streamsize	stream_size (void) const

  /// Specifies that \p T is printed by using it as an index into \p Names string array.
#define LOOKUP_TEXT_STREAMABLE(T,Names,nNames)	\
    namespace ustl {				\
	inline ostringstream& operator<< (ostringstream& os, const T& v) {	\
	    os << Names[min(uoff_t(v),uoff_t(nNames-1))];			\
	    return os;				\
	}					\
    }


	/// \class cmemlink cmemlink.h ustl.h
	/// \ingroup MemoryManagement
	///
	/// \brief A read-only pointer to a sized block of memory.
	///
	/// Use this class the way you would a const pointer to an allocated unstructured block.
	/// The pointer and block size are available through member functions and cast operator.
	///
	/// Example usage:
	///
	/// \code
	///     void* p = malloc (46721);
	///     cmemlink a, b;
	///     a.link (p, 46721);
	///     assert (a.size() == 46721));
	///     b = a;
	///     assert (b.size() == 46721));
	///     assert (b.DataAt(34) == a.DataAt(34));
	///     assert (0 == memcmp (a, b, 12));
	/// \endcode
	///
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
		inline		cmemlink(void) : _data(nullptr), _size(0) { }
		inline		cmemlink(const void* p, size_type n) : _data(const_pointer(p)), _size(n) { assert(p || !n); }
		inline		cmemlink(const cmemlink& l) : _data(l._data), _size(l._size) {}
		inline virtual     ~cmemlink(void) noexcept {}
		void		link(const void* p, size_type n) {
			if (!p && n)
				throw 0; // std::bad_alloc(n);
			unlink();
			relink(p, n);
		}
		inline void		link(const cmemlink& l) { link(l.begin(), l.size()); }
		inline void		link(const void* first, const void* last) { link(first, std::distance(reinterpret_cast<const char*>( first), reinterpret_cast<const char*>(last))); }
		inline void		relink(const void* p, size_type n);
		virtual void	unlink(void) noexcept { _data = nullptr; _size = 0; }
		inline rcself_t	operator= (const cmemlink& l) { link(l); return *this; }
		bool		operator== (const cmemlink& l) const noexcept {
			return l._size == _size && (l._data == _data || 0 == std::memcmp(l._data, _data, _size));
		}
		inline void		swap(cmemlink& l) { std::swap(_data, l._data); std::swap(_size, l._size); }
		inline size_type	size(void) const { return _size; }
		inline size_type	max_size(void) const { return size(); }
		inline size_type	readable_size(void) const { return size(); }
		inline bool		empty(void) const { return !size(); }
		inline const_pointer	data(void) const { return _data; }
		inline const_pointer	cdata(void) const { return _data; }
		inline iterator	begin(void) const { return iterator(cdata()); }
		inline iterator	iat(size_type i) const { assert(i <= size()); return begin() + i; }
		inline iterator	end(void) const { return iat(size()); }
		inline void		resize(size_type n) { _size = n; }
	//	inline void		read(istream&) { assert(!"ustl::cmemlink is a read-only object."); }
		//void		write(ostream& os) const;
		//size_type		stream_size(void) const noexcept;
		//void		text_write(ostringstream& os) const;
		//void		write_file(const char* filename, int mode = 0644) const;
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
		inline void		link(const void* first, const void* last) { link(first, std::distance(reinterpret_cast<const char*>(  first), reinterpret_cast<const char*>(last))); }
		inline void		link(void* first, void* last) { link(first, std::distance(reinterpret_cast<const char*>(first), reinterpret_cast<const char*>(last))); }
		inline void		relink(const void* p, size_type n) { cmemlink::relink(p, n); }
		inline void		relink(void* p, size_type n) { cmemlink::relink(p, n); }
		inline void		swap(memlink& l) { cmemlink::swap(l); }
		void		fill(const_iterator cstart, const void* p, size_type elSize, size_type elCount = 1) noexcept {
			assert(data() || !elCount || !elSize);
			assert(cstart >= begin() && cstart + elSize * elCount <= end());
			iterator start = const_cast<iterator>(cstart);
			if (elSize == 1)
				std::fill_n(start, elCount, *reinterpret_cast<const uint8_t*>(p));
			else while (elCount--)
				start = std::copy_n(const_iterator(p), elSize, start);
		}
		inline void		insert(const_iterator start, size_type size);
		inline void		erase(const_iterator start, size_type size);
		//void		read(istream& is);
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

	//----------------------------------------------------------------------

	/// A fast alternative to link which can be used when relinking to the same block (i.e. when it is resized)
	inline void cmemlink::relink(const void* p, size_type n)
	{
		_data = reinterpret_cast<const_pointer>(p);
		_size = n;
	}

	//----------------------------------------------------------------------



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
	template<typename ALLOC = std::allocator<char>>
	class memblock : public memlink {
	public:
		memblock(ALLOC alloc = ALLOC()) noexcept : memlink(), _capacity(0), _alloc(alloc) { }
		memblock(const void* p, size_type n, ALLOC alloc = ALLOC()) : memlink(), _capacity(0), _alloc(alloc) { assign(p, n); }
		memblock(size_type n, ALLOC alloc = ALLOC()) : memlink(), _capacity(0), _alloc(alloc) { resize(n); }
		memblock(const cmemlink& b) : memlink(), _capacity(0), _alloc(b._alloc) { assign(b); }
		memblock(const memlink& b) : memlink(), _capacity(0), _alloc(b._alloc) { assign(b); }
		memblock(const memblock& b) : memlink(), _capacity(0), _alloc(b._alloc) { assign(b); }
		~memblock(void) noexcept { deallocate(); }
		inline			memblock(memblock&& b) : memlink(), _capacity(0) { swap(b); }
		inline memblock&		operator= (memblock&& b) { swap(b); return *this; }
		void unlink(void) noexcept { _capacity = 0; memlink::unlink(); }
		inline void			assign(const cmemlink& l) { assign(l.cdata(), l.readable_size()); }
		inline const memblock&	operator= (const cmemlink& l) { assign(l); return *this; }
		inline const memblock&	operator= (const memlink& l) { assign(l); return *this; }
		inline const memblock&	operator= (const memblock& l) { assign(l); return *this; }
		inline void			swap(memblock& l) noexcept { memlink::swap(l); ::ustl::swap(_capacity, l._capacity); }
		void			assign(const void* p, size_type n) {
			assert((p != (const void*)cdata() || size() == n) && "Self-assignment can not resize");
			resize(n);
			copy_n(const_pointer(p), n, begin());
		}
		void reserve(size_type newSize, bool bExact = false) {
			if ((newSize += minimumFreeCapacity()) <= _capacity)
				return;
			pointer oldBlock(is_linked() ? nullptr : data());
			const size_t alignedSize(NextPow2(newSize));
			if (!bExact)
				newSize = alignedSize;
			pointer newBlock = (pointer)ALLOC.allocate(newSize, oldBlock);
			if (!newBlock)
				throw bad_alloc(newSize);
			if (!oldBlock & (cdata() != nullptr))
				copy_n(cdata(), min(size() + 1, newSize), newBlock);
			link(newBlock, size());
			_capacity = newSize;
		}
		void resize(size_type newSize, bool bExact = true)
		{
			if (_capacity < newSize + minimumFreeCapacity())
				reserve(newSize, bExact);
			memlink::resize(newSize);
		}
		iterator			insert(const_iterator start, size_type size) {
			const uoff_t ip = start - begin();
			assert(ip <= size());
			resize(size() + n, false);
			memlink::insert(iat(ip), n);
			return iat(ip);
		}
		iterator			erase(const_iterator start, size_type size) {
			const uoff_t ep = start - begin();
			assert(ep + n <= size());
			reserve(size() - n);
			iterator iep = iat(ep);
			memlink::erase(iep, n);
			memlink::resize(size() - n);
			return iep;
		}
		inline void			clear(void) noexcept { resize(0); }
		inline size_type		capacity(void) const { return _capacity; }
		inline bool			is_linked(void) const { return !capacity(); }
		inline size_type		max_size(void) const { return is_linked() ? memlink::max_size() : SIZE_MAX; }
		inline void			manage(memlink& l) { manage(l.begin(), l.size()); }
		void deallocate(void) noexcept {
			if (_capacity) {
				assert(cdata() && "Internal error: space allocated, but the pointer is nullptr");
				assert(data() && "Internal error: read-only block is marked as allocated space");
				_alloc.deallocate(data(), _capacity);
			}
			unlink();
		}
		void shrink_to_fit(void) {
			if (is_linked())
				return;
			pointer newBlock = (pointer)LLOC.allocate(size(), begin());
			if (!newBlock && size())
				throw bad_alloc(size());
			_capacity = size();
			memlink::relink(newBlock, size());
		}
		void manage(void* p, size_type n) noexcept {
			assert(p || !n);
			assert(!_capacity && "Already managing something. deallocate or unlink first.");
			link(p, n);
			_capacity = n;
		}
		void copy_link(void) {
			const pointer p(begin());
			const size_t sz(size());
			if (is_linked())
				unlink();
			assign(p, sz);
		}
		//void			read(istream& is);
		//void			read_file(const char* filename);
	protected:
		virtual size_type minimumFreeCapacity(void) const noexcept { return 0; }
	private:
		size_type			_capacity;	///< Number of bytes allocated by Resize.
		ALLOC _alloc;
	};



	/// \class vector uvector.h ustl.h
	/// \ingroup Sequences
	///
	/// \brief STL vector equivalent.
	///
	/// Provides a typed array-like interface to a managed memory block, including
	/// element access, iteration, modification, resizing, and serialization. In
	/// this design elements frequently undergo bitwise move, so don't put it in
	/// here if it doesn't support it. This mostly means having no self-pointers.
	///
	template <typename T>
		class vector {
		public:
			typedef T				value_type;
			typedef value_type*			pointer;
			typedef const value_type*		const_pointer;
			typedef value_type&			reference;
			typedef const value_type&		const_reference;
			typedef pointer			iterator;
			typedef const_pointer		const_iterator;
			typedef memblock::size_type		size_type;
			typedef memblock::written_size_type	written_size_type;
			typedef memblock::difference_type	difference_type;
			typedef std::reverse_iterator<iterator>	reverse_iterator;
			typedef std::reverse_iterator<const_iterator>	const_reverse_iterator;
		public:
			inline			vector(void);
			inline explicit		vector(size_type n);
			vector(size_type n, const T& v);
			vector(const vector& v);
			vector(const_iterator i1, const_iterator i2);
			inline			~vector(void) noexcept;
			inline const vector&	operator= (const vector& v);
			inline bool			operator== (const vector& v) const { return _data == v._data; }
			inline			operator cmemlink (void) const { return cmemlink(_data); }
			inline			operator cmemlink (void) { return cmemlink(_data); }
			inline			operator memlink (void) { return memlink(_data); }
			inline void			reserve(size_type n, bool bExact = false);
			inline void			resize(size_type n, bool bExact = true);
			inline size_type		capacity(void) const { return _data.capacity() / sizeof(T); }
			inline size_type		size(void) const { return _data.size() / sizeof(T); }
			inline size_type		max_size(void) const { return _data.max_size() / sizeof(T); }
			inline bool			empty(void) const { return _data.empty(); }
			inline iterator		begin(void) { return iterator(_data.begin()); }
			inline const_iterator	begin(void) const { return const_iterator(_data.begin()); }
			inline iterator		end(void) { return iterator(_data.end()); }
			inline const_iterator	end(void) const { return const_iterator(_data.end()); }
			inline const_iterator	cbegin(void) const { return begin(); }
			inline const_iterator	cend(void) const { return end(); }
			inline reverse_iterator	rbegin(void) { return reverse_iterator(end()); }
			inline const_reverse_iterator	rbegin(void) const { return const_reverse_iterator(end()); }
			inline reverse_iterator	rend(void) { return reverse_iterator(begin()); }
			inline const_reverse_iterator	rend(void) const { return const_reverse_iterator(begin()); }
			inline const_reverse_iterator	crbegin(void) const { return rbegin(); }
			inline const_reverse_iterator	crend(void) const { return rend(); }
			inline pointer		data(void) { return pointer(_data.data()); }
			inline const_pointer	data(void) const { return const_pointer(_data.data()); }
			inline const_pointer	cdata(void) const { return const_pointer(_data.cdata()); }
			inline iterator		iat(size_type i) { assert(i <= size()); return begin() + i; }
			inline const_iterator	iat(size_type i) const { assert(i <= size()); return begin() + i; }
			inline reference		at(size_type i) { assert(i < size()); return begin()[i]; }
			inline const_reference	at(size_type i) const { assert(i < size()); return begin()[i]; }
			inline reference		operator[] (size_type i) { return at(i); }
			inline const_reference	operator[] (size_type i) const { return at(i); }
			inline reference		front(void) { return at(0); }
			inline const_reference	front(void) const { return at(0); }
			inline reference		back(void) { assert(!empty()); return end()[-1]; }
			inline const_reference	back(void) const { assert(!empty()); return end()[-1]; }
			inline void			push_back(const T& v = T());
			inline void			pop_back(void) { destroy(end() - 1); _data.memlink::resize(_data.size() - sizeof(T)); }
			inline void			clear(void) { destroy(begin(), end()); _data.clear(); }
			inline void			shrink_to_fit(void) { _data.shrink_to_fit(); }
			inline void			deallocate(void) noexcept;
			inline void			assign(const_iterator i1, const_iterator i2);
			inline void			assign(size_type n, const T& v);
			inline void			swap(vector& v) { _data.swap(v._data); }
			inline iterator		insert(const_iterator ip, const T& v);
			inline iterator		insert(const_iterator ip, size_type n, const T& v);
			inline iterator		insert(const_iterator ip, const_iterator i1, const_iterator i2);
			inline iterator		erase(const_iterator ep, size_type n = 1);
			inline iterator		erase(const_iterator ep1, const_iterator ep2);
			inline void			manage(pointer p, size_type n) { _data.manage(p, n * sizeof(T)); }
			inline bool			is_linked(void) const { return _data.is_linked(); }
			inline void			unlink(void) { _data.unlink(); }
			inline void			copy_link(void) { _data.copy_link(); }
			inline void			link(const_pointer p, size_type n) { _data.link(p, n * sizeof(T)); }
			inline void			link(pointer p, size_type n) { _data.link(p, n * sizeof(T)); }
			inline void			link(const vector& v) { _data.link(v); }
			inline void			link(vector& v) { _data.link(v); }
			inline void			link(const_pointer first, const_pointer last) { _data.link(first, last); }
			inline void			link(pointer first, pointer last) { _data.link(first, last); }
//			inline void			read(istream& is) { container_read(is, *this); }
//			inline void			write(ostream& os) const { container_write(os, *this); }
//			inline void			text_write(ostringstream& os) const { container_text_write(os, *this); }
//			inline size_t		stream_size(void) const { return container_stream_size(*this); }

			inline			vector(vector&& v) : _data(move(v._data)) {}
			inline			vector(std::initializer_list<T> v) : _data() { uninitialized_copy_n(v.begin(), v.size(), append_hole(v.size())); }
			inline vector&		operator= (vector&& v) { swap(v); return *this; }
			template <typename... Args>
			inline iterator		emplace(const_iterator ip, Args&&... args);
			template <typename... Args>
			inline void			emplace_back(Args&&... args);
			inline void			push_back(T&& v) { emplace_back(move(v)); }
			inline iterator		insert(const_iterator ip, T&& v) { return emplace(ip, move(v)); }
			inline iterator		insert(const_iterator ip, std::initializer_list<T> v) { return insert(ip, v.begin(), v.end()); }

		protected:
			inline iterator		insert_space(const_iterator ip, size_type n);
		private:
			inline iterator		insert_hole(const_iterator ip, size_type n);
			inline iterator		append_hole(size_type n);
		private:
			memblock<std::allocator<char>>		_data;	///< Raw element data, consecutively stored.
		};

		/// Allocates space for at least \p n elements.
		template <typename T>
		inline void vector<T>::reserve(size_type n, bool bExact)
		{
			_data.reserve(n * sizeof(T), bExact);
		}

		template <typename T>
		inline typename vector<T>::iterator vector<T>::append_hole(size_type n)
		{
			_data.reserve(_data.size() + n * sizeof(T));
			_data.memlink::resize(_data.size() + n * sizeof(T));
			return end() - n;
		}

		/// Resizes the vector to contain \p n elements.
		template <typename T>
		inline void vector<T>::resize(size_type n, bool bExact)
		{
			destroy(begin() + n, end());
			const size_type nb = n * sizeof(T);
			if (_data.capacity() < nb)
				reserve(n, bExact);
			uninitialized_default_construct_n(end(), (nb - _data.size()) / sizeof(T));
			_data.memlink::resize(nb);
		}

		/// Calls element destructors and frees storage.
		template <typename T>
		inline void vector<T>::deallocate(void) noexcept
		{
			destroy(begin(), end());
			_data.deallocate();
		}

		/// Initializes empty vector.
		template <typename T>
		inline vector<T>::vector(void)
			:_data()
		{
		}

		/// Initializes a vector of size \p n.
		template <typename T>
		inline vector<T>::vector(size_type n)
			:_data()
		{
			resize(n);
		}

		/// Copies \p n elements from \p v.
		template <typename T>
		vector<T>::vector(size_type n, const T& v)
			:_data()
		{
			uninitialized_fill_n(append_hole(n), n, v);
		}

		/// Copies \p v.
		template <typename T>
		vector<T>::vector(const vector<T>& v)
			:_data()
		{
			uninitialized_copy_n(v.begin(), v.size(), append_hole(v.size()));
		}

		/// Copies range [\p i1, \p i2]
		template <typename T>
		vector<T>::vector(const_iterator i1, const_iterator i2)
			:_data()
		{
			uninitialized_copy(i1, i2, append_hole(distance(i1, i2)));
		}

		/// Destructor
		template <typename T>
		inline vector<T>::~vector(void) noexcept
		{
			destroy(begin(), end());
		}

		/// Copies the range [\p i1, \p i2]
		template <typename T>
		inline void vector<T>::assign(const_iterator i1, const_iterator i2)
		{
			assert(i1 <= i2);
			resize(distance(i1, i2));
			::ustl::copy(i1, i2, begin());
		}

		/// Copies \p n elements with value \p v.
		template <typename T>
		inline void vector<T>::assign(size_type n, const T& v)
		{
			resize(n);
			::ustl::fill(begin(), end(), v);
		}

		/// Copies contents of \p v.
		template <typename T>
		inline const vector<T>& vector<T>::operator= (const vector<T>& v)
		{
			assign(v.begin(), v.end());
			return *this;
		}

		/// Inserts \p n uninitialized elements at \p ip.
		template <typename T>
		inline typename vector<T>::iterator vector<T>::insert_hole(const_iterator ip, size_type n)
		{
			const uoff_t ipmi = distance(_data.begin(), memblock::const_iterator(ip));
			reserve(size() + n);
			return iterator(_data.insert(_data.iat(ipmi), n * sizeof(T)));
		}

		/// Inserts \p n uninitialized elements at \p ip.
		template <typename T>
		inline typename vector<T>::iterator vector<T>::insert_space(const_iterator ip, size_type n)
		{
			iterator ih = insert_hole(ip, n);
			uninitialized_default_construct_n(ih, n);
			return ih;
		}

		/// Inserts \p n elements with value \p v at offsets \p ip.
		template <typename T>
		inline typename vector<T>::iterator vector<T>::insert(const_iterator ip, size_type n, const T& v)
		{
			iterator d = insert_hole(ip, n);
			uninitialized_fill_n(d, n, v);
			return d;
		}

		/// Inserts value \p v at offset \p ip.
		template <typename T>
		inline typename vector<T>::iterator vector<T>::insert(const_iterator ip, const T& v)
		{
			iterator d = insert_hole(ip, 1);
			construct_at(d, v);
			return d;
		}

		/// Inserts range [\p i1, \p i2] at offset \p ip.
		template <typename T>
		inline typename vector<T>::iterator vector<T>::insert(const_iterator ip, const_iterator i1, const_iterator i2)
		{
			assert(i1 <= i2);
			iterator d = insert_hole(ip, distance(i1, i2));
			uninitialized_copy(i1, i2, d);
			return d;
		}

		/// Removes \p count elements at offset \p ep.
		template <typename T>
		inline typename vector<T>::iterator vector<T>::erase(const_iterator ep, size_type n)
		{
			iterator d = const_cast<iterator>(ep);
			destroy_n(d, n);
			return iterator(_data.erase(memblock::iterator(d), n * sizeof(T)));
		}

		/// Removes elements from \p ep1 to \p ep2.
		template <typename T>
		inline typename vector<T>::iterator vector<T>::erase(const_iterator ep1, const_iterator ep2)
		{
			assert(ep1 <= ep2);
			return erase(ep1, distance(ep1, ep2));
		}

		/// Inserts value \p v at the end of the vector.
		template <typename T>
		inline void vector<T>::push_back(const T& v)
		{
			construct_at(append_hole(1), v);
		}


		/// Constructs value at \p ip
		template <typename T>
		template <typename... Args>
		inline typename vector<T>::iterator vector<T>::emplace(const_iterator ip, Args&&... args)
		{
			return new (insert_hole(ip, 1)) T(forward<Args>(args)...);
		}

		/// Constructs value at the end of the vector.
		template <typename T>
		template <typename... Args>
		inline void vector<T>::emplace_back(Args&&... args)
		{
			new (append_hole(1)) T(forward<Args>(args)...);
		}


		/// Returns the number of bytes required to UTF-8 encode \p v.
		inline size_t Utf8Bytes(wchar_t v)
		{
			if ((uint32_t)v < 128)
				return 1;
			size_t n;
#if __i386__ || __x86_64__
			uint32_t r = 0;
			asm("bsr\t%2, %%eax\n\t"
				"add\t$4, %0\n\t"
				"div\t%3":"=a"(n), "+d"(r) : "r"(v), "c"(5));
#else
			static const uint32_t c_Bounds[7] = { 0x0000007F, 0x000007FF, 0x0000FFFF, 0x001FFFFF, 0x03FFFFFF, 0x7FFFFFFF, 0xFFFFFFFF };
			for (n = 0; c_Bounds[n++] < uint32_t(v););
#endif
			return n;
		}

		/// Measures the size of a wchar_t array in UTF-8 encoding.
		inline size_t Utf8Bytes(const wchar_t* first, const wchar_t* last)
		{
			size_t bc = 0;
			for (; first < last; ++first)
				bc += Utf8Bytes(*first);
			return bc;
		}

		/// Returns the number of bytes in a UTF-8 sequence that starts with \p c.
		inline size_t Utf8SequenceBytes(wchar_t c)	// a wchar_t to keep c in a full register
		{
			// Count the leading bits. Header bits are 1 * nBytes followed by a 0.
			//	0 - single byte character. Take 7 bits (0xFF >> 1)
			//	1 - error, in the middle of the character. Take 6 bits (0xFF >> 2)
			//	    so you will keep reading invalid entries until you hit the next character.
			//	>2 - multibyte character. Take remaining bits, and get the next bytes.
			// All errors are ignored, since the user can not correct them.
			//
			wchar_t mask = 0x80;
			size_t nBytes = 0;
			for (; c & mask; ++nBytes)
				mask >>= 1;
			return nBytes ? nBytes : 1; // A sequence is always at least 1 byte.
		}

		//----------------------------------------------------------------------

		/// \class utf8in_iterator utf8.h ustl.h
		/// \ingroup IteratorAdaptors
		///
		/// \brief An iterator adaptor to character containers for reading UTF-8 encoded text.
		///
		/// For example, you can copy from ustl::string to ustl::vector<wchar_t> with
		/// copy (utf8in (str.begin()), utf8in (str.end()), back_inserter(wvect));
		/// There is no error handling; if the reading frame slips you'll get extra
		/// characters, one for every misaligned byte. Although it is possible to skip
		/// to the start of the next character, that would result in omitting the
		/// misformatted character and the one after it, making it very difficult to
		/// detect by the user. It is better to write some strange characters and let
		/// the user know his file is corrupted. Another problem is overflow on bad
		/// encodings (like a 0xFF on the end of a string). This is checked through
		/// the end-of-string nul character, which will always be there as long as
		/// you are using the string class.
		///
		template <typename Iterator, typename WChar = wchar_t>
		class utf8in_iterator {
		public:
			typedef typename std::iterator_traits<Iterator>::value_type	value_type;
			typedef typename std::iterator_traits<Iterator>::difference_type	difference_type;
			typedef typename std::iterator_traits<Iterator>::pointer		pointer;
			typedef typename std::iterator_traits<Iterator>::reference	reference;
			typedef std::input_iterator_tag					iterator_category;
		public:
			explicit			utf8in_iterator(const Iterator& is) : _i(is), _v(0) { Read(); }
			utf8in_iterator(const utf8in_iterator& i) : _i(i._i), _v(i._v) {}
			inline const utf8in_iterator& operator= (const utf8in_iterator& i) { _i = i._i; _v = i._v; return *this; }
			inline Iterator		base(void) const { return _i - (Utf8Bytes(_v) - 1); }
			/// Reads and returns the next value.
			inline WChar		operator* (void) const { return _v; }
			inline utf8in_iterator&	operator++ (void) { ++_i; Read(); return *this; }
			inline utf8in_iterator	operator++ (int) { utf8in_iterator old(*this); operator++(); return old; }
			inline utf8in_iterator&	operator+= (size_t n) { while (n--) operator++(); return *this; }
			inline utf8in_iterator	operator+ (size_t n) { utf8in_iterator v(*this); return v += n; }
			inline bool			operator== (const utf8in_iterator& i) const { return _i == i._i; }
			inline bool			operator< (const utf8in_iterator& i) const { return _i < i._i; }
			difference_type		operator- (const utf8in_iterator& i) const;
		private:
			void			Read(void);
		private:
			Iterator			_i;
			WChar			_v;
		};

		/// Steps to the next character and updates current returnable value.
		template <typename Iterator, typename WChar>
		void utf8in_iterator<Iterator, WChar>::Read(void)
		{
			const utf8subchar_t c = *_i;
			size_t nBytes = Utf8SequenceBytes(c);
			_v = c & (0xFF >> nBytes);	// First byte contains bits after the header.
			while (--nBytes && *++_i)	// Each subsequent byte has 6 bits.
				_v = (_v << 6) | (*_i & 0x3F);
		}

		/// Returns the distance in characters (as opposed to the distance in bytes).
		template <typename Iterator, typename WChar>
		typename utf8in_iterator<Iterator, WChar>::difference_type
			utf8in_iterator<Iterator, WChar>::operator- (const utf8in_iterator<Iterator, WChar>& last) const
		{
			difference_type dist = 0;
			for (Iterator first(last._i); first < _i; ++dist)
				first = advance(first, Utf8SequenceBytes(*first));
			return dist;
		}

		//----------------------------------------------------------------------

		/// \class utf8out_iterator utf8.h ustl.h
		/// \ingroup IteratorAdaptors
		///
		/// \brief An iterator adaptor to character containers for writing UTF-8 encoded text.
		///
		template <typename Iterator, typename WChar = wchar_t>
		class utf8out_iterator {
		public:
			typedef typename std::iterator_traits<Iterator>::value_type	value_type;
			typedef typename std::iterator_traits<Iterator>::difference_type	difference_type;
			typedef typename std::iterator_traits<Iterator>::pointer		pointer;
			typedef typename std::iterator_traits<Iterator>::reference	reference;
			typedef std::output_iterator_tag					iterator_category;
		public:
			explicit			utf8out_iterator(const Iterator& os) : _i(os) {}
			utf8out_iterator(const utf8out_iterator& i) : _i(i._i) {}
			inline const Iterator&	base(void) const { return _i; }
			/// Writes \p v into the stream.
			utf8out_iterator&		operator= (WChar v);
			inline utf8out_iterator&	operator* (void) { return *this; }
			inline utf8out_iterator&	operator++ (void) { return *this; }
			inline utf8out_iterator&	operator++ (int) { return *this; }
			inline bool			operator== (const utf8out_iterator& i) const { return _i == i._i; }
			inline bool			operator< (const utf8out_iterator& i) const { return _i < i._i; }
		private:
			Iterator			_i;
		};

		/// Writes \p v into the stream.
		template <typename Iterator, typename WChar>
		utf8out_iterator<Iterator, WChar>& utf8out_iterator<Iterator, WChar>::operator= (WChar v)
		{
			const size_t nBytes = Utf8Bytes(v);
			if (nBytes > 1) {
				// Write the bits 6 bits at a time, except for the first one,
				// which may be less than 6 bits.
				wchar_t shift = nBytes * 6;
				*_i++ = ((v >> (shift -= 6)) & 0x3F) | (0xFF << (8 - nBytes));
				while (shift)
					*_i++ = ((v >> (shift -= 6)) & 0x3F) | 0x80;
			}
			else	// If only one byte, there is no header.
				*_i++ = v;
			return *this;
		}

		//----------------------------------------------------------------------

		/// Returns a UTF-8 adaptor writing to \p i. Useful in conjuction with back_insert_iterator.
		template <typename Iterator>
		inline utf8out_iterator<Iterator> utf8out(Iterator i)
		{
			return utf8out_iterator<Iterator>(i);
		}

		/// Returns a UTF-8 adaptor reading from \p i.
		template <typename Iterator>
		inline utf8in_iterator<Iterator> utf8in(Iterator i)
		{
			return utf8in_iterator<Iterator>(i);
		}



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
		class string : public memblock<> {
		public:
			typedef char		value_type;
			typedef unsigned char	uvalue_type;
			typedef value_type*		pointer;
			typedef const value_type*	const_pointer;
			typedef wchar_t		wvalue_type;
			typedef wvalue_type*	wpointer;
			typedef const wvalue_type*	const_wpointer;
			typedef pointer		iterator;
			typedef const_pointer	const_iterator;
			typedef value_type&		reference;
			typedef value_type		const_reference;
			typedef std::reverse_iterator<iterator>		reverse_iterator;
			typedef std::reverse_iterator<const_iterator>	const_reverse_iterator;
			typedef utf8in_iterator<const_iterator>		utf8_iterator;
			typedef size_t		pos_type;

			static constexpr const pos_type npos = INT_MAX;	///< Value that means the end of string.
		public:
			inline			string(void) noexcept : memblock() { relink("", 0); }
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
			inline utf8_iterator	utf8_begin(void) const { return utf8_iterator(begin()); }
			inline utf8_iterator	utf8_end(void) const { return utf8_iterator(end()); }
			inline const_reference	at(pos_type pos) const { assert(pos <= size() && begin()); return begin()[pos]; }
			inline reference		at(pos_type pos) { assert(pos <= size() && begin()); return begin()[pos]; }
			inline const_iterator	iat(pos_type pos) const { return begin() + ((pos) && pos >= npos ? size() : std::min(size_type(pos), size())); }
			inline iterator		iat(pos_type pos) { return const_cast<iterator>(const_cast<const string*>(this)->iat(pos)); }
			const_iterator		wiat(pos_type i) const noexcept;
			inline iterator		wiat(pos_type i) { return const_cast<iterator>(const_cast<const string*>(this)->wiat(i)); }
			inline const_reference	front(void) const { return at(0); }
			inline reference		front(void) { return at(0); }
			inline const_reference	back(void) const { return at(size() - 1); }
			inline reference		back(void) { return at(size() - 1); }
			inline size_type		length(void) const { return std::distance(utf8_begin(), utf8_end()); }
			inline string&		append(const_iterator i1, const_iterator i2) { return append(i1, std::distance(i1, i2)); }
			string&	   		append(const_pointer s, size_type len);
			string&	   		append(const_pointer s);
			string&			append(size_type n, value_type c);
			inline string&		append(size_type n, wvalue_type c) { insert(size(), n, c); return *this; }
			inline string&		append(const_wpointer s1, const_wpointer s2) { insert(size(), s1, s2); return *this; }
			inline string&		append(const_wpointer s) { const_wpointer se(s); for (; se&&*se; ++se) {} return append(s, se); }
			inline string&		append(const string& s) { return append(s.begin(), s.end()); }
			inline string&		append(const string& s, pos_type o, size_type n) { return append(s.iat(o), s.iat(o + n)); }
			inline void			push_back(value_type c) { resize(size() + 1); end()[-1] = c; }
			inline void			push_back(wvalue_type c) { append(1, c); }
			inline void			pop_back(void) { resize(size() - 1); }
			inline string&		assign(const_iterator i1, const_iterator i2) { return assign(i1, std::distance(i1, i2)); }
			string&	    		assign(const_pointer s, size_type len);
			string&	    		assign(const_pointer s);
			inline string&		assign(const_wpointer s1, const_wpointer s2) { clear(); return append(s1, s2); }
			inline string&		assign(const_wpointer s1) { clear(); return append(s1); }
			inline string&		assign(const string& s) { return assign(s.begin(), s.end()); }
			inline string&		assign(const string& s, pos_type o, size_type n) { return assign(s.iat(o), s.iat(o + n)); }
			inline string&		assign(size_type n, value_type c) { clear(); return append(n, c); }
			inline string&		assign(size_type n, wvalue_type c) { clear(); return append(n, c); }
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
			inline const string&	operator= (const_wpointer s) { return assign(s); }
			inline const string&	operator+= (const string& s) { return append(s.begin(), s.size()); }
			inline const string&	operator+= (value_type c) { push_back(c); return *this; }
			inline const string&	operator+= (const_pointer s) { return append(s); }
			inline const string&	operator+= (wvalue_type c) { return append(1, c); }
			inline const string&	operator+= (uvalue_type c) { return operator+= (value_type(c)); }
			inline const string&	operator+= (const_wpointer s) { return append(s); }
			inline string		operator+ (const string& s) const;
			inline bool			operator== (const string& s) const { return memblock::operator== (s); }
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
			string&			insert(pos_type ip, size_type n, wvalue_type c);
			string&			insert(pos_type ip, const_wpointer first, const_wpointer last, size_type n = 1);
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
			inline string&		replace(pos_type rp, size_type n, const string& s, size_t sp, size_type slen) { return replace(iat(rp), iat(rp + n), s.iat(sp), s.iat(sp + slen)); }
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
			int				format(const char* fmt, ...);
		//	void			read(istream&);
		//	void			write(ostream& os) const;
			size_t			stream_size(void) const noexcept;
			static size_t		hash(const char* f1, const char* l1) noexcept;
#if HAVE_CPP11
			using initlist_t = std::initializer_list<value_type>;
			inline			string(string&& v) : memblock(move(v)) {}
			inline			string(initlist_t v) : memblock() { assign(v.begin(), v.size()); }
			inline string&		assign(string&& v) { swap(v); return *this; }
			inline string&		assign(initlist_t v) { return assign(v.begin(), v.size()); }
			inline string&		append(initlist_t v) { return append(v.begin(), v.size()); }
			inline string&		operator+= (initlist_t v) { return append(v.begin(), v.size()); }
			inline string&		operator= (string&& v) { return assign(move(v)); }
			inline string&		operator= (initlist_t v) { return assign(v.begin(), v.size()); }
			inline iterator		insert(const_iterator ip, initlist_t v) { return insert(ip, v.begin(), v.end()); }
			inline string&		replace(const_iterator first, const_iterator last, initlist_t v) { return replace(first, last, v.begin(), v.end()); }
#endif
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
			std::fill_n(iat(oldn), std::max(ptrdiff_t(n - oldn), 0), c);
		}

		//----------------------------------------------------------------------
		// Operators needed to avoid comparing pointer to pointer

#define PTR_STRING_CMP(op, impl)	\
inline bool op (const char* s1, const string& s2) { return impl; }
		PTR_STRING_CMP(operator==, (s2 == s1))
			PTR_STRING_CMP(operator!=, (s2 != s1))
			PTR_STRING_CMP(operator<, (s2 >  s1))
			PTR_STRING_CMP(operator<=, (s2 >= s1))
			PTR_STRING_CMP(operator>, (s2 <  s1))
			PTR_STRING_CMP(operator>=, (s2 <= s1))
#undef PTR_STRING_CMP

			inline string operator+ (const char* cs, const string& ss) { string r; r.reserve(strlen(cs) + ss.size()); r += cs; r += ss; return r; }

		//----------------------------------------------------------------------

		inline size_t hash_value(const char* first, const char* last)
		{
			return string::hash(first, last);
		}
		inline size_t hash_value(const char* v)
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






};





#endif



#endif