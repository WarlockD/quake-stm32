// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#pragma once
#include "memblock.h"
#include "utf8.h"



namespace ustl {
	// templated helper interface that lets us handle the basic things strings do
	// switched alot of the string functions to static globals
	namespace util {
		using pos_type = size_t;
		using size_type = size_t;
		using hashvalue_t = uint32_t;
		static constexpr pos_type npos = pos_type(-1);
		constexpr static inline size_t strlen(const char* s, size_t sz = 0) { return *s == '\0' ? sz : strlen(s + 1, sz + 1); }
		hashvalue_t str_hash(const char* first, const char* last) noexcept; // static
		int str_compare(const char* first1, const char* last1, const char* first2, const char* last2) noexcept;
		pos_type str_find(const cmemlink&  hay, char c, pos_type pos = 0U)  noexcept;
		pos_type str_find(const cmemlink&  hay, const cmemlink&  needle, pos_type pos = 0U) noexcept;
		inline pos_type	str_find(const cmemlink&  hay, unsigned char c, pos_type pos = 0)  noexcept { return str_find(hay, char(c), pos); }
		inline pos_type	str_find(const cmemlink&  hay, const char* p, pos_type pos) { return str_find(hay, cmemlink(p, ::strlen(p)), pos); }
		inline pos_type	str_find(const cmemlink&  hay, const char* p, pos_type pos, size_type count) { return str_find(hay, cmemlink(p, count), pos); }


		pos_type str_rfind(const cmemlink&  hay, char c, pos_type pos = npos)  noexcept;
		pos_type str_rfind(const cmemlink&  hay, const cmemlink& s, pos_type pos = npos)  noexcept;
		inline pos_type	str_rfind(const cmemlink&  hay, const char* p, pos_type pos) { return str_rfind(hay, cmemlink(p, strlen(p)), pos); }
		inline pos_type	str_rfind(const cmemlink&  hay, const char* p, pos_type pos, size_type count) { return str_rfind(hay, cmemlink(p, count), pos); }

		pos_type str_find_first_of(const cmemlink&  hay, const cmemlink& s, pos_type pos = 0U)  noexcept;
		pos_type str_find_last_of(const cmemlink&  hay, const cmemlink& s, pos_type pos = npos)  noexcept;
		pos_type str_find_last_not_of(const cmemlink&  hay, const cmemlink& s, pos_type pos = npos)  noexcept;
		pos_type str_find_first_not_of(const cmemlink&  hay, const cmemlink& s, pos_type pos = 0U)  noexcept;


	
	}
	template<bool CONST>
	struct string_traits {
		using difference_type = std::ptrdiff_t;
		using size_type = size_t;
		using pos_type = size_type;
		static constexpr pos_type npos = pos_type(-1);

		using value_type = char;
		using uvalue_type = unsigned char;

		using const_pointer = const value_type*;
		using const_reference = const value_type&;
		using pointer = typename std::conditional_t<CONST, const_pointer, value_type*>;
		using reference = typename std::conditional_t<CONST, const_reference, value_type&>;
		using iterator = pointer;
		using const_iterator = const_pointer;
		using reverse_iterator = ::ustl::reverse_iterator<iterator>;
		using const_reverse_iterator = ::ustl::reverse_iterator<const_iterator>;

		using wvalue_type = wchar_t;
		using const_wpointer = const wvalue_type*;
		using const_wreference = const wvalue_type&;
		using wpointer = typename std::conditional_t<CONST, const_wpointer, wvalue_type*>;
		using wreference = typename std::conditional_t<CONST, const_wreference, wvalue_type&>;
		using utf8_iterator = utf8in_iterator<const_iterator>;
	};
	// string helper gives a class bare bones stuff so we can do things like
	// string search and make a simple string view class
	struct string_helper_empty_base {
		constexpr string_helper_empty_base() {}
	};
	struct string_helper_interface_base {
		using traits_t = string_traits<true>;
		using difference_type = typename traits_t::difference_type;
		using size_type = typename traits_t::size_type;
		using pos_type = typename traits_t::pos_type;
		static constexpr pos_type npos = traits_t::npos;
		using value_type = typename traits_t::value_type;
		using uvalue_type = typename traits_t::uvalue_type;
		using const_pointer = typename traits_t::const_pointer;
		using const_reference = typename traits_t::const_reference;
		using pointer = typename traits_t::pointer;
		using reference = typename traits_t::reference;
		using iterator = typename traits_t::iterator;
		using const_iterator = typename traits_t::const_iterator;
		using reverse_iterator = typename traits_t::reverse_iterator;
		using const_reverse_iterator = typename traits_t::const_reverse_iterator;

		using wvalue_type = typename traits_t::wvalue_type;
		using const_wpointer = typename traits_t::const_wpointer;
		using const_wreference = typename traits_t::const_wreference;
		using wpointer = typename traits_t::wpointer;
		using wreference = typename traits_t::wreference;
		using utf8_iterator = typename traits_t::utf8_iterator;
		virtual const char* data() const = 0;
		virtual size_t size() const = 0;
		virtual ~string_helper_interface_base() {}
	};

	template<typename _CONTAINER> class string_builder;

	template<typename _CONTAINER>
	class string_helper : public _CONTAINER {
		friend string_builder<_CONTAINER>;
		//using _CONTAINER::data();
		//using _CONTAINER::size();
	public:
		//container_t _container;
		using top_t = string_helper<_CONTAINER>;
		using container_t = _CONTAINER;
		using traits_t = string_traits<true>;
		using difference_type = typename traits_t::difference_type;
		using size_type = typename traits_t::size_type;
		using pos_type = typename traits_t::pos_type;
		static constexpr pos_type npos = traits_t::npos;
		using value_type = typename traits_t::value_type;
		using uvalue_type = typename traits_t::uvalue_type;
		using const_pointer = typename traits_t::const_pointer;
		using const_reference = typename traits_t::const_reference;
		using pointer = typename traits_t::pointer;
		using reference = typename traits_t::reference;
		using iterator = typename traits_t::iterator;
		using const_iterator = typename traits_t::const_iterator;
		using reverse_iterator = typename traits_t::reverse_iterator;
		using const_reverse_iterator = typename traits_t::const_reverse_iterator;

		using wvalue_type = typename traits_t::wvalue_type;
		using const_wpointer = typename traits_t::const_wpointer;
		using const_wreference = typename traits_t::const_wreference;
		using wpointer = typename traits_t::wpointer;
		using wreference = typename traits_t::wreference;
		using utf8_iterator = typename traits_t::utf8_iterator;

		// the real interface

		inline const_pointer data() const { return container_t::data(); }
		inline size_type size() const { return container_t::size(); }
		inline bool empty() const { return size() == 0U; }
		inline const_iterator begin() const { return data(); }
		inline const_iterator end() const { return  data() + size(); }

		// forward the constructor
		string_helper() { relink("", 0U); }
		string_helper(const_pointer p, size_type s) { relink(p, s); }
		string_helper(const_pointer p) { relink(p, util::strlen(p)); }
		template<typename C>
		string_helper(const string_helper<C>& l) { relink(l.data(), l.size()); }
		template<typename C>
		string_helper(const top_t& l) { relink(l.data(), l.size()); }
		string_helper(top_t&& move) : container_t(std::move(*static_cast<container_t*>(std::addressof(move)))) {}
		string_helper(const container_t& c) : container_t(c) {  }
		string_helper(container_t&& c) : container_t(std::move(c)) {  }


		template<typename T>
		inline top_t&	operator= (const string_helper<T>&s) { link(s.begin(), s.end()); return *this; }

		//inline void link(const top_t& s) { builder_t::link(s.container()); }

		inline const_reference front() const { return *data(); }
		inline const_reference back() const { return  *(data() + size()); }
		inline size_type	length(void) const { return distance(utf8_begin(), utf8_end()); }

		inline const_reference at(pos_type pos) const { return *(data() + pos); }
		inline const_reference operator[](pos_type pos) const { return *(data() + pos); }

		inline const_iterator iat(pos_type pos) const { return begin() + min(size_type(pos), size()); }


		template<typename T>
		inline	int compare(const string_helper<T>& s) const {
			return util::str_compare(data(), data() + size(), s.data(), s.data() + s.size());
		}
		template<typename T>
		inline int compare(pos_type start, size_type len, const string_helper<T>& s) const {
			return util::str_compare(iat(start), iat(start + len), s.begin(), s.end());;
		}
		template<typename T>
		inline int compare(pos_type s1, size_type l1, const string_helper<T>&  s, pos_type s2, size_type l2) const {
			return util::str_compare(iat(s1), iat(s1 + l1), s.iat(s2), s.iat(s2 + l2));
		}
		inline int			compare(const_pointer s) const { return util::str_compare(begin(), end(), s, s + strlen(s)); }
		inline int			compare(pos_type s1, size_type l1, const_pointer s, size_type l2) const { return util::str_compare(iat(s1), iat(s1 + l1), s, s + l2); }
		inline int			compare(pos_type s1, size_type l1, const_pointer s) const { return compare(s1, l1, s, strlen(s)); }

		template<typename T>
		inline bool	operator== (const string_helper<T>& s) const { return size() == s.size() && (data() == s.data() || ::memcmp(data(), s.data(), size()) == 0); }
		inline bool	operator== (const quake::string_view& s) const { return size() == s.size() && (data() == s.data() || ::memcmp(data(), s.data(), size()) == 0); }
		inline bool	operator== (const_pointer s) const noexcept { return ((s == nullptr || s == "") && size() == 0) || size() == util::strlen(s) && 0 == ::memcmp(data(), s, size()); }
		inline bool	operator== (value_type c) const { return size() == 1 && c == *data(); }
		inline bool	operator== (uvalue_type c) const { return operator== (value_type(c)); }

		template<typename T>
		inline bool operator!= (const string_helper<T>& s) const { return !operator== (s); }
		inline bool	operator!= (const quake::string_view& s) const { return !operator== (s); }
		inline bool	operator!= (const_pointer s) const { return !operator== (s); }
		inline bool	operator!= (value_type c) const { return !operator== (c); }
		inline bool	operator!= (uvalue_type c) const { return !operator== (c); }

		template<typename T>
		inline bool	operator< (const string_helper<T>& s) const { return 0 > compare(s); }
		inline bool	operator< (const_pointer s) const { return 0 > compare(s); }
		inline bool	operator< (value_type c) const { return 0 > util::str_compare(begin(), end(), &c, &c + 1); }
		inline bool	operator< (uvalue_type c) const { return operator< (value_type(c)); }
		inline bool	operator> (const_pointer s) const { return 0 < compare(s); }

		pos_type			find(value_type c, pos_type pos = 0) const noexcept { return util::str_find(*this, c, pos); }
		template<typename T>
		pos_type			find(const string_helper<T>& s, pos_type pos = 0) const noexcept { return util::str_find(*this, s, pos); }
		inline pos_type		find(uvalue_type c, pos_type pos = 0) const noexcept { return find(value_type(c), pos); }
		inline pos_type		find(const_pointer p, pos_type pos, size_type count) const { return find(string_view(p, count), pos); }

		pos_type			rfind(value_type c, pos_type pos = npos) const noexcept { return util::str_rfind(*this, c, pos); }
		template<typename T>
		pos_type			rfind(const string_helper<T>& s, pos_type pos = npos) const noexcept { return util::str_rfind(*this, s, pos); }
		inline pos_type		rfind(uvalue_type c, pos_type pos = npos) const noexcept { return rfind(value_type(c), pos); }
		inline pos_type		rfind(const_pointer p, pos_type pos, size_type count) const { return rfind(string_view(p, count), pos); }

		template<typename T>
		pos_type			find_first_of(const string_helper<T>& s, pos_type pos = npos) const noexcept { return util::str_find_first_of(*this, s, pos); }
		inline pos_type		find_first_of(value_type c, pos_type pos = npos) const { return find_first_of(string_view(&c, 1), pos); }
		inline pos_type		find_first_of(uvalue_type c, pos_type pos = npos) const { return find_first_of(value_type(c), pos); }
		inline pos_type		find_first_of(const_pointer p, pos_type pos, size_type count) const { return find_first_of(string_view(p, count), pos); }

		template<typename T>
		pos_type			find_first_not_of(const string_helper<T>& s, pos_type pos = npos) const noexcept { return util::str_find_first_not_of(*this, s, pos); }
		inline pos_type		find_first_not_of(value_type c, pos_type pos = npos) const { return find_first_not_of(string_view(&c, 1), pos); }
		inline pos_type		find_first_not_of(uvalue_type c, pos_type pos = npos) const { return find_first_not_of(value_type(c), pos); }
		inline pos_type		find_first_not_of(const_pointer p, pos_type pos, size_type count) const { return find_first_not_of(string_view(p, count), pos); }

		template<typename T>
		pos_type			find_last_of(const string_helper<T>& s, pos_type pos = npos) const noexcept { return util::str_find_last_of(*this, s, pos); }
		inline pos_type		find_last_of(value_type c, pos_type pos = npos) const { return find_last_of(string_view(&c, 1), pos); }
		inline pos_type		find_last_of(uvalue_type c, pos_type pos = npos) const { return find_last_of(value_type(c), pos); }
		inline pos_type		find_last_of(const_pointer p, pos_type pos, size_type count) const { return find_last_of(string_view(p, count), pos); }

		template<typename T>
		pos_type			find_last_not_of(const string_helper<T>& s, pos_type pos = npos) const noexcept { return util::str_find_last_not_of(*this, s, pos); }
		inline pos_type		find_last_not_of(value_type c, pos_type pos = npos) const { return find_last_not_of(string_view(&c, 1), pos); }
		inline pos_type		find_last_not_of(uvalue_type c, pos_type pos = npos) const { return find_last_not_of(value_type(c), pos); }
		inline pos_type		find_last_not_of(const_pointer p, pos_type pos, size_type count) const { return find_last_not_of(string_view(p, count), pos); }

		hashvalue_t		hash() const noexcept { return util::str_hash(data(), data() + size()); }

		// test if we can do this?
	

		inline string_helper<cmemlink> substr(size_type pos=0, size_type count=npos) const {
			assert(pos < size());
			return string_helper<cmemlink>(data() + pos, min(count, size() - pos));
		}
		inline string_helper<cmemlink> remove_prefix(size_type n) const {
			assert(n < size());
			return string_helper<cmemlink>(data() + n, size() - n);
		}
		inline string_helper<cmemlink> remove_suffix(size_type n) const {
			assert(n < size());
			return string_helper<cmemlink>(data(), size() - n);
		}

		constexpr bool is_number() const { return data() != nullptr && size() > 0 && (util::is_char_number(data()[0]) || ((data()[0] == '+' || data()[0] == '-') && is_char_number(data()[1]))); }

		const_iterator to_number(float& v) const {
			char* str;
			v = strtof(data(), &str);
			return str != data() ? const_iterator(str) : data();
		}
		const_iterator to_number(double& v) const {
			char* str;
			v = strtod(data(), &str);
			return str != data() ? const_iterator(str) : data();
		}
		const_iterator to_number(int32_t& v, int base = 0) const {
			char* str;
			v = strtol(data(), &str, base);
			return str != data() ? const_iterator(str) : data();
		}
		const_iterator to_number(int64_t& v, int base = 0) const {
			char* str;
			v = strtoll(data(), &str, base);
			return str != data() ? const_iterator(str) : data();
		}
		container_t& container() { return static_cast<container_t&>(*this); }
		const container_t& container() const { return static_cast<const container_t&>(*this); }

	};

	//----------------------------------------------------------------------
	// Operators needed to avoid comparing pointer to pointer

#define PTR_STRING_CMP(op, impl) template<typename T> inline bool op (const char* s1, const string_helper<T>& s2) { return impl; }
	PTR_STRING_CMP(operator==, (s2 == s1))
		PTR_STRING_CMP(operator!=, (s2 != s1))
		PTR_STRING_CMP(operator<, (s2 >  s1))
		PTR_STRING_CMP(operator<=, (s2 >= s1))
		PTR_STRING_CMP(operator>, (s2 <  s1))
		PTR_STRING_CMP(operator>=, (s2 <= s1))
#undef PTR_STRING_CMP

		//----------------------------------------------------------------------
		// String-number conversions

#define STRING_TO_INT_CONVERTER(name,type,func)	\
template<typename T> \
inline type name (const string_helper<T>& str, size_t* idx = nullptr, int base = 10) \
{					\
    const char* sp = str.data();	\
    char* endp = nullptr;		\
    type r = func (sp, idx ? &endp : nullptr, base);\
    if (idx)				\
	*idx = endp - sp;		\
    return r;				\
}
		STRING_TO_INT_CONVERTER(stoi, int, strtol)
		STRING_TO_INT_CONVERTER(stol, long, strtol)
		STRING_TO_INT_CONVERTER(stoul, unsigned long, strtoul)
#if HAVE_LONG_LONG
		STRING_TO_INT_CONVERTER(stoll, long long, strtoll)
		STRING_TO_INT_CONVERTER(stoull, unsigned long long, strtoull)
#endif
#undef STRING_TO_INT_CONVERTER

#define STRING_TO_FLOAT_CONVERTER(name,type,func) \
template<typename T> \
inline type name (const  string_helper<T>&  str, size_t* idx = nullptr) \
{					\
    const char* sp = str.data();	\
    char* endp = nullptr;		\
    type r = func (sp, idx ? &endp : nullptr);\
    if (idx)				\
	*idx = endp - sp;		\
    return r;				\
}
		STRING_TO_FLOAT_CONVERTER(stof, float, strtof)
		STRING_TO_FLOAT_CONVERTER(stod, double, strtod)
		STRING_TO_FLOAT_CONVERTER(stold, long double, strtold)
#undef STRING_TO_FLOAT_CONVERTER



	using string_view = string_helper<cmemlink>;


	class cstring_container {
	public:
		using const_pointer = const char*;
		using size_type = size_t;
		const_pointer data() const { return _str; }
		size_t size() const { return util::strlen(_str); }
		cstring_container() : _str("") {}
		cstring_container(const char* str) : _str(str) {}
		void relink(const_pointer p, size_type s) { _str = p; assert(util::strlen(p) == s); }
		const char* c_str() const {
			return _str;
		}
	private:
		const_pointer _str;
	};

	using cstring =  string_helper<cstring_container>;


	template<typename _CONTAINER>
	class string_builder : public string_helper<_CONTAINER> {
	public:
		using container_t = _CONTAINER;
		using base_t = string_helper<_CONTAINER>;
		using top_t = string_builder<_CONTAINER>;

		using traits_t = string_traits<false>;
		using difference_type = typename traits_t::difference_type;
		using size_type = typename traits_t::size_type;
		using pos_type = typename traits_t::pos_type;
		static constexpr pos_type npos = traits_t::npos;
		using value_type = typename traits_t::value_type;
		using const_pointer = typename traits_t::const_pointer;
		using const_reference = typename traits_t::const_reference;
		using pointer = typename traits_t::pointer;
		using reference = typename traits_t::reference;
		using iterator = typename traits_t::iterator;
		using const_iterator = typename traits_t::const_iterator;
		using reverse_iterator = typename traits_t::reverse_iterator;
		using const_reverse_iterator = typename traits_t::const_reverse_iterator;

		using wvalue_type = typename traits_t::wvalue_type;
		using const_wpointer = typename traits_t::const_wpointer;
		using const_wreference = typename traits_t::const_wreference;
		using wpointer = typename traits_t::wpointer;
		using wreference = typename traits_t::wreference;
		using utf8_iterator = typename traits_t::utf8_iterator;

		string_builder() { assign("", 0U); }
		string_builder(const_pointer p, size_type s) { assign(p, s); }
		string_builder(const_pointer p) { assign(p); }
		template<typename T>
		string_builder(const string_helper<T>& l) { assign(.data(), l.size()); }
		template<typename T>
		string_builder(const string_builder<T>& l) { assign(.data(), l.size()); }

		string_builder(const top_t& l) { this->assign(l.data(), l.size()); }
		inline const string_builder&	operator= (const string_builder&s) { this->link(s.begin(), s.end()); return *this; }

		string_builder(const container_t& c) :base_t(c) {  }
		string_builder(container_t&& c) : base_t(std::move(c)) {  }

		// forwards
		inline const_pointer data() const { return base_t::data(); }
		inline const_reference front() const { return base_t::front(); }
		inline const_reference back() const { return  base_t::back(); }

		inline const_reference at(pos_type pos) const { return base_t::at(pos); }
		inline const_reference operator[](pos_type pos) const { return base_t::operator[](pos); }
		inline const_iterator iat(pos_type pos) const { return base_t::iat(pos); }

		inline pointer data() { return container_t::data(); }
		inline iterator begin() { return data(); }
		inline iterator end() { return data() + size(); }

		inline reference front() { return *data(); }
		inline reference back() { return  *(data() + size()); }

		inline reference at(pos_type pos) { return *(data() + pos); }
		inline reference operator[](pos_type pos) { return *(data() + pos); }

		inline iterator iat(pos_type pos) { return begin() + min(size_type(pos), size()); }


		inline size_type max_size() const {
			return container_t::capacity() ? container_t::capacity() - 1 : container_t::size();
		}

		inline const_pointer c_str() const { return data(); }
		inline size_type	capacity(void) const { return container_t::capacity() - 1; }

		inline void	reserve(size_type n, bool exact = false) {
			if (n > (container_t::capacity() + 1))
				container_t::reserve(n + 1, exact);
		}
		/// Resize the string to \p n characters. New space contents is undefined.
		inline void resize(size_type n, bool exact = true) {
			reserve(n, exact);
			container_t::resize(n);
			at(n) = '\0';
		}

		/// Returns the number of bytes required to write this object to a stream.
		size_type stream_size(void) const noexcept { return Utf8Bytes(size()) + size(); }


		inline top_t&		append(const_iterator i1, const_iterator i2) { return append(i1, distance(i1, i2)); }
		top_t&	   			append(const_pointer s, size_type len) {
			if (len == 0 || s == nullptr) {
				resize(size() + len);
				copy_n(s, len, end() - len);
			}
			return *this;
		}
		top_t&	   			append(const_pointer s) {
			if (!s) s = "";
			append(s, util::strlen(s));
			return *this;
		}
		top_t&				append(size_type n, value_type c) {
			if (n > 0) {
				resize(size() + n);
				fill_n(end() - n, n, c);
			}
			return *this;
		}
		inline top_t&		append(size_type n, wvalue_type c) { insert(size(), n, c); return *this; }
		inline top_t&		append(const_wpointer s1, const_wpointer s2) { insert(size(), s1, s2); return *this; }
		inline top_t&		append(const_wpointer s) { const_wpointer se(s); for (; se&&*se; ++se) {} return append(s, se); }
		template<typename T>
		inline top_t&		append(const string_helper<T>& s) { return append(s.begin(), s.end()); }
		template<typename T>
		inline top_t&		append(const string_helper<T>& s, pos_type o, size_type n) { return append(s.iat(o), s.iat(o + n)); }
		inline void			push_back(value_type c) { resize(size() + 1); end()[-1] = c; }
		inline void			push_back(wvalue_type c) { append(1, c); }
		inline void			pop_back(void) { resize(size() - 1); }

		inline top_t&		assign(const_iterator i1, const_iterator i2) { return assign(i1, distance(i1, i2)); }
		inline top_t&		assign(const quake::string_view& sv) { return assign(sv.data(), sv.size()); }
		top_t&	    		assign(const_pointer s, size_type len) {
			resize(len);
			if (len) copy_n(s, len, begin());
			return *this;
		}
		top_t&	    		assign(const_pointer s) {
			if (!s) s = "";
			assign(s, util::strlen(s));
			return *this;
		}
		inline top_t&		assign(const_wpointer s1, const_wpointer s2) { clear(); return append(s1, s2); }
		inline top_t&		assign(const_wpointer s1) { clear(); return append(s1); }
		template<typename T, typename C>
		inline top_t&		assign(const string_helper<T>& s) { return assign(s.begin(), s.end()); }
		template<typename T>
		inline top_t&		assign(const string_helper<T>& s, pos_type o, size_type n) { return assign(s.iat(o), s.iat(o + n)); }
		inline top_t&		assign(size_type n, value_type c) { clear(); return append(n, c); }
		inline top_t&		assign(size_type n, wvalue_type c) { clear(); return append(n, c); }
		size_type			copy(pointer p, size_type n, pos_type pos = 0) const noexcept {
			assert(p && n && start <= size());
			const size_type btc = min(n, size() - start);
			copy_n(iat(start), btc, p);
			return btc;
		}
		void clear() { resize(0U); }
		top_t& insert(pos_type ipo, size_type n, wvalue_type c) {
			iterator ip(iat(ipo));
			ip = iterator(memblock::insert(memblock::iterator(ip), n * Utf8Bytes(c)));
			fill_n(utf8out(ip), n, c);
			*end() = 0;
			return top();
		}
		top_t& insert(pos_type ipo, const wvalue_type* first, const wvalue_type* last, const size_type n) {
			iterator ip(iat(ipo));
			size_type nti = distance(first, last), bti = 0;
			for (size_type i = 0; i < nti; ++i)
				bti += Utf8Bytes(first[i]);
			ip = iterator(_container.insert(iterator(ip), n * bti));
			utf8out_iterator<iterator> uout(utf8out(ip));
			for (size_type j = 0; j < n; ++j)
				for (size_type k = 0; k < nti; ++k, ++uout)
					*uout = first[k];
			*end() = 0;
			return top();
		}
		/// Inserts character \p c into this string at \p start.
		iterator insert(const_iterator start, size_type n, value_type c) {
			memblock::iterator ip = memblock::insert(const_iterator(start), n);
			fill_n(ip, n, c);
			*end() = 0;
			return iterator(ip);
		}
		/// Inserts \p count instances of string \p s at offset \p start.
		iterator insert(const_iterator start, const_pointer s, size_type n) {
			if (!s) s = "";
			return insert(start, s, s + strlen(s), n);
		}
		iterator insert(const_iterator start, const_pointer first, const_pointer last, size_type n) {
			assert(first <= last);
			assert(begin() <= start && end() >= start);
			assert((first < begin() || first >= end() || size() + abs_distance(first, last) < capacity()) && "Insertion of self with autoresize is not supported");
			container_t::iterator ip = iterator(_container.insert(container_t::const_iterator(start), distance(first, last) * n));
			fill(ip, first, distance(first, last), n);
			*end() = 0;
			return iterator(ip);
		}
		inline top_t&		insert(pos_type ip, size_type n, value_type c) { insert(iat(ip), n, c); return top(); }
		inline top_t&		insert(pos_type ip, const_pointer s) { insert(iat(ip), s, s + strlen(s)); return top(); }
		inline top_t&		insert(pos_type ip, const_pointer s, size_type nlen) { insert(iat(ip), s, s + nlen); return top(); }
		template<typename T>
		inline top_t&		insert(pos_type ip, const string_helper<T>& s) { insert(iat(ip), s.c_str(), s.size()); return top(); }
		template<typename T>
		inline top_t&		insert(pos_type ip, const string_helper<T>& s, size_type sp, size_type slen) { insert(iat(ip), s.iat(sp), s.iat(sp + slen)); return top(); }
		inline top_t&		insert(int ip, size_type n, value_type c) { insert(pos_type(ip), n, c); return top(); }
		inline top_t&		insert(int ip, const_pointer s, size_type nlen) { insert(pos_type(ip), s, nlen); return top(); }

		inline iterator		insert(const_iterator start, value_type c) { return insert(start, 1u, c); }




		template<typename T>
		inline top_t&	operator= (const string_builder<T>&s) { return assign(s.begin(), s.end()); }
		template<typename T>
		inline top_t&	operator= (const string_helper<T>&s) { return assign(s.begin(), s.end()); }
		inline top_t&	operator= (const_reference c) { return assign(&c, 1); }
		inline top_t&	operator= (const_pointer s) { return assign(s); }
		inline top_t&	operator= (const_wpointer s) { return assign(s); }
		template<typename T>
		inline top_t&	operator+= (const string_helper<T>& s) { return append(s.begin(), s.size()); }
		inline top_t&	operator+= (value_type c) { push_back(c); return *this; }
		inline top_t&	operator+= (const_pointer s) { return append(s); }
		inline top_t&	operator+= (wvalue_type c) { return append(1, c); }
		inline top_t&	operator+= (uvalue_type c) { return operator+= (value_type(c)); }
		inline top_t&	operator+= (const_wpointer s) { return append(s); }
		template<typename T>
		inline top_t		operator+ (const string_helper<T>& s) const {
			top_t result(top());
			result += s;
			return result;
		}
		/// Erases \p size bytes at \p ep.
		iterator erase(const_iterator ep, size_type n) {
			iterator rv = _container.erase(container_tconst_iterator(ep), n);
			*end() = 0;
			return rv
		}
		top_t& erase(pos_type epo, size_type n) {
			erase(iat(epo), min(n, size() - epo));
			return top();
		}
		inline top_t&		erase(int epo, size_type n = npos) { return erase(pos_type(epo), n); }
		inline iterator		erase(const_iterator first, const_iterator last) { return erase(first, size_type(distance(first, last))); }
		inline iterator		eraser(pos_type first, pos_type last) { return erase(iat(first), iat(last)); }

		/// Replaces range [\p start, \p start + \p len] with string \p s.
		top_t& replace(const_iterator first, const_iterator last, const_pointer s) {
			if (!s) s = "";
			replace(first, last, s, s + util::strlen(s));
			return top();
		}

		/// Replaces range [\p start, \p start + \p len] with \p count instances of string \p s.
		top_t& replace(const_iterator first, const_iterator last, const_pointer i1, const_pointer i2, size_type n) {
			assert(first <= last);
			assert(n || distance(first, last));
			assert(first >= begin() && first <= end() && last >= first && last <= end());
			assert((i1 < begin() || i1 >= end() || abs_distance(i1, i2) * n + size() < capacity()) && "Replacement by self can not autoresize");
			const size_type bte = distance(first, last), bti = distance(i1, i2) * n;
			container_t::const_iterator rp = static_cast<container_t::const_iterator>(first);
			if (bti < bte)
				rp = _container.erase(rp, bte - bti);
			else if (bte < bti)
				rp = _container.insert(rp, bti - bte);
			fill(rp, i1, distance(i1, i2), n);
			*end() = 0;
			return top();
		}
		template <typename InputIt>
		top_t&			replace(const_iterator first, const_iterator last, InputIt first2, InputIt last2) { return replace(first, last, first2, last2, 1); }
		template<typename T, typename C>
		inline top_t&		replace(const_iterator first, const_iterator last, const string_helper<T>& s) { return replace(first, last, s.begin(), s.end()); }
		inline top_t&		replace(const_iterator first, const_iterator last, const_pointer s, size_type slen) { return replace(first, last, s, s + slen); }
		inline top_t&		replace(const_iterator first, const_iterator last, size_type n, value_type c) { return replace(first, last, &c, &c + 1, n); }
		template<typename T>
		inline top_t&		replace(pos_type rp, size_type n, const string_helper<T>& s) { return replace(iat(rp), iat(rp + n), s); }
		template<typename T>
		inline top_t&		replace(pos_type rp, size_type n, const string_helper<T>& s, uoff_t sp, size_type slen) { return replace(iat(rp), iat(rp + n), s.iat(sp), s.iat(sp + slen)); }
		inline top_t&		replace(pos_type rp, size_type n, const_pointer s, size_type slen) { return replace(iat(rp), iat(rp + n), s, s + slen); }
		inline top_t&		replace(pos_type rp, size_type n, const_pointer s) { return replace(iat(rp), iat(rp + n), string(s)); }
		inline top_t&		replace(pos_type rp, size_type n, size_type count, value_type c) { return replace(iat(rp), iat(rp + n), count, c); }

#if HAVE_CPP11
		using initlist_t = std::initializer_list<value_type>;
		template<typename T>
		inline top_t&		assign(string_helper<T>&& v) { swap(v); return top(); }
		inline top_t&		assign(initlist_t v) { return assign(v.begin(), v.size()); }
		inline top_t&		append(initlist_t v) { return append(v.begin(), v.size()); }
		inline top_t&		operator+= (initlist_t v) { return append(v.begin(), v.size()); }
		template<typename T>
		inline top_t&		operator= (string_helper<T>&& v) { return assign(move(v)); }
		inline top_t&		operator= (initlist_t v) { return assign(v.begin(), v.size()); }
		inline iterator		insert(const_iterator ip, initlist_t v) { return insert(ip, v.begin(), v.end()); }
		inline top_t&		replace(const_iterator first, const_iterator last, initlist_t v) { return replace(first, last, v.begin(), v.end()); }
#endif
		operator const char*() const { return c_str(); }


		/// Equivalent to a vsprintf on the string.
		int vformat(const char* fmt, va_list args)
		{
#if HAVE_VA_COPY
			va_list args2;
#else
#define args2 args
#undef __va_copy
#define __va_copy(x,y)
#endif
			int rv = size();
			int wcap;
			do {
				__va_copy(args2, args);
				rv = vsnprintf(data(), wcap = capacity(), fmt, args2);
				resize(rv);
			} while (rv >= wcap);
			return rv;
		}

		/// Equivalent to a sprintf on the string.
		int format(const char* fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			const int rv = vformat(fmt, args);
			va_end(args);
			return rv;
		}

		/// Reads the object from stream \p os
		void read(istream& is)
		{
			char szbuf[8];
			is >> szbuf[0];
			size_t szsz(Utf8SequenceBytes(szbuf[0]) - 1), n = 0;
			if (!is.verify_remaining("read", "ustl::string", szsz)) return;
			is.read(szbuf + 1, szsz);
			n = *utf8in(szbuf);
			if (!is.verify_remaining("read", "ustl::string", n)) return;
			resize(n);
			is.read(data(), size());
		}

		/// Writes the object to stream \p os
		void write(ostream& os) const
		{
			const uint32_t sz(size());
			assert(sz == size() && "No support for writing strings larger than 4G");

			char szbuf[8];
			utf8out_iterator<char*> szout(szbuf);
			*szout = sz;
			size_t szsz = distance(szbuf, szout.base());

			if (!os.verify_remaining("write", "ustl::string", szsz + sz)) return;
			os.write(szbuf, szsz);
			os.write(data(), sz);
		}


	};
	template<typename T>
	inline string operator+ (const char* cs, const string_builder<T>& ss) { string r; r.reserve(strlen(cs) + ss.size()); r += cs; r += ss; return r; }
	/// \class string ustring.h ustl.h
	/// \ingroup Sequences
	///
	/// \brief STL basic_string&lt;char&gt; equivalent.
	///
	/// An STL container for text string manipulation.
	/// Differences from C++ standard:
	///	- string is a class, not a template. Wide characters are assumed to be
	///		encoded with utf8 at all times except when rendering or editing,
	///		where you would use a utf8 iterator.
	/// 	- format member function - you can, of course use an \ref ostringstream,
	///		which also have format functions, but most of the time this way
	///		is more convenient. Because uSTL does not implement locales,
	///		format is the only way to create localized strings.
	/// 	- const char* cast operator. It is much clearer to use this than having
	/// 		to type .c_str() every time.
	/// 	- length returns the number of _characters_, not bytes.
	///		This function is O(N), so use wisely.
	///
	/// An additional note is in order regarding the use of indexes. All indexes
	/// passed in as arguments or returned by find are byte offsets, not character
	/// offsets. Likewise, sizes are specified in bytes, not characters. The
	/// rationale is that there is no way for you to know what is in the string.
	/// There is no way for you to know how many characters are needed to express
	/// one thing or another. The only thing you can do to a localized string is
	/// search for delimiters and modify text between them as opaque blocks. If you
	/// do anything else, you are hardcoding yourself into a locale! So stop it!
	///
	using fixed_string = string_builder<fixed_memblock>;
	template<size_t N>
	using static_string = string_builder<static_memblock<N>>;


	using string = string_builder<memblock>;






	//----------------------------------------------------------------------

	inline hashvalue_t hash_value(const char* first, const char* last)
	{
		return util::str_hash(first, last);
	}
	inline hashvalue_t hash_value(const char* v)
	{
		return hash_value(v, v + strlen(v));
	}

	
#if 0
#define NUMBER_TO_STRING_CONVERTER(type,fmts)\
    inline string to_string (type v) { string r; r.format(fmts,v); return r; }
		NUMBER_TO_STRING_CONVERTER(int, "%d")
		NUMBER_TO_STRING_CONVERTER(long, "%ld")
		NUMBER_TO_STRING_CONVERTER(unsigned long, "%lu")
#if HAVE_LONG_LONG
		NUMBER_TO_STRING_CONVERTER(long long, "%lld")
		NUMBER_TO_STRING_CONVERTER(unsigned long long, "%llu")
#endif
		NUMBER_TO_STRING_CONVERTER(float, "%f")
		NUMBER_TO_STRING_CONVERTER(double, "%lf")
		NUMBER_TO_STRING_CONVERTER(long double, "%Lf")
#undef NUMBER_TO_STRING_CONVERTER
#endif

};
