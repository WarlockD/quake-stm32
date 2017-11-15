// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.
//
/// \file strmsize.h
/// \brief This file contains stream_size_of functions for basic types and *STREAMABLE macros.
/// stream_size_of functions return the size of the object's data that is written or
/// read from a stream.

#pragma once
#include <type_traits>
#include <utility>
namespace qstl {

	using streamoff = std::streamoff;
	using streamsize = std::streamsize;


#ifndef DOXYGEN_SHOULD_SKIP_THIS
	// The compiler issues a warning if an unsigned type is compared to 0.
	template <typename T, bool IsSigned> struct __is_negative { inline constexpr bool operator()(const T& v) const { return v < 0; } };
	template <typename T> struct __is_negative<T, false> { inline constexpr bool operator()(const T&) const { return false; } };
	/// Warning-free way to check if \p v is negative, even if for unsigned types.
	template <typename T> inline constexpr bool is_negative(const T& v) { 
		using test = __is_negative<T, std::is_signed_v<T>>;
		return test()(v);
	}
#endif

	/// \brief Returns the absolute value of \p v
	/// Unlike the stdlib functions, this is inline and works with all types.
	template <typename T> inline constexpr T absv(T v) { return is_negative(v) ? -v : v; }

	/// \brief Returns -1 for negative values, 1 for positive, and 0 for 0
	template <typename T> inline constexpr T sign(T v) { return (0 < v) - is_negative(v); }

	/// Returns the absolute value of the distance i1 and i2
	template <typename T1, typename T2>
	inline constexpr size_t abs_distance(T1 i1, T2 i2) { return absv(std::distance(i1, i2)); }

	/// Returns the size of \p n elements of size \p T
	template <typename T>
	inline constexpr size_t size_of_elements(size_t n, const T*) { return n * sizeof(T); }

	/// Returns the greatest common divisor
	template <typename T>
	constexpr T gcd(T a, T b)
	{
		return b ? gcd(b, a%b) : absv(a);
	}

	/// Returns the least common multiple
	template <typename T>
	constexpr T lcm(T a, T b)
	{
		return a / gcd(a, b)*b;
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
		if (n1 < 0)
			adj = -adj;
		return (n1 + adj) / n2;
	}

	/// Sets the contents of \p pm to 1 and returns true if the previous value was 0.
	inline bool TestAndSet(int* pm) { const int oldVal(*pm); *pm = 1; return !oldVal; }
	inline bool TestAndSet(bool* pm) { const bool oldVal(*pm); *pm = true; return !oldVal; }
	template<typename T>
	constexpr inline size_t BitsInType(const T&) { return sizeof(T) / 8U; }
	/// Returns the index of the first set bit in \p v or \p nbv if none.
	inline size_t  FirstBit(uint32_t v, size_t  nbv)
	{
		size_t n = nbv;
		if (v) for (uint32_t m = uint32_t(1) << (n = 31); !(v & m); m >>= 1) --n;
		return n;
	}

	/// Returns the index of the first set bit in \p v or \p nbv if none.
	inline size_t  FirstBit(uint64_t v, size_t nbv)
	{
		size_t  n = nbv;
		if (v) for (uint64_t m = uint64_t(1) << (n = 63); !(v & m); m >>= 1) --n;
		return n;
	}

	/// Returns the next power of 2 >= \p v.
	/// Values larger than UINT32_MAX/2 will return 2^0
	inline uint32_t NextPow2(uint32_t v)
	{
		uint32_t r = v - 1;
		r = FirstBit(r, r);
		if (r >= BitsInType(r) - 1) r = uint32_t(-1);
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

	/// Indexes into a static array with bounds limit
	template <typename T, size_t N>
	inline constexpr T& VectorElement(T(&v)[N], size_t i) { return v[std::min(i, N - 1)]; }

	/// The alignment performed by default.
	const size_t c_DefaultAlignment = __alignof(void*);

	/// \brief Rounds \p n up to be divisible by \p grain
	template <typename T>
	inline constexpr T AlignDown(T n, size_t grain = c_DefaultAlignment) { return n - n % grain; }

	/// \brief Rounds \p n up to be divisible by \p grain
	template <typename T>
	inline constexpr T Align(T n, size_t grain = c_DefaultAlignment) { return AlignDown(n + grain - 1, grain); }



/// For partial specialization of stream_size_of for objects
template <typename T> struct object_stream_size {
    inline streamsize operator()(const T& v) const { return v.stream_size(); }
};
template <typename T> struct integral_object_stream_size {
    inline streamsize operator()(const T& v) const { return sizeof(v); }
};
/// Returns the size of the given object. Overloads for standard types are available.
template <typename T>
inline streamsize stream_size_of (const T& v) {
    typedef typename std::conditional_t<std::is_integral_v<T>,
	integral_object_stream_size<T>, object_stream_size<T>> stream_sizer_t;
    return stream_sizer_t()(v);
}

/// \brief Returns the recommended stream alignment for type \p T. Override with ALIGNOF.
/// Because this is occasionally called with a null value, do not access the argument!
template <typename T>
inline size_t stream_align_of (const T&){ return std::is_integral_v<T> ? __alignof(T) : c_DefaultAlignment;}

#define ALIGNOF(type,grain)	\
namespace qstl {		\
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
    namespace qstl {		\
	inline istream& operator>> (qstl::istream& is, T& v)		{ is.iread(v);  return is; }	\
	inline ostream& operator<< (qstl::ostream& os, const T& v)	{ os.iwrite(v); return os; }	\
	inline ostream& operator<< (qstl::ostream& os, T& v)		{ os.iwrite(v); return os; }	\
	template<> inline streamsize stream_size_of(const T& v)	{ return sizeof(v); }		\
    }

/// Declares that T contains read, write, and stream_size methods. This is no longer needed and is deprecated.
#define STD_STREAMABLE(T)

/// Declares \p T to be writable to text streams. This is no longer needed and is deprecated.
#define TEXT_STREAMABLE(T)

/// Declares that T is to be cast into TSUB for streaming.
#define CAST_STREAMABLE(T,TSUB)	\
    namespace qstl {		\
	inline istream& operator>> (qstl::istream& is, T& v)		{ TSUB sv; is >> sv; v = (T)(sv); return is; }	\
	inline ostream& operator<< (qstl::ostream& os, const T& v)	{ os << TSUB(v); return os; }			\
	template<> inline streamsize stream_size_of(const T& v)	{ return stream_size_of (TSUB(v)); }		\
    }

/// Placed into a class it declares the methods required by STD_STREAMABLE. Syntactic sugar.
#define DECLARE_STD_STREAMABLE				\
    public:						\
	void		read (qstl::istream& is);		\
	void		write (qstl::ostream& os) const;	\
	streamsize	stream_size (void) const

/// Specifies that \p T is printed by using it as an index into \p Names string array.
#define LOOKUP_TEXT_STREAMABLE(T,Names,nNames)	\
    namespace ustl {				\
	inline ostringstream& operator<< (qstl::ostringstream& os, const T& v) {	\
	    os << Names[min(uoff_t(v),uoff_t(nNames-1))];			\
	    return os;				\
	}					\
    }
