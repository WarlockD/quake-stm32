// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#pragma once
#include "ualgobase.h"
#include "uttraits.h"
#include <exception>
#include <stdexcept>

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

	/// Example: HAS_MEMBER_FUNCTION(read, void (O::*)(istream&)); has_member_function_read<vector<int>>::value == true
	namespace priv {
		template <typename T>
		class _is_zero_terminated {
			template <typename O, const char*(O::*)() const> struct test_for_c_str {};
			template <typename O> static std::true_type found(test_for_c_str<O, &O::c_str>*);
			template <typename O> static std::false_type found(...);
		public:
			using type = decltype(found<T>(nullptr));
		};
		template <typename T>
		class _has_reserve_function {
			template <typename O, void(O::*)(size_t, bool)> struct test_for_reserve {};
			template <typename O> static std::true_type found(test_for_reserve<O, &O::reserve>*);
			template <typename O> static std::false_type found(...);
		public:
			using type = decltype(found<T>(nullptr));
		};
		template <typename T, typename TT>
		class _has_cdata_function {
			template <typename O, const TT*(O::*)() const> struct test_for_has_cdata {};
			template <typename O> static std::true_type found(test_for_has_cdata<O, &O::data>*);
			template <typename O> static std::false_type found(...);
		public:
			using type = decltype(found<T>(nullptr));
		};
		template <typename T,typename TT>
		class _has_data_function {
			template <typename O, TT*(O::*)()> struct test_for_has_data {};
			template <typename O> static std::true_type found(test_for_has_data<O, &O::data>*);
			template <typename O> static std::false_type found(...);
		public:
			using type = decltype(found<T>(nullptr));
		};
		template <typename T>
		class _has_size_function {
			template <typename O, size_t(O::*)() const> struct test_for_size {};
			template <typename O> static std::true_type found(test_for_size<O, &O::size>*);
			template <typename O> static std::false_type found(...);
		public:
			using type = decltype(found<T>(nullptr));
		};
		template <typename T>
		class _has_relink_function {
			template <typename O, size_t(O::*)(const void* ptr, size_t s) const> struct test_for_relink {};
			template <typename O> static std::true_type found(test_for_relink<O, &O::relink>*);
			template <typename O> static std::false_type found(...);
		public:
			using type = decltype(found<T>(nullptr));
		};
	}
	template <typename T>
	struct is_zero_terminated : public priv::_is_zero_terminated<T>::type {};
	template <typename T, typename TT = char>
	struct has_cdata_function : public priv::_has_cdata_function<T,TT>::type {};
	template <typename T, typename TT = char>
	struct has_data_function : public priv::_has_data_function<T,TT>::type {};
	template <typename T>
	struct has_size_function : public priv::_has_size_function<T>::type {};
	template <typename T,typename TT=char>
	struct has_cdata_size_function : public std::conditional_t<has_size_function<T>::value && has_cdata_function<T,TT>::value, std::true_type, std::false_type> {};

	template <typename T>
	struct has_reserve_function : public priv::_has_reserve_function<T>::type {};
	template <typename T>
	struct has_relink_function : public priv::_has_relink_function<T>::type {};

	template<typename T>
	struct type_size_trait { constexpr inline size_t operator()() const {  return sizeof(T); } };


	template<typename T, bool WRITEABLE, bool RESIZABLE, typename TYPE_SIZE = type_size_trait<std::decay_t<T>>>
	struct link_traits {
		static constexpr bool can_write = WRITEABLE;
		static constexpr bool can_read = true;
		static constexpr bool can_resize = RESIZABLE;
		using size_trait = TYPE_SIZE;
		using value_type = std::decay_t<T>;
		using pointer = std::conditional_t<can_write, value_type*, const value_type*>;
		using const_pointer = const value_type*;
		using reference = std::conditional_t<can_write, value_type&, const value_type&>;
		using const_reference = const value_type&;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using const_iterator = const_pointer;
		using iterator = pointer;
		static constexpr size_type npos = std::numeric_limits<size_type>::max();
		static constexpr size_type strlen(const char*p, size_type s = 0) { return *p == '\0' ? s : strlen(p + 1, s + 1); }
		static inline const_pointer cast(const void* ptr) { return reinterpret_cast<const_pointer>(ptr); }
		static inline pointer cast(void* ptr) { return reinterpret_cast<pointer>(ptr); }
		static inline const_pointer cast(const void* ptr, size_t count, size_t type_size) { return reinterpret_cast<const_pointer>(reinterpret_cast<const char*>(ptr) + (count * type_size)); }
		static inline pointer cast( void* ptr, size_t count, size_t type_size) { return reinterpret_cast<pointer>(reinterpret_cast<char*>(ptr) + (count * type_size)); }

		static inline pointer cast(void* ptr, size_t count) { return cast(ptr, count, size_trait()()); }
		static inline const_pointer cast(const void* ptr, size_t count) { return cast(ptr, count, size_trait()()); }

		static inline int compare(const void* left, size_type left_size, const void* right, size_type right_size) { 
			if (left != right && left_size != right_size) {
				const int rvbylen = sign(int(left_size - right_size));
				int rv = ::memcmp(left, right, std::min(left_size, right_size)*size_trait()());
				return rv ? rv : rvbylen;
			}
			else return 0;
		}
	};

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
	class cmemlink {
	public:
		typedef char		value_type;
		typedef const value_type*	pointer;
		typedef const value_type*	const_pointer;
		typedef value_type		reference;
		typedef value_type		const_reference;
		typedef size_t		size_type;
		typedef uint32_t		written_size_type;
		typedef ptrdiff_t		difference_type;
		typedef const_pointer	const_iterator;
		typedef const_iterator	iterator;
		typedef const cmemlink&	rcself_t;
	public:
		constexpr inline		cmemlink(void) : _data(nullptr), _size(0) { }
		constexpr inline		cmemlink(const void* p, size_type n) : _data(const_pointer(p)), _size(n) { assert(p || !n); }
		constexpr inline		cmemlink(const cmemlink& l) : _data(l._data), _size(l._size) {}
		inline virtual     ~cmemlink(void) noexcept {}
		void		link(const void* p, size_type n);
		inline void		link(const cmemlink& l) { link(l.begin(), l.size()); }
		inline void		link(const void* first, const void* last) { link(first, distance(first, last)); }
		inline void		relink(const void* p, size_type n);
		virtual void	unlink(void) noexcept { _data = nullptr; _size = 0; }
		inline rcself_t	operator= (const cmemlink& l) { link(l); return *this; }
		bool		operator== (const cmemlink& l) const noexcept;
		inline void		swap(cmemlink& l) { ::ustl::swap(_data, l._data); ::ustl::swap(_size, l._size); }
		constexpr inline size_type	size(void) const { return _size; }
		constexpr inline size_type	max_size(void) const { return size(); }
		inline size_type	readable_size(void) const { return size(); }
		constexpr inline bool		empty(void) const { return !size(); }
		constexpr inline const_pointer	data(void) const { return _data; }
		constexpr inline const_pointer	cdata(void) const { return _data; }
		inline iterator	begin(void) const { return iterator(cdata()); }
		inline iterator	iat(size_type i) const { assert(i <= size()); return begin() + i; }
		inline iterator	end(void) const { return iat(size()); }
		inline void		resize(size_type n) { _size = n; }
		inline void		read(istream&) { assert(!"ustl::cmemlink is a read-only object."); }
		void		write(ostream& os) const;
		size_type		stream_size(void) const noexcept;
		void		text_write(ostringstream& os) const;
		void		write_file(const char* filename, int mode = 0644) const;
	private:
		const_pointer	_data;		///< Pointer to the data block (const)
		size_type		_size;		///< size of the data block
	};

	//----------------------------------------------------------------------

	/// A fast alternative to link which can be used when relinking to the same block (i.e. when it is resized)
	inline void cmemlink::relink(const void* p, size_type n)
	{
		_data = reinterpret_cast<const_pointer>(p);
		_size = n;
	}

	//----------------------------------------------------------------------

	/// Use with cmemlink-derived classes to link to a static array
#define static_link(v)	link (VectorBlock(v))
	// fuck this shit, tryuing to make this too complcated with to many templates
	template<typename T,typename LINK= cmemlink>
	struct cdata_t {
		using value_type = T;
		using const_pointer = const value_type*;
		using pointer = const_pointer;
		using const_reference = const value_type&;
		using reference = const_reference;
		using const_iterator = const_pointer;
		using iterator = const_iterator;
		using reference = const_reference;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;
		static constexpr size_type npos = size_t(-1);
		constexpr inline cdata_t() : data() { }
		constexpr inline cdata_t(const_pointer p, size_type s) : _data(p, s * sizeof(T)), _count(s) { }
		constexpr inline cdata_t(const LINK& l) : _data(l.data(),l.size()), _count(l.size()/sizeof(T)) { }

		constexpr inline const_pointer data() const { return reinterpret_cast<const_pointer>(_data.data()); }
		constexpr inline size_type size() const { return _count; }
		constexpr inline bool		empty(void) const { return !size(); }

		constexpr const_reference at(size_t i) const { assert(i < _count); return data()[i]; }
		constexpr const_reference operator[](size_t i) const { assert(i < _count); return data()[i]; }

		constexpr const_iterator begin() const { return data(); }
		constexpr const_iterator end() const { return data() + size(); }
		constexpr const_reference front() const { return *data(); }
		constexpr const_reference back() const { return *(data() + size()-1); }

		inline void link(const void* p, size_type s) { _data.link(p, s); }
		inline void	link(const LINK& l) { _data.link(l); }
		inline void link(const void* first, const void*  last) { _data.link(first, last); }

		void swap(cdata_t& l) { _data.swap(l.sub_data); }

		inline cdata_t sub_data(size_type pos = 0, size_type count = npos) const {
			assert(pos < size());
			return cdata_t(data() + pos, min(count, size() - pos));
		}
		inline cdata_t remove_prefix(size_type n) const {
			assert(n < size());
			return cdata_t(data() + n, size() - n);
		}
		inline cdata_t remove_suffix(size_type n) const {
			assert(n < size());
			return cdata_t(data(), size() - n);
		}
		inline bool operator==(const cdata_t& r) const { return _data == r._data; }
		inline bool operator!=(const cdata_t& r) const { return !(*this == r); }
	protected:
		LINK _data;
		size_type _count;
	};


	//----------------------------------------------------------------------



	//----------------------------------------------------------------------

	/// Use with cmemlink-derived classes to link to a static array
#define static_link(v)	link (VectorBlock(v))

}
