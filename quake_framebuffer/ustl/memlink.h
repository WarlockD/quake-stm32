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
	template<typename TRAITS>
	struct link_interface : public clink_interface<TRAITS> {
		using traits = TRAITS;
		using base_t = clink_interface<TRAITS>;
		using value_type = typename traits::value_type;
		using pointer = typename traits::pointer;
		using const_pointer = typename traits::const_pointer;
		using reference = typename traits::reference;
		using const_reference = typename traits::const_reference;
		using size_type = typename traits::size_type;
		using difference_type = typename traits::difference_type;
		using const_iterator = typename traits::const_iterator;
		using iterator = typename traits::iterator;

		virtual pointer data()  = 0;
		const_pointer data() const { return base_t::data(); }
		size_type size() const { return base_t::size(); }
		const_iterator begin() const { return base_t::begin();}
		const_iterator end() const { return base_t::end(); }
		iterator begin()  { return iterator(data()); }
		iterator end()  { return iterator(data()) + size(); }

	protected:




		virtual ~link_interface() {}
	};




	template<typename _BASE, typename _INTERFACE>
	class memlink_t : public _INTERFACE { //clink_interface<_TRAITS> {
	public:
		using base_t = _BASE;
		using ibase_t = _INTERFACE;
		using traits = typename ibase_t::traits;
		using self_t = memlink_t<base_t, ibase_t>;
		using value_type = typename traits::value_type;
		using pointer = typename traits::pointer;
		using const_pointer = typename traits::const_pointer;
		using reference = typename traits::reference;
		using const_reference = typename traits::const_reference;
		using size_type = typename traits::size_type;
		using difference_type = typename traits::difference_type;
		using const_iterator = typename traits::const_iterator;
		using iterator = typename traits::iterator;
		//static_assert(has_size_function<BASE>::value, "Missing size()");
		//static_assert(has_cdata_function<BASE, value_type>::value, "Missing data()");

		// writable interfaces
		// interface 
		memlink_t() :base_t() {}
		memlink_t(void* p, size_type n) : base_t() { link(p, n); }
		memlink_t(const void* p, size_type n) : base_t() { link(p, n); }
		template<typename B, typename I>
		memlink_t(const cmemlink_t<B,I>& l) : base_t() { link(l); }


		inline reference at(size_type i) { assert(i < size());  return traits::cast(data(), i); }
		inline reference operator[](size_type i) { return traits::cast(data(), i); }		

		void link(void* p, size_type n) {
			if (!p && n) throw bad_alloc(n);
			if (p != data() || n != size()) // don't relink self
				relink(p, n);
		}
		template<typename BASE, typename I>
		inline void	link(memlink_t<BASE, I>& l) { link(l.begin(), l.size()); }
		inline void link(void* first, void*  last) { return  link(first, distance(first, last)); }
		// we can have a diffrent base, but the type and traits have to be the same
		template<typename BASE, typename I>
		void swap(memlink_t<BASE, I>& l) {
			if (std::addressof(l) != this) {
				auto p = data();
				auto s = size();
				link(l.data, l.size());
				l.link(p, s);
			}
		}
	protected:
		virtual void relink(void* p, size_type s) = 0;
		void fill(const_iterator cstart, const void* p, size_type elSize, size_type elCount = sizeof(value_type)) noexcept {
			assert(data() || !elCount || !elSize);
			assert(cstart >= begin() && cstart + elSize * elCount <= end());
			iterator start = const_cast<iterator>(cstart);
			if (elSize == 1)
				std::fill_n(start, elCount, *reinterpret_cast<const uint8_t*>(p));
			else {
				while (elCount--) {
					::memcpy(start, p, elSize);
					start += elSize;
				}
			}
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

	struct data_t : public memlink_t<data_t, link_interface<link_traits<char, true, false>>> {
		inline data_t() : _data(nullptr), _size(0U) { }
		inline data_t(void* p, size_type s) : _data(reinterpret_cast<pointer>(p)), _size(s) { }
		inline data_t(const void* p, size_type s) : data_t(const_cast<void*>(p),s) { }
		operator cdata_t() const { return cdata_t(data(), size()); }

		// interface
		inline const_pointer data() const override final { return _data; }
		inline pointer data()  override final { return _data; }
		inline size_type size() const override final { return _size; }

	protected:
		void relink(const void* p, size_type s) override final { _data = const_cast<pointer>(traits::cast(p)); _size = s; }
		void relink(void* p, size_type s) override final { _data = traits::cast(p); _size = s; }
		pointer _data;
		size_type _size;
	};

	class memlink : public data_t {
	public:
		using traits = link_traits<char, true, false>;
		using value_type = typename traits::value_type;
		using pointer = typename traits::pointer;
		using const_pointer = typename traits::const_pointer;
		using reference = typename traits::reference;
		using const_reference = typename traits::const_reference;
		using size_type = typename traits::size_type;
		using difference_type = typename traits::difference_type;
		using const_iterator = typename traits::const_iterator;
		using iterator = typename traits::iterator;
		typedef const memlink&		rcself_t;
		inline		memlink(void) : data_t() {}
		inline		memlink(void* p, size_type n) : data_t(p, n) {}
		inline		memlink(const void* p, size_type n) : base_t(p, n) {}
		template<typename LINK, typename TOP>
		inline		memlink(const cmemlink_t<LINK, TOP, char>& l) : memlink(l.data(), l.size()) {}

		inline iterator	iat(size_type i); //{ return begin() + i; }
		inline const_iterator iat(size_type i) const;// { return begin() + i; }

		size_type		writable_size(void) const { return size(); }
		inline memlink&	operator= (const cmemlink& l);// { link(l.data(), l.size()); return *this; }
		inline memlink&	operator= (rcself_t l) { base_t::operator= (l); return *this; }

		inline void relink(void* ptr, size_t n) { _data.relink(ptr, n); }
		inline void relink(const void* ptr, size_t n) { _data.relink(const_cast<void*>(ptr), n); }

		void		read(istream& is);

	};

	inline memlink::iterator	memlink::iat(size_type i) { return begin() + i; }
	inline memlink::const_iterator memlink::iat(size_type i) const { return begin() + i; }
	inline memlink&	memlink::operator= (const cmemlink& l) { link(l.data(), l.size()); return *this; }
	/// Use with memlink-derived classes to allocate and link to stack space.
#define alloca_link(m,n)	(m).link (alloca (n), (n))

}
