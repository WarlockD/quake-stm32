// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#pragma once
#include "uexception.h"

/// Just like malloc, but throws on failure.
extern "C" void* tmalloc (size_t n) __attribute__((malloc));
/// Just like free, but doesn't crash when given a nullptr.
extern "C" void nfree (void* p) noexcept;

#if WITHOUT_LIBSTDCPP

#if __clang__
    // Turn off exception spec warnings for operator new/delete
    // uSTL does not use explicit throw specifications on them
    #pragma GCC diagnostic ignored "-Wmissing-exception-spec"
#endif

//
// These are replaceable signatures:
//  - normal single new and delete (no arguments, throw @c bad_alloc on error)
//  - normal array new and delete (same)
//  - @c nothrow single new and delete (take a @c nothrow argument, return
//    @c nullptr on error)
//  - @c nothrow array new and delete (same)
//
//  Placement new and delete signatures (take a memory address argument,
//  does nothing) may not be replaced by a user's program.
//
void* operator new (size_t n);
void* operator new[] (size_t n);
void  operator delete (void* p) noexcept;
void  operator delete[] (void* p) noexcept;
#if HAVE_CPP14
void  operator delete (void* p, size_t) noexcept;
void  operator delete[] (void* p, size_t) noexcept;
#endif

// Default placement versions of operator new.
inline void* operator new (size_t, void* p) noexcept	{ return p; }
inline void* operator new[] (size_t, void* p) noexcept	{ return p; }

// Default placement versions of operator delete.
inline void  operator delete  (void*, void*) noexcept	{ }
inline void  operator delete[](void*, void*) noexcept	{ }

#endif	// WITHOUT_LIBSTDCPP
