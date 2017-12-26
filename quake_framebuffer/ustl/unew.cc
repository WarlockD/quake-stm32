// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "unew.h"

void* tmalloc (size_t n)
{
    void* p = malloc (n);
    if (!p)
	throw ustl::bad_alloc (n);
    return p;
}

void nfree (void* p) noexcept
{
    if (p)
	free (p);
}

#if WITHOUT_LIBSTDCPP
#if __APPLE__	// MacOS lives in the stone age and does not support aliases

void* operator new (size_t n)	{ return tmalloc(n); }
void* operator new[] (size_t n)	{ return tmalloc(n); }

void  operator delete (void* p) noexcept	{ nfree(p); }
void  operator delete[] (void* p) noexcept	{ nfree(p); }
#if HAVE_CPP14
void  operator delete (void* p, size_t) noexcept	{ nfree(p); }
void  operator delete[] (void* p, size_t) noexcept	{ nfree(p); }
#endif // HAVE_CPP14

#else // __APPLE__

void* operator new (size_t n)	WEAKALIAS("tmalloc");
void* operator new[] (size_t n)	WEAKALIAS("tmalloc");

void  operator delete (void* p) noexcept	WEAKALIAS("nfree");
void  operator delete[] (void* p) noexcept	WEAKALIAS("nfree");
#if HAVE_CPP14
void  operator delete (void* p, size_t n) noexcept	WEAKALIAS("nfree");
void  operator delete[] (void* p, size_t n) noexcept	WEAKALIAS("nfree");
#endif // HAVE_CPP14

#endif // __APPLE__
#endif // WITHOUT_LIBSTDCPP
