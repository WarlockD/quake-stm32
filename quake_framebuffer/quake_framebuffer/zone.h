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
/*
 memory allocation


H_??? The hunk manages the entire memory block given to quake.  It must be
contiguous.  Memory can be allocated from either the low or high end in a
stack fashion.  The only way memory is released is by resetting one of the
pointers.

Hunk allocations should be given a name, so the Hunk_Print () function
can display usage.

Hunk allocations are guaranteed to be 16 byte aligned.

The video buffers are allocated high to avoid leaving a hole underneath
server allocations when changing to a higher video mode.


Z_??? Zone memory functions used for small, dynamic allocations like text
strings from command input.  There is only about 48K for it, allocated at
the very bottom of the hunk.

Cache_??? Cache memory is for objects that can be dynamically loaded and
can usefully stay persistant between levels.  The size of the cache
fluctuates from level to level.

To allocate a cachable object


Temp_??? Temp memory is used for file loading and surface caching.  The size
of the cache memory is adjusted so that there is a minimum of 512k remaining
for temp memory.


------ Top of Memory -------

high hunk allocations

<--- high hunk reset point held by vid

video buffer

z buffer

surface cache

<--- high hunk used

cachable memory

<--- low hunk used

client and server low hunk allocations

<-- low hunk reset point held by host

startup hunk allocations

Zone block

----- Bottom of Memory -----



*/
#ifndef _QUAKE_ZONE_H_
#define _QUAKE_ZONE_H_

#include "quakedef.h"
#include "common.h"

#include <string>
#include <sstream>
#include <vector>
#include <variant>
#include <unordered_map>

void Memory_Init(void *buf, size_t size);

void Z_Free(void *ptr);
void *Z_Malloc(size_t size);			// returns 0 filled memory
void *Z_TagMalloc(size_t size, int tag);


void Z_CheckHeap(void);


void *Hunk_Alloc(size_t size);		// returns 0 filled memory
void *Hunk_AllocName(int size, const quake::string_view& name);

void *Hunk_HighAllocName(size_t size, const quake::string_view& name);

int	Hunk_LowMark(void);
void Hunk_FreeToLowMark(int mark);

int	Hunk_HighMark(void);
void Hunk_FreeToHighMark(int mark);

void *Hunk_TempAlloc(size_t size);

void Hunk_Check(void);

struct cache_user_t
{
	void	*data;
} ;

void Cache_Flush(void);

void *Cache_Check(cache_user_t *c);
// returns the cached data, and moves to the head of the LRU list
// if present, otherwise returns NULL

void Cache_Free(cache_user_t *c);

void *Cache_Alloc(cache_user_t *c, int size, const quake::string_view& name);
// Returns NULL if all purgable data was tossed and there still
// wasn't enough room.

void Cache_Report(void);

class ZObject {
	void* operator new(size_t n) { return Z_Malloc(n); }
	void operator delete(void* p) { Z_Free(p); }
};

template <typename  T>
class ZAllocator
{
public:
	using std_allocator = std::allocator<T>;
	using size_type = size_t;
	using difference_type = ptrdiff_t;
	using value_type = T;
	using pointer = value_type*;
	using const_pointer = const value_type*;
	using reference = value_type&;
	using const_reference = const value_type&;


	ZAllocator() {}
	ZAllocator(const ZAllocator&) {}

	pointer   allocate(size_type n, const void * = 0) { return (T*)Z_Malloc(n); }
	void      deallocate(void* p, size_type n) { (void)n; Z_Free(p); }


	pointer           address(reference x) const { return &x; }
	const_pointer     address(const_reference x) const { return &x; }
	ZAllocator<T>&  operator=(const ZAllocator&) { return *this; }
	void              construct(pointer p, const T& val) { new ((T*)p) T(val); }
	void              destroy(pointer p) { p->~T(); }
	size_type         max_size() const { return size_t(-1); }

	template <class U>
	struct rebind { typedef ZAllocator<U> other; };

	template <class U>
	ZAllocator(const ZAllocator<U>&) {}

	template <class U>
	ZAllocator& operator=(const ZAllocator<U>&) { return *this; }
};
// carful with this,
template <typename  T>
class HAllocator
{
public:
	using std_allocator = std::allocator<T>;
	using size_type = size_t;
	using difference_type = ptrdiff_t;
	using value_type = T;
	using pointer = value_type*;
	using const_pointer = const value_type*;
	using reference = value_type&;
	using const_reference = const value_type&;


	HAllocator() : _mark(Hunk_LowMark()) {}
	HAllocator(const HAllocator&) {}
	~HAllocator() { Hunk_FreeToLowMark(_mark); }
	pointer   allocate(size_type n, const void * pptr = 0) { return (T*)Hunk_AllocName(n, "halloc"); }
	void      deallocate(void* p, size_type n) { (void)n; }


	pointer           address(reference x) const { return &x; }
	const_pointer     address(const_reference x) const { return &x; }
	ZAllocator<T>&  operator=(const ZAllocator&) { return *this; }
	void              construct(pointer p, const T& val) { new ((T*)p) T(val); }
	void              destroy(pointer p) { p->~T(); }
	size_type         max_size() const { return size_t(-1); }

	template <class U>
	struct rebind { typedef HAllocator<U> other; };

	template <class U>
	HAllocator(const HAllocator<U>&) {}

	template <class U>
	HAllocator& operator=(const HAllocator<U>&) { return *this; }
private:
	int _mark;
};
template <typename  T>
class CacheAllocator
{
	// kind of a hack I know, figure it out better once I figure out the 
	// the cache system
	struct tracker_t {
		cache_user_t user;
		tracker_t *next;
		tracker_t() : user{ nullptr }, next(nullptr) {}
		tracker_t(size_t size) : user{ nullptr }, next(nullptr) {
			Cache_Alloc(&user, size,"stl");
		}
		~tracker_t() {
			if (user.data) Cache_Free(&user);
		}
		static void* operator new(size_t size) { return Zmalloc(size); }
		static void operator delete(void* ptr) { ZFree(ptr); }
	};
public:
	using std_allocator = std::allocator<T>;
	using size_type = size_t;
	using difference_type = ptrdiff_t;
	using value_type = T;
	using pointer = value_type*;
	using const_pointer = const value_type*;
	using reference = value_type&;
	using const_reference = const value_type&;


	CacheAllocator() : _cache{ nullptr } {}
	CacheAllocator(const CacheAllocator&) : _cache{ nullptr } { assert(0); } // watch copies
	~CacheAllocator() {
		assert(_cache);
	}
	pointer   allocate(size_type n, const void * old_ptr = nullptr) { 
		(void)old_ptr;
		tracker_t* tracker = new tracker_t(n);
		if (tracker->user.data) {
			tracker->next = _cache;
			_cache = tracker;
			return tracker->user.data;

		}
		else {
			delete tracker;
			return nullptr;
		}
	}
	void      deallocate(void* p, size_type size=0) { 
		(void)size;
		auto p = nullptr;
		for (auto n = _cache : n!= nullptr; p=n,n = n->next) {
			if (n->user.data == p) {
				if (p) p->next = n->next;
				else _cache = n->next;
				delete n;
				break;
			}
		}
		assert(0); // not found?
	}


	pointer           address(reference x) const { return &x; }
	const_pointer     address(const_reference x) const { return &x; }
	ZAllocator<T>&  operator=(const ZAllocator&) { return *this; }
	void              construct(pointer p, const T& val) { new ((T*)p) T(val); }
	void              destroy(pointer p) { p->~T(); }
	size_type         max_size() const { return size_t(-1); }

	template <class U>
	struct rebind { typedef CacheAllocator<U> other; };

	template <class U>
	CacheAllocator(const CacheAllocator<U>&) {}

	template <class U>
	CacheAllocator& operator=(const CacheAllocator<U>&) { return *this; }
private:
	tracker_t* _cache;
};

namespace quake {
	using string = std::basic_string<char, std::char_traits<char>, ZAllocator<char>>;
	using stringstream = std::basic_stringstream<char, std::char_traits<char>, ZAllocator<char>>;
}
// some basic wrappers to use the doom memory system
using ZString = std::basic_string<char, std::char_traits<char>, ZAllocator<char>>;
using HString = std::basic_string<char, std::char_traits<char>, HAllocator<char>>;
using CacheString = std::basic_string<char, std::char_traits<char>, ZAllocator<char>>;
using CacheStringStream = std::basic_stringstream<char, std::char_traits<char>, CacheAllocator<char>>;
using ZStringStream = std::basic_stringstream<char, std::char_traits<char>, ZAllocator<char>>;

template<typename T>
using ZVector = std::vector<T, ZAllocator<T>>;



#endif