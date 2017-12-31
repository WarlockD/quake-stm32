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

void *Cache_Alloc(cache_user_t *c, int size, const quake::string_view&  name);
// Returns NULL if all purgable data was tossed and there still
// wasn't enough room.

void Cache_Report(void);




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
template<typename T,typename ALLOC = std::allocator<char>>
class custom_size_allocator {
public:
	typedef T value_type;
	typedef std::true_type propagate_on_container_copy_assignment;
	typedef std::true_type propagate_on_container_move_assignment;
	typedef std::true_type propagate_on_container_swap;
	template<typename U,typename A>
	struct rebind {
		typedef custom_size_allocator<U,A> other;
	};
	typedef T * pointer;
	typedef const T * const_pointer;
	typedef T & reference;
	typedef const T & const_reference;

	custom_size_allocator() : _alloc(), _type_size(sizeof(T)) {}
	custom_size_allocator(size_t size) : _alloc(), _type_size(size) { assert(size >= sizeof(T)); }
	custom_size_allocator(ALLOC a, size_t size) : _alloc(a), _type_size(size) { assert(size >= sizeof(T)); }

	template<typename U,typename A>
	custom_size_allocator(const custom_size_allocator<U,A> &copy) : _alloc(copy._alloc), _type_size(copy._type_size) {}
	custom_size_allocator(const custom_size_allocator &copy) : _alloc(copy._alloc), _type_size(copy._type_size) {}
	custom_size_allocator(custom_size_allocator && move) : _alloc(std::move(move._alloc)), _type_size(move._type_size) {}
	custom_size_allocator & operator=(const custom_size_allocator &) { _alloc = copy._alloc; _type_size = copy._type_size; return *this; }
	custom_size_allocator & operator=(custom_size_allocator && move) { _alloc = std::move(move._alloc); _type_size = std::move(move._type_size); return *this; }


	bool operator==(const custom_size_allocator & other) const { return this == &other; }
	bool operator!=(const custom_size_allocator & other) const { return !(*this == other); }
	T * allocate(size_t count)
	{
		char* ptr = _alloc.allocate(_type_size* count);
		assert(ptr);
		return reinterpret_cast<T*>(ptr);
	}
	void deallocate(T * ptr, size_t count)
	{
		char* ptr = reinterpret_cast<char*>(ptr);
		_alloc.deallocate(ptr, count * _type_size);
	}
private:
	ALLOC _alloc;
	size_t _type_size;
};
namespace umm {
	class umm_allocator {
	public:
		using bindex_t = unsigned short int;
		constexpr static size_t max_align_t = alignof(std::max_align_t);
	private:
		struct umm_block_t* _heap;
		size_t _numblocks;
		bindex_t umm_blocks(size_t size);
		void umm_make_new_block(bindex_t c, bindex_t blocks, bindex_t freemask);
		void umm_disconnect_from_free_list(bindex_t c);
		void umm_assimilate_up(bindex_t c);
		bindex_t umm_assimilate_down(bindex_t c, bindex_t freemask);
		bool umm_pointer_in_heap(void* p) const noexcept;
	public:
		umm_allocator() : _heap(nullptr), _numblocks(0U) {}
		umm_allocator(void* heap, size_t heap_size);
		size_t numblocks() const;
		size_t maxsize() const;
		// ----------------------------------------------------------------------------

		void* allocate(std::size_t n);
		void* allocate(std::size_t n, void* ptr); // basicly this is realloc
		void deallocate(void * ptr);
		bool operator==(const umm_allocator& a) const { return _heap == a._heap; }
		bool operator!=(const umm_allocator& a) const { return _heap != a._heap; }
	};
}

template <class T>
class umm_allocator {
	using value_type = T;
	template <class _Up> struct rebind { using other = umm_allocator<_Up>; };

	umm_allocator(const umm_allocator&) = default;
	umm_allocator& operator=(const umm_allocator&) = delete;

	umm_allocator(const umm::umm_allocator& a) noexcept : _a(a) {}
	template <class U>
	umm_allocator(const umm_allocator<U>& a) noexcept : _a(a._a) {}
	umm_allocator(const umm::umm_allocator& a) : _a(a) {}

	T* allocate(std::size_t n) { return reinterpret_cast<T*>(_a.allocate(n*sizeof(T)); }
	void deallocate(T* p, std::size_t n) noexcept { _a.deallocate(reinterpret_cast<char*>(p)); } //, n * sizeof(T));


	template <class U> friend class umm_allocator;

	template<typename U>
	bool operator==(const umm_allocator<U>& other) const { return &_a == &other._a; }
	template<typename U>
	bool operator!=(const umm_allocator<U>& other) const { return &_a != &other._a; }
private:
	umm::umm_allocator  _a;
};

template <class T,size_t _SIZE>
class umm_stack_allocator : public umm_allocator<T> {
	using value_type = T;
	template <class _Up> struct rebind { using other = umm_allocator<_Up>; };

	umm_stack_allocator(const umm_stack_allocator&) = default;
	umm_stack_allocator& operator=(const umm_stack_allocator&) = delete;

	umm_stack_allocator() noexcept : umm_allocator<T>(umm::umm_allocator(_data.data(), _data.size()) {}


	template <class U, size_t S> friend class umm_stack_allocator<U,S>;

	template<typename U,size_t S>
	bool operator==(const umm_stack_allocator<U,S>& other) const { return &_data == &other._data; }
	template<typename U, size_t S>
	bool operator!=(const umm_stack_allocator<U,S>& other) const { return &_data != &other._data; }
private:
	std::array<char, _SIZE> _data;
};

template <class T, size_t SIZE>
class umm_stack_allocator
{
public:
	using value_type = T;
	template <class _Up,size_t SIZE> struct rebind { using other = umm_stack_allocator<_Up,SIZE>; };

	short_alloc(const short_alloc&) = default;
	short_alloc& operator=(const short_alloc&) = delete;

	short_alloc() noexcept : _a(a)
	{
		static_assert(size % alignment == 0,
			"size N needs to be a multiple of alignment Align");
	}
	template <class U>
	short_alloc(const short_alloc<U>& a) noexcept : _a(a._a) {}


	T* allocate(std::size_t n) { return reinterpret_cast<T*>(_a.allocate<alignof(T)>(n * sizeof(T))); }
	void deallocate(T* p, std::size_t n) noexcept { _a.deallocate(reinterpret_cast<char*>(p), n * sizeof(T)); }

	template <class U> friend class short_alloc;

	template<typename U>
	bool operator==(const short_alloc<U>& other) const { return &_a == &other._a; }
	template<typename U>
	bool operator!=(const short_alloc<U>& other) const { return &_a != &other._a; }

private:
	std::array<char, size> _buffer;
	umm_allocator _umm;
};

namespace quake{


}


class memory_area
{
	constexpr static size_t alignment = alignof(std::max_align_t);
	char* _memory;
	size_t _capasity;
	char** _ptr; // stored in the first part of _memory
public:
	memory_area() noexcept : _memory(nullptr), _capasity(0U), _ptr(nullptr) {}
	// we store _ptr inside memory so that we are allowed to copy and move
	memory_area(char* memory, size_t capasity) noexcept : _memory(memory),_capasity(capasity- alignment), _ptr(&memory) { *_ptr = memory + alignment; }
	memory_area(const memory_area& copy) : _memory(copy._memory),  _capasity(copy._capasity),  _ptr(copy._ptr) {}
	memory_area& operator=(const memory_area&copy) { _memory = copy._memory; _capasity = copy._capasity; _ptr=copy._ptr; return *this; } 
	memory_area(memory_area&& move) : _memory(move._memory), _capasity(move._capasity), _ptr(move._ptr) { move._memory = nullptr; move._capasity = 0U; }
	memory_area& operator=(memory_area&& move) { _memory = move._memory; _capasity = move._capasity; _ptr = move._ptr; move._memory = nullptr; move._capasity = 0U;  return *this; }
	~memory_area() {}

	template <std::size_t ReqAlign>
	char* allocate(std::size_t n) {
		static_assert(ReqAlign <= alignment, "alignment is too small for this arena");
		assert(pointer_in_buffer(*_ptr) && "short_alloc has outlived arena");
		auto const aligned_n = align_up(n);
		if (static_cast<decltype(aligned_n)>(_memory + _capasity - *_ptr) >= aligned_n)
		{
			char* r = *_ptr;
			*_ptr += aligned_n;
			return r;
		}
		assert(0); // ugh
		static_assert(alignment <= alignof(std::max_align_t), "you've chosen an "
			"alignment that is larger than alignof(std::max_align_t), and "
			"cannot be guaranteed by normal operator new");
		return static_cast<char*>(::operator new(n));
	}
	void deallocate(char* p, std::size_t n) noexcept {
		assert(pointer_in_buffer(*_ptr) && "short_alloc has outlived arena");
		if (pointer_in_buffer(p))
		{
			n = align_up(n);
			if (p + n == *_ptr)
				*_ptr = p;
		}
		else
			::operator delete(p);
	}
	std::size_t used() const noexcept { return static_cast<std::size_t>(*_ptr - _memory); }
	void reset() noexcept { *_ptr = _memory; }

private:
	static size_t align_up(std::size_t n) noexcept { return (n + (alignment - 1)) & ~(alignment - 1); }

	bool pointer_in_buffer(char* p) noexcept { return _memory <= p && p <= _memory + _capasity; }
};
template<size_t stack_size>
class stack_area : public memory_area {
	char _buffer[stack_size+ memory_area::alignment];
public:
	stack_area() :memory_area(_buffer, stack_size) {}

};

template <class T>
class short_alloc
{
public:
	using value_type = T;
	template <class _Up> struct rebind { using other = short_alloc<_Up>; };

	short_alloc(const short_alloc&) = default;
	short_alloc& operator=(const short_alloc&) = delete;

	short_alloc(memory_area& a) noexcept : _a(a)
	{
		static_assert(size % alignment == 0,
			"size N needs to be a multiple of alignment Align");
	}
	template <class U>
	short_alloc(const short_alloc<U>& a) noexcept : _a(a._a) {}


	T* allocate(std::size_t n) { return reinterpret_cast<T*>(_a.allocate<alignof(T)>(n * sizeof(T))); }
	void deallocate(T* p, std::size_t n) noexcept { _a.deallocate(reinterpret_cast<char*>(p), n * sizeof(T)); }

	template <class U> friend class short_alloc;

	template<typename U>
	bool operator==(const short_alloc<U>& other) const { return &_a == &other._a; }
	template<typename U>
	bool operator!=(const short_alloc<U>& other) const { return &_a != &other._a; }

private:
	memory_area& _a;
};



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

using StringArgs = ZVector<quake::string_view>;


#endif