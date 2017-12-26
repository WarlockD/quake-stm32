// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#pragma once
#include "memblock.h"
#include "utf8.h"
#include <string_view>

namespace ustl {
	// templated helper interface that lets us handle the basic things strings do
	// switched alot of the string functions to static globals
	namespace util {
		using pos_type = size_t;
		using size_type = size_t;
		using hashvalue_t = uint32_t;
		static constexpr pos_type npos = pos_type(-1);

		hashvalue_t str_hash(const char* first, const char* last) noexcept; // static
		int str_compare(const char* first1, const char* last1, const char* first2, const char* last2) noexcept;
		pos_type str_find(const cmemlink&  hay,  char c, pos_type pos = 0U)  noexcept;
		pos_type str_find(const cmemlink&  hay, const cmemlink&  needle, pos_type pos=0U) noexcept;
		inline pos_type	str_find(const cmemlink&  hay, unsigned char c, pos_type pos = 0)  noexcept { return str_find(hay, char(c), pos); }
		inline pos_type	str_find(const cmemlink&  hay, const char* p, pos_type pos) { return str_find(hay, cmemlink(p, ::strlen(p)), pos); }
		inline pos_type	str_find(const cmemlink&  hay, const char* p, pos_type pos, size_type count)  { return str_find(hay, cmemlink(p, count), pos); }


		pos_type str_rfind(const cmemlink&  hay, char c, pos_type pos= npos)  noexcept;
		pos_type str_rfind(const cmemlink&  hay, const cmemlink& s, pos_type pos = npos)  noexcept;
		inline pos_type	str_rfind(const cmemlink&  hay, const char* p, pos_type pos) { return str_rfind(hay, cmemlink(p, ::strlen(p)), pos); }
		inline pos_type	str_rfind(const cmemlink&  hay, const char* p, pos_type pos, size_type count) { return str_rfind(hay, cmemlink(p, count), pos); }

		pos_type str_find_first_of(const cmemlink&  hay, const cmemlink& s, pos_type pos = 0U)  noexcept;
		pos_type str_find_last_of(const cmemlink&  hay, const cmemlink& s, pos_type pos = npos)  noexcept;
		pos_type str_find_last_not_of(const cmemlink&  hay, const cmemlink& s, pos_type pos = npos)  noexcept;
		pos_type str_find_first_not_of(const cmemlink&  hay, const cmemlink& s, pos_type pos = 0U)  noexcept;
	}

	// string helper gives a class bare bones stuff so we can do things like
	// string search and make a simple string view class


	template<typename LINK>
	class string_helper: public LINK {
	protected:
		using link_type = LINK;
	public:
		using traits = typename LINK::traits;
		using value_type = typename traits::value_type;

		static_assert(std::is_same<char, value_type>::value, "Only char types!");
		using uvalue_type = unsigned char;
		using pointer = typename traits::pointer;
		using const_pointer = typename traits::const_pointer;
		using reference = typename traits::reference;
		using const_reference = typename traits::const_reference;
		using size_type = typename traits::size_type;
		using difference_type = typename traits::difference_type;
		using const_iterator = typename traits::const_iterator;
		using iterator = typename traits::iterator;
		typedef ::ustl::reverse_iterator<iterator>		reverse_iterator;
		typedef ::ustl::reverse_iterator<const_iterator>	const_reverse_iterator;
		typedef utf8in_iterator<const_iterator>		utf8_iterator;
		using pos_type = size_type;

		using wvalue_type = wchar_t;
		using wpointer = wvalue_type*;
		using const_wpointer = const wvalue_type*;

		static constexpr const pos_type npos = std::numeric_limits<pos_type>::max();

		// this is mainly here for returns on substring
		string_helper() : LINK() {}
		template<typename ... Args> // lazy hack
		string_helper(Args&& ... args) : LINK(std::forward<Args>(args)...) {}

		template<class Q = LINK>
		typename std::enable_if<traits::can_read || traits::can_write, const_reference>::type
			operator[](size_type i) const { return LINK::data()[i]; }

		template<class Q = LINK>
		typename std::enable_if<!std::is_const<reference>::value, reference>::type
			operator[](size_type i) { return LINK::data()[i]; }
#if 0
		inline const_reverse_iterator	rbegin(void) const { return const_reverse_iterator(end()); }
		inline const_reverse_iterator	crbegin(void) const { return rbegin(); }
		inline const_reverse_iterator	rend(void) const { return const_reverse_iterator(begin()); }
		inline const_reverse_iterator	crend(void) const { return rend(); }
#endif
		inline utf8_iterator	utf8_begin(void) const { return utf8_iterator(LINK::begin()); }
		inline utf8_iterator	utf8_end(void) const { return utf8_iterator(LINK::end()); }
#if 0
		inline const_reference	at(pos_type pos) const { assert(pos <= LINK::size() && LINK::begin()); return LINK::begin()[pos]; }
		inline const_iterator	iat(pos_type pos) const { return LINK::begin() +  min(size_type(pos), LINK::size()); }
#endif
		const_iterator wiat(pos_type i) const noexcept {
			utf8in_iterator<const char*> cfinder(LINK::begin());
			cfinder += i;
			return cfinder.base();
		}
		inline const_reference	front(void) const { return at(0); }
		inline const_reference	back(void) const { return at(size() - 1); }
		inline size_type		length(void) const { return distance(utf8_begin(), utf8_end()); }
		template<typename L>
		inline int			compare(const string_helper<L>& s) const { return util::str_compare(LINK::begin(), LINK::end(), s.begin(), s.end()); }
		template<typename L>
		inline int			compare(pos_type start, size_type len, const string_helper<L>& s) const { return util::str_compare(LINK::iat(start), LINK::iat(start + len), s.begin(), s.end()); }
		template<typename L>
		inline int			compare(pos_type s1, size_type l1, const string_helper<L>&  s, pos_type s2, size_type l2) const { return util::str_compare(LINK::iat(s1), LINK::iat(s1 + l1), s.iat(s2), s.iat(s2 + l2)); }
		inline int			compare(const_pointer s) const { return util::str_compare(LINK::begin(), LINK::end(), s, s + ::strlen(s)); }
		inline int			compare(pos_type s1, size_type l1, const_pointer s, size_type l2) const { return util::str_compare(LINK::iat(s1), LINK::iat(s1 + l1), s, s + l2); }
		inline int			compare(pos_type s1, size_type l1, const_pointer s) const { return util::str_compare(s1, l1, s, ::strlen(s)); }

		template<typename L>
		inline bool			operator== (const string_helper<L>& s) const { return LINK::operator== (s); }
		inline bool			operator== (const_pointer s) const noexcept { return ((s == nullptr || s == "") && LINK::size() == 0)|| LINK::size() == ::strlen(s) && 0 == ::memcmp(LINK::data(), s, LINK::size()); }
		inline bool			operator== (value_type c) const { return LINK::size() == 1 && c == at(0); }
		inline bool			operator== (uvalue_type c) const { return operator== (value_type(c)); }
		template<typename L>
		inline bool			operator!= (const string_helper<L>& s) const { return !operator== (s); }
		inline bool			operator!= (const_pointer s) const { return !operator== (s); }
		inline bool			operator!= (value_type c) const { return !operator== (c); }
		inline bool			operator!= (uvalue_type c) const { return !operator== (c); }
		template<typename L>
		inline bool			operator< (const string_helper<L>& s) const { return 0 > compare(s); }
		inline bool			operator< (const_pointer s) const { return 0 > compare(s); }
		inline bool			operator< (value_type c) const { return 0 > compare(LINK::begin(), LINK::end(), &c, &c + 1); }
		inline bool			operator< (uvalue_type c) const { return operator< (value_type(c)); }
		inline bool			operator> (const_pointer s) const { return 0 < compare(s); }

		pos_type			find(value_type c, pos_type pos = 0) const noexcept { return util::str_find(to_base(), c, pos); }
		template<typename L>
		pos_type			find(const string_helper<L>& s, pos_type pos = 0) const noexcept { return util::str_find(to_base(), s, pos); }
		inline pos_type		find(uvalue_type c, pos_type pos = 0) const noexcept { return find(value_type(c), pos); }
		inline pos_type		find(const_pointer p, pos_type pos, size_type count) const {  return find(base_type(p, count), pos); }
		
		pos_type			rfind(value_type c, pos_type pos = npos) const noexcept { return util::str_rfind(to_base(), c, pos); }
		template<typename L>
		pos_type			rfind(const string_helper<L>&s, pos_type pos = npos) const noexcept { return util::str_rfind(to_base(), s, pos); }
		inline pos_type		rfind(uvalue_type c, pos_type pos = npos) const noexcept { return rfind(value_type(c), pos); }
		inline pos_type		rfind(const_pointer p, pos_type pos, size_type count) const { return rfind(base_type(p, count), pos); }

		template<typename L>
		pos_type			find_first_of(const string_helper<L>& s, pos_type pos = npos) const noexcept { return util::str_find_first_of(to_base(), s, pos); }
		inline pos_type		find_first_of(value_type c, pos_type pos = npos) const { return find_first_of(base_type(&c, 1), pos); }
		inline pos_type		find_first_of(uvalue_type c, pos_type pos = npos) const { return find_first_of(value_type(c), pos); }
		inline pos_type		find_first_of(const_pointer p, pos_type pos, size_type count) const { return find_first_of(base_type(p, count), pos); }

		template<typename L>
		pos_type			find_first_not_of(const string_helper<L>&  s, pos_type pos = npos) const noexcept { return util::str_find_first_not_of(to_base(), s, pos); }
		inline pos_type		find_first_not_of(value_type c, pos_type pos = npos) const { return find_first_not_of(base_type(&c, 1), pos); }
		inline pos_type		find_first_not_of(uvalue_type c, pos_type pos = npos) const { return find_first_not_of(value_type(c), pos); }
		inline pos_type		find_first_not_of(const_pointer p, pos_type pos, size_type count) const { return find_first_not_of(base_type(p, count), pos); }

		template<typename L>
		pos_type			find_last_of(const string_helper<L>&  s, pos_type pos = npos) const noexcept { return util::str_find_last_of(to_base(), s, pos); }
		inline pos_type		find_last_of(value_type c, pos_type pos = npos) const { return find_last_of(base_type(&c, 1), pos); }
		inline pos_type		find_last_of(uvalue_type c, pos_type pos = npos) const { return find_last_of(value_type(c), pos); }
		inline pos_type		find_last_of(const_pointer p, pos_type pos, size_type count) const { return find_last_of(base_type(p, count), pos); }
		
		template<typename L>
		pos_type			find_last_not_of(const string_helper<L>& s, pos_type pos = npos) const noexcept { return util::str_find_last_not_of(to_base(), s, pos); }
		inline pos_type		find_last_not_of(value_type c, pos_type pos = npos) const { return find_last_not_of(base_type(&c, 1), pos); }
		inline pos_type		find_last_not_of(uvalue_type c, pos_type pos = npos) const { return find_last_not_of(value_type(c), pos); }
		inline pos_type		find_last_not_of(const_pointer p, pos_type pos, size_type count) const {  return find_last_not_of(base_type(p,count), pos); }

		hashvalue_t		hash() const noexcept { return util::str_hash(LINK::begin(), LINK::end()); }

		constexpr string_helper<cmemlink> substr(size_type pos = 0, size_type count = npos) const {
			return (pos > size())
				? (throw std::out_of_range("data_helpers::substr"), std::cmemlink())
				: string_helper<cmemlink>(data() + pos, std::min(count, size() - pos));
		}
		constexpr bool is_number() const { return !empty() && (util::is_char_number(at(0)) || ((at(0) == '+' || at(0) == '-') && is_char_number(at(1)))); }

		const_iterator to_number(float& v) const {
			char* str;
			v = strtof(data(), &str);
			return str <= end() ? const_iterator(str) : begin();
		}
		const_iterator to_number(int32_t& f, int base = 0) const {
			char* str;
			v = strtol(data(), &str, base);
			return str <= end() ? const_iterator(str) : begin();
		}
		const_iterator to_number(int64_t& f, int base = 0) const {
			char* str;
			v = strtoll(data(), &str, base);
			return str <= end() ? const_iterator(str) : begin();
		}
		constexpr cmemlink remove_prefix(size_type n) const {
			return (n > size())
				? (throw std::out_of_range("data_helpers::remove_prefix"), std::string_view())
				: cmemlink(data() + n, size() - n);
		}
		constexpr cmemlink remove_suffix(size_type n) const {
			return (n > size())
				? (throw std::out_of_range("data_helpers::remove_suffix"), std::string_view())
				: cmemlink(data(), size() - n);
		}

	};
	class string_view : public string_helper<cmemlink> {
	public:
		using helper = string_helper<cmemlink>;
		string_view() : helper("", 0) {}
		string_view(const char* s) : helper(s, ::strlen(s)) {}
		string_view(const char* s, size_t i) : helper(s, i) {}
		string_view(const cmemlink& s) : helper(s.data(), s.size()) {}

	};


	template<typename L>
	static inline ostream& operator<<(ostream& os, const string_helper<L>& sv) { os.write(sv.data(), sv.size());  return os; }

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
	class string : public string_helper<memblock> {
	public:
		using helper = string_helper<memblock>;
		using value_type = typename helper::value_type;
		using uvalue_type = typename helper::uvalue_type;
		using pointer = typename helper::pointer;
		using const_pointer = typename helper::const_pointer;
		using wvalue_type = typename helper::wvalue_type;
		using wpointer = typename helper::wpointer;
		using const_wpointer = typename helper::const_wpointer;
		using iterator = typename helper::iterator;
		using const_iterator = typename helper::const_iterator;
		using reference = typename helper::reference;
		using const_reference = typename helper::const_reference;
		using value_type = typename helper::value_type;
		using reverse_iterator = typename helper::reverse_iterator;
		using const_reverse_iterator = typename helper::const_reverse_iterator;
		using utf8_iterator = typename helper::utf8_iterator;

		using pos_type = typename helper::pos_type;
		using size_type = typename helper::size_type;
		static constexpr const pos_type npos = helper::npos;	///< Value that means the end of string.
	public:
		inline			string(void) noexcept : helper() { relink("", 0); }
		string(const string& s);
		inline			string(const string& s, pos_type o, size_type n = npos);
		inline explicit		string(const cmemlink& l);
		string(const_pointer s) noexcept;
		inline			string(const_pointer s, size_type len);
		inline			string(const std::string_view& s) : string(s.data(), s.size()) {}
		inline			string(const_pointer s1, const_pointer s2);
		string(size_type n, value_type c);
		inline			~string(void) noexcept { }
		inline pointer		data(void) { return string::pointer(memblock::data()); }
		inline const_pointer	data(void) const { return string::const_pointer(memblock::data()); }
		inline const_pointer	c_str(void) const { return string::const_pointer(memblock::data()); }
		inline size_type		max_size(void) const { size_type s(memblock::max_size()); return s - !!s; }
		inline size_type		capacity(void) const { size_type c(memblock::capacity()); return c - !!c; }
		void			resize(size_type n);
		inline void			resize(size_type n, value_type c);
		inline void			clear(void) { resize(0); }
		
		inline const_iterator	cbegin(void) const { return base_t::base_t::begin(); }
		inline iterator		end(void) { return iterator(memblock::end()); }
		inline const_iterator	end(void) const { return const_iterator(memblock::end()); }
		inline const_iterator	cend(void) const { return end(); }
		inline reverse_iterator	rbegin(void) { return reverse_iterator(end()); }
		inline const_reverse_iterator	rbegin(void) const { return const_reverse_iterator(end()); }
		inline const_reverse_iterator	crbegin(void) const { return rbegin(); }
		inline reverse_iterator	rend(void) { return reverse_iterator(begin()); }
		inline const_reverse_iterator	rend(void) const { return const_reverse_iterator(begin()); }
		inline const_reverse_iterator	crend(void) const { return rend(); }
		inline utf8_iterator	utf8_begin(void) const { return utf8_iterator(begin()); }
		inline utf8_iterator	utf8_end(void) const { return utf8_iterator(end()); }
		inline const_reference	at(pos_type pos) const { assert(pos <= size() && begin()); return begin()[pos]; }
		inline reference		at(pos_type pos) { assert(pos <= size() && begin()); return begin()[pos]; }
		inline const_iterator	iat(pos_type pos) const { return begin() + (__builtin_constant_p(pos) && pos >= npos ? size() : min(size_type(pos), size())); }
		inline iterator		iat(pos_type pos) { return const_cast<iterator>(const_cast<const string*>(this)->iat(pos)); }
		const_iterator		wiat(pos_type i) const noexcept;
		inline iterator		wiat(pos_type i) { return const_cast<iterator>(const_cast<const string*>(this)->wiat(i)); }
		inline const_reference	front(void) const { return at(0); }
		inline reference		front(void) { return at(0); }
		inline const_reference	back(void) const { return at(size() - 1); }
		inline reference		back(void) { return at(size() - 1); }
		inline size_type		length(void) const { return distance(utf8_begin(), utf8_end()); }
		inline string&		append(const_iterator i1, const_iterator i2) { return append(i1, distance(i1, i2)); }
		string&	   		append(const_pointer s, size_type len);
		string&	   		append(const_pointer s);
		string&			append(size_type n, value_type c);
		inline string&		append(size_type n, wvalue_type c) { insert(size(), n, c); return *this; }
		inline string&		append(const_wpointer s1, const_wpointer s2) { insert(size(), s1, s2); return *this; }
		inline string&		append(const_wpointer s) { const_wpointer se(s); for (; se&&*se; ++se) {} return append(s, se); }
		inline string&		append(const string& s) { return append(s.begin(), s.end()); }
		inline string&		append(const string& s, pos_type o, size_type n) { return append(s.iat(o), s.iat(o + n)); }
		inline void			push_back(value_type c) { resize(size() + 1); end()[-1] = c; }
		inline void			push_back(wvalue_type c) { append(1, c); }
		inline void			pop_back(void) { resize(size() - 1); }
		inline string&		assign(const_iterator i1, const_iterator i2) { return assign(i1, distance(i1, i2)); }
		string&	    		assign(const_pointer s, size_type len);
		inline string&		assign(const std::string_view& s) { assign(s.data(), s.size()); return *this; }
		string&	    		assign(const_pointer s);
		inline string&		assign(const_wpointer s1, const_wpointer s2) { clear(); return append(s1, s2); }
		inline string&		assign(const_wpointer s1) { clear(); return append(s1); }
		inline string&		assign(const string& s) { return assign(s.begin(), s.end()); }
		inline string&		assign(const string& s, pos_type o, size_type n) { return assign(s.iat(o), s.iat(o + n)); }
		inline string&		assign(size_type n, value_type c) { clear(); return append(n, c); }
		inline string&		assign(size_type n, wvalue_type c) { clear(); return append(n, c); }
		size_type			copy(pointer p, size_type n, pos_type pos = 0) const noexcept;
		inline size_type		copyto(pointer p, size_type n, pos_type pos = 0) const noexcept { size_type bc = copy(p, n - 1, pos); p[bc] = 0; return bc; }
			inline			operator const value_type* (void) const;
		inline			operator value_type* (void);
		inline const string&	operator= (const string& s) { return assign(s.begin(), s.end()); }
		inline const string&	operator= (const_reference c) { return assign(&c, 1); }
		inline const string&	operator= (const_pointer s) { return assign(s); }
		inline const string&	operator= (const_wpointer s) { return assign(s); }
		inline const string&	operator+= (const string& s) { return append(s.begin(), s.size()); }
		inline const string&	operator+= (value_type c) { push_back(c); return *this; }
		inline const string&	operator+= (const_pointer s) { return append(s); }
		inline const string&	operator+= (wvalue_type c) { return append(1, c); }
		inline const string&	operator+= (uvalue_type c) { return operator+= (value_type(c)); }
		inline const string&	operator+= (const_wpointer s) { return append(s); }
		inline string		operator+ (const string& s) const;
		inline bool			operator== (const string& s) const { return memblock::operator== (s); }
		inline bool			operator!= (const string& s) const { return !operator== (s); }
		inline bool			operator< (const string& s) const { return 0 > compare(s); }
		inline string&		insert(pos_type ip, size_type n, value_type c) { insert(iat(ip), n, c); return *this; }
		inline string&		insert(pos_type ip, const_pointer s) { insert(iat(ip), s, s + strlen(s)); return *this; }
		inline string&		insert(pos_type ip, const_pointer s, size_type nlen) { insert(iat(ip), s, s + nlen); return *this; }
		inline string&		insert(pos_type ip, const string& s) { insert(iat(ip), s.c_str(), s.size()); return *this; }
		inline string&		insert(pos_type ip, const string& s, size_type sp, size_type slen) { insert(iat(ip), s.iat(sp), s.iat(sp + slen)); return *this; }
		string&			insert(pos_type ip, size_type n, wvalue_type c);
		string&			insert(pos_type ip, const_wpointer first, const_wpointer last, size_type n = 1);
		inline string&		insert(int ip, size_type n, value_type c) { insert(pos_type(ip), n, c); return *this; }
		inline string&		insert(int ip, const_pointer s, size_type nlen) { insert(pos_type(ip), s, nlen); return *this; }
		iterator			insert(const_iterator start, size_type n, value_type c);
		inline iterator		insert(const_iterator start, value_type c) { return insert(start, 1u, c); }
		iterator			insert(const_iterator start, const_pointer s, size_type n);
		iterator			insert(const_iterator start, const_pointer first, const_iterator last, size_type n = 1);
		iterator			erase(const_iterator epo, size_type n = 1);
		string&			erase(pos_type epo = 0, size_type n = npos);
		inline string&		erase(int epo, size_type n = npos) { return erase(pos_type(epo), n); }
		inline iterator		erase(const_iterator first, const_iterator last) { return erase(first, size_type(distance(first, last))); }
		inline iterator		eraser(pos_type first, pos_type last) { return erase(iat(first), iat(last)); }
		string&			replace(const_iterator first, const_iterator last, const_pointer i1, const_pointer i2, size_type n);
		template <typename InputIt>
		string&			replace(const_iterator first, const_iterator last, InputIt first2, InputIt last2) { return replace(first, last, first2, last2, 1); }
		inline string&		replace(const_iterator first, const_iterator last, const string& s) { return replace(first, last, s.begin(), s.end()); }
		string&			replace(const_iterator first, const_iterator last, const_pointer s);
		inline string&		replace(const_iterator first, const_iterator last, const_pointer s, size_type slen) { return replace(first, last, s, s + slen); }
		inline string&		replace(const_iterator first, const_iterator last, size_type n, value_type c) { return replace(first, last, &c, &c + 1, n); }
		inline string&		replace(pos_type rp, size_type n, const string& s) { return replace(iat(rp), iat(rp + n), s); }
		inline string&		replace(pos_type rp, size_type n, const string& s, uoff_t sp, size_type slen) { return replace(iat(rp), iat(rp + n), s.iat(sp), s.iat(sp + slen)); }
		inline string&		replace(pos_type rp, size_type n, const_pointer s, size_type slen) { return replace(iat(rp), iat(rp + n), s, s + slen); }
		inline string&		replace(pos_type rp, size_type n, const_pointer s) { return replace(iat(rp), iat(rp + n), string(s)); }
		inline string&		replace(pos_type rp, size_type n, size_type count, value_type c) { return replace(iat(rp), iat(rp + n), count, c); }
		inline string		substr(pos_type o = 0, size_type n = npos) const { return string(*this, o, n); }
		inline void			swap(string& v) { memblock::swap(v); }
	int				vformat(const char* fmt, va_list args);
	int				format(const char* fmt, ...);
		void			read(istream&);
		void			write(ostream& os) const;
		size_t			stream_size(void) const noexcept;
#if HAVE_CPP11
		using initlist_t = std::initializer_list<value_type>;
		inline			string(string&& v) : helper(move(v)) {}
		inline			string(initlist_t v) : helper() { assign(v.begin(), v.size()); }
		inline string&		assign(string&& v) { swap(v); return *this; }
		inline string&		assign(initlist_t v) { return assign(v.begin(), v.size()); }
		inline string&		append(initlist_t v) { return append(v.begin(), v.size()); }
		inline string&		operator+= (initlist_t v) { return append(v.begin(), v.size()); }
		inline string&		operator= (string&& v) { return assign(move(v)); }
		inline string&		operator= (initlist_t v) { return assign(v.begin(), v.size()); }
		inline iterator		insert(const_iterator ip, initlist_t v) { return insert(ip, v.begin(), v.end()); }
		inline string&		replace(const_iterator first, const_iterator last, initlist_t v) { return replace(first, last, v.begin(), v.end()); }
#endif
	private:
		virtual size_type		minimumFreeCapacity(void) const noexcept final override;
	};

	//----------------------------------------------------------------------

	/// Assigns itself the value of string \p s
	inline string::string(const cmemlink& s)
		: helper()
	{
		assign(const_iterator(s.begin()), s.size());
	}

	/// Assigns itself a [o,o+n) substring of \p s.
	inline string::string(const string& s, pos_type o, size_type n)
		: helper()
	{
		assign(s, o, n);
	}

	/// Copies the value of \p s of length \p len into itself.
	inline string::string(const_pointer s, size_type len)
		: helper()
	{
		assign(s, len);
	}

	/// Copies into itself the string data between \p s1 and \p s2
	inline string::string(const_pointer s1, const_pointer s2)
		: helper()
	{
		assert(s1 <= s2 && "Negative ranges result in memory allocation errors.");
		assign(s1, s2);
	}

	/// Returns the pointer to the first character.
	inline string::operator const string::value_type* (void) const
	{
		assert((!end() || !*end()) && "This string is linked to data that is not 0-terminated. This may cause serious security problems. Please assign the data instead of linking.");
		return begin();
	}

	/// Returns the pointer to the first character.
	inline string::operator string::value_type* (void)
	{
		assert((end() && !*end()) && "This string is linked to data that is not 0-terminated. This may cause serious security problems. Please assign the data instead of linking.");
		return begin();
	}

	/// Concatenates itself with \p s
	inline string string::operator+ (const string& s) const
	{
		string result(*this);
		result += s;
		return result;
	}

	/// Resize to \p n and fill new entries with \p c
	inline void string::resize(size_type n, value_type c)
	{
		const size_type oldn = size();
		resize(n);
		fill_n(iat(oldn), max(ssize_t(n - oldn), 0), c);
	}

	//----------------------------------------------------------------------
	// Operators needed to avoid comparing pointer to pointer

#define PTR_STRING_CMP(op, impl)	\
template<typename L> inline bool op (const char* s1, const string_helper<L>& s2) { return impl; }

	PTR_STRING_CMP(operator==, (s2 == s1))
		PTR_STRING_CMP(operator!=, (s2 != s1))
		PTR_STRING_CMP(operator<, (s2 > s1))
		PTR_STRING_CMP(operator<=, (s2 >= s1))
		PTR_STRING_CMP(operator>, (s2 < s1))
		PTR_STRING_CMP(operator>=, (s2 <= s1))
#undef PTR_STRING_CMP

		inline string operator+ (const char* cs, const string& ss) { string r; r.reserve(strlen(cs) + ss.size()); r += cs; r += ss; return r; }

	//----------------------------------------------------------------------

	inline hashvalue_t hash_value(const char* first, const char* last)
	{
		return util::str_hash(first, last);
	}
	inline hashvalue_t hash_value(const char* v)
	{
		return hash_value(v, v + strlen(v));
	}

	//----------------------------------------------------------------------
	// String-number conversions

#define STRING_TO_INT_CONVERTER(name,type,func)	\
inline type name (const string& str, size_t* idx = nullptr, int base = 10) \
{					\
    const char* sp = str.c_str();	\
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
inline type name (const string& str, size_t* idx = nullptr) \
{					\
    const char* sp = str.c_str();	\
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

};
