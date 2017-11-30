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

//#include "common.h"
// included in common

namespace quake {
	// used for some templates
	enum class Alloc {
		ZMalloc,
		HulkLow,
		HulkHigh,
		Cache,
		Temp
	};
}
#define	PR_STRING_ALLOCSLOTS	256
void Memory_Init(void *buf, size_t size);

void Z_Free(void *ptr);
void *Z_Malloc(size_t size);			// returns 0 filled memory
void *Z_TagMalloc(size_t size, const void* tag );
void *Z_Realloc(void *ptr, size_t size, const void* tag = nullptr);
char *Z_Strdup(const char *s);

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


void *umm_info(void *ptr, int force);
void *umm_malloc(size_t size);
void *umm_realloc(void *ptr, size_t size);
void umm_free(void *ptr);
size_t umm_maxsize();



namespace type_tags {
	// https://stackoverflow.com/questions/23715212/template-specialization-containers
	// working on a method to prity print containers and their types
	struct unkonwn_stl { static constexpr const char* debug_name = "unkonwn stl"; };
	struct unordered_map : unkonwn_stl { static constexpr const char* debug_name = "unordered_map"; };
	struct string : unkonwn_stl { static constexpr const char* debug_name = "string"; };
	struct vector : unkonwn_stl { static constexpr const char* debug_name = "vector"; };
	struct list : unkonwn_stl { static constexpr const char* debug_name = "list"; };
	struct stream : unkonwn_stl { static constexpr const char* debug_name = "stream"; };
	struct pair : unkonwn_stl { static constexpr const char* debug_name = "pair"; };
	template <typename C, typename E=void> struct container_traits {
		using category = unkonwn_stl;
	};

	template <typename T, typename A>
	struct container_traits<std::vector<T, A>, std::true_type> {
		using category = vector;
	};

	template <typename T, typename A>
	struct container_traits<std::list<T, A>, std::true_type> {
		using category = list;
	};

	template <typename K, typename V, typename H, typename E, typename A>
	struct container_traits<std::unordered_map<K, V, H, E, A>, std::true_type> {
		using category = unordered_map;
	};

	template <typename L,typename R>
	struct container_traits<std::pair<L, R>, std::true_type> {
		using category = pair;
	};


	template <typename L, typename R>
	constexpr const char* _get_type_string_t(std::pair<L,R>) { return "pair"; }
	constexpr const char* _get_type_string(unkonwn_stl) { return "unkonwn stl"; }
	constexpr const char* _get_type_string(unordered_map) { return "unordered_map"; }
	constexpr const char* _get_type_string(string) { return "string"; }
	constexpr const char* _get_type_string(vector) { return "vector"; }
	constexpr const char* _get_type_string(stream) { return "stream"; }
	constexpr const char* _get_type_string(list) { return "list"; }
	constexpr const char* _get_type_string(pair) { return "pair"; }

	template<typename T>
	constexpr const char* get_type_string(const T*) { return _get_type_string(typename container_traits<T>::category{}); }
	constexpr const char* get_type_string(float) { return "float"; }
	constexpr const char* get_type_string(int32_t) { return "int32_t"; }
	constexpr const char* get_type_string(uint32_t) { return "uint32_t"; }
	constexpr const char* get_type_string(char) { return "char"; }
	constexpr const char* get_type_string(uint8_t) { return "byte"; }
	constexpr const char* get_type_string(short) { return "short"; }
	constexpr const char* get_type_string(uint16_t) { return "uint16_t"; }



#if 0
	template <typename Container, typename Compare>
	void sort_helper(Container& c, Compare f, vectorlike_tag) {
		std::sort(c.begin(), c.end(), f);
	}

	template <typename Container, typename Compare>
	void sort_helper(Container& c, Compare f, listlike_tag) {
		c.sort(f);
	}
#endif
}

// uses ummalloc
template <typename T>
class UAllocator
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
	

	//using propagate_on_container_move_assignment = std::true_type;
	template <class U>
	struct rebind { using other =  UAllocator<U> ; };

	UAllocator() noexcept  : type_tag() {
		type_tag = typeid(T).name();
	}
	UAllocator(const UAllocator& copy) noexcept : type_tag(copy.tag()) {}
	UAllocator(UAllocator&& move) noexcept : type_tag(move.tag()) {}
	template <class U>
	UAllocator(const UAllocator<U>&copy) noexcept : type_tag(copy.tag()) {}
	template <class U>
	UAllocator& operator=(const UAllocator<U>& copy) noexcept { type_tag = copy.tag();  return *this; }
	~UAllocator() noexcept {}

	pointer   allocate(size_type n)  {
		assert(n >0);
		//void* ret = umm_malloc(n);

		void* ptr = Z_TagMalloc(n*sizeof(T), type_tag);
		assert(ptr != 0);
		return reinterpret_cast<T*>(ptr);
	}
	void   deallocate(void* p, size_type n) { 
		//(void)n; umm_free(p); 
		(void)n; Z_Free(p);
	}
#if 0
	pointer           address(reference x) const { return &x; }
	const_pointer     address(const_reference x) const { return &x; }


	void              construct(pointer p, const T& val) { new ((T*)p) T(val); }
	void              destroy(pointer p) { p->~T(); }
	size_type         max_size() const { return size_t(-1); }

	pointer           address(reference x) const { return &x; }
	const_pointer     address(const_reference x) const { return &x; }
	UAllocator<T>&  operator=(const UAllocator&) { return *this; }
	template<typename ... Args>
	void              construct(pointer p, Args&& ...args) { new ((T*)p) T(std::forward<Args>(args)...); }
	void              destroy(pointer p) { p->~T(); }
	size_type         max_size() const { return umm_maxsize(); }
#endif
	const char* tag() const { return type_tag; }
private:
	const char* type_tag;
};
//template <typename T>
//const char* UAllocatorTraits<T>::debug_name = "unkonwn stl";



template <typename T, typename U>
inline bool operator == (const UAllocator<T>&, const UAllocator<U>&) {
	return true;
}

template <typename T, typename U>
inline bool operator != (const UAllocator<T>&, const UAllocator<U>&) {
	return false;
}


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
#include <allocators>
#include <scoped_allocator>
template<typename T>
struct z_delete {
	void operator()(T *ptr) const noexcept { if (ptr) Z_Free(ptr); }
};


template<typename T>
using ZUniquePtr = std::unique_ptr<T, z_delete<T>>;


// some basic wrappers to use the doom memory system
using ZString = std::basic_string<char, std::char_traits<char>, UAllocator<char>>;

using ZStringStream = std::basic_stringstream<char, std::char_traits<char>, UAllocator<char>>;

using UString = std::basic_string<char, std::char_traits<char>, UAllocator<char>>;
template<typename T>
using UVector = std::vector<T, UAllocator<T>>;
template<typename T>
using UList = std::list<T, UAllocator<T>>;


template<typename K, typename V, typename H= std::hash<K>, typename E= std::equal_to<K>>
using UMap = std::unordered_map<K, V, H, E, UAllocator<std::pair<const K, V>>>;


//using StringList = std::vector<UString, UAllocator<UString>>;
using StringList = std::list<UString, UAllocator<UString>>;
using SStringList = std::vector<UString, std::scoped_allocator_adaptor<UString>>;

using SStream = std::basic_stringstream<char, std::char_traits<char>, UAllocator<char>>;

template<typename T>
using ZVector = std::vector<T, UAllocator<T>>;



#endif