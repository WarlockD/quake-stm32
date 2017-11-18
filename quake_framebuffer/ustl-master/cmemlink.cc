// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.
#include "icommon.h"
#include "cmemlink.h"
#include "strmsize.h"

namespace qstl {

/// \brief Attaches the object to pointer \p p of size \p n.
///
/// If \p p is nullptr and \p n is non-zero, bad_alloc is thrown and current
/// state remains unchanged.
///
void cmemlink::link (const void* p, size_type n)
{
	if (!p && n)
		throw std::bad_alloc(); // (n);
    unlink();
    relink (p, n);
}
size_t AlignWrite(ostream& os, const char* cdata, size_t size) {
	const cmemlink::written_size_type sz(size);
	assert(sz == size && "No support for writing memblocks larger than 4G");
	os << sz;
	os.write(cdata, size);
	size_t aligment = stream_align_of(sz);
	if (aligment != sz) {
		size += aligment;
		while (aligment--) os.put(0);
	}
	return size;
}
/// Writes the object to stream \p os
void cmemlink::write (ostream& os) const
{
	AlignWrite(os, cdata(), size());
}

/// Writes the object to stream \p os
void cmemlink::text_write (ostringstream& os) const
{
    os.write (begin(), readable_size());
}

/// Returns the number of bytes required to write this object to a stream.
cmemlink::size_type cmemlink::stream_size (void) const noexcept
{
    const written_size_type sz (size());
    return static_cast<cmemlink::size_type>(Align (stream_size_of (sz) + sz, stream_align_of(sz)));
}

/// Writes the data to file \p "filename".
void cmemlink::write_file (const char* filename, int mode) const
{
    fstream f;
    f.exceptions (fstream::badbit  | fstream::failbit);
    f.open (filename, fstream::out | fstream::trunc, mode);
    f.write (cdata(), readable_size());
    f.close();
}

/// Compares to memory block pointed by l. Size is compared first.
bool cmemlink::operator== (const cmemlink& l) const noexcept
{
    return l._size == _size &&
	    (l._data == _data || 0 == std::memcmp (l._data, _data, _size));
}

} // namespace ustl
