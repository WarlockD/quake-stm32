#include	"quakedef.h"

#include "qvector.h"


namespace quake {
	namespace detail {

		size_type str_hash(const_pointer first, const_pointer last) noexcept {
			size_type h = 0;
			// This has the bits flowing into each other from both sides of the number
			for (; first < last; ++first)
				h = static_cast<difference_type>(*first) + ((h << 7) | (h >> ((sizeof(size_type) * 8) - 7)));
			return h;
		}
		int str_length_compare(size_type __n1, size_type __n2) noexcept {
			const difference_type __diff = __n1 - __n2;
			if (__diff > std::numeric_limits<int>::max()) return std::numeric_limits<int>::max();
			if (__diff < std::numeric_limits<int>::min()) return std::numeric_limits<int>::min();
			return static_cast<int>(__diff);
		}
		int str_compare(const_pointer __str1, size_type __n1, const_pointer __str2, size_type __n2) noexcept {
			int __diff = str_length_compare(__n1, __n2);
			if (__diff == 0) __diff = traits_type::compare(__str1, __str2, __n1);
			//	if (__diff == 0) for (size_t i = 0; i < __n1 && (__diff = int(__str1[i]) - int(__str2[i])) == 0; i++);
			return __diff;
		}
		int str_case_compare(const_pointer __str1, size_type __n1, const_pointer __str2, size_type __n2) noexcept {
			int __diff = str_length_compare(__n1, __n2);
			if (__diff == 0) for (size_t i = 0; i < __n1 && (__diff = int(char_tolower(__str1[i])) - int(char_tolower(__str2[i]))) == 0; i++);
			return __diff;
		}
		size_type str_find(const_pointer __str1, size_type __n1, value_type c) noexcept {
			const_pointer p = traits_type::find(__str1, __n1, c);
			return p ? p - __str1 : npos;
		}
		size_type str_find(const_pointer __str1, size_type __n1, const_pointer __str2, size_type __n2) noexcept {
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
		size_type	str_rfind(const_pointer __str1, size_type __n1, const_pointer __str2, size_type __n2)noexcept {
			if (__n2 <= __n1) {
				for (size_t pos = (__n1 - __n2); pos >= 0; --pos) {
					if (traits_type::eq(__str1[pos], __str2[0])
						&& traits_type::compare(__str1 + pos + 1,
							__str2 + 1, __n2 - 1) == 0)
						return pos;
				}
			}
			return npos;
		}
		/// Returns the offset of the first occurence of one of characters in \p s of size \p n after \p pos.
		size_type str_find_first_of(const_pointer __str1, size_type __n1, const_pointer __str2, size_type __n2)noexcept
		{
			for (size_type i = 0; i < __n1; i++) {
				if (str_find(__str2, __n2, __str1[i]) != npos)
					return i;

			}
			return npos;

		}
		/// Returns the offset of the first occurence of one of characters in \p s of size \p n after \p pos.
		size_type find_first_not_of(const_pointer __str1, size_type __n1, const_pointer __str2, size_type __n2)noexcept
		{
			for (size_type i = 0; i < __n1; i++) {
				if (str_find(__str2, __n2, __str1[i]) == npos)
					return i;
				return npos;
			}
		}
		/// Returns the offset of the first occurence of one of characters in \p s of size \p n after \p pos.
		size_type find_last_of(const_pointer __str1, size_type __n1, const_pointer __str2, size_type __n2)noexcept
		{
			for (size_type i = __n1; i >= 0; --i) {
				if (str_find(__str2, __n2, __str1[i]) != npos)
					return i;
		
			}
			return npos;
		}
		/// Returns the offset of the first occurence of one of characters in \p s of size \p n after \p pos.
		size_type find_last_not_of(const_pointer __str1, size_type __n1, const_pointer __str2, size_type __n2)noexcept
		{
			for (size_type i = __n1; i >= 0; --i) {
				if (str_find(__str2, __n2, __str1[i]) == npos)
					return i;
			}
			return npos;
		}
	}




}