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
	template<typename _TRAITS>
	struct clink_interface {
		using traits = _TRAITS;
		using self_t = clink_interface<_TRAITS>;
		using value_type = typename traits::value_type;
		using pointer = typename traits::pointer;
		using const_pointer = typename traits::const_pointer;
		using reference = typename traits::reference;
		using const_reference = typename traits::const_reference;
		using size_type = typename traits::size_type;
		using difference_type = typename traits::difference_type;
		using const_iterator = typename traits::const_iterator;
		using iterator = typename traits::iterator;
		// interface
		virtual const_pointer data() const = 0;
		virtual size_type size() const = 0;
		//helpers
		const_iterator begin() const { return const_iterator(data()); }
		const_iterator end() const { return const_iterator(data()) + size(); }
		
		inline bool empty(void) const { return!size(); }

		template<typename TRAITS>
		inline bool operator==(const clink_interface<TRAITS>& r) const {
			return size() == r.size() && (data() == r.data() || ::memcmp(data(), r.data(), size()) == 0);
		}
		template<typename TRAITS>
		inline bool operator!=(const clink_interface<TRAITS>& r) const { return !(*this == r); }

		virtual ~clink_interface() {}
	};
	// fuck this shit, tryuing to make this too complcated with to many templates
	struct cdata_t : public cmemlink_t<cdata_t, clink_interface<link_traits<char, false, false>>> {
		using base_t = cmemlink_t<cdata_t, clink_interface<link_traits<char, false, false>>>;
		using traits = base_t::traits;
		using value_type = typename traits::value_type;
		using pointer = typename traits::pointer;
		using const_pointer = typename traits::const_pointer;
		using reference = typename traits::reference;
		using const_reference = typename traits::const_reference;
		using size_type = typename traits::size_type;
		using difference_type = typename traits::difference_type;
		using const_iterator = typename traits::const_iterator;
		using iterator = typename traits::iterator;

		constexpr inline cdata_t() : _data(nullptr), _size(0U) { }
		constexpr inline cdata_t(const void* p, size_type s) : _data(reinterpret_cast<const char*>(p)), _size(s) { }
		template<typename B, typename I>
		constexpr inline cdata_t(const cmemlink_t<B, I>& l) : _data(l.data()), _size(l.size()) { }

		// interface
		inline const_pointer data() const override final { return _data; }
		inline size_type size() const override final { return _size; }
	protected:
		void relink(const void* p, size_type s) override final { _data = traits::cast(p); _size = s; }
		const_pointer _data;
		size_type _size;
	};

	template<typename _BASE, typename _INTERFACE>
	struct cmemlink_t : public _INTERFACE { //clink_interface<_TRAITS> {
		using base_t = _BASE;
		using ibase_t = _INTERFACE;
		using traits = typename ibase_t::traits;
		using self_t = cmemlink_t<base_t, ibase_t>;
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

		void unlink() { relink(nullptr, 0U);  }
		void link(const void* p, size_type n) {
			if (!p && n) throw bad_alloc(n);
			if (p != data() || n != size()) // don't relink self
				relink(p, n);
		}
		template<typename BASE,typename I>
		inline void	link(cmemlink_t<BASE,I>& l) { link(l.begin(), l.size()); }
		inline void link(const void* first, const void*  last) { return  link(first, distance(first, last)); }
		// we can have a diffrent base, but the type and traits have to be the same
		template<typename BASE, typename I>
		void swap(cmemlink_t<BASE,I>& l) {
			if (std::addressof(l) != this) {
				auto p = data();
				auto s = size();
				link(l.data, l.size());
				l.link(p, s);
			}
		}
	protected:
		inline base_t sub_data(size_type pos = 0, size_type count = traits::npos) const {
			if (pos > size()) throw std::out_of_range("size_terminated_data::sub_data");
			base_t b;
			b.link(data() + pos, min(count, size() - pos));
			return std::move(b);
		}
		inline base_t remove_prefix(size_type n, std::false_type) const {
			if (n > size()) throw std::out_of_range("size_terminated_data::sub_data");
			base_t b;
			b.link(data() + n, size() - n);
			return std::move(b);
		}
		inline base_t remove_suffix(size_type n, std::false_type) const {
			if (n > size()) throw std::out_of_range("size_terminated_data::sub_data");
			base_t b;
			b.link(data(), size() - n);
			return std::move(b);
		}
	
		// we need this
		virtual void relink(const void* p, size_type s) = 0;
	private:
		base_t& base() { return *static_cast<base_t*>(this); }
		const base_t& base() const { return *static_cast<const base_t*>(this); }

	
	};



	class cmemlink : public cdata_t {
	public:
		using base_t = cmemlink_t<cdata_t, cmemlink>;
		using written_size_type = uint32_t;

		inline cmemlink(void) : cdata_t() { }
		inline cmemlink(const void* p, size_type n) : cdata_t(p,n) { }
		inline cmemlink(const cdata_t& l) : cdata_t(l){  }

		inline iterator	iat(size_type i) const { assert(i <= size()); return begin() + i; }
		inline size_type	max_size(void) const { return size(); }
		inline size_type	readable_size(void) const { return size(); }

		inline void		resize(size_type n) { relink(data(), n); }
		inline void		read(istream&) { assert(!"ustl::cmemlink is a read-only object."); }
		void		write(ostream& os) const;
		size_type		stream_size(void) const noexcept;
		void		text_write(ostringstream& os) const;
		void		write_file(const char* filename, int mode = 0644) const;
		/// A fast alternative to link which can be used when relinking to the same block (i.e. when it is resized)
		size_type capacity() const { return size(); }
	};

	//----------------------------------------------------------------------



	//----------------------------------------------------------------------

	/// Use with cmemlink-derived classes to link to a static array
#define static_link(v)	link (VectorBlock(v))

}
