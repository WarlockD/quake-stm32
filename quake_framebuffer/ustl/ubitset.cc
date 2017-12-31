// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "ubitset.h"

namespace ustl {

	/// Copies bits from \p v of size \p n into \p buf as MSB "1011001..." LSB
	/// If \p buf is too small, MSB bits will be truncated.
	template<typename T>
	void convert_to_bitstring(const bitset_value_type* v, size_t n, string_builder<T>& buf) noexcept
	{
		string::iterator stri = buf.end();
		for (size_t i = 0; i < n && stri > buf.begin(); ++i)
			for (bitset_value_type b = 1; b && stri > buf.begin(); b <<= 1)
				*--stri = (v[i] & b) ? '1' : '0';
	}

	/// Copies bits from \p buf as MSB "1011001..." LSB into \p v of size \p n.
	template<typename T>
	void convert_from_bitstring(const string_helper<T>& buf, bitset_value_type* v, size_t n) noexcept
	{
		string::const_iterator stri = buf.end();
		for (size_t i = 0; i < n; ++i) {
			for (bitset_value_type b = 1; b; b <<= 1) {
				if (stri == buf.begin() || *--stri == '0')
					v[i] &= ~b;
				else
					v[i] |= b;
			}
		}
	}

} // namespace ustl
