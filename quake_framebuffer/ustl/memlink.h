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
	class memlink : public cmemlink {
	public:
		typedef value_type*			pointer;
		typedef cmemlink::pointer		const_pointer;
		typedef cmemlink::const_iterator	const_iterator;
		typedef pointer			iterator;
		typedef const memlink&		rcself_t;
	public:
		inline		memlink(void) : cmemlink() {}
		inline		memlink(void* p, size_type n) : cmemlink(p, n) {}
		inline		memlink(const void* p, size_type n) : cmemlink(p, n) {}
		inline		memlink(rcself_t l) : cmemlink(l) {}
		inline explicit	memlink(const cmemlink& l) : cmemlink(l) {}
		inline pointer	data(void) { return const_cast<pointer>(cmemlink::data()); }
		inline const_pointer	data(void) const { return cmemlink::data(); }
		inline iterator	begin(void) { return iterator(data()); }
		inline iterator	iat(size_type i) { assert(i <= size()); return begin() + i; }
		inline iterator	end(void) { return iat(size()); }
		inline const_iterator	begin(void) const { return cmemlink::begin(); }
		inline const_iterator	end(void) const { return cmemlink::end(); }
		inline const_iterator	iat(size_type i) const { return cmemlink::iat(i); }
		size_type		writable_size(void) const { return size(); }
		inline rcself_t	operator= (const cmemlink& l) { cmemlink::operator= (l); return *this; }
		inline rcself_t	operator= (rcself_t l) { cmemlink::operator= (l); return *this; }
		inline void		link(const void* p, size_type n) { cmemlink::link(p, n); }
		inline void		link(void* p, size_type n) { cmemlink::link(p, n); }
		inline void		link(const cmemlink& l) { cmemlink::link(l); }
		inline void		link(memlink& l) { cmemlink::link(l); }
		inline void		link(const void* first, const void* last) { link(first, distance(first, last)); }
		inline void		link(void* first, void* last) { link(first, distance(first, last)); }
		inline void		relink(const void* p, size_type n) { cmemlink::relink(p, n); }
		inline void		relink(void* p, size_type n) { cmemlink::relink(p, n); }
		inline void		swap(memlink& l) { cmemlink::swap(l); }
		void			fill(const_iterator start, const void* p, size_type elsize, size_type elCount = 1) noexcept;
		inline void		insert(const_iterator start, size_type size);
		inline void		erase(const_iterator start, size_type size);
		void		read(istream& is);
	};

	
	/// Shifts the data in the linked block from \p start to \p start + \p n.
	/// The contents of the uncovered bytes is undefined.
	inline void memlink::insert(const_iterator cstart, size_type n)
	{
		assert(data() || !n);
		assert(cmemlink::begin() || !n);
		assert(cstart >= begin() && cstart + n <= end());
		iterator start = const_cast<iterator>(cstart);
		rotate(start, end() - n, end());
	}

	/// Shifts the data in the linked block from \p start + \p n to \p start.
	/// The contents of the uncovered bytes is undefined.
	inline void memlink::erase(const_iterator cstart, size_type n)
	{
		assert(data() || !n);
		assert(cmemlink::begin() || !n);
		assert(cstart >= begin() && cstart + n <= end());
		iterator start = const_cast<iterator>(cstart);
		rotate(start, start + n, end());
	}

	template<typename T = char, typename LINK = memlink>
	struct data_t : public  cdata_t<T, LINK> {
		using base_t = cdata_t<T,LINK>;
		using value_type = T;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using reference = value_type&;
		using const_reference = const value_type&;
		using iterator = pointer;
		using const_iterator = const_pointer;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;
		static constexpr size_type npos = size_t(-1);

		constexpr inline data_t() : base_t() { }
		constexpr inline data_t(const_pointer p, size_type s) : base_t(p,s) { }
		constexpr inline data_t(const LINK& l) : base_t(reinterpret_cast<const_pointer>( l.data()),l.size()) { }

		constexpr inline pointer data()  { return reinterpret_cast<pointer>(_data.data()); }

		constexpr reference at(size_t i)  { assert(i < _count); return data()[i]; }
		constexpr reference operator[](size_t i)  { assert(i < _count); return data()[i]; }

		constexpr iterator begin()  { return data(); }
		constexpr iterator end()  { return data() + size(); }
		constexpr reference front()  { return *data(); }
		constexpr reference back()  { return *(data() + size() - 1); }

		inline void link(void* p, size_type s) { link(const_cast<const void*>(p), s); }
		inline void	link(data_t& l) { link(l.data(), l.size()); }
		inline void link(void* first, void*  last) { return  link(first, distance(first, last)); }

		void fill(const_iterator cstart, const void* p, size_type elSize, size_type elCount = 1) noexcept {
			assert(data() || !elCount || !elSize);
			assert(cstart >= begin() && cstart + elSize * elCount <= end());
			iterator start = const_cast<iterator>(cstart);
			if (elSize == 1)
				fill_n(start, elCount, *reinterpret_cast<const_pointer>(p));
			else while (elCount--)
				start = copy_n(const_iterator(p), elSize, start);
		}
		void insert(const_iterator cstart, size_type n) {
			assert(data() || !n);
			assert(begin() || !n);
			assert(cstart >= begin() && cstart + n <= end());
			iterator start = const_cast<iterator>(cstart);
			std::rotate(start, end() - n, end());
		}
		void erase(const_iterator cstart, size_type n) {
			assert(data() || !n);
			assert(begin() || !n);
			assert(cstart >= begin() && cstart + n <= end());
			iterator start = const_cast<iterator>(cstart);
			std::rotate(start, start + n, end());
		}
	};

	/// Use with memlink-derived classes to allocate and link to stack space.
#define alloca_link(m,n)	(m).link (alloca (n), (n))

}
