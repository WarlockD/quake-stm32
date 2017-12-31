// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "ustring.h"
#include "mistream.h"
#include "mostream.h"
#include "ualgo.h"
#include <stdio.h>	// for vsnprintf (in string::format)

namespace ustl {



	//utility functions
	namespace util {
		using pos_type = size_t;
		using size_type = size_t;
		using hashvalue_t = uint32_t;
		/// Returns a hash value for [first, last)
		hashvalue_t str_hash(const char* first, const char* last) noexcept // static
		{
			hashvalue_t h = 0;
			// This has the bits flowing into each other from both sides of the number
			for (; first < last; ++first)
				h = *first + ((h << 7) | (h >> (BitsInType(hashvalue_t) - 7)));
			return h;
		}
		/// Returns comparison value regarding string \p s.
		/// The return value is:
		/// \li 1 if this string is greater (by value, not length) than string \p s
		/// \li 0 if this string is equal to string \p s
		/// \li -1 if this string is less than string \p s
		///
		int str_compare(const char* first1, const char* last1, const char* first2, const char* last2) noexcept
		{
			assert(first1 <= last1 && (first2 <= last2 || !last2) && "Negative ranges result in memory allocation errors.");
			const size_type len1 = distance(first1, last1);
			const size_type len2 = distance(first2, last2);
			const int rvbylen = sign(int(len1 - len2));
			int rv = ::memcmp(first1, first2, min(len1, len2));
			return rv ? rv : rvbylen;
		}


		/// Returns the offset of the first occurence of \p c after \p pos.
		pos_type str_find(const cmemlink&  hay, char c, pos_type pos)  noexcept {
			auto found = ::ustl::find(hay.data() + pos, hay.data() + hay.size(), c);
			return found < hay.end() ? (size_t)distance(hay.data(), found) : size_t(-1);
		}
		pos_type str_find(const cmemlink&  hay, const cmemlink&  needle, pos_type pos) noexcept {
			size_t endi = needle.size() - 1;
			char endchar = needle.data()[endi];
			size_t lastPos = endi;
			while (lastPos-- && needle.data()[lastPos] != endchar);
			const size_t skip = endi - lastPos;
			auto i = hay.data() + pos + endi;
			for (; i < hay.end() && (i = ::ustl::find(i, hay.end(), endchar)) < hay.end(); i += skip)
				if (memcmp(i - endi, needle.data(), needle.size()) == 0)
					return distance(hay.data(), i) - endi;
			return size_t(-1);
		}
		pos_type str_rfind(const cmemlink&  hay, char c, pos_type pos)  noexcept {
			for (pos_type i = min(pos, pos_type(hay.size() - 1)); i >= 0; --i)
				if (hay.data()[i] == c) return i;
			return npos;
		}
		pos_type str_rfind(const cmemlink&  hay, const cmemlink& s, pos_type pos)  noexcept {
			auto d = hay.data() + pos - 1;
			auto sp = hay.begin() + s.size() - 1;
			auto m = s.end() - 1;
			for (long int i = 0; d > sp && size_type(i) < s.size(); --d)
				for (i = 0; size_type(i) < s.size(); ++i)
					if (m[-i] != d[-i])
						break;
			return d > sp ? (pos_type)distance(hay.data(), d + 2 - s.size()) : npos;
		}
		pos_type str_find_first_of(const cmemlink&  hay, const cmemlink& s, pos_type pos)  noexcept {
			for (size_type i = min(size_type(pos), hay.size()); i < hay.size(); ++i)
				if (str_find(s, hay.data()[i]) != npos)
					return i;
			return npos;
		}
		pos_type str_find_last_of(const cmemlink&  hay, const cmemlink& s, pos_type pos )  noexcept {
			for (pos_type i = min(size_type(pos), hay.size() - 1); i >= 0; --i)
				if (str_find(s, hay.data()[i]) != npos)
					return i;
			return npos;
		}
		pos_type str_find_last_not_of(const cmemlink&  hay, const cmemlink& s, pos_type pos)  noexcept {
			for (pos_type i = min(pos, pos_type(hay.size() - 1)); i >= 0; --i)
				if (str_find(s, hay.data()[i]) == npos)
					return i;
			return npos;
		}
		pos_type str_find_first_not_of(const cmemlink&  hay, const cmemlink& s, pos_type pos )  noexcept {
			for (size_type i = min(size_type(pos), hay.size()); i < hay.size(); ++i)
				if (str_find(s, hay.data()[i]) == npos)
					return i;
			return npos;
		}

	}
}
