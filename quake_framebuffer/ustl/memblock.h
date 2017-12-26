// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#pragma once
#include "memlink.h"
#include "umemory.h"

namespace ustl {
	// ideas from c++17
	class memory_resource {
		// For exposition only
		static constexpr size_t max_align = alignof(max_align_t);
	public:
		virtual ~memory_resource() {}
		void* allocate(size_t bytes, size_t alignment = max_align) { return do_allocate(bytes, alignment); }
		void* reallocate(void* ptr, size_t bytes, size_t alignment = max_align) {
			return ptr == nullptr ? do_allocate(bytes, alignment) : do_reallocate(ptr, bytes, alignment);
		}
		void deallocate(void* p, size_t bytes, size_t alignment = max_align) { do_deallocate(p, bytes, alignment); }
		bool is_equal(const memory_resource& other) const noexcept { return do_is_equal(other); }
	protected:
		virtual void* do_allocate(size_t bytes, size_t alignment) = 0;
		virtual void* do_reallocate(void* p, size_t bytes, size_t alignment) = 0;
		virtual void do_deallocate(void* p, size_t bytes, size_t alignment) = 0;

		virtual bool do_is_equal(const memory_resource& other) const noexcept = 0;
	};
	inline bool operator==(const memory_resource& a, const memory_resource& b) noexcept { return &a == &b || a.is_equal(b); }
	inline bool operator!=(const memory_resource& a, const memory_resource& b) noexcept { return !(a == b); }
	memory_resource* get_default_memory_resource();

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
class memblock : public memlink {
public:
				memblock (memory_resource* r=nullptr) noexcept;
				memblock (const void* p, size_type n);
    explicit			memblock (size_type n, memory_resource* r = nullptr);
    explicit			memblock (const cmemlink& b, memory_resource* r = nullptr);
    explicit			memblock (const memlink& b, memory_resource* r = nullptr);
				memblock (const memblock& b, memory_resource* r = nullptr);
    virtual			~memblock (void) noexcept;
	template<typename B>
    inline void			assign (const cmemlink_t<B,char>& l)	{ assign (l.cdata(), l.readable_size()); }
	template<typename B>
    inline const memblock&	operator= (const cmemlink_t<B, char>& l)	{ assign (l); return *this; }
    inline void			swap (memblock& l) noexcept	{ memlink::swap (l); ::ustl::swap (_capacity, l._capacity); }
    void			assign (const void* p, size_type n);
    void			reserve (size_type newSize, bool bExact = false);
    void			resize (size_type newSize, bool bExact = true);
    iterator			insert (const_iterator start, size_type size);
    iterator			erase (const_iterator start, size_type size);
    inline void			clear (void) noexcept		{ resize (0); }
    inline size_type		capacity (void) const		{ return _capacity; }
    inline bool			is_linked (void) const		{ return !capacity(); }
    inline size_type		max_size (void) const		{ return is_linked() ? size() : SIZE_MAX; }
    inline void			manage (memlink& l)		{ manage (l.begin(), l.size()); }
    void			deallocate (void) noexcept;
    void			shrink_to_fit (void);
    void			manage (void* p, size_type n) noexcept;
    void			copy_link (void);
    void			read (istream& is);
    void			read_file (const char* filename);
#if HAVE_CPP11
    inline			memblock (memblock&& b)		: memlink(), _capacity(0) { swap (b); }
    inline memblock&		operator= (memblock&& b)	{ swap (b); return *this; }
#endif
	void set_memory_resource(memory_resource* r);
	memory_resource* get_memory_resorce() const { return _memory_resource; }
protected:
	virtual size_type		minimumFreeCapacity(void) const noexcept;
private:
    size_type			_capacity;	///< Number of bytes allocated by Resize.
	memory_resource* _memory_resource;
};

} // namespace ustl
