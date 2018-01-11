#pragma once
#include <vector>
#include <array>
#include <stdexcept>
#include <cassert>
#include <utility>
#include <algorithm>
#include <iostream>
#include <memory>
#include <tuple>
#include <utility>


// quake vector stuff

#include "safeint3.hpp"

// cx math stuff
// constent log10 stuff
#include <constexpr\cx_math.h> 
namespace quake {
	//template<typename T>
	//using vector = std::vector<T>;
	//using string = std::string;
	// meta heper types from gcc
	template<bool __v>
	using __bool_constant = std::integral_constant<bool, __v>;

	template<typename...> struct __or_;

	template<> struct __or_<> : public std::false_type { };
	template<typename _B1> struct __or_<_B1> : public _B1 { };
	template<typename _B1, typename _B2> struct __or_<_B1, _B2> : public std::conditional<_B1::value, _B1, _B2>::type { };
	template<typename _B1, typename _B2, typename _B3, typename... _Bn> struct __or_<_B1, _B2, _B3, _Bn...>: public std::conditional<_B1::value, _B1, __or_<_B2, _B3, _Bn...>>::type { };

	template<typename...> struct __and_;
	template<> struct __and_<>: public std::true_type{ };
	template<typename _B1>struct __and_<_B1>: public _B1{ };
	template<typename _B1, typename _B2> struct __and_<_B1, _B2>: public std::conditional<_B1::value, _B2, _B1>::type{ };
	template<typename _B1, typename _B2, typename _B3, typename... _Bn>struct __and_<_B1, _B2, _B3, _Bn...>: public std::conditional<_B1::value, __and_<_B2, _B3, _Bn...>, _B1>::type{ };
	// its in c++17 but lets put it here in case we don't have that

	template<typename _Pp> struct __not_ : public __bool_constant<!bool(_Pp::value)> { };

	template<typename... _Bn> struct conjunction : __and_<_Bn...> { };
	template<typename... _Bn> struct disjunction : __or_<_Bn...> { };
	template<typename _Pp> struct negation : __not_<_Pp> { };
	template<typename... _Bn>  constexpr bool conjunction_v = conjunction<_Bn...>::value;
	template<typename... _Bn>  constexpr bool disjunction_v = disjunction<_Bn...>::value;
	template<typename _Pp>  constexpr bool negation_v = negation<_Pp>::value;

	// kind of a hack since we don't have gcc
	// also, technicaly, gcc increases the precision then downlgrades on overflow, this couldd 
	// work but just htis for now
	//https://stackoverflow.com/questions/199333/how-to-detect-integer-overflow
#if 0
	template<typename T>
	typename std::enable_if<std::is_integral<T>::value,bool>::type
		constexpr inline	__builtin_add_overflow( T a, T x, T* v, ) {
		*v = a + x;
		if ((x > 0) && (a > std::numeric_limits<T>::max() - x)) return true;/* `a + x` would overflow */;
		if ((x < 0) && (a < std::numeric_limits<T>::min() - x)) return true;/* `a + x` would underflow */;
		return false;
	}
	// kind of a hack since we don't have gcc
	template<typename T>
	typename std::enable_if<std::is_integral<T>::value, bool>::type
	constexpr inline	__builtin_mul_overflow(T a, T x, T* v, ) {
		*v = a * x;
		if (a > std::numeric_limits<T>::max() / x) return true;/* `a * x` would overflow */;
		if ((a < std::numeric_limits<T>::min() / x))return true; /* `a * x` would underflow */;
		// there may be need to check for -1 for two's complement machines
		if ((a == -1) && (x == std::numeric_limits<T>::min())) return true;/* `a * x` can overflow */
		if ((x == -1) && (a == std::numeric_limits<T>::min())) return true;/* `a * x` (or `a / x`) can overflow */
		return false;
	}
#endif
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
			static constexpr size_type npos = std::numeric_limits<size_type>::max();
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
		static constexpr inline size_t _str_length(const char* p, size_t s) { return p[s] == '\0' ? s : _str_length(p, s + 1); }
		static constexpr inline size_t str_length(const char* p) { return _str_length(p, 0); }
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
		size_type str_find(const_pointer __str1, size_type __n1, size_type __pos, value_type c)noexcept;
		size_type str_find(const_pointer __str1, size_type __n1, size_type __pos, const_pointer __str2, size_type __n2)noexcept;
		size_type str_rfind(const_pointer __str1, size_type __n1, size_type __pos, value_type c) noexcept;
		size_type str_rfind(const_pointer __str1, size_type __n1, size_type __pos, const_pointer __str2, size_type __n2)noexcept;

		size_type str_find_first_of(const_pointer __str1, size_type __n1, size_type __pos, const_pointer __str2, size_type __n2)noexcept;
		size_type find_first_not_of(const_pointer __str1, size_type __n1, size_type __pos, const_pointer __str2, size_type __n2)noexcept;
		size_type find_last_of(const_pointer __str1, size_type __n1, size_type __pos, const_pointer __str2, size_type __n2)noexcept;
		size_type find_last_not_of(const_pointer __str1, size_type __n1, size_type __pos, const_pointer __str2, size_type __n2)noexcept;

		// bear bones to chars.
		// from http://gcc.1065356.n8.nabble.com/attachment/1365368/0/patch.txt
		template<typename _Tp>
		constexpr  size_t __to_chars_len(_Tp __value, int __base = 10) {
			static_assert(std::is_integral<_Tp>::value, "implementation bug");
			static_assert(std::is_unsigned<_Tp>::value, "implementation bug");
			size_t __n = 1;
			const int __b2 = __base  * __base;
			const int __b3 = __b2 * __base;
			const int __b4 = __b3 * __base;
			for (;;) {
				if (__value < __base) return __n;
				if (__value < __b2) return __n + 1;
				if (__value < __b3) return __n + 2;
				if (__value < __b4) return __n + 3;
				__value /= (unsigned)__b4;
				__n += 4;
			}
		}
		struct to_chars_result {
			char* ptr;
			std::error_code ec;
		};
		struct from_chars_result {
			const char* ptr;
			std::error_code ec;
		};
		template<typename _Tp, typename... _Types> using __is_one_of = __or_<std::is_same<_Tp, _Types>...>;

		template<typename _Tp> using __is_to_chars_type = __and_<std::is_integral<_Tp>, __not_<__is_one_of<_Tp, bool, char16_t, char32_t, wchar_t>>>;

		template<typename _Tp, typename... _Types> using __is_one_of = __or_<std::is_same<_Tp, _Types>...>;
		template<typename _Tp> using __to_chars_result_type = std::enable_if_t<__is_to_chars_type<_Tp>::value, to_chars_result>;


		template<typename _Tp>
		to_chars_result __to_chars(char* __first, char* __last, _Tp __val, int __base)
		{
			static_assert(std::is_integral<_Tp>::value, "implementation bug");
			static_assert(std::is_unsigned<_Tp>::value, "implementation bug");

			constexpr char __digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";

			to_chars_result __res;

			const unsigned __len = __to_chars_len(__val, __base);

			if ((__last - __first) < __len)
			{
			__res.ptr = __last;
			__res.ec = std::make_error_code(std::errc::value_too_large);
			return __res;
			}

			unsigned __pos = __len - 1;

			if (__base == 10)
			{
				constexpr char __digits[201] =
					"0001020304050607080910111213141516171819"
					"2021222324252627282930313233343536373839"
					"4041424344454647484950515253545556575859"
					"6061626364656667686970717273747576777879"
					"8081828384858687888990919293949596979899";
				while (__val >= 100)
				{
					auto const __num = (__val % 100) * 2;
					__val /= 100;
					__first[__pos] = __digits[__num + 1];
					__first[__pos - 1] = __digits[__num];
					__pos -= 2;
				}
				if (__val >= 10)
				{
					auto const __num = __val * 2;
					__first[__pos] = __digits[__num + 1];
					__first[__pos - 1] = __digits[__num];
				}
				else
					__first[__pos] = '0' + __val;
			}
			else if (__base == 8)
			{
				constexpr char __digits[129] =
					"00010203040506071011121314151617"
					"20212223242526273031323334353637"
					"40414243444546475051525354555657"
					"60616263646566677071727374757677";
				while (__val >= 64)
				{
					auto const __num = (__val % 64) * 2;
					__val /= 64;
					__first[__pos] = __digits[__num + 1];
					__first[__pos - 1] = __digits[__num];
					__pos -= 2;
				}
				if (__val >= 8)
				{
					auto const __num = __val * 2;
					__first[__pos] = __digits[__num + 1];
					__first[__pos - 1] = __digits[__num];
				}
				else
					__first[__pos] = '0' + __val;
			}
			else if (__base == 2)
			{
				while (__pos)
				{
					__first[__pos--] = '0' + (__val & 1);
					__val >>= 1;
				}
				*__first = '0' + (__val & 1);
			}
			else
			{
				while (__val >= __base)
				{
					auto const __quo = __val / __base;
					auto const __rem = __val % __base;
					__first[__pos--] = __digits[__rem];
					__val = __quo;
				}
				*__first = __digits[__val];
			}
			__res.ptr = __first + __len;
			return __res;
		}
		enum class __chars_format {
			scientific ,
			fixed ,
			hex ,
			general = fixed | scientific
		};
		
		/**
		* Double to ASCII
		*/

		//http://www.microchip.com/forums/m183763.aspx
		to_chars_result __fast_to_chars(char* first, char* last, float val, const __chars_format fmt = __chars_format::general,  size_t precision = 8);
		
		template<typename _Tp>
		bool __raise_and_add(_Tp& __val, int __base, unsigned char __c)
		{
			Safe
			if (SafeMultiply(__val, __base, __val) || SafeAdd(__val, __c, __val))
				return false;
			return true;
		}
		template<typename _Tp>
		bool __from_chars_binary(const char*& __first, const char* __last, _Tp& __val)
		{
			static_assert(std::is_integral<_Tp>::value, "implementation bug");
			static_assert(std::is_unsigned<_Tp>::value, "implementation bug");

			const ptrdiff_t __len = __last - __first;
			int __i = 0;
			while (__i < __len)
			{
				if (__first[__i] == '0')
					__val <<= 1;
				else if (__first[__i] == '1')
					(__val <<= 1) |= 1;
				else
					break;
				__i++;
			}
			__first += __i;
			return __i <= (sizeof(_Tp) * 8);
		}

		template<typename _Tp>
		bool __from_chars_digit(const char*& __first, const char* __last, _Tp& __val, int __base)
		{
			static_assert(std::is_integral<_Tp>::value, "implementation bug");
			static_assert(std::is_unsigned<_Tp>::value, "implementation bug");

			auto __matches = [__base](char __c) {
				return '0' <= __c && __c <= ('0' + (__base - 1));
			};

			while (__first != __last)
			{
				const char __c = *__first;
				if (__matches(__c))
				{
					if (!__raise_and_add(__val, __base, __c - '0'))
					{
						while (++__first != __last && __matches(*__first))
							;
						return false;
					}
					__first++;
				}
				else
					return true;
			}
			return true;
		}

		constexpr bool __consecutive_chars(const char* __s, int __n)
		{
			for (int __i = 1; __i < __n; ++__i)
				if (__s[__i] != (__s[__i - 1] + 1))
					return false;
			return true;
		}

		template<typename _Tp>
		bool __from_chars_alnum(const char*& __first, const char* __last, _Tp& __val, int __base)
		{
			const int __b = __base - 10;
			bool valid = true;
			while (__first != __last)
			{
				unsigned char __c = *__first;
				if (std::isdigit(__c))
					__c -= '0';
				else
				{
					constexpr char __abc[] = "abcdefghijklmnopqrstuvwxyz";
					unsigned char __lc = std::tolower(__c);
					constexpr bool __consecutive = __consecutive_chars(__abc, 26);
					if (constexpr __consecutive)
					{
						// Characters 'a'..'z' are consecutive
						if (std::isalpha(__c) && (__lc - 'a') < __b)
							__c = __lc - 'a' + 10;
						else
							break;
					}
					else
					{
						if (auto __p = ::memchr(__abc, __lc, __b))
							__c = static_cast<const char*>(__p) - __abc;
						else
							break;
					}
				}

				if (valid)
					__valid = __raise_and_add(__val, __base, __c);
				__first++;
			}
			return __valid;
		}

		template<typename _Tp>
		using __from_chars_result_type = std::enable_if_t<__is_to_chars_type<_Tp>::value, from_chars_result>;
	}
	template<typename _Tp> detail::__to_chars_result_type<_Tp> to_chars(char* __first, char* __last, _Tp __value, int __base = 10) {
		assert(2 <= __base && __base <= 36);
		if constexpr( std::is_integral<_Tp>::value) {
			if constexpr ( std::is_unsigned<_Tp>::value) {
				return detail::__to_chars(__first, __last, __value, __base);
			}
			else {
				using _Up = std::make_unsigned_t<_Tp>;
				_Up __unsigned_val;
				if (__value < 0) {
					if (__first != __last)
						*__first++ = '-';
					__unsigned_val = _Up(~__value) + _Up(1);
				}
				else __unsigned_val = __value;
				return detail::__to_chars(__first, __last, __unsigned_val, __base);
			}
		}
		else if constexpr( std::is_floating_point<_Tp>::value) {
			if constexpr( std::is_same<float, std::decay_t<_Tp>>::value) {
				using _Up = typename std::make_unsigned<int>::type;
				_Up front = static_cast<_Up>(__value < 0.0f ? -__value : _value);
				_Up exp = static_cast<_Up>(((__value < 0.0f ? -__value : _value) - static_cast<_Tp>(front))) ;
				if (front == 0 && exp == 0 && __value != 0.0f) {
					assert(0); // not supported yet
					throw std::invalid_argument("float to small for me to support yet");
				}
				if (__value < 0.0f)  if (__first != __last) *__first++ = '-';
				__first = detail::__to_chars(__first, __last, front); 
				*__first++ = '.';
				auto ret = detail::__to_chars(__first, __last, exp); // sooo hacky
			}
			else if constexpr( std::is_same<float, std::decay_t<__Ty>>::value) {
				assert(0); // not supported yet
				throw std::invalid_argument("double not supported yet");
			}
		}
		assert(0); // not supported yet
		throw std::invalid_argument("conversion not supported yet");
	}

	template<typename _Tp> size_t to_chars_len(const _Tp __value, int __base = 10) {
		assert(2 <= __base && __base <= 36);
		using _Up = std::make_unsigned<_Tp>::type;
		const _Up __unsigned_val = __value;
		size_t count = detail::__to_chars_len(__unsigned_val, __base);
		if constexpr( std::is_signed<_Tp>::value) if (__value < 0) ++count;
		return count;
	}

	template<typename _Tp>
		detail::__from_chars_result_type<_Tp>
			from_chars(const char* __first, const char* __last, _Tp& __value, int __base = 10) {
			assert(2 <= __base && __base <= 36);

			detail::from_chars_result __res{ __first,{} };

			int __sign = 1;
			if constexpr(std::is_signed<_Tp>::value)
				if (__first != __last && *__first == '-')
				{
					__sign = -1;
					++__first;
				}

			using _Up = std::make_unsigned<_Tp>::type;
			_Up __val = 0;

			const auto __start = __first;
			bool __valid;
			if (__base == 2)
				__valid = detail::__from_chars_binary(__first, __last, __val);
			else if (__base <= 10)
				__valid = detail::__from_chars_digit(__first, __last, __val, __base);
			else
				__valid = detail::__from_chars_alnum(__first, __last, __val, __base);

			if (__first == __start)
				__res.ec = std::make_error_code(errc::invalid_argument);
			else
			{
				__res.ptr = __first;
				if (!__valid)
					__res.ec = std::make_error_code(errc::result_out_of_range);
				else
				{
					if constexpr(std::is_signed<_Tp>::value)
					{
						_Tp __tmp;
						if (SafeMultiply(__val, __sign, __tmp))
							__res.ec = std::make_error_code(errc::result_out_of_range);
						else
							__value = __tmp;
					}
					else
					{
						if  constexpr(std::numeric_limits<_Up>::max() > std::numeric_limits<_Tp>::max())
						{
							if (__val > std::numeric_limits<_Tp>::max())
								__res.ec = std::make_error_code(errc::result_out_of_range);
							else
								__value = __val;
						}
						else
							__value = __val;
					}
				}
			}

			return __res;
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
		static constexpr size_type npos = detail::npos;

		// from base
		constexpr inline const_pointer data() const { return static_cast<const _TOP*>(this)->data(); }
		constexpr inline size_type size() const { return static_cast<const _TOP*>(this)->size(); }
		constexpr inline size_type length() const { return static_cast<const _TOP*>(this)->size(); }

		constexpr inline bool empty() const { return size() == 0U; }
		constexpr inline const_iterator begin() const { return data(); }
		constexpr inline const_iterator end() const { return  data() + size(); }
		constexpr inline const_reverse_iterator rbegin() const { return reverse_iterator(begin()); }
		constexpr inline const_reverse_iterator rend() const { return  reverse_iterator(end() ()); }

		constexpr inline const_reference front() const { return *data(); }
		constexpr inline const_reference back() const { return  *(data() + size()); }

		constexpr inline const_reference at(size_type pos) const { assert(pos < size()); return *(data() + pos); }
		constexpr inline const_reference operator[](size_type pos) const { assert(pos < size()); return *(data() + pos); }

		size_type copy(value_type* __str, size_type __n, size_type __pos = 0) const {
			assert(__pos);
			const size_type __rlen = std::min(__n, this->size() - __pos);
			for (auto __begin = this->data() + __pos,
				__end = __begin + __rlen; __begin != __end;)
				*__str++ = *__begin++;
			// just because, lets make it zero terminated
			__str[__rlen] = '\0'; // not sure this is in the standard, but put it here anyway
			return __rlen;
		}
		template<size_type N>
		size_type copy(value_type(&__str)[N], size_type __pos = 0) const { return copy(__str, N, __pos); }


		constexpr int compare(size_type __pos1, size_type __n1, const_pointer __str, size_type __n2) const noexcept {
			return detail::str_compare(data(), std::min(__n1, size() - __pos1), __str, __n2);
		}
		template<typename T>
		constexpr int compare(size_type __pos1, size_type __n1, const string_helper<T>& __str) const { return compare(__pos1, __n1, __str.data(), __str.size()); }
		template<typename T>
		constexpr int compare(size_type __pos1, size_type __n1, const string_helper<T>&__str, size_type __pos2, size_type __n2) const {
			return compare(__pos1, __n1, __str.data() + __pos2, __n2 - __pos2);
		}
		constexpr int compare(size_type __pos1, size_type __n1, const_pointer __str) const { return compare(__pos1, __n1, __str, traits_type::length(__str)); }
		template<typename T>
		constexpr int compare(const string_helper<T>& __str) const { return compare(0U, size(), __str.data(), __str.size()); }
		constexpr int compare(const_pointer __str) const { return compare(0U, size(), __str, traits_type::length(__str)); }


		constexpr size_type find(value_type c, size_type pos = 0) const noexcept { return detail::str_find(data(), size(), pos, c); }
		constexpr  inline size_type find(const_pointer p, size_type pos, size_type n) const { return detail::str_find(data(), size(), pos, p, n); }


		constexpr size_type find(const_pointer __str, size_type __pos = 0) const noexcept { return this->find(__str, __pos, detail::str_length(__str)); }
		template<typename T> constexpr size_type find(string_helper<T> __str, size_type __pos = 0) const noexcept { return this->find(__str.data(), __pos, __str.size()); }
		constexpr size_type rfind(value_type c, size_type pos = npos) const noexcept { return detail::str_rfind(data(), size(), pos, c); }
		inline size_type	rfind(const_pointer p, size_type pos, size_type n) const { return detail::str_rfind(data(), size(), pos,p, n); }
		template<typename T> constexpr size_type rfind(string_helper<T> __str, size_type __pos = npos) const noexcept { return this->rfind(__str.data(), __pos, __str.size()); }
		constexpr size_type rfind(const_pointer __str, size_type __pos = npos) const noexcept { return this->rfind(__str, __pos, detail::str_length(__str)); }

		constexpr size_type find_first_of(const_pointer __str, size_type __pos, size_type __n) const noexcept {
			return detail::find_first_of(data(), size(),__pos, __str, __n);
		}

		template<typename T> constexpr size_type find_first_of(string_helper<T> __str, size_type __pos = 0) const noexcept { return this->find_first_of(__str.data(), __pos, __str.size()); }
		constexpr size_type find_first_of(value_type __c, size_type __pos = 0) const noexcept { return this->find(__c, __pos); }
		constexpr size_type find_first_of(const_pointer __str, size_type __pos = 0) const noexcept { return this->find_first_of(__str, __pos, detail::str_length(__str)); }

		constexpr size_type  find_last_of(const_pointer __str, size_type __pos, size_type __n) const noexcept {
			return detail::find_last_of(data(), size(), __pos, __str, __n);
		}

		template<typename T> constexpr size_type find_last_of(string_helper<T> __str, size_type __pos = npos) const noexcept { return this->find_last_of(__str.data(), __pos, __str.size()); }
		constexpr size_type find_last_of(value_type __c, size_type __pos = npos) const noexcept { return this->rfind(__c, __pos); }
		constexpr size_type find_last_of(const_pointer __str, size_type __pos = npos) const noexcept { return this->find_last_of(__str, __pos, detail::str_length(__str)); }

		constexpr size_type find_first_not_of(const_pointer __str, size_type __pos, size_type __n) const noexcept {
			return detail::find_first_not_of(data(), size(), __pos, __str, __n);
		}
		constexpr size_type find_first_not_of(value_type __c, size_type __pos) const noexcept {
			return detail::find_first_not_of(data(),__pos, size(),__pos, &c, 1); // should optimize this
		}

		template<typename T> constexpr size_type find_first_not_of(string_helper<T> __str, size_type __pos = 0) const noexcept { return this->find_first_not_of(__str.data(), __pos, __str.size()); }
		constexpr size_type find_first_not_of(const_pointer __str, size_type __pos = 0) const noexcept { return this->find_first_not_of(__str, __pos, string_info::str_length(__str)); }


		constexpr size_type find_last_not_of(const_pointer __str, size_type __pos, size_type __n) const noexcept {
			return detail::find_last_not_of(data(), size(), __pos, __str, __n);
		}
		constexpr size_type find_last_not_of(value_type __c, size_type __pos= npos) const noexcept {
			return detail::find_last_not_of(data(), __pos, size(), __pos, &c, 1); // should optimize this
		}

		template<typename T> constexpr size_type find_last_not_of(string_helper<T> __str, size_type __pos = npos) const noexcept { return this->find_last_not_of(__str.data(), __pos, __str.size()); }
		constexpr size_type find_last_not_of(const_pointer __str, size_type __pos = npos) const noexcept { return this->find_last_not_of(__str, __pos, string_info::str_length(__str)); }

		constexpr bool is_number() const { return data() != nullptr && size() > 0 && (util::is_char_number(data()[0]) || ((data()[0] == '+' || data()[0] == '-') && is_char_number(data()[1]))); }

		const_iterator to_number(float& v) const {
			const char* str;
			v = strtof(data(), &str);
			return str != data() ? const_iterator(str) : data();
		}
		const_iterator to_number(double& v) const {
			const char* str;
			v = strtod(data(), &str);
			return str != data() ? const_iterator(str) : data();
		}
		const_iterator to_number(int32_t& v, int base = 0) const {
			const char* str;
			v = strtol(data(), &str, base);
			return str != data() ? const_iterator(str) : data();
		}
		const_iterator to_number(int64_t& v, int base = 0) const {
			const char* str;
			v = strtoll(data(), &str, base);
			return str != data() ? const_iterator(str) : data();
		}
		template<typename U>	constexpr bool operator==(const string_helper<U>& other) const { return size() == other.size() && (data() == other.data() || compare(other) == 0); }
		template<typename U>	constexpr bool operator!=(const string_helper<U>& other) const { return !(*this == other); }
		template<typename U>	constexpr bool operator<(const string_helper<U>& other) const { return compare(other) < 0;}
		template<typename U>	constexpr bool operator>(const string_helper<U>& other) const { return compare(other) > 0;}
		template<typename U>	constexpr bool operator<=(const string_helper<U>& other) const { return compare(other) <= 0;}
		template<typename U>	constexpr bool operator>=(const string_helper<U>& other) const { return compare(other) >= 0; }


#if 0
		constexpr bool operator==(const base_type& other) const { return size() == other.size() && (data() == other.data() || compare(other) == 0); }
		constexpr bool operator!=(const base_type& other) const { return !(*this == other); }
		constexpr bool operator<(const base_type& other) const { return compare(other) < 0; }
		constexpr bool operator>(const base_type& other) const { return compare(other) > 0; }
		constexpr bool operator<=(const base_type& other) const { return compare(other) <= 0; }
		constexpr bool operator>=(const base_type& other) const { return compare(other) >= 0; }
#endif
	};
#if 0
	template<typename U, typename V>
	constexpr bool operator==(const U& l, const string_helper<U>& r) { return l.size() == r.size() && (l.data() == r.data() || l.compare(r.other) == 0); }
	template<typename U, typename V>
	constexpr bool operator!=(const U& l, const string_helper<U>& r) { return !(l != r); }

	template<typename U, typename V>
	constexpr bool operator==(const string_helper<V>& l, const string_helper<U>& r) { return l.size() == r.size() && (l.data() == r.data() || l.compare(r.other) == 0); }
	template<typename U, typename V>
	constexpr bool operator!=(const string_helper<V>& l, const string_helper<U>& r) { return !(l != r); }

#endif
	template<typename _TOP>
	class string_builder : public string_helper<_TOP> {
		_TOP& top() { return *static_cast<_TOP*>(this); }
		const _TOP& top() const { return *static_cast<const _TOP*>(this); }
	public:
		using base_type = _TOP;
		using pointer = value_type*;
		using reference = value_type&;
		using iterator = pointer;
		using reverse_iterator = std::reverse_iterator<iterator>;
		template<typename U>
		struct is_convertable_to_view { // quick hack
			static constexpr bool value = std::is_convertible<const U&, const string_view&>::value && !std::is_convertible<const U&, const_pointer>::value;
		};

		constexpr inline const_pointer data() const { return static_cast<const _TOP*>(this)->data(); }
		inline pointer data()  { return static_cast<_TOP*>(this)->data(); }
		constexpr inline size_type size() const { return static_cast<const _TOP*>(this)->size(); }
		constexpr inline size_type length() const { return static_cast<const _TOP*>(this)->size(); }
		constexpr inline size_type capacity() const { return static_cast<const _TOP*>(this)->capacity(); }

		inline iterator begin()  { return data(); }
		inline iterator end()  { return  data() + size(); }
		inline reverse_iterator rbegin() { return reverse_iterator(begin());  }
		inline reverse_iterator rend() { return  reverse_iterator(end() ()); }

		inline reference front()  { return *data(); }
		inline reference back()  { return  *(data() + size()); }

		inline reference at(size_type pos) { assert(pos < size()); return *(data() + pos); }
		inline reference operator[](size_type pos)  { assert(pos < size()); return *(data() + pos); }

		inline const char* c_str() const { return data(); }
		inline void reserve(size_type s) {
			if (capacity() < (s + 1))
				static_cast<_TOP*>(this)->_reserve(s + 1);
		}
		inline void resize(size_type s) {
			reserve(s);
			static_cast<_TOP*>(this)->_resize(s);
			*end() = '\0'; // reserve always make sure we have space for this
		}
		void clear() { resize(0U); }
		// all assigns
		inline base_type& assign(const_pointer p1, size_t s1) {
			resize(s1);
			::memcpy(data(), p1, s1);
			return top();
		}
		inline base_type& assign(size_type count, value_type c) {
			resize(count);
			::memset(data(), c, count); // should fix this for wchar_t
			return top();
		}
		inline base_type& assign(const_pointer p1) { return assign(p1, traits_type::length(p1)); }
		// iterate assign
		template<typename ITR>
		inline base_type& assign(ITR s1, ITR s2) {
			assert(s1 < s2);
			resize(std::distance(s1, s2));
			std::uninitialized_copy(s1, s2, begin());
			return top();
		}
#if 0
		inline base_type& assign(std::initializer_list<value_type> l) {
			resize(l.size());
			std::uninitialized_copy(l.begin(), l.end(), begin());
			return top();
		}
		inline base_type& append(std::initializer_list<value_type> l) {
			resize(l.size() + size());
			std::uninitialized_copy(l.begin(), l.end(), begin() + size());
			return top();
		}
#endif
		template<typename U>
		inline base_type& assign(const string_builder<U>& h, size_type pos = 0U, size_type count = npos) {
			assert(pos < h.size());
			return assign(h.data() + pos, std::min(h.size(), count - pos));
		}
		template<typename U>
		inline base_type& assign(const string_helper<U>& h) { return assign(h.data(), h.size()); }

		template<typename U>
		typename std::enable_if<is_convertable_to_view<U>::value, string_builder&>::type
			inline  assign(const U& hh, size_type pos, size_type count = npos) {
			const string_view h(h);
			assert(pos < h.size());
			return assign(h.data() + pos, std::min(h.size(), count - pos));
		}
		// all appeneds
		inline base_type& append(const_pointer p1, size_t s1) {
			auto at = end();
			resize(s1 + size());
			::memcpy(at, p1, s1);
			return top();
		}
		inline base_type& append(size_type count, value_type c) {
			resize(count + size());
			std::uninitialized_fill_n(begin() + size(), count, c);  // should fix this for wchar_t
			return top();
		}
		inline base_type& append(const_pointer p1) { return append(p1, traits_type::length(p1)); }

		// iterate assign
		template<typename ITR>
		inline base_type& append(ITR s1, ITR s2) {
			assert(s1 < s2);
			resize(std::distance(s1, s2) + size());
			std::uninitialized_copy(s1, s2, begin() + size());
			return top();
		}


		template<typename U>
		inline base_type& append(const string_builder<U>& h, size_type pos = 0U, size_type count = npos) {
			assert(pos < h.size());
			return append(h.data() + pos, std::min(h.size(), count - pos));
		}
		template<typename U>
		inline base_type& append(const string_helper<U>& h) {
			return append(h.data(), h.size());
		}

		template<typename U>
		typename std::enable_if<is_convertable_to_view<U>::value, string_builder&>::type
			inline  append(const U& hh, size_type pos, size_type count = npos) {
			const string_view h(h);
			assert(pos < h.size());
			return append(h.data() + pos, std::min(h.size(), count - pos));
		}
		// we don't cover all types
		base_type& insert(size_type pos, value_type ch) {
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
		base_type& insert(size_type pos, const string_helper<U>& sv) {
			const size_type len = std::distance(first, last);
			auto it = insert_hole(pos, sv.size());
			std::uninitialized_copy(sv.begin(), sv.end(), it);
			return *this;

		}
		template<typename T>
		typename std::enable_if<is_convertable_to_view<T>::value, base_type&>::type
			insert(size_type index, const T& t, size_type pos, size_type count = npos) {
			const string_view h(t);
			assert(pos < t.size());
			count = std::min(count, h.size() - pos);
			auto it = insert_hole(pos, count);
			std::uninitialized_copy(h.begin(), h.end(), it);
			return *this;
		}
		base_type& erase(size_type index = 0, size_type count = npos) {
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
			resize(size() + 1);
			back() = ch;
			
		}
		void pop_back() {
			if (size() > 0) resize(size() - 1);
		}
		// replaces just a few of them
		base_type& replace(const_iterator first, const_iterator last, const_pointer i1, const_pointer i2, size_type n) {
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
			std::copy(i1, i2, const_cast<iterator>( rp));
			//std::fill(rp, i1, std::distance(i1, i2), n);
			*end() = 0;
			return top();
		}
		template <typename InputIt>
		base_type&	replace(const_iterator first, const_iterator last, InputIt first2, InputIt last2) { return replace(first, last, first2, last2, 1); }
		inline base_type&		replace(const_iterator first, const_iterator last, const_pointer s, size_type slen) { return replace(first, last, s, s + slen); }
		inline base_type&		replace(const_iterator first, const_iterator last, size_type n, value_type c) { return replace(first, last, &c, &c + 1, n); }
		template<typename U>
		inline base_type&		replace(size_type rp, size_type n, const string_helper<U>& s) { return replace(begin() + rp, begin() + (rp + n), s); }
		template<typename U>
		inline base_type&		replace(size_type rp, size_type n, const string_helper<U>& s, size_type sp, size_type slen) { return replace(begin() + (rp), begin() + (rp + n), s.begin() + (sp), s.begin() + (sp + slen)); }
		inline base_type&		replace(size_type rp, size_type n, const_pointer s, size_type slen) { return replace(begin() + (rp), begin() + (rp + n), s, s + slen); }
		inline base_type&		replace(size_type rp, size_type n, const_pointer s) { return replace(begin() + (rp), begin() + (rp + n), s,::strlen(s)); }
		inline base_type&		replace(size_type rp, size_type n, size_type count, value_type c) { return replace(begin() + (rp), begin() + (rp + n), count, c); }


		template<typename U>
		base_type& operator+=(const string_helper<U>& str) { return append(str); }
		base_type& operator+=(const_pointer s) { return append(s); } // humm not needed?
		base_type& operator+=(value_type c) { push_back(c); return top(); }


		template<typename U>
		base_type& operator=(const string_helper<U>& str) { return assign(str); }
		base_type& operator=(value_type c) { return assign(c); }
		base_type& operator=(const_pointer s) { return assign(s); } // humm not needed?

		void assign_vprint(const char* fmt, va_list va) {
			va_list apc;
			int n;
			do {
				va_copy(apc, va);
				n = vsnprintf(data(), capacity(), fmt, apc);
				va_end(apc);
				assert(n > 0);
				if (size_t(n) < capacity()) break;
				reserve(n + 1);
			} while (true);
			resize(n);
		}
		void assign_print(const char* fmt, ...) {
			va_list va;
			va_start(va, fmt);
			assign_vprint(fmt, va);
			va_end(va);
		}
		void append_vprint(const char* fmt, va_list va) {
			va_list apc;
			int n;
			do {
				va_copy(apc, va);
				n = vsnprintf(end(), capacity()-size(), fmt, apc);
				va_end(apc);
				assert(n > 0);
				if (size_t(n) < (capacity()- size())) break;
				reserve(size()+ n + 1);
			} while (true);
			resize(size() + n);
		}
		void append_print(const char* fmt, ...) {
			va_list va;
			va_start(va, fmt);
			append_vprint(fmt, va);
			va_end(va);
		}
	private:
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
			const_iterator cstart = const_cast<const_iterator>(begin() + ip);
			_insert_hole(cstart, n);
			return const_cast<iterator>( cstart);
		}
	};
	
	// stream operations so we don't need to go the whole ostream route
	// handles simple numbers but no formating
	// template interface to help with streaming
	//
	template<typename U>
	inline	U& operator<<(string_builder<U>& sb, char c){// -> typename string_builder<U>::base_type& {
		sb.push_back(c);
		return *static_cast<U*>(std::addressof(sb));
	}
	template<typename U>
	inline	U& operator<<(string_builder<U>& sb, const char* s) {//-> typename string_builder<U>::base_type& {
		return sb.append(s);
	}
	template<typename U,typename UU>
	inline	U& operator<<(string_builder<U>& sb, const string_helper<UU>& s) {//-> typename string_builder<U>::base_type& {
		return sb.append(s);
	}


	template<typename U, typename T>
	typename std::enable_if< std::is_integral<T>::value && !std::is_same<T,char>::value, U&>::type
	inline	operator<<(string_builder<U>& sb, const T v) {
		auto start = sb.end();
		sb.resize(sb.size() + to_chars_len(v)); // set amount of digits
		to_chars(start, sb.end(), v);
		return *static_cast<U*>(std::addressof(sb));
	}

	//----------------------------------------------------------------------
	// String-number conversions

#define STRING_TO_INT_CONVERTER(name,type,func)	\
template<typename U> \
inline type name (const string_helper<U>& str, size_t* idx = nullptr, int base = 10) \
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
template<typename U> \
inline type name (const string_helper<U>& str, size_t* idx = nullptr) \
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

	// now that we have the basic functions, lets make our shit string view
	class string_view : public string_helper<string_view> {
	public:

		constexpr string_view() : _data(""), _size(0U) { };
		// is below to much?
		constexpr string_view(const_pointer p, size_t s) : _data(p == nullptr || s == 0U ? "" : p), _size(p == nullptr || s == 0U ? 0 : s) {}
		constexpr string_view(const_pointer p) : string_view(p, detail::str_length(p)) {}
		constexpr string_view(const_iterator p1, const_iterator p2) : string_view(p1, std::distance(p1,p2)) {}
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
		template<typename U> // string builders are always null termnated
		constexpr cstring(const string_builder<U>& p) : cstring(p.c_str()) {}
	
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
	// ok had to wait till here to define some forwards for string_helper, I could use string view but cstring is just made for this
	template<typename U>
	inline constexpr bool operator==(const string_helper<U>& l, const char* r)  { return l == cstring(r); }
	template<typename U>
	inline constexpr bool operator!=(const string_helper<U>& l, const char* r)  { return !(l == r); }
	template<typename U>
	inline constexpr bool operator==(const char* l, const string_helper<U>& r)  { return r == l; }
	template<typename U>
	inline constexpr bool operator!=(const char* l, const string_helper<U>& r)  { return r != l; }



	// zstring, strring is dynamicly made and used
	// however if it was asigned a const string on construction it does NOT allocate it
	// untill it is used
	class zstring : public string_builder<zstring> {
		friend string_builder<zstring>; // friend the class so it can acces _reserve/_resize
		char* _data;
		size_t _size;
		size_t _capacity;
		void _reserve(size_t s); // in the ccp
		void _resize(size_t s);
	public:
		constexpr zstring() : _data(nullptr), _size(0U), _capacity(0U) {}
		constexpr zstring(const_pointer str) : _data(const_cast<char*>(str)), _size(detail::str_length(str)), _capacity(0U) { }
		template<typename U>
		constexpr zstring(const string_helper<U>& p) : _data(const_cast<char*>(p.data())), _size(p.size()), _capacity(0U) {}

		inline constexpr const_pointer data() const { return _data; }
		pointer data() { 
			if (_capacity == 0U && _data)  // we turn it into live data in this case
				_reserve(size());
			return _data; 
		}
		inline constexpr size_type size() const { return _size; }
		inline inline constexpr size_type capacity() const { return _capacity; }
		const char* c_str() const { return _data; }

	};
	// a stack string is ALLWAYs assigned to the built in buffer, it dosn't have lazy copy
	template<size_t _SIZE>
	class stack_string : public string_builder<stack_string<_SIZE>> {
	public:
		friend string_builder<stack_string<_SIZE>>; // friend the class so it can acces _reserve/_resize
		constexpr size_t capacity() const { return _SIZE; }
		constexpr const_pointer data() const { return _buffer; }
		pointer data() { return _buffer; }
		constexpr size_type size() const { return _size; }


		stack_string() : _size(0U) { _buffer[0] = '\0'; }
		stack_string(const_pointer p) : _size(0U) { assign(p); }
		stack_string(const_pointer p, size_t s) : _size(0U) { assign(p, s); }
		template<typename ITR>
		stack_string(ITR p1, ITR p2) : _size(0U) { assign(p1, p2); }
		template<typename U>
		stack_string(const string_helper<U>& p) : _size(0U) { assign(p); }

		constexpr inline string_view substr(size_type pos = 0, size_type count = npos) const {
			assert(pos < size());
			return string_view(data() + pos, std::min(count, size() - pos));
		}
	private:
		friend string_helper<stack_string<_SIZE>>;
		void _reserve(size_t s) { assert(s < _SIZE); }
		void _resize(size_t s) { assert(s < _SIZE); _size = s; }
		char _buffer[_SIZE + 1];
		size_t _size;
	};

	// fixed string
	class fixed_string : public string_builder<fixed_string> {
	public:
		friend string_builder<fixed_string>; // friend the class so it can acces _reserve/_resize
		constexpr size_t capacity() const { return _capacity; }
		constexpr const_pointer data() const { return _buffer; }
		pointer data() { return _buffer; }
		constexpr size_type size() const { return _size; }
		void manage(void* data, size_type cap) {
			_buffer = reinterpret_cast<char*>(data);
			_capacity = cap;
			_size = 0U;
			_buffer[0] = '\0';
		}
		constexpr fixed_string() : _buffer(nullptr), _size(0U), _capacity(0U){ }
		fixed_string(void* data, size_type cap)  { manage(data, cap); }
		constexpr inline string_view substr(size_type pos = 0, size_type count = npos) const {
			assert(pos < size());
			return string_view(data() + pos, std::min(count, size() - pos));
		}
	private:
		void _reserve(size_t s) { assert(s < _capacity); }
		void _resize(size_t s) { assert(s < _capacity); _size = s; }
		char* _buffer;
		size_t _size;
		size_t _capacity;
	};
	// static vector
	// mainly for debug, but we can also use it for hulk
	// cannot be resized
	template <typename T>
	class array_view {
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
		inline void manage(void * ptr, size_t element_count) {
			_data = reinterpret_cast<pointer>(ptr);
			_size = element_count;
		}
		inline					array_view() : _data(nullptr), _size(0U) {}
		inline					array_view(void* data, size_t element_count) : _data(reinterpret_cast<pointer>(data)), _size(element_count) {}

		inline					array_view(const array_view& copy) : _data(copy._data), _size(copy._size) {} // not sure if this is a good idea
		inline					array_view(array_view&& move) : _data(move._data), _size(move._size) { move._data = nullptr; move._size = 0U; } 

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
		inline void			swap(array_view& v) {
			if (std::addressof(v) != this) { std::swap(_data, v._data); std::swap(_size, v._size); }
		}
		inline size_type indexof(const_iterator it) const {
			return (it >= begin() && it < end()) ? std::distance(begin(),it): npos;
		}
	protected:
		T*	_data;
		size_type _size;
	};
	template <typename T>
	class array_buidler : public array_view<T> {
	public:
		using base_type = array_view<T>;
		inline array_buidler() : base_type(), _capacity(0U) {}
		inline array_buidler(void* data, size_type capacity, size_type element_count) : base_type(data, element_count), _capacity(capacity) {}
		inline array_buidler(const array_buidler& copy) : base_type(copy), _capacity(copy._capacity) {} // not sure if this is a good idea
		inline array_buidler(const base_type& copy) : base_type(copy), _capacity(copy.size()) {}
		inline array_buidler(array_buidler&& move) : base_type(move), _capacity(move._capacity) { }

		inline size_type capacity() const { return _capacity; }
		inline void manage(void * ptr, size_type element_count) {
			_capacity = element_count;
			base_type::manage(ptr, element_count);
		}
		inline void resize_uninitialized(size_type i) {
			assert(i < capacity());
			_size = i;
		}
		inline void resize(size_type i) {
			if (size() > i)
				std::destroy_n(begin() + i, end());
			else
				std::uninitialized_default_construct<T>(end(), end() + i);
			resize_uninitialized(i);
		}
		template<typename U>
		inline void push_back(const U& v) {
			assert(size() + 1 < capacity());
			pointer p = _data + _size++;
			if constexpr(std::is_assignable<T, U>::value)
				*p = v;
			else if constexpr(std::is_constructible<T, U>::value)
				new(p) value_type(v);
			else {
				assert(0); // cannot copy or construct?
			}
		}
		void pop_back() { if (size() != 0) { std::destroy_at(end()); --_size; } }

	public:
		size_type _capacity;
	};


	template <typename T, size_t _CAPACITY>
	class stack_array   {
	public:
		using value_type = T;
		using array_type = std::array<T, _CAPACITY>; 
		using size_type = typename array_type::size_type;
		using diffrence_type = ptrdiff_t;
		using pointer = value_type*;
		using const_pointer = const pointer;
		using reference = value_type&;
		using const_reference = const pointer;
		using iterator = typename array_type::iterator;
		using const_iterator = typename array_type::const_iterator;
		using reverse_iterator = typename array_type::reverse_iterator;
		using const_reverse_iterator = typename array_type::const_reverse_iterator;
		constexpr inline size_type capacity() const { return _CAPACITY; }
		constexpr inline size_type size() const { return _count; }

		inline void resize_uninitialized(size_type i) {
			assert(i < capacity());
			_count = i;
		}
		template<typename ITR>
		void assign(ITR a, ITR b) {
			assert(a < b && std::distance(a,b) < capacity());
			std::uninitialized_copy(a, b, begin());
			resize_uninitialized(std::distance(a, b));
		}
		template<typename ITR>
		void assign_move(ITR a, ITR b) {
			assert(a < b && std::distance(a, b) < capacity());
			std::uninitialized_move(a, b, begin());
			resize_uninitialized(std::distance(a, b));
		}
		template<typename ITR>
		void append(ITR a, ITR b) {
			assert(a < b && std::distance(a, b) < capacity()+size());
			std::uninitialized_copy(a, b, begin()+size());
			resize_uninitialized(std::distance(a, b) + size());
		}
		void clear() {
			std::destroy(begin(), end());
			resize_uninitialized(0); 
		}
		inline void resize(size_type i) {
			if (size() > i)
				std::destroy_n(begin() + i, end());
			else
				std::uninitialized_default_construct<T>(end(), end() + i);
			resize_uninitialized(i);
		}

		template<typename U>
		inline void push_back(const U& v) {
			assert(size() + 1 < capacity());
			pointer p = _data + _count++;
			if constexpr(std::is_constructible<T, U>::value)
				new(p) value_type(v);
			else {
				assert(0); // cannot copy or construct?
			}
		}
		void pop_back() { if (size() != 0) { std::destroy_at(end()); --_size; } }


		inline stack_array() : _count(0) { }
		template<size_t CAP>
		inline stack_array(const stack_array<T,CAP>& copy)  { assign(copy.begin(), copy.end()); }
		inline stack_array(stack_array&& move) { assign(move.begin(), move.end()); move.clear(); }


		inline iterator		begin(void) { return _data.begin(); }
		inline iterator		end(void) { return begin() + size(); }
		inline reference	at(size_type i) { return _data[i]; }
		inline constexpr const_reference	at(size_type i) const { return _data[i]; }
		inline constexpr const_reference	operator[] (size_type i) const { return _data[i]; }

		inline reference	operator[] (size_type i) { return _data[i]; }
		inline const_iterator	begin(void) const { return _data.begin(); }
		inline const_iterator	end(void) const { return begin() + size(); }
		inline reference front() { return _data[0]; }
		inline reference back() { return _data[_count - 1]; }
		inline const_reference front() const { return _data[0]; }
		inline const_reference back() const{ return _data[_count - 1]; }
		inline constexpr bool		empty(void) const { return size() == 0; }

		inline void			fill(const_reference v) { std::fill(begin(), end(), v); }

		inline size_type indexof(const_iterator it) const {
			return (it >= begin() && it < end()) ? std::distance(begin(), it) : npos;
		}
	private:
		array_type _data;
		size_t _count;
	};
}

