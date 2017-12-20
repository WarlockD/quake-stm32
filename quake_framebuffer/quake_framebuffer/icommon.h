#pragma once
// internal header for some functions


#include "quakedef.h"


// because, some times, we got to use something we know that works, and might be a shit ton of macros
#include "iqueue.h"
namespace quake {
	/// Returns the number of elements in a static vector
#define VectorSize(v)	(sizeof(v) / sizeof(*v))


	/// Returns the end() for a static vector
	template <typename T, size_t N> inline constexpr T* VectorEnd(T(&a)[N]) { return &a[N]; }

	/// Expands into a ptr,size expression for the given static vector; useful as link arguments.
#define VectorBlock(v)	&(v)[0], VectorSize(v)
	/// Expands into a begin,end expression for the given static vector; useful for algorithm arguments.
#define VectorRange(v)	&(v)[0], VectorEnd(v)

	/// Returns the number of bits in the given type
#define BitsInType(t)	(sizeof(t) * CHAR_BIT)

	/// Returns the mask of type \p t with the lowest \p n bits set.
#define BitMask(t,n)	(t(~t(0)) >> (BitsInType(t) - (n)))

	/// Argument that is used only in debug builds (as in an assert)
	/// Shorthand for container iteration.
#define foreach(type,i,ctr)	for (type i = (ctr).begin(); i != (ctr).end(); ++ i)
	/// Shorthand for container reverse iteration.
#define eachfor(type,i,ctr)	for (type i = (ctr).rbegin(); i != (ctr).rend(); ++ i)


	/// Indexes into a static array with bounds limit
	template <typename T, size_t N>
	inline constexpr T& VectorElement(T(&v)[N], size_t i) { return v[std::min(i, N - 1)]; }

	/// The alignment performed by default.
	constexpr static size_t c_DefaultAlignment = sizeof(void*);

	/// \brief Rounds \p n up to be divisible by \p grain
	template <typename T>
	inline constexpr T AlignDown(T n, size_t grain = c_DefaultAlignment)
	{
		return n - n % grain;
	}

	/// \brief Rounds \p n up to be divisible by \p grain
	template <typename T>
	inline constexpr T Align(T n, size_t grain = c_DefaultAlignment)
	{
		return AlignDown(n + grain - 1, grain);
	}

	/// Returns a nullptr pointer cast to T.
	template <typename T>
	inline constexpr T* NullPointer(void)
	{
		return nullptr;
	}

	// The compiler issues a warning if an unsigned type is compared to 0.
	namespace priv {
		template <typename T, bool IsSigned> struct __is_negative { inline constexpr bool operator()(const T& v) const { return v < 0; } };
		template <typename T> struct __is_negative<T, false> { inline constexpr bool operator()(const T&) const { return false; } };
	}

	/// Warning-free way to check if \p v is negative, even if for unsigned types.
	template <typename T> inline constexpr bool is_negative(const T& v) { 
		return priv::__is_negative<T, std::is_signed<T>::value>()(v);
	}


	/// \brief Returns the absolute value of \p v
	/// Unlike the stdlib functions, this is inline and works with all types.
	template <typename T>
	inline constexpr T absv(T v)
	{
		return is_negative(v) ? -v : v;
	}

	/// \brief Returns -1 for negative values, 1 for positive, and 0 for 0
	template <typename T>
	inline constexpr T sign(T v)
	{
		return (0 < v) - is_negative(v);
	}

	/// Returns the absolute value of the distance i1 and i2
	template <typename T1, typename T2>
	inline constexpr size_t abs_distance(T1 i1, T2 i2)
	{
		return absv(std::distance(i1, i2));
	}

	/// Returns the size of \p n elements of size \p T
	template <typename T>
	inline constexpr size_t size_of_elements(size_t n, const T*)
	{
		return n * sizeof(T);
	}

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
	inline size_t FirstBit(uint32_t v, size_t nbv)
	{
		size_t n = nbv;
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
	inline size_t FirstBit(uint64_t v, size_t nbv)
	{
		size_t n = nbv;
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
}