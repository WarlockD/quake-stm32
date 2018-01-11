#include	"quakedef.h"

#include "qvector.h"
#include "qftoa.h"

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

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
		size_type str_find(const_pointer __str1, size_type __n1, size_type __pos, value_type c) noexcept {
			assert(__pos < __n1);
			const_pointer p = traits_type::find(__str1+ __pos, __n1- __pos, c);
			return p ? p - __str1 : npos;
		}
		size_type str_find(const_pointer __str1, size_type __n1, size_type __pos, const_pointer __str2, size_type __n2) noexcept {
			if (__n2 > 0U && __n2 <= (__n1 - __pos)) {
				for (; __pos <= (__n1 - __n2); ++__pos)
					if (traits_type::eq(__str1[__pos], __str2[0])
						&& traits_type::compare(__str1 + __pos + 1,
							__str2 + 1, __n2 - 1) == 0)
						return __pos;
			}
			return npos;
		}
		size_type str_rfind(const_pointer __str1, size_type __n1, size_type __pos, value_type c) noexcept {
			if (__n1 > 0) {
				for (__pos = std::min(__pos, __n1 - 1) ; __pos >= 0; --__pos)
					if (traits_type::eq(__str1[__pos], c))
						return __pos;
			}
			return npos;
		}
		// from ustl, IO wonder if we have to even check if _n1 > 0
		size_type	str_rfind(const_pointer __str1, size_type __n1, size_type __pos, const_pointer __str2, size_type __n2)noexcept {
			if (__n1 > 0) {
				__pos = std::min(__n1 - 1, __pos);
				const_pointer d = __str1 + __pos - 1;
				const_pointer sp = __str1 + __n2 - 1; // 
				const_pointer m = __str2 + __n2 - 1;
				for (int i = 0; d > sp && size_type(i) < __n2; --d)
					for (i = 0; size_type(i) < __n2; ++i)
						if (m[-i] != d[-i]) break; // smart we compare it backwards
				if (d > sp)
					return   (size_type)std::distance(__str1, d + 2 - __n2);
			}
			return npos;
		}
		/// Returns the offset of the first occurence of one of characters in \p s of size \p n after \p pos.
		size_type str_find_first_of(const_pointer __str1, size_type __n1, size_type __pos, const_pointer __str2, size_type __n2)noexcept
		{
			if (__n2 == 1) return str_find(__str1, __n1, __pos, __str2[0]);
			for (__pos = std::min(__pos, __n1); __pos< __n1; __pos++) // simple and soo not optimized
				if (traits_type::find(__str2, __n2, __str1[__pos])!=nullptr) return __pos;
			return npos;
		}
		/// Returns the offset of the first occurence of one of characters in \p s of size \p n after \p pos.
		size_type find_first_not_of(const_pointer __str1, size_type __n1, size_type __pos, const_pointer __str2, size_type __n2)noexcept
		{
			if (__n2 == 1) return str_rfind(__str1, __n1, __pos, __str2[0]);
			for (__pos = std::min(__pos, __n1); __pos< __n1; __pos++) 
				if (traits_type::find(__str2, __n2, __str1[__pos]) == nullptr) return __pos;
			return npos;
		}
		/// Returns the offset of the first occurence of one of characters in \p s of size \p n after \p pos.
		size_type find_last_of(const_pointer __str1, size_type __n1, size_type __pos, const_pointer __str2, size_type __n2)noexcept
		{
			for (__pos = std::min(__pos, __n1-1); int(__pos)>=0; __pos--)
				if (traits_type::find(__str2, __n2, __str1[__pos]) != nullptr) 
					return __pos;
			return npos;
		}
		/// Returns the offset of the first occurence of one of characters in \p s of size \p n after \p pos.
		size_type find_last_not_of(const_pointer __str1, size_type __n1, size_type __pos, const_pointer __str2, size_type __n2)noexcept
		{
			for (__pos = std::min(__pos, __n1 - 1); int(__pos) >= 0; __pos--)
				if (traits_type::find(__str2, __n2, __str1[__pos]) == nullptr) return __pos;
			return npos;
		}

		to_chars_result __fast_to_chars(char* first, char* last, float val, const __chars_format fmt ,  size_t precision ) {

			char* end = fftoa::diy_fp<float>::to_string(val, first, precision);
			assert(end < last);
			return { end, std::error_code() };
		}
	}

	void zstring::_reserve(size_t s) {
		if (s <= _capacity) return;
		pointer newBlock = nullptr;
		if (_capacity == 0U && _data && _size) { // if we have exsiting const data
			string_view sv = *this; // save it
			_capacity = std::min(s, _size + 1);
			_size = 0; // clear the size
			_data = (pointer)Z_Malloc(std::min(s, _size +1)); // alloc fresh data
			assign(sv); // then assign it with the old const data
		}
		else {
			// otherwise just reallocate
			pointer newBlock = (pointer)Z_Realloc(_data, s);
			assert(newBlock);
			_data = newBlock;
			_capacity = s;
		}
	}
	void zstring::_resize(size_t s) {
		assert(s < _capacity); // the engine should run reserve first, so don't run here
		_size = s;
	}
}