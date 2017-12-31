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




	// memlink that has a capasity fixed 
	class fixed_memblock : public memlink {
	public:
		typedef value_type*			pointer;
		typedef cmemlink::pointer		const_pointer;
		typedef cmemlink::const_iterator	const_iterator;
		typedef pointer			iterator;
		typedef const fixed_memblock&		rcself_t;
	protected:
		void* _buffer;
		size_type _capacity;
	public:
		void link(const void* p, size_type n) { memlink::link(p, n);  _capacity = 0U; }

		void manage(void* p, size_type n, size_type s) noexcept {
			assert(p || !n);
			assert(s <= n);
			assert(!_capacity && "Already managing something. deallocate or unlink first.");
			memlink::relink(p, s);
			_buffer = p;
			_capacity = n;
		}
		inline void manage(void* p, size_type n) noexcept { manage(p, n, n); }
		inline void	manage(memlink& l) { manage(l.begin(), l.size()); }
		inline size_type	capacity(void) const { return _capacity; }
		inline bool			is_linked(void) const { return  _buffer != data(); }
		inline size_type	max_size(void) const { return  is_linked() ? size() : _capacity; }

		inline		fixed_memblock(void) : memlink(), _capacity(0U), _buffer(nullptr) {}
		inline		fixed_memblock(void* p, size_type c, size_type n) { manage(p, c, n); }
		inline		fixed_memblock(void* p, size_type c) : fixed_memblock(p,c,c) { }
		inline explicit	fixed_memblock(const memlink& l) :fixed_memblock(const_cast<char*>(l.data()), l.size()) {}

		/// "Instantiate" a linked block by allocating and copying the linked data.
		void copy_link(void) { reserve(size()); }
		void reserve(size_type newSize, bool bExact = false) {
			assert(newSize <= _capacity);
			// we have to copy the data we gotlinked
			pointer oldBlock(is_linked() ? nullptr : data());
			if (is_linked()) {
				copy_n(data(), size(), reinterpret_cast<pointer>(_buffer));
				manage(_buffer, _capacity, size());
			}
		}
		void resize(size_type newSize, bool bExact = true) {
			if (_capacity < newSize)
				reserve(newSize, bExact);
			memlink::resize(newSize);
		}
		void clear() { memlink::resize(0U); }

		/// Copies data from \p p, \p n.
		void assign(const void* p, size_type n) {
			assert((p != (const void*)data() || size() == n) && "Self-assignment can not resize");
			resize(n);
			copy_n(const_pointer(p), n, begin());
		}
		inline void assign(const cmemlink& l) { assign(l.data(), l.size()); }
		inline const fixed_memblock&	operator= (const cmemlink& l) { assign(l); return *this; }
		inline void swap(fixed_memblock& l) noexcept {
			if (std::addressof(l) != this) {
				memlink::swap(l);
				std::swap(_capacity, l._capacity);
				std::swap(_buffer, l._buffer);
			}
		}
		iterator insert(const_iterator start, size_type n) {
			const uoff_t ip = start - begin();
			assert(ip <= size());
			resize(size() + n, false);
			memlink::insert(iat(ip), n);
			return iat(ip);
		}
		iterator erase(const_iterator start, size_type n) {
			const uoff_t ep = start - begin();
			assert(ep + n <= size());
			reserve(size());	// copy-on-write
			iterator iep = iat(ep);
			memlink::erase(iep, n);
			resize(size() - n);
			return iep;
		}
	};
	template<size_t N>
	class static_memblock : public  fixed_memblock {
	public:
		typedef value_type*			pointer;
		typedef cmemlink::pointer		const_pointer;
		typedef cmemlink::const_iterator	const_iterator;
		typedef pointer			iterator;
		typedef const static_memblock&		rcself_t;
	protected:
		char _buffer[N];
		using fixed_memblock::manage; /// you can't remanage this cause we control it
	public:
		inline	static_memblock(void) : fixed_memlink(_buffer, N, N) {}
		inline void swap(static_memblock& l) noexcept {
			if (std::addressof(l) != this) {
				memlink::swap(l); // basicly we relink both obhects, otherwise we have to do a full copy swap here
			}
		}
		inline const static_memblock&	operator= (const cmemlink& l) { assign(l); return *this; }
	};

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
		using base_t = memlink;
		using self_t = memblock;
		using value_type = typename base_t::value_type;
		using pointer = typename base_t::pointer;
		using const_pointer = typename base_t::const_pointer;
		using reference = typename base_t::reference;
		using const_reference = typename base_t::const_reference;
		using size_type = typename base_t::size_type;
		using difference_type = typename base_t::difference_type;
		using const_iterator = typename base_t::const_iterator;
		using iterator = typename base_t::iterator;


		static constexpr bool can_resize = true;   // we can resize data	
		static constexpr bool can_reserve = true; // we can reserve space
		memblock(memory_resource* r = nullptr) noexcept;
		memblock(const void* p, size_type n);
		explicit			memblock(size_type n, memory_resource* r = nullptr);
		explicit			memblock(const cmemlink& b, memory_resource* r = nullptr);
		explicit			memblock(const memlink& b, memory_resource* r = nullptr);
		memblock(const memblock& b, memory_resource* r = nullptr);
		virtual			~memblock(void) noexcept;

		void			assign(const void* p, size_type n);
		inline void			assign(const cmemlink& l) { assign(l.data(), l.size()); }
		inline const memblock&	operator= (const cmemlink& l) { assign(l); return *this; }
		inline void			swap(memblock& l) noexcept {
			if (std::addressof(l) != this) {
				memlink::swap(l);
				std::swap(_capacity, l._capacity);
				std::swap(_memory_resource, l._memory_resource);
			}
		}
		
		void			reserve(size_type newSize, bool bExact = false);
		void			resize(size_type newSize, bool bExact = true);
		iterator			insert(const_iterator start, size_type size);
		iterator			erase(const_iterator start, size_type size);
		inline void			clear(void) noexcept { resize(0); }
		inline size_type		capacity(void) const { return _capacity; }
		inline bool			is_linked(void) const { return !capacity(); }
		inline size_type		max_size(void) const { return is_linked() ? size() : SIZE_MAX; }
		inline void			manage(memlink& l) { manage(l.begin(), l.size()); }
		void			deallocate(void) noexcept;
		void			shrink_to_fit(void);
		void			manage(void* p, size_type n) noexcept;
		void			copy_link(void);
		void			read(istream& is);
		void			read_file(const char* filename);
#if HAVE_CPP11
		inline			memblock(memblock&& b) : memlink(), _capacity(0) { swap(b); }
		inline memblock&		operator= (memblock&& b) { swap(b); return *this; }
#endif
		void set_memory_resource(memory_resource* r);
		memory_resource* get_memory_resorce() const { return _memory_resource; }
	protected:
		virtual size_type		minimumFreeCapacity(void) const noexcept;
	private:
		size_type		  _capacity;	///< Number of bytes allocated by Resize.
		memory_resource* _memory_resource;
	};

}
