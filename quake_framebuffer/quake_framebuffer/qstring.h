#pragma once


#include <array>
#include <ostream>
#include <istream>
#include <string_view>
#include <string>

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif


namespace quake {
	namespace string_info {
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

		inline static size_type str_hash(const_pointer first, const_pointer last) noexcept {
			size_type h = 0;
			// This has the bits flowing into each other from both sides of the number
			for (; first < last; ++first)
				h = static_cast<difference_type>(*first) + ((h << 7) | (h >> ((sizeof(size_type) * 8) - 7)));
			return h;
		}
		inline static  int str_length_compare(size_type __n1, size_type __n2) noexcept {
			const difference_type __diff = __n1 - __n2;
			if (__diff > std::numeric_limits<int>::max()) return std::numeric_limits<int>::max();
			if (__diff < std::numeric_limits<int>::min()) return std::numeric_limits<int>::min();
			return static_cast<int>(__diff);
		}
		inline static   int str_compare(const_pointer __str1, size_type __n1, const_pointer __str2, size_type __n2)  noexcept {
			int __diff = str_length_compare(__n1, __n2);
			if (__diff == 0) __diff = traits_type::compare(__str1, __str2, __n1);
			//	if (__diff == 0) for (size_t i = 0; i < __n1 && (__diff = int(__str1[i]) - int(__str2[i])) == 0; i++);
			return __diff;
		}
		inline static   int str_case_compare(const_pointer __str1, size_type __n1, const_pointer __str2, size_type __n2)  noexcept {
			int __diff = str_length_compare(__n1, __n2);
			if (__diff == 0) for (size_t i = 0; i < __n1 && (__diff = int(char_tolower(__str1[i])) - int(char_tolower(__str2[i]))) == 0; i++);
			return __diff;
		}
		inline static  size_type str_find(const_pointer __str1, size_type __n1, value_type c) noexcept {
			const_pointer p = traits_type::find(__str1, __n1, c);
			return p ? p - __str1 : npos;
		}
		inline static  size_type str_find(const_pointer __str1, size_type __n1, const_pointer __str2, size_type __n2) noexcept {
			if (__n2 == 0) return 0;
			if (__n2 <= __n1) {
				for (size_t pos = 0; pos <= (__n1 - __n2); ++pos)
					if (traits_type::eq(__str1[pos], __str2[0])
						&& traits_type::compare(__str1 + pos + 1,
							__str2 + 1, __n2 - 1) == 0)
						return pos;
			}
			return npos;
		}

	}
	// basic overrides



	class cstring_container {
	public:
		using value_type = char;
		using traits_type = std::char_traits<value_type>;
		constexpr const char* data() const { return _str; }
		constexpr size_t size() const { return _str == nullptr ? 0U : string_info::str_length(_str); }
		constexpr const char* c_str() const { return _str; }
		constexpr cstring_container() : _str(nullptr) {}
		constexpr cstring_container(const char* p) : _str(p) {}
	private:
		const char* _str;
	};

	class cstring_size_container {
	public:
		using value_type = char;
		using traits_type = std::char_traits<value_type>;
		constexpr const char* data() const { return _str; }
		constexpr size_t size() const { return _size; }
		constexpr cstring_size_container() : _str(nullptr), _size(0U) {}
		constexpr cstring_size_container(const char* p) : _str(p), _size(string_info::str_length(p)) {}
		constexpr cstring_size_container(const char* p, size_t s) : _str(s == 0U ? nullptr : p), _size(p == nullptr ? 0U : s) {}
	private:
		const char* _str;
		size_t _size;
	};

	class fixed_string_container {
		char* _buffer;
		size_t _size;
		size_t _capacity;
	public:
		using value_type = char;
		using traits_type = std::char_traits<value_type>;
		inline const char* data() const { return _buffer; }
		inline char* data() { return _buffer; }
		inline size_t size() const { return _size; }
		inline size_t capacity() const { return _capacity; }
		inline void reserve(size_t s) { assert(s <= _capacity); }
		inline void resize(size_t s) { assert(s <= _capacity); _size = s; }
		inline void manage(char* buffer, size_t size) { _buffer = buffer; _capacity = size; _size = 0U; } // be sure to clear this 
		inline fixed_string_container() : _buffer(nullptr), _size(0U), _capacity(0U) {}
		inline fixed_string_container(char* buffer, size_t size) : _buffer(buffer), _size(0U), _capacity(size) {}

	};
	template<size_t N>
	class static_string_container {
		char _buffer[N];
		size_t _size;
	public:
		using value_type = char;
		using traits_type = std::char_traits<value_type>;
		inline const char* data() const { return _buffer; }
		inline char* data() { return _buffer; }
		inline size_t size() const { return _size; }
		constexpr inline size_t capacity() const { return N; }
		inline void reserve(size_t s) { assert(s <= capacity()); }
		inline void resize(size_t s) { assert(s <= capacity()); _size = s; }
		inline static_string_container() : _size(0U) {}

	};
	class memalloc_string_container {
		char* _buffer;
		size_t _size;
		size_t _capacity;
	public:
		using value_type = char;
		using traits_type = std::char_traits<value_type>;
		inline  const char* data() const { return _buffer; }
		inline  char* data() { return _buffer; }
		inline  size_t size() const { return _size; }
		inline  size_t capacity() const { return _capacity; }
		inline void reserve(size_t s) { _buffer = (char*)realloc(_buffer, _capacity = s); assert(_buffer); }
		inline void resize(size_t s) { assert(s <= _capacity); _size = s; }
		inline void manage(char* buffer, size_t size) { _buffer = buffer; _capacity = size; _size = 0U; } // be sure to clear this 
		inline memalloc_string_container() : _buffer((char*)malloc(1)), _size(0U), _capacity(1U) { assert(_buffer); }
		// have to handle copys because of pointers
		inline memalloc_string_container(const memalloc_string_container& copy) :
			_buffer((char*)malloc(copy.capacity())), _size(copy.size()), _capacity(copy.capacity()) {
			::memcpy(_buffer, copy._buffer, copy._capacity);
		}
		inline memalloc_string_container(memalloc_string_container&& move) : _buffer(nullptr), _size(0U), _capacity(0U) {
			std::swap(_capacity, move._capacity); std::swap(_size, move._size); std::swap(_buffer, move._buffer);
		}
		inline ~memalloc_string_container() {
			if (_buffer) free(_buffer);
		}

	};

	template<typename BASE> class string_builder;

	template<typename _BASE>
	class string_helper : public _BASE {
	public:
		using base_type = _BASE;
		using value_type = typename base_type::value_type;
		using char_info = string_info::char_info<value_type, true>;
		using traits_type = typename char_info::traits_type;
		using difference_type = typename char_info::difference_type;
		using size_type = typename char_info::size_type;
		using const_pointer = typename char_info::const_pointer;
		using pointer = typename char_info::pointer;
		using const_reference = typename char_info::const_reference;
		using reference = typename char_info::reference;
		using const_iterator = typename char_info::const_iterator;
		using iterator = typename char_info::iterator;
		using const_reverse_iterator = typename char_info::const_reverse_iterator;
		using reverse_iterator = typename char_info::reverse_iterator;
		static constexpr size_type npos = char_info::npos;
	private:
		friend string_builder<_BASE>;
		template<typename U>
		using is_data_size_constructable = std::is_constructible<U, const_pointer, size_type>;
		template<typename U>
		using is_c_str_constructable = std::is_constructible<U, const_pointer>;
		static constexpr bool is_base_data_size_constructable = is_data_size_constructable<base_type>::value;
		static constexpr bool is_base_c_ptr_constructable = is_c_str_constructable<base_type>::value;


		static constexpr int _S_compare(size_type __n1, size_type __n2) noexcept {
			const difference_type __diff = __n1 - __n2;
			if (__diff > std::numeric_limits<int>::max())
				return std::numeric_limits<int>::max();
			if (__diff < std::numeric_limits<int>::min())
				return std::numeric_limits<int>::min();
			return static_cast<int>(__diff);
		}
		constexpr size_type _M_check(size_type __pos, const char* __s) const noexcept(false) {
			if (__pos > this->size())
				std::out_of_range(
					//	"%s: __pos (which is %zu) > "
					//	"this->size() (which is %zu)"),
					//	__s, __pos, this->size()
				);
			return __pos;
		}
		// NB: _M_limit doesn't check for a bad __pos value.
		constexpr size_type _M_limit(size_type __pos, size_type __off) const noexcept {
			const bool __testoff = __off < this->size() - __pos;
			return __testoff ? __off : this->size() - __pos;
		}

	public:
		// from base
		constexpr inline const_pointer data() const { return base_type::data(); }
		constexpr inline size_type size() const { return base_type::size(); }
		constexpr inline size_type length() const { return base_type::size(); }

		constexpr string_helper() noexcept : base_type() { }
		template<typename U = base_type, typename std::enable_if<is_data_size_constructable<U>::value>::type...>
		constexpr string_helper(const_pointer p, size_type s) noexcept : base_type(s == 0U ? nullptr : p, p == nullptr ? 0 : s) {}
		template<typename U = base_type, typename std::enable_if<is_c_str_constructable<U>::value>::type...>
		constexpr string_helper(const_pointer p) noexcept : base_type(p) {}

		string_helper& operator=(const string_helper&) noexcept = default;

		template<typename U>
		constexpr string_helper(const string_helper<U>& h) noexcept : string_helper(h.data(), h.size()) {}


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
			return string_info::str_compare(data(), std::min(__n1, size() - __pos1), __str, __n2);
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
			size_type ret = npos;
			if (pos < size()) {
				const size_type n = size() - pos;
				const_pointer p = traits_type::find(data() + pos, n, c);
				if (p) ret = p - data();
			}
			return ret;
		}
		constexpr  inline size_type find(const_pointer p, size_type pos, size_type n) const {
			if (__n == 0) return __pos <= size() ? pos : npos;
			if (__n <= size()) {
				for (; pos <= size() - n; ++pos)
					if (traits_type::eq(data()[__pos], p[0])
						&& traits_type::compare(data() + pos + 1,
							p + 1, n - 1) == 0)
						return pos;
			}
			return npos;
		}
		constexpr size_type find(const_pointer __str, size_type __pos = 0) const noexcept { return this->find(__str, __pos, string_info::str_length(__str)); }
		template<typename T>
		constexpr size_type find(string_helper<T> __str, size_type __pos = 0) const noexcept { return this->find(__str.data(), __pos, __str.size()); }

		inline size_type	rfind(const_pointer p, size_type pos, size_type n) const {
			if (n <= size()) {
				pos = std::min(size_type(size() - n), pos);
				do {
					if (traits_type::compare(data() + pos, p, n) == 0) return pos;
				} while (pos-- > 0);
			}
			return npos;
		}
		constexpr size_type rfind(value_type c, size_type pos = 0) const noexcept {
			size_type n = size();
			if (n > 0) {
				if (--n > pos)
					n = pos;
				for (++n; n-- > 0; )
					if (traits_type::eq(data()[n], c))
						return size();
			}
			return npos;
		}
		template<typename T>
		constexpr size_type rfind(string_helper<T> __str, size_type __pos = 0) const noexcept { return this->rfind(__str.data(), __pos, __str.size()); }
		constexpr size_type rfind(const_pointer __str, size_type __pos = 0) const noexcept { return this->rfind(__str, __pos, string_info::str_length(__str)); }

		constexpr size_type find_first_of(const_pointer __str, size_type __pos, size_type __n) const noexcept {
			for (; __n && __pos < this->size(); ++__pos)
			{
				const_pointer __p = traits_type::find(__str, __n, this->data()[__pos]);
				if (__p) return __pos;
			}
			return npos;
		}
		template<typename T>
		constexpr size_type find_first_of(string_helper<T> __str, size_type __pos = 0) const noexcept { return this->find_first_of(__str.data(), __pos, __str.size()); }
		constexpr size_type find_first_of(value_type __c, size_type __pos = 0) const noexcept { return this->find(__c, __pos); }
		constexpr size_type find_first_of(const_pointer __str, size_type __pos = 0) const noexcept { return this->find_first_of(__str, __pos, string_info::str_length(__str)); }

		constexpr size_type  find_last_of(const_pointer __str, size_type __pos, size_type __n) const noexcept {
			size_type __size = this->size();
			if (__size && __n)
			{
				if (--__size > __pos) __size = __pos;
				do {
					if (traits_type::find(__str, __n, this->data()[__size]))
						return __size;
				} while (__size-- != 0);
			}
			return npos;
		}
		template<typename T>
		constexpr size_type find_last_of(string_helper<T> __str, size_type __pos = 0) const noexcept { return this->find_last_of(__str.data(), __pos, __str.size()); }
		constexpr size_type find_last_of(value_type __c, size_type __pos = 0) const noexcept { return this->rfind(__c, __pos); }
		constexpr size_type find_last_of(const_pointer __str, size_type __pos = 0) const noexcept { return this->find_last_of(__str, __pos, string_info::str_length(__str)); }

		constexpr size_type find_first_not_of(const_pointer __str, size_type __pos, size_type __n) const noexcept {
			for (; __pos < this->size(); ++__pos)
				if (!traits_type::find(__str, __n, this->data()[__pos]))
					return __pos;
			return npos;
		}
		constexpr size_type find_first_not_of(value_type __c, size_type __pos) const noexcept
		{
			for (; __pos < this->size(); ++__pos)
				if (!traits_type::eq(this->data()[__pos], __c))
					return __pos;
			return npos;
		}
		template<typename T>
		constexpr size_type find_first_not_of(string_helper<T> __str, size_type __pos = 0) const noexcept { return this->find_first_not_of(__str.data(), __pos, __str.size()); }
		constexpr size_type find_first_not_of(const_pointer __str, size_type __pos = 0) const noexcept { return this->find_first_not_of(__str, __pos, string_info::str_length(__str)); }


		constexpr size_type find_last_not_of(const_pointer __str, size_type __pos, size_type __n) const noexcept {
			size_type __size = this->size();
			if (__size)
			{
				if (--__size > __pos)
					__size = __pos;
				do
				{
					if (!traits_type::find(__str, __n, this->_M_str[__size]))
						return __size;
				} while (__size--);
			}
			return npos;
		}
		constexpr size_type find_last_not_of(value_type __c, size_type __pos) const noexcept {
			size_type __size = this->size();
			if (__size)
			{
				if (--__size > __pos)
					__size = __pos;
				do
				{
					if (!traits_type::eq(this->data()[__size], __c))
						return __size;
				} while (__size--);
			}
			return npos;
		}
		template<typename T>
		constexpr size_type find_last_not_of(string_helper<T> __str, size_type __pos = 0) const noexcept { return this->find_last_not_of(__str.data(), __pos, __str.size()); }
		constexpr size_type find_last_not_of(const_pointer __str, size_type __pos = 0) const noexcept { return this->find_last_not_of(__str, __pos, string_info::str_length(__str)); }

		//template<typename U = base_type>
		//typename std::enable_if<std::is_constructible<U, const_pointer, size_type, string_helper>::value>::type
		//typename std::conditional<std::is_constructible<U, const_pointer, size_type, string_helper>::value


		using string_view = string_helper<cstring_size_container>;
		//template<typename U = base_type>
		inline string_view substr(size_type pos = 0, size_type count = npos) const {
			//	using return_type = typename std::conditional<std::is_constructible<U, const_pointer, size_type>::value, U, string_helper<cstring_size_container>>::type;
			assert(pos < size());
			return string_view(data() + pos, std::min(count, size() - pos));
		}
		//template<typename U = base_type>
		inline string_view remove_prefix(size_type n) const {
			//using return_type = typename std::conditional<std::is_constructible<U, const_pointer, size_type>::value, U, string_helper<cstring_size_container>>::type;
			assert(n < size());
			return string_view(data() + n, size() - n);
		}
		//template<typename U = base_type>
		inline string_view remove_suffix(size_type n) const {
			//	using return_type = typename std::conditional<std::is_constructible<U, const_pointer, size_type>::value, U, string_helper<cstring_size_container>>::type;
			assert(n < size());
			return string_view(data(), size() - n);
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
	};

	namespace __detail
	{
		//  Identity transform to make ADL work with just one argument.
		//  See n3766.html.
		template<typename _Tp = void>
		struct __identity { typedef _Tp type; };
		template<> struct __identity<void>;

		template<typename _Tp>
		using __idt = typename __identity<_Tp>::type;

		// Identity transform to create a non-deduced context, so that only one
		// argument participates in template argument deduction and the other
		// argument gets implicitly converted to the deduced type. See n3766.html.
		//template<typename _Tp> using __idt = std::common_type_t<_Tp>;
	}

	template<typename T1, typename T2>
	constexpr bool operator==(const string_helper<T1>& __x, const string_helper<T2>& __y) noexcept { return __x.size() == __y.size() && (__x.data() == __y.data() || __x.compare(__y) == 0); }
	template<typename T1, typename T2>
	constexpr bool operator==(const string_helper<T1>& __x, __detail::__idt<string_helper<T2>> __y) noexcept { return __x.size() == __y.size() && (__x.data() == __y.data() || __x.compare(__y) == 0); }
	template<typename T1, typename T2>
	constexpr bool operator==(__detail::__idt<string_helper<T1>> __x, const string_helper<T2>& __y) noexcept { return __x.size() == __y.size() && (__x.data() == __y.data() || __x.compare(__y) == 0); }
	template<typename T1, typename T2>
	constexpr bool operator!=(const string_helper<T1>& __x, const string_helper<T2>& __y) noexcept { return !(__x == __y); }
	template<typename T1, typename T2>
	constexpr bool operator!=(const string_helper<T1>& __x, __detail::__idt<string_helper<T2>> __y) noexcept { return !(__x == __y); }
	template<typename T1, typename T2>
	constexpr bool operator!=(__detail::__idt<string_helper<T1>> __x, const string_helper<T2> &__y) noexcept { return !(__x == __y); }
#define COMPARE_OPS(OP) \
	template<typename T1, typename T2> \
		constexpr bool operator##OP##(const string_helper<T1> __x, const string_helper<T2>& __y) noexcept { return __x.compare(__y) OP 0; } \
	template<typename T1, typename T2> \
	constexpr bool operator##OP##(const string_helper<T1>& __x, __detail::__idt<string_helper<T2>> __y) noexcept { return __x.compare(__y) OP 0; } \
	template<typename T1, typename T2> \
	constexpr bool operator##OP##(__detail::__idt<string_helper<T1>> __x, const string_helper<T2>& __y) noexcept { return __x.compare(__y) OP 0; } 
	COMPARE_OPS(< )
		COMPARE_OPS(> )
		COMPARE_OPS(<= )
		COMPARE_OPS(>= )
#undef COMPARE_OPS
		template<typename T1>
	inline std::ostream& operator<<(std::ostream& os, string_helper<T1> sh) {
		os.write(sh.data(), sh.size());
		return os;
	}
	using cstring = string_helper<cstring_container>;
	using string_view = string_helper<cstring_size_container>;


	// now for string
	template<typename _BASE>
	class string_builder : public string_helper<_BASE> {

	public:
		using base_type = _BASE;
		using helper = string_helper<_BASE>;
		using value_type = typename base_type::value_type;
		using char_info = string_info::char_info<value_type, false>;
		using traits_type = typename char_info::traits_type;
		using difference_type = typename char_info::difference_type;
		using size_type = typename char_info::size_type;
		using const_pointer = typename char_info::const_pointer;
		using pointer = typename char_info::pointer;
		using const_reference = typename char_info::const_reference;
		using reference = typename char_info::reference;
		using const_iterator = typename char_info::const_iterator;
		using iterator = typename char_info::iterator;
		using const_reverse_iterator = typename char_info::const_reverse_iterator;
		using reverse_iterator = typename char_info::reverse_iterator;
		static constexpr size_type npos = char_info::npos;
	private:
		template<typename U>
		using is_data_size_constructable = std::is_constructible<U, const_pointer, size_type>;
		template<typename U>
		using is_c_str_constructable = std::is_constructible<U, const_pointer>;
		static constexpr bool is_base_data_size_constructable = is_data_size_constructable<base_type>::value;
		static constexpr bool is_base_c_ptr_constructable = is_c_str_constructable<base_type>::value;


		static constexpr int _S_compare(size_type __n1, size_type __n2) noexcept {
			const difference_type __diff = __n1 - __n2;
			if (__diff > std::numeric_limits<int>::max())
				return std::numeric_limits<int>::max();
			if (__diff < std::numeric_limits<int>::min())
				return std::numeric_limits<int>::min();
			return static_cast<int>(__diff);
		}
		constexpr size_type _M_check(size_type __pos, const char* __s) const noexcept(false) {
			if (__pos > this->size())
				std::out_of_range(
					//	"%s: __pos (which is %zu) > "
					//	"this->size() (which is %zu)"),
					//	__s, __pos, this->size()
				);
			return __pos;
		}
		// NB: _M_limit doesn't check for a bad __pos value.
		constexpr size_type _M_limit(size_type __pos, size_type __off) const noexcept {
			const bool __testoff = __off < this->size() - __pos;
			return __testoff ? __off : this->size() - __pos;
		}
		void _insert_hole(const_iterator cstart, size_type n) {
			assert(data() || !n);
			assert(begin() || !n);
			assert(cstart >= begin() && cstart + n <= end());
			iterator start = const_cast<iterator>(cstart);
			std::rotate(start, end() - n, end());
		}
		void _erase_hole(const_iterator cstart, size_type n) {
			assert(data() || !n);
			assert(begin() || !n);
			assert(cstart >= begin() && cstart + n <= end());
			iterator start = const_cast<iterator>(cstart);
			std::rotate(start, start + n, end());
		}
		/// Shifts the data in the linked block from \p start + \p n to \p start.
		iterator erase_hole(const_iterator start, size_type n)
		{
			const size_type ep = start - begin();
			assert(ep + n <= size());
			reserve(size() - n);
			iterator iep = begin() + ep;
			_erase_hole(iep, n);
			resize(size() - n);
			return iep;
		}
		/// Shifts the data in the linked block from \p start to \p start + \p n.
		iterator insert_hole(const_iterator start, size_type n) {
			const size_type ip = start - begin();
			assert(ip <= size());
			resize(size() + n);
			const_iterator start = const_cast<const_iterator>(begin() + ip);
			_insert_hole(start, n);
			return start;
		}

		template<typename U>
		struct is_convertable_to_view { // quick hack
			static constexpr bool value = std::is_convertible<const U&, const string_view&>::value && !std::is_convertible<const U&, const_pointer>::value;
		};
	public:


		// from base
		inline pointer data() { return base_type::data(); }
		inline const_pointer data() const { return base_type::data(); }
		inline size_type size() const { return base_type::size(); }
		inline size_type length() const { return base_type::size(); }
		inline size_type capacity() const { return base_type::capacity(); }
		inline const char* c_str() const { return data(); }
		inline void reserve(size_type s) {
			if (capacity() < (s + 1))
				return base_type::reserve(s + 1);
		}
		inline void resize(size_type s) {
			reserve(s);
			base_type::resize(s);
			data()[s] = '\0'; // reserve always make sure we have space for this
		}
		void clear() { resize(0U); }
		// all assigns
		inline string_builder& assign(const_pointer p1, size_t s1) {
			resize(s1);
			::memcpy(data(), p1, s1);
			return *this;
		}
		inline string_builder& assign(size_type count, value_type c) {
			resize(count);
			::memset(data(), c, s1); // should fix this for wchar_t
			return *this;
		}
		inline string_builder& assign(const_pointer p1) { return assign(p1, traits_type::length(p1)); }
		// iterate assign
		template<typename ITR>
		inline string_builder& assign(ITR s1, ITR s2) {
			assert(s1 < s2);
			resize(std::distance(s1, s2));
			std::uninitialized_copy(s1, s2, begin());
			return *this;
		}
		inline string_builder& assign(std::initializer_list<value_type> l) {
			resize(l.size());
			std::uninitialized_copy(l.begin(), l.end(), begin());
			return *this;
		}

		template<typename U>
		inline string_builder& assign(const string_builder<U>& h, size_type pos = 0U, size_type count = npos) {
			assert(pos < h.size());
			return assign(h.data() + pos, std::min(h.size(), count - pos));
		}
		template<typename U>
		inline string_builder& assign(const string_helper<U>& h) { return assign(h.data(), h.size()); }

		template<typename U>
		typename std::enable_if<is_convertable_to_view<U>::value, string_builder&>::type
			inline  assign(const U& hh, size_type pos, size_type count = npos) {
			const string_view h(h);
			assert(pos < h.size());
			return assign(h.data() + pos, std::min(h.size(), count - pos));
		}
		// all appeneds
		inline string_builder& append(const_pointer p1, size_t s1) {
			resize(s1 + size());
			::memcpy(data() + size(), p1, s1);
			return *this;
		}
		inline string_builder& append(size_type count, value_type c) {
			resize(count + size());
			::memset(data() + size(), c, s1); // should fix this for wchar_t
			return *this;
		}
		inline string_builder& append(const_pointer p1) { return assign(p1, traits_type::length(p1)); }
		// iterate assign
		template<typename ITR>
		inline string_builder& append(ITR s1, ITR s2) {
			assert(s1 < s2);
			resize(std::distance(s1, s2) + size());
			std::uninitialized_copy(s1, s2, begin() + size());
			return *this;
		}
		inline string_builder& append(std::initializer_list<value_type> l) {
			resize(l.size() + size());
			std::uninitialized_copy(l.begin(), l.end(), begin() + size());
			return *this;
		}

		template<typename U>
		inline string_builder& append(const string_builder<U>& h, size_type pos = 0U, size_type count = npos) {
			assert(pos < h.size());
			return assign(h.data() + pos, std::min(h.size(), count - pos));
		}
		template<typename U>
		inline string_builder& append(const string_helper<U>& h) {
			return assign(h.data(), h.size());
		}

		template<typename U>
		typename std::enable_if<is_convertable_to_view<U>::value, string_builder&>::type
			inline  append(const U& hh, size_type pos, size_type count = npos) {
			const string_view h(h);
			assert(pos < h.size());
			return assign(h.data() + pos, std::min(h.size(), count - pos));
		}
		// we don't cover all types
		string_builder& insert(size_type pos, value_type ch) {
			auto it = insert_hole(begin() + pos, 1);
			*it = ch;
			return it;
		}
		iterator insert(const_iterator pos, value_type ch) {
			auto it = insert_hole(pos, 1);
			*it = ch;
			return it;
		}
		void insert(const_iterator pos, size_type count, value_type ch) {
			if (count > 0) {
				auto it = insert_hole(pos, count);
				::memset(it, ch, count); // hack
			}
		}

		template< class ITR >
		void insert(const_iterator pos, ITR first, ITR last) {
			const size_type len = std::distance(first, last);
			auto it = insert_hole(pos, len);
			std::uninitialized_copy(first, last, it);
		}
		template<typename U>
		string_builder& insert(size_type pos, const string_helper<U>& sv) {
			const size_type len = std::distance(first, last);
			auto it = insert_hole(pos, sv.size());
			std::uninitialized_copy(sv.begin(), sv.end(), it);
			return *this;

		}
		template<typename T>
		typename std::enable_if<is_convertable_to_view<T>::value, string_builder&>::type
			insert(size_type index, const T& t, size_type pos, size_type count = npos) {
			const string_view h(t);
			assert(pos < t.size());
			count = std::min(count, h.size() - pos);
			auto it = insert_hole(pos, count);
			std::uninitialized_copy(h.begin(), h.end(), it);
			return *this;
		}
		string_builder& erase(size_type index = 0, size_type count = npos) {
			count = std::min(size(), count - index);
			auto it = erase_hole(begin() + index, count);
			return *this;
		}
		iterator erase(const_iterator position) {
			auto it = erase_hole(position, 1);
			return it;
		}
		iterator erase(const_iterator first, const_iterator last) {
			const size_type count = std::distance(first, last);
			auto it = erase_hole(first, count);
			return it;
		}
		void push_back(value_type ch) {
			*end() = ch;
			resize(size() + 1);
		}
		void pop_back() {
			if (size() > 0) resize(size() - 1);
		}
		// replaces just a few of them
		string_builder& replace(const_iterator first, const_iterator last, const_pointer i1, const_pointer i2, size_type n) {
			assert(first <= last);
			assert(n || std::distance(first, last));
			assert(first >= begin() && first <= end() && last >= first && last <= end());
			//assert((i1 < begin() || i1 >= end() || abs_distance(i1, i2) * n + size() < capacity()) && "Replacement by self can not autoresize");
			const size_type bte = std::distance(first, last);
			const size_type bti = std::distance(i1, i2) * n;
			const_iterator rp = static_cast<const_iterator>(first);
			if (bti < bte)
				rp = erase_hole(rp, bte - bti);
			else if (bte < bti)
				rp = insert_hole(rp, bti - bte);
			std::fill(rp, std::i1, distance(i1, i2), n);
			*end() = 0;
			return *this;
		}
		template <typename InputIt>
		string_builder&	replace(const_iterator first, const_iterator last, InputIt first2, InputIt last2) { return replace(first, last, first2, last2, 1); }
		// operators
		operator string_view() const { return string_view(data(), size()); }

		template<typename U>
		string_builder& operator+=(const string_helper<U>& str) { return append(str); }

		string_builder& operator+=(value_type c) { return append(c); }
		string_builder& operator+=(const char* s) { return append(s); } // humm not needed?


		constexpr string_builder() noexcept : helper() { if (data()) resize(0U); } // make sure its zero terminated
		constexpr string_builder(const_pointer p, size_type s) noexcept : helper() { assign(p, s); }
		constexpr string_builder(const_pointer p) noexcept : helper() { assign(p); }
		template<typename U>
		constexpr string_builder(const string_helper<U>& h) noexcept : helper() { assign(h); }
		template<typename U>
		constexpr string_builder& operator=(const string_helper<U>& h) noexcept { return assign(h); }

		constexpr inline bool empty() const { return size() == 0U; }
		constexpr inline const_iterator begin() const { return data(); }
		constexpr inline const_iterator end() const { return  data() + size(); }

		constexpr inline const_reference front() const { return *data(); }
		constexpr inline const_reference back() const { return  *(data() + size()); }

		constexpr inline const_reference at(size_type pos) const { return *(data() + pos); }
		constexpr inline const_reference operator[](size_type pos) const { return *(data() + pos); }

		inline iterator begin() { return data(); }
		inline iterator end() { return  data() + size(); }

		inline reference front() { return *data(); }
		inline reference back() { return  *(data() + size()); }

		inline reference at(size_type pos) { return *(data() + pos); }
		inline reference operator[](size_type pos) { return *(data() + pos); }




#if 0
		size_type copy(pointer __str, size_type __n, size_type __pos = 0) const {
			assert(_pos);
			const size_type __rlen = std::min(__n, this->size() - __pos);
			for (auto __begin = this->data() + __pos,
				__end = __begin + __rlen; __begin != __end;)
				*__str++ = *__begin++;
			return __rlen;
		}
		constexpr int compare(size_type __pos1, size_type __n1, const_pointer __str, size_type __n2) const noexcept {
			return string_info::str_compare(data(), std::min(__n1, size() - __pos1), __str, __n2);
		}
		template<typename T>
		constexpr int compare(size_type __pos1, size_type __n1, string_helper<T> __str) const { return compare(__pos1, __n1, __str.data(), __str.size()); }
		template<typename T>
		constexpr int compare(size_type __pos1, size_type __n1, basic_string_view<T>__str, size_type __pos2, size_type __n2) const {
			return compare(__pos1, __n1, __str.data() + __pos2, __n2 - __pos2);
		}
		constexpr int compare(size_type __pos1, size_type __n1, const_pointer __str) const { return compare(__pos1, __n1, __str, traits_type::length(__str)); }
		template<typename T>
		constexpr int compare(string_helper<T> __str) const { return compare(0U, size(), __str.data(), __str.size()); }
		constexpr int compare(const_pointer __str) const { return compare(0U, size(), __str, traits_type::length(__str)); }


		constexpr size_type find(value_type c, size_type pos = 0) const noexcept {
			size_type ret = npos;
			if (pos < size()) {
				const size_type n = size() - pos;
				const_pointer p = traits_type::find(data() + pos, n, c);
				if (p) ret = p - data();
			}
			return ret;
		}
		constexpr  inline size_type find(const_pointer p, size_type pos, size_type n) const {
			if (__n == 0) return __pos <= size() ? pos : npos;
			if (__n <= size()) {
				for (; pos <= size() - n; ++pos)
					if (traits_type::eq(data()[__pos], p[0])
						&& traits_type::compare(data() + pos + 1,
							p + 1, n - 1) == 0)
						return pos;
			}
			return npos;
		}
		constexpr size_type find(const_pointer __str, size_type __pos = 0) const noexcept { return this->find(__str, __pos, traits_type::length(__str)); }
		template<typename T>
		constexpr size_type find(string_helper<T> __str, size_type __pos = 0) const noexcept { return this->find(__str.data(), __pos, __str.size()); }
		constexpr size_type find(const_pointer __str, size_type __pos = 0) const noexcept { return this->find(__str, __pos, traits_type::length(__str)); }

		inline size_type	rfind(const_pointer p, size_type pos, size_type n) const {
			if (n <= size()) {
				pos = std::min(size_type(size() - n), pos);
				do {
					if (traits_type::compare(data() + pos, p, n) == 0) return pos;
				} while (pos-- > 0);
			}
			return npos;
		}
		constexpr size_type rfind(value_type c, size_type pos = 0) const noexcept {
			size_type n = size();
			if (n > 0) {
				if (--n > pos)
					n = pos;
				for (++n; n-- > 0; )
					if (traits_type::eq(data()[n], c))
						return __size;
			}
			return npos;
		}
		constexpr size_type rfind(const_pointer __str, size_type __pos = 0) const noexcept { return this->rfind(__str, __pos, traits_type::length(__str)); }
		template<typename T>
		constexpr size_type rtfind(string_helper<T> __str, size_type __pos = 0) const noexcept { return this->rfind(__str.data(), __pos, __str.size()); }
		constexpr size_type rfind(const_pointer __str, size_type __pos = 0) const noexcept { return this->rfind(__str, __pos, traits_type::length(__str)); }

		constexpr size_type find_first_of(const_pointer __str, size_type __pos, size_type __n) const noexcept {
			for (; __n && __pos < this->size(); ++__pos)
			{
				const_pointer __p = traits_type::find(__str, __n, this->data()[__pos]);
				if (__p) return __pos;
			}
			return npos;
		}
		template<typename T>
		constexpr size_type find_first_of(string_helper<T> __str, size_type __pos = 0) const noexcept { return this->find_first_of(__str.data(), __pos, __str.size()); }
		constexpr size_type find_first_of(value_type __c, size_type __pos = 0) const noexcept { return this->find(__c, __pos); }
		constexpr size_type find_first_of(const_pointer __str, size_type __pos = 0) const noexcept { return this->find_first_of(__str, __pos, traits_type::length(__str)); }

		constexpr size_type  find_last_of(const_pointer __str, size_type __pos, size_type __n) const noexcept {
			size_type __size = this->size();
			if (__size && __n)
			{
				if (--__size > __pos) __size = __pos;
				do {
					if (traits_type::find(__str, __n, this->data()[__size]))
						return __size;
				} while (__size-- != 0);
			}
			return npos;
		}
		template<typename T>
		constexpr size_type find_last_of(string_helper<T> __str, size_type __pos = 0) const noexcept { return this->find_last_of(__str.data(), __pos, __str.size()); }
		constexpr size_type find_last_of(value_type __c, size_type __pos = 0) const noexcept { return this->rfind(__c, __pos); }
		constexpr size_type find_last_of(const_pointer __str, size_type __pos = 0) const noexcept { return this->find_last_of(__str, __pos, traits_type::length(__str)); }

		constexpr size_type find_first_not_of(const_pointer __str, size_type __pos, size_type __n) const noexcept {
			for (; __pos < this->size(); ++__pos)
				if (!traits_type::find(__str, __n, this->data()[__pos]))
					return __pos;
			return npos;
		}
		constexpr size_type find_first_not_of(value_type __c, size_type __pos) const noexcept
		{
			for (; __pos < this->size(); ++__pos)
				if (!traits_type::eq(this->data()[__pos], __c))
					return __pos;
			return npos;
		}
		template<typename T>
		constexpr size_type find_last_of(string_helper<T> __str, size_type __pos = 0) const noexcept { return this->find_first_not_of(__str.data(), __pos, __str.size()); }
		constexpr size_type find_last_of(const_pointer __str, size_type __pos = 0) const noexcept { return this->find_first_not_of(__str, __pos, traits_type::length(__str)); }


		constexpr size_type find_last_not_of(const _CharT* __str, size_type __pos, size_type __n) const noexcept {
			size_type __size = this->size();
			if (__size)
			{
				if (--__size > __pos)
					__size = __pos;
				do
				{
					if (!traits_type::find(__str, __n, this->_M_str[__size]))
						return __size;
				} while (__size--);
			}
			return npos;
		}
		constexpr size_type find_last_not_of(_CharT __c, size_type __pos) const noexcept {
			size_type __size = this->size();
			if (__size)
			{
				if (--__size > __pos)
					__size = __pos;
				do
				{
					if (!traits_type::eq(this->data()[__size], __c))
						return __size;
				} while (__size--);
			}
			return npos;
		}
		template<typename T>
		constexpr size_type find_last_not_of(string_helper<T> __str, size_type __pos = 0) const noexcept { return this->find_last_not_of(__str.data(), __pos, __str.size()); }
		constexpr size_type find_last_not_of(const_pointer __str, size_type __pos = 0) const noexcept { return this->find_last_not_of(__str, __pos, traits_type::length(__str)); }

		//template<typename U = base_type>
		//typename std::enable_if<std::is_constructible<U, const_pointer, size_type, string_helper>::value>::type
		//typename std::conditional<std::is_constructible<U, const_pointer, size_type, string_helper>::value



		template<typename U = base_type>
		inline substr_return_type substr(size_type pos = 0, size_type count = npos) const {
			using return_type = typename std::conditional<std::is_constructible<U, const_pointer, size_type>::value, U, string_helper<cstring_size_container>>::type;
			assert(pos < size());
			return return_type(data() + pos, std::min(count, size() - pos));
		}
		template<typename U = base_type>
		inline auto remove_prefix(size_type n) const {
			using return_type = typename std::conditional<std::is_constructible<U, const_pointer, size_type>::value, U, string_helper<cstring_size_container>>::type;
			assert(n < size());
			return return_type(data() + n, size() - n);
		}
		template<typename U = base_type>
		inline auto remove_suffix(size_type n) const {
			using return_type = typename std::conditional<std::is_constructible<U, const_pointer, size_type>::value, U, string_helper<cstring_size_container>>::type;
			assert(n < size());
			return return_type(data(), size() - n);
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
#endif
	};
	using string = string_builder<memalloc_string_container>;
	using zstring = string;// std::basic_string<char, std::char_traits<char>, ZAllocator<char>>;

	using string_buffer = string_builder<fixed_string_container>;
	template<size_t _SIZE>
	using fixed_string = string_builder<static_string_container<_SIZE>>;
};



// eveything that has to do with the string system herer we go

using cstring_t = quake::cstring;


struct string_t : public quake::cstring {
public:
	static  const char* intern(const char* str);
	static  const char* intern(const char* str, size_t size);
	static  const char* intern(const quake::string_view&  str);
	inline string_t() : quake::cstring() {}
	template<typename T>
	inline string_t(const quake::string_helper<T>& s) : quake::cstring(intern(s.data(), s.size())) {}
	inline string_t(const quake::string_view&  str) : quake::cstring(intern(str)) { }
	inline string_t(const char*  str) : quake::cstring(intern(str)) { }
	friend class pr_system_t;
};

namespace std {
	template<>
	struct hash<quake::string_helper<quake::cstring_container>> {
		inline constexpr size_t operator()(const quake::string_helper<quake::cstring_container>& s) const { return  quake::string_info::str_hash(s.begin(), s.end()); }
	};
	template<>
	struct hash<string_t> {
		inline constexpr size_t operator()(const string_t& s) const { return  quake::string_info::str_hash(s.begin(), s.end()); }
	};
	template<>
	struct hash<quake::string_helper<quake::cstring_size_container>> {
		inline constexpr size_t operator()(const quake::string_helper<quake::cstring_size_container>& s) const { return  quake::string_info::str_hash(s.begin(), s.end()); }
	};
	template<>
	struct hash<quake::string_builder<quake::fixed_string_container>> {
		inline constexpr size_t operator()(const quake::string_builder<quake::fixed_string_container>& s) const { return  quake::string_info::str_hash(s.begin(), s.end()); }
	};
	template<>
	struct hash<quake::string_builder<quake::memalloc_string_container>> {
		inline constexpr size_t operator()(const quake::string_builder<quake::memalloc_string_container>& s) const { return  quake::string_info::str_hash(s.begin(), s.end()); }
	};
}