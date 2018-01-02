#pragma once
#include <vector>
#include <array>
#include <stdexcept>
#include <cassert>
// quake vector stuff

namespace quake {
	template<typename T>
	using vector = std::vector<T>;
	using string = std::string;

	namespace detail {
		using value_type = char; // quake uses char for all stsrings, not sure if we want to dick with utf8 yet
		template<typename _CHART, bool IS_CONST = true>
		struct char_info {
			using traits_type = std::char_traits<_CHART>;
			using difference_type = ptrdiff_t;
			using size_type = size_t;
			using value_type = _CHART;
			using const_pointer = const value_type*;
			using pointer = std::conditional_t<IS_CONST, const_pointer, _CHART*>;
			using const_reference = const value_type&;
			using reference = std::conditional_t<IS_CONST, const_reference, _CHART&>;
			using const_iterator = const_pointer;
			using iterator = pointer;
			using const_reverse_iterator = std::reverse_iterator<const_iterator>;
			using reverse_iterator = std::reverse_iterator<iterator>;
			static constexpr size_type npos = size_type(-1);
		};
		using _char_info = char_info<value_type, false>;
		using traits_type = typename _char_info::traits_type;
		using difference_type = typename _char_info::difference_type;
		using size_type = typename _char_info::size_type;
		using const_pointer = typename _char_info::const_pointer;
		using pointer = typename _char_info::pointer;
		using const_reference = typename _char_info::const_reference;
		using reference = typename _char_info::reference;
		using const_iterator = typename _char_info::const_iterator;
		using iterator = typename _char_info::iterator;
		using const_reverse_iterator = typename _char_info::const_reverse_iterator;
		using reverse_iterator = typename _char_info::reverse_iterator;
		static constexpr size_type npos = _char_info::npos;
		static constexpr inline size_t str_length(const char* p, size_t s = 0) { return p[s] == '\0' ? s : str_length(p, s + 1); }
		// const string to lower https://gist.github.com/texus/8d867996e7a073e1498e8c18d920086c/b9e33044c834dfc0a2f34ade344467145870f841
		static constexpr value_type char_tolower(value_type c) {
			return traits_type::to_int_type(c) >= traits_type::to_int_type('A') || traits_type::to_int_type(c) >= traits_type::to_int_type('Z') ?
				traits_type::to_char_type(traits_type::to_int_type(c) + (traits_type::to_int_type('a') - traits_type::to_int_type('A'))) : c;
		}
		// Our compile time string class that is used to pass around the converted string
		template <std::size_t N>
		class const_to_lower_str {
		private:
			const char s[N + 1]; // One extra byte to fill with a 0 value
		public:
			// Constructor that is given the char array and an integer sequence to use parameter pack expansion on the array
			template <typename T, T... Nums>
			constexpr const_to_lower_str(const char(&str)[N], std::integer_sequence<T, Nums...>) : s{ char_tolower(str[Nums])..., 0 } { }
			// Compile time access operator to the characters
			constexpr char operator[] (std::size_t i) const { return s[i]; }
			// Get a pointer to the array at runtime. Even though this happens at runtime, this is a fast operation and much faster than the actual conversion
			constexpr operator const char*() const { return s; }
			constexpr size_t size() const { return N; }
		};
		// The code that we are actually going to call
		template <std::size_t N>
		constexpr inline const_to_lower_str<N> str_tolower(const char(&str)[N]) { return { str, std::make_integer_sequence<size_t, N>() }; }

		size_type str_hash(const_pointer first, const_pointer last)noexcept;
		int str_compare(const_pointer __str1, size_type __n1, const_pointer __str2, size_type __n2)noexcept;
		int str_case_compare(const_pointer __str1, size_type __n1, const_pointer __str2, size_type __n2)noexcept;
		size_type str_find(const_pointer __str1, size_type __n1, value_type c)noexcept;
		size_type str_find(const_pointer __str1, size_type __n1, const_pointer __str2, size_type __n2)noexcept;
		size_type	str_rfind(const_pointer __str1, size_type __n1, const_pointer __str2, size_type __n2)noexcept;
		size_type str_find_first_of(const_pointer __str1, size_type __n1, const_pointer __str2, size_type __n2)noexcept;
		size_type find_first_not_of(const_pointer __str1, size_type __n1, const_pointer __str2, size_type __n2)noexcept;
		size_type find_last_of(const_pointer __str1, size_type __n1, const_pointer __str2, size_type __n2)noexcept;
		size_type find_last_not_of(const_pointer __str1, size_type __n1, const_pointer __str2, size_type __n2)noexcept;
	}
	// string helper has the basic template functions for the class
	// need exceptions
	template<typename _TOP>
	class string_helper  {
	public:
		using base_type = _TOP;
		using value_type = char;
		using traits_type = std::char_traits<value_type>;
		using difference_type = std::ptrdiff_t;
		using size_type = std::size_t;
		using const_pointer = const value_type*;
		using pointer = const_pointer;
		using const_reference = const value_type&;
		using reference = const_reference;
		using const_iterator = const_pointer;
		using iterator = pointer;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using reverse_iterator = std::reverse_iterator<iterator>;
		static constexpr size_type npos = std::numeric_limits<size_type>::max();

		// from base
		constexpr inline const_pointer data() const { return static_cast<const _TOP*>(this)->data(); }
		constexpr inline size_type size() const { return static_cast<const _TOP*>(this)->size(); }
		constexpr inline size_type length() const { return static_cast<const _TOP*>(this)->size(); }

		constexpr inline bool empty() const { return size() == 0U; }
		constexpr inline const_iterator begin() const { return data(); }
		constexpr inline const_iterator end() const { return  data() + size(); }

		constexpr inline const_reference front() const { return *data(); }
		constexpr inline const_reference back() const { return  *(data() + size()); }

		constexpr inline const_reference at(size_type pos) const { return *(data() + pos); }
		constexpr inline const_reference operator[](size_type pos) const { return *(data() + pos); }

		size_type copy(pointer __str, size_type __n, size_type __pos = 0) const {
			assert(_pos);
			const size_type __rlen = std::min(__n, this->size() - __pos);
			for (auto __begin = this->data() + __pos,
				__end = __begin + __rlen; __begin != __end;)
				*__str++ = *__begin++;
			return __rlen;
		}
		constexpr int compare(size_type __pos1, size_type __n1, const_pointer __str, size_type __n2) const noexcept {
			return detail::str_compare(data(), std::min(__n1, size() - __pos1), __str, __n2);
		}
		template<typename T>
		constexpr int compare(size_type __pos1, size_type __n1, string_helper<T> __str) const { return compare(__pos1, __n1, __str.data(), __str.size()); }
		template<typename T>
		constexpr int compare(size_type __pos1, size_type __n1, string_helper<T>__str, size_type __pos2, size_type __n2) const {
			return compare(__pos1, __n1, __str.data() + __pos2, __n2 - __pos2);
		}
		constexpr int compare(size_type __pos1, size_type __n1, const_pointer __str) const { return compare(__pos1, __n1, __str, traits_type::length(__str)); }
		template<typename T>
		constexpr int compare(string_helper<T> __str) const { return compare(0U, size(), __str.data(), __str.size()); }
		constexpr int compare(const_pointer __str) const { return compare(0U, size(), __str, traits_type::length(__str)); }


		constexpr size_type find(value_type c, size_type pos = 0) const noexcept {
			assert(pos < size());
			return detail::str_find(data()+pos,size()-pos, c);
		}
		constexpr  inline size_type find(const_pointer p, size_type pos, size_type n) const {
			if (n==0 || n > size() - pos) return npos;
			assert(pos < size());
			return detail::str_find(data() + pos, size() - pos, p, n);
		}
		constexpr size_type find(const_pointer __str, size_type __pos = 0) const noexcept { return this->find(__str, __pos, detail::str_length(__str)); }
		template<typename T>
		constexpr size_type find(string_helper<T> __str, size_type __pos = 0) const noexcept { return this->find(__str.data(), __pos, __str.size()); }

		inline size_type	rfind(const_pointer p, size_type pos, size_type n) const {
			assert(pos < size());
			return detail::str_rfind(data() + pos, size() - pos, p, n);
		}
		constexpr size_type rfind(value_type c, size_type pos = 0) const noexcept {
			assert(pos < size());
			return detail::str_rfind(data() + pos, size() - pos, c);
		}
		template<typename T>
		constexpr size_type rfind(string_helper<T> __str, size_type __pos = 0) const noexcept { return this->rfind(__str.data(), __pos, __str.size()); }
		constexpr size_type rfind(const_pointer __str, size_type __pos = 0) const noexcept { return this->rfind(__str, __pos, string_info::str_length(__str)); }

		constexpr size_type find_first_of(const_pointer __str, size_type __pos, size_type __n) const noexcept {
			assert(__pos < size());
			return detail::find_first_of(data() + __pos, size() - __pos, __str, __n);
		}
		template<typename T>
		constexpr size_type find_first_of(string_helper<T> __str, size_type __pos = 0) const noexcept { return this->find_first_of(__str.data(), __pos, __str.size()); }
		constexpr size_type find_first_of(value_type __c, size_type __pos = 0) const noexcept { return this->find(__c, __pos); }
		constexpr size_type find_first_of(const_pointer __str, size_type __pos = 0) const noexcept { return this->find_first_of(__str, __pos, string_info::str_length(__str)); }

		constexpr size_type  find_last_of(const_pointer __str, size_type __pos, size_type __n) const noexcept {
			assert(__pos < size());
			return detail::find_last_of(data() + __pos, size() - __pos, __str, __n);
		}
		template<typename T>
		constexpr size_type find_last_of(string_helper<T> __str, size_type __pos = 0) const noexcept { return this->find_last_of(__str.data(), __pos, __str.size()); }
		constexpr size_type find_last_of(value_type __c, size_type __pos = 0) const noexcept { return this->rfind(__c, __pos); }
		constexpr size_type find_last_of(const_pointer __str, size_type __pos = 0) const noexcept { return this->find_last_of(__str, __pos, string_info::str_length(__str)); }

		constexpr size_type find_first_not_of(const_pointer __str, size_type __pos, size_type __n) const noexcept {
			assert(__pos < size());
			return detail::find_first_not_of(data() + __pos, size() - __pos, __str, __n);
		}
		constexpr size_type find_first_not_of(value_type __c, size_type __pos) const noexcept {
			assert(__pos < size());
			return detail::find_first_not_of(data() + __pos, size() - __pos, &c, 1); // should optimize this
		}
		template<typename T>
		constexpr size_type find_first_not_of(string_helper<T> __str, size_type __pos = 0) const noexcept { return this->find_first_not_of(__str.data(), __pos, __str.size()); }
		constexpr size_type find_first_not_of(const_pointer __str, size_type __pos = 0) const noexcept { return this->find_first_not_of(__str, __pos, string_info::str_length(__str)); }


		constexpr size_type find_last_not_of(const_pointer __str, size_type __pos, size_type __n) const noexcept {
			assert(__pos < size());
			return detail::find_last_not_of(data() + __pos, size() - __pos, __str, __n);
		}
		constexpr size_type find_last_not_of(value_type __c, size_type __pos) const noexcept {
			assert(__pos < size());
			return detail::find_last_not_of(data() + __pos, size() - __pos, &c, 1); // should optimize this
		}
		template<typename T>
		constexpr size_type find_last_not_of(string_helper<T> __str, size_type __pos = 0) const noexcept { return this->find_last_not_of(__str.data(), __pos, __str.size()); }
		constexpr size_type find_last_not_of(const_pointer __str, size_type __pos = 0) const noexcept { return this->find_last_not_of(__str, __pos, string_info::str_length(__str)); }

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
		template<typename U>	constexpr bool operator==(const string_helper<U>& other) const { return size() == other.size() && (data() == other.data() || compare(other) == 0); }
		template<typename U>	constexpr bool operator!=(const string_helper<U>& other) const { return !(*this == other); }
		template<typename U>	constexpr bool operator<(const string_helper<U>& other) const { return compare(other) < 0;}
		template<typename U>	constexpr bool operator>(const string_helper<U>& other) const { return compare(other) > 0;}
		template<typename U>	constexpr bool operator<=(const string_helper<U>& other) const { return compare(other) <= 0;}
		template<typename U>	constexpr bool operator>=(const string_helper<U>& other) const { return compare(other) >= 0; }
		constexpr bool operator==(const base_type& other) const { return size() == other.size() && (data() == other.data() || compare(other) == 0); }
		constexpr bool operator!=(const base_type& other) const { return !(*this == other); }
		constexpr bool operator<(const base_type& other) const { return compare(other) < 0; }
		constexpr bool operator>(const base_type& other) const { return compare(other) > 0; }
		constexpr bool operator<=(const base_type& other) const { return compare(other) <= 0; }
		constexpr bool operator>=(const base_type& other) const { return compare(other) >= 0; }
	};
	template<typename U>
	constexpr bool operator==(const U& l, const string_helper<U>& r) { return l.size() == r.size() && (l.data() == r.data() || l.compare(r.other) == 0); }
	template<typename U>
	constexpr bool operator!=(const U& l, const string_helper<U>& r) { return !(l != r); }

	// now that we have the basic functions, lets make our shit string view
	class string_view : public string_helper<string_view> {
	public:

		constexpr string_view() : _data(""), _size(0U) { };
		// is below to much?
		constexpr string_view(const_pointer p, size_t s) : _data(p == nullptr || s == 0U ? "" : p), _size(p == nullptr || s == 0U ? 0 : s) {}
		constexpr string_view(const_pointer p) : string_view(p, detail::str_length(p)) {}
		template<typename U>
		constexpr string_view(const string_helper<U>& p) : string_view(p.data(),p.size()) {}

		constexpr const_pointer data() const { return _data; }
		constexpr size_type size() const { return _size; }
		constexpr inline string_view substr(size_type pos = 0, size_type count = npos) const {
			assert(pos < size());
			return string_view(data() + pos, std::min(count, size() - pos));
		}
		constexpr void remove_prefix(size_type n)  {
			assert(n < size());
			_size -= n;
			_data += n;
		}
		//template<typename U = base_type>
		constexpr void remove_suffix(size_type n)  {
			assert(n < size());
			_size -= n;
		}
	private:
		const char* _data;
		size_t _size;
	};
	// now that we have the basic functions, lets make our shit string view
	class cstring : public string_helper<cstring> {
	public:

		constexpr cstring() : _data("") { };
		// is below to much?
		constexpr cstring(const_pointer p) : _data(p) {}
		template<typename U>
		constexpr cstring(const string_helper<U>& p) : cstring(p.data()) {
			if (p.data()[p.size()] != '\0') throw std::length_error("cstring string not zero terminated!");
		}

		constexpr const_pointer data() const { return _data; }
		constexpr size_type size() const { return detail::str_length(_data); }
		const char* c_str() const { return _data; }
		constexpr inline string_view substr(size_type pos = 0, size_type count = npos) const {
			assert(pos < size());
			return string_view(data() + pos, std::min(count, size() - pos));
		}
	private:
		const char* _data;
	};
	// static vector
	// mainly for debug, but we can also use it for hulk
	// cannot be resized
	template <typename T>
	class fixed_array {
	public:
		using value_type = T;
		using  size_type = size_t;
		using diffrence_type = ptrdiff_t;
		using pointer = value_type*;
		using const_pointer = const pointer;
		using reference = value_type&;
		using const_reference = const pointer;
		using iterator = pointer;
		using const_iterator = const_pointer;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		static constexpr size_type npos = std::numeric_limits<size_type>::max();
	public:
#if HAVE_CPP11
		using initlist_t = std::initializer_list<value_type>;
		inline array&		operator+= (initlist_t v) { for (size_type i = 0; i < N; ++i) _v[i] += v.begin()[i]; return *this; }
		inline array&		operator-= (initlist_t v) { for (size_type i = 0; i < N; ++i) _v[i] -= v.begin()[i]; return *this; }
		inline array&		operator*= (initlist_t v) { for (size_type i = 0; i < N; ++i) _v[i] *= v.begin()[i]; return *this; }
		inline array&		operator/= (initlist_t v) { for (size_type i = 0; i < N; ++i) _v[i] /= v.begin()[i]; return *this; }
		inline array		operator+ (initlist_t v) const { array result; for (size_type i = 0; i < N; ++i) result[i] = _v[i] + v.begin()[i]; return result; }
		inline array		operator- (initlist_t v) const { array result; for (size_type i = 0; i < N; ++i) result[i] = _v[i] - v.begin()[i]; return result; }
		inline array		operator* (initlist_t v) const { array result; for (size_type i = 0; i < N; ++i) result[i] = _v[i] * v.begin()[i]; return result; }
		inline array		operator/ (initlist_t v) const { array result; for (size_type i = 0; i < N; ++i) result[i] = _v[i] / v.begin()[i]; return result; }
#endif
		inline void manage(void * ptr, size_t element_count, bool init_all = false) {
			_data = reinterpret_cast<pointer>(ptr);
			_size = element_count;
			if (init_all) {
				for (reference a : *this) {
					new(std::addressof(a)) value_type;
				}
			}
		}
		inline					fixed_array() : _data(nullptr), _size(0U) {}
		inline					fixed_array(void* data, size_t element_count,bool init_all=false) : _data(reinterpret_cast<pointer>(data)), _size(element_count) {
			for (reference a : *this) {
				new(std::addressof(a)) value_type;
			}
		}
		inline					fixed_array(const fixed_array& copy) : _data(copy._data), _size(copy._size) {} // not sure if this is a good idea
		inline					fixed_array(fixed_array&& move) : _data(move._data), _size(move._size) { move._data = nullptr; move._size = 0U; } 

		inline iterator		begin(void) { return _data; }
		inline iterator		end(void) { return begin() + _size; }
		inline reference		at(size_type i) { return _data[i]; }
		inline reference		operator[] (size_type i) { return _data[i]; }
		inline constexpr const_iterator	begin(void) const { return _data; }
		inline constexpr const_iterator	end(void) const { return begin() + _size; }
		inline constexpr size_type		size(void) const { return _size; }
		inline constexpr size_type		max_size(void) const { return _size(); }
		inline constexpr bool		empty(void) const { return size() == 0; }
		inline constexpr const_reference	at(size_type i) const { return _data[i]; }
		inline constexpr const_reference	operator[] (size_type i) const { return _data[i]; }
		inline void			fill(const_reference v) { std::fill(begin(), end(), v); }
		inline void			swap(fixed_array& v) {
			if (std::addressof(v) != this) { std::swap(_data, v._data); std::swap(_size, v._size); }
		}
		inline size_type indexof(const_iterator it) const {
			return (it >= begin() && it < end()) ? std::distance(begin(),it): npos;
		}
	public:
		T*	_data;
		size_type _size;
	};

}

