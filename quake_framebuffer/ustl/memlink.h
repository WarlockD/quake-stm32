// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#pragma once
#include "cmemlink.h"
#include "ualgo.h"

namespace ustl {

/// \class memlink memlink.h ustl.h
/// \ingroup MemoryManagement
///
/// \brief Wrapper for pointer to block with size.
///
/// Use this class the way you would a pointer to an allocated unstructured block.
/// The pointer and block size are available through member functions and cast operator.
///
/// Example usage:
/// \code
///     void* p = malloc (46721);
///     memlink a, b;
///     a.link (p, 46721);
///     assert (a.size() == 46721));
///     b = a;
///     assert (b.size() == 46721));
///     assert (b.begin() + 34 == a.begin + 34);
///     assert (0 == memcmp (a, b, 12));
///     a.fill (673, b, 42, 67);
///     b.erase (87, 12);
/// \endcode
///
	template<typename BASE, typename T = char>
	class memlink_t : public cmemlink_t<BASE,T> {


	public:
		using base_t = BASE;
		using traits = link_traits<T, true, false>;
		using pointer = typename traits::pointer;
		using const_pointer = typename traits::const_pointer;
		using reference = typename traits::reference;
		using const_reference = typename traits::const_reference;
		using size_type = typename traits::size_type;
		using difference_type = typename traits::difference_type;
		using const_iterator = typename traits::const_iterator;
		using iterator = typename traits::iterator;
		// writable interfaces
		inline iterator	begin(void) { return iterator(_data()); }
		inline iterator	end(void) { return iterator(_data()) + _size(); }
		reference at(size_type i)  { assert(i < size());  return data()[i]; }
		inline reference operator[](size_type i)  { return at(i); }

		void fill(const_iterator start, const void* p, size_type elsize, size_type elCount = 1) noexcept {
			assert(data() || !elCount || !elSize);
			assert(cstart >= begin() && cstart + elSize * elCount <= end());
			iterator start = const_cast<iterator>(cstart);
			if (elSize == 1)
				std::fill_n(start, elCount, *reinterpret_cast<const uint8_t*>(p));
			else while (elCount--)
				start = std::copy_n(const_iterator(p), elSize, start);
		}
		inline void	insert(const_iterator start, size_type size) {
			assert(data() || !n);
			assert(base_t::begin() || !n);
			assert(cstart >= begin() && cstart + n <= end());
			iterator start = const_cast<iterator>(cstart);
			std::rotate(start, end() - n, end());
		}
		inline void	erase(const_iterator start, size_type size) {
			assert(data() || !n);
			assert(base_t::begin() || !n);
			assert(cstart >= begin() && cstart + n <= end());
			iterator start = const_cast<iterator>(cstart);
			std::rotate(start, start + n, end());
		}
	private:
		pointer _data() { return static_cast<base_t*>(this)->data(); }
		const_pointer _data() const { return static_cast<const base_t*>(this)->data(); }
		size_type _size() const { return static_cast<const base_t*>(this)->size(); }
	};

class memlink : public memlink_t<memlink,char> {

public:
	using base_t = memlink_t<memlink,char>;
	using cbase_t = cmemlink_t<memlink, char>;
	using traits = link_traits<char, true, false>;
	using pointer = typename traits::pointer;
	using const_pointer = typename traits::const_pointer;
	using reference = typename traits::reference;
	using const_reference = typename traits::const_reference;
	using size_type = typename traits::size_type;
	using difference_type = typename traits::difference_type;
	using const_iterator = typename traits::const_iterator;
	using iterator = typename traits::iterator;
    typedef const memlink&		rcself_t;
public:
    inline		memlink (void)				: _data(nullptr), _size(0U) {}
    inline		memlink (void* p, size_type n)		: _data(reinterpret_cast<pointer>(p)), _size(n) {}
    inline		memlink (const void* p, size_type n) : memlink(const_cast<void*>(p), n) {}
    inline		memlink (rcself_t l)			: base_t(l) {}
    inline explicit	memlink (const cmemlink& l)		: base_t() { link(l.data(), l.size()); }
	inline const_pointer data() const { return _data; }
	inline pointer data()  { return _data; }
	//inline const_iterator begin() const { return base_t::base_t::begin(); }
//	inline const_iterator end() const { return base_t::base_t::end(); }
	inline size_type size() const { return _size; }
	inline iterator	iat(size_type i)  { return begin() + i; }
	inline const_iterator	iat(size_type i) const { return cbase_t::begin() + i; }

    size_type		writable_size (void) const		{ return size(); }
	inline rcself_t	operator= (const cmemlink& l) { link(l.data(), l.size()); return *this; }
    inline rcself_t	operator= (rcself_t l)			{ base_t::operator= (l); return *this; }
    inline void		link (const void* p, size_type n)	{ base_t::link (p, n); }
    inline void		link (void* p, size_type n)		{ base_t::link (p, n); }
    inline void		link (const cmemlink& l)		{ base_t::link (l.data(),l.size()); }
    inline void		link (memlink& l)			{ base_t::link (l); }
    inline void		link (const void* first, const void* last)	{ link (first, distance (first, last)); }
    inline void		link (void* first, void* last)		{ link (first, distance (first, last)); }
	inline void relink(void* ptr, size_t n) { _data = reinterpret_cast<char*>(ptr); _size = n; }
	inline void relink(const void* ptr, size_t n) { relink(const_cast<void*>(ptr), n); }
  
   
    void		read (istream& is);
private:
	pointer _data;
	size_t _size;
};


/// Use with memlink-derived classes to allocate and link to stack space.
#define alloca_link(m,n)	(m).link (alloca (n), (n))

} // namespace ustl
