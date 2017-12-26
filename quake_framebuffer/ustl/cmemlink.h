// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#pragma once
#include "ualgobase.h"

/// The ustl namespace contains all ustl classes and algorithms.
namespace ustl {

	class istream;
	class ostream;
	class ostringstream;

	/// \class cmemlink cmemlink.h ustl.h
	/// \ingroup MemoryManagement
	///
	/// \brief A read-only pointer to a sized block of memory.
	///
	/// Use this class the way you would a const pointer to an allocated unstructured block.
	/// The pointer and block size are available through member functions and cast operator.
	///
	/// Example usage:
	///
	/// \code
	///     void* p = malloc (46721);
	///     cmemlink a, b;
	///     a.link (p, 46721);
	///     assert (a.size() == 46721));
	///     b = a;
	///     assert (b.size() == 46721));
	///     assert (b.DataAt(34) == a.DataAt(34));
	///     assert (0 == memcmp (a, b, 12));
	/// \endcode
	///
	template<typename T, bool WRITEABLE, bool RESIZABLE>
	struct link_traits {
		static constexpr bool can_write = WRITEABLE;
		static constexpr bool can_read = true;
		static constexpr bool can_resize = RESIZABLE;
		using value_type = std::decay_t<T>;
		using pointer = std::conditional_t<can_write, value_type*,const value_type*>;
		using const_pointer = const value_type*;
		using reference = std::conditional_t<can_write, value_type&, const value_type&>;
		using const_reference = const value_type&;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using const_iterator = const_pointer;
		using iterator = pointer;
	};

	template<typename BASE, typename T=char>
	class cmemlink_t {
	public:
		using base_t = BASE;
		using traits = link_traits<T, false, false>;
		using pointer = typename traits::pointer;
		using const_pointer = typename traits::const_pointer;
		using reference = typename traits::reference;
		using const_reference = typename traits::const_reference;
		using size_type = typename traits::size_type;
		using difference_type = typename traits::difference_type;
		using const_iterator = typename traits::const_iterator;
		using iterator = typename traits::iterator;
		using rcself_t = const cmemlink_t<T>&;

		// interface 
		constexpr cmemlink_t() noexcept {}
		auto begin() const { return base_t::const_iterator(_data()); }
		auto end() const { return base_t::const_iterator(_data()) + _size(); }
		const_reference at(size_type i) const { assert(i < _size());  return _data()[i]; }
		inline const_reference operator[](size_type i) const { return at(i); }
		inline bool empty(void) const { return !_size(); }
		virtual ~cmemlink_t() noexcept  {}
		
		inline bool operator==(const cmemlink_t& r) {
			return _size() == r._size() && (_data() == r._data() || 0 == memcmp(_data(), r._data(), r._size()));
		}
		inline bool operator!=(const cmemlink_t& r) { return !(*this == r); }
		
		inline void	link(const cmemlink_t& l) { link(l.begin(), l.size()); }
		inline void	link(const void* first, const void*  last) { link(first, distance(first, last)); }
		inline void	unlink() { relink(nullptr, 0); }
		void link(const void* p, size_type n) {
			if (!p && n) throw bad_alloc(n);
			if (p == data() && n == size()) return; // don't relink self
			unlink();
			_relink(p, n);
		}
		template<typename BASE, typename T>
		void swap(cmemlink_t<BASE,T>& l) { 
			using in_link_t = cmemlink_t<BASE, T>;
			if (std::addressof(l) != this) {
				auto ptr = l._data();
				auto ssize = l._size();
				l._relink(_data(), _size());
				_relink(ptr, ssize);
			}
		}
	private:
		const_pointer _data() const { return static_cast<const base_t*>(this)->data(); }
		size_type _size() const { return static_cast<const base_t*>(this)->size(); }
		void _relink(const void*  p, size_type n) noexcept { static_cast<const BASE*>(this)->relink(p, n); }
	};



	class cmemlink : public cmemlink_t<cmemlink, char> {
	public:
		typedef const cmemlink&	rcself_t;
		using written_size_type = uint32_t;
		inline		cmemlink(void) : _data(nullptr), _size(0) { }
		inline		cmemlink(const void* p, size_type n) : _data(const_pointer(p)), _size(n) { assert(p || !n); }
		inline		cmemlink(const cmemlink& l) : _data(l._data), _size(l._size) {}
		inline virtual     ~cmemlink(void) noexcept {}
		inline rcself_t	operator= (const cmemlink& l) { link(l); return *this; }
		bool		operator== (const cmemlink& l) const noexcept;
		inline void		swap(cmemlink& l) { ::ustl::swap(_data, l._data); ::ustl::swap(_size, l._size); }
		

		size_type size() const  { return _size; }
		const_pointer data(void) const { return _data; }
		void relink(const void* ptr, size_t n) { _data = reinterpret_cast<const char*>(ptr); _size = n; }
		inline iterator	iat(size_type i) const { assert(i <= size()); return begin() + i; }


		inline size_type	max_size(void) const { return size(); }
		inline size_type	readable_size(void) const { return size(); }
		inline const_pointer	cdata(void) const { return _data; }

	
		inline void		resize(size_type n) { _size = n; }
		inline void		read(istream&) { assert(!"ustl::cmemlink is a read-only object."); }
		void		write(ostream& os) const;
		size_type		stream_size(void) const noexcept;
		void		text_write(ostringstream& os) const;
		void		write_file(const char* filename, int mode = 0644) const;
		/// A fast alternative to link which can be used when relinking to the same block (i.e. when it is resized)

	protected:
	
	private:
		const_pointer	_data;		///< Pointer to the data block (const)
		size_type		_size;		///< size of the data block
	};

	//----------------------------------------------------------------------



	//----------------------------------------------------------------------

	/// Use with cmemlink-derived classes to link to a static array
#define static_link(v)	link (VectorBlock(v))

}
