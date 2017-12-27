// config.h - Generated from config.h.in by configure.
#pragma once

// Define to the one symbol short name of this package.
#define USTL_NAME	"@PKG_NAME@"
// Define to the full name and version of this package.
#define USTL_STRING	"@PKG_NAME@ @PKG_VERSTR@"
// Define to the version of this package.
#define USTL_VERSION	@PKG_VERSION@
// Define to the address where bug reports for this package should be sent.
#define USTL_BUGREPORT	"@PKG_BUGREPORT@"

/// Define to 1 if you want stream operations to throw exceptions on
/// insufficient data or insufficient space. All these errors should
/// be preventable in output code; the input code should verify the
/// data in a separate step. It slows down stream operations a lot,
/// but it is your decision. By default only debug builds throw.
///
#define WANT_STREAM_BOUNDS_CHECKING 1

#if !WANT_STREAM_BOUNDS_CHECKING && !defined(NDEBUG)
    #define WANT_STREAM_BOUNDS_CHECKING 1
#endif

/// Define to 1 if you want backtrace symbols demangled.
/// This adds some 15k to the library size, and requires that you link it and
/// any executables you make with the -rdynamic flag (increasing library size
/// even more). By default only the debug build does this.
#undef WANT_NAME_DEMANGLING

#if !WANT_NAME_DEMANGLING && !defined(NDEBUG)
    #define WANT_NAME_DEMANGLING 1
#endif

/// Define to 1 if you want to build without libstdc++
#define WITHOUT_LIBSTDCPP 1

/// Define GNU extensions if unavailable.
#ifndef __GNUC__
    /// GCC (and some other compilers) define '__attribute__'; ustl is using this
    /// macro to alert the compiler to flag inconsistencies in printf/scanf-like
    /// function calls.  Just in case '__attribute__' is undefined, make a dummy.
    /// 
    #ifndef __attribute__
	#define __attribute__(p)
    #endif
#define WEAKALIAS(sym)

#else
#define WEAKALIAS(sym)		__attribute__((weak,alias(sym)))


#endif



#define DLL_EXPORT 
#define DLL_LOCAL
#define INLINE inline
#define HAVE_CPP11 1
#define HAVE_CPP14 1
#define STDC_HEADERS 1
#define STDUNIX_HEADERS 1
#define HAVE_ALLOCA_H 1

#include <cstdint>
#include <cstddef>

/// This macro returns 1 if the value of x is known at compile time.
#ifndef __builtin_constant_p
#define __builtin_constant_p(x)	0
#endif


// Byte order macros, converted in utypes.h
#define USTL_LITTLE_ENDIAN	__ORDER_LITTLE_ENDIAN__
#define USTL_BIG_ENDIAN		__ORDER_BIG_ENDIAN__
#define USTL_BYTE_ORDER		__BYTE_ORDER__

#if __i386__ || __x86_64__
    #define __x86__ 1
#endif

// GCC vector extensions
#if (__MMX__ || __SSE__) && __GNUC__ >= 3
    #undef HAVE_VECTOR_EXTENSIONS
#endif

#if CPU_HAS_SSE && __GNUC__
    #define __sse_align	__attribute__((aligned(16)))
#else
    #define __sse_align	
#endif

#if HAVE_CPP11
    #define USTL_LIBSUPCPP_LINKARG	" @libsupc++@"
    static_assert (sizeof(USTL_LIBSUPCPP_LINKARG) != sizeof(""),
	"configure was unable to find required library libsupc++.a . This "
	"may occur if your distribution violates the Linux Filesystem "
	"Standard by not storing system libraries in /usr/lib . To "
	"correct the problem, please find libsupc++.a and rerun configure "
	"with the --libdir=/usr/lib/libsupcppdir option, allowing the library "
	"to be found and for uSTL to be installed in the location expected "
	"by your distribution.");

#endif
#undef WITHOUT_LIBSTDCPP 