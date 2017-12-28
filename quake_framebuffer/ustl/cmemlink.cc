// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "cmemlink.h"
#include "ofstream.h"
#include "strmsize.h"
#include "ualgo.h"

namespace ustl {

	/// \brief Attaches the object to pointer \p p of size \p n.
	///
	/// If \p p is nullptr and \p n is non-zero, bad_alloc is thrown and current
	/// state remains unchanged.
	///
	
	/// Writes the object to stream \p os
	void cmemlink::write(ostream& os) const
	{
		const written_size_type sz(size());
		assert(sz == size() && "No support for writing memblocks larger than 4G");
		os << sz;
		os.write(data(), sz);
		os.align(stream_align_of(sz));
	}

	/// Writes the object to stream \p os
	void cmemlink::text_write(ostringstream& os) const
	{
		os.write(begin(), readable_size());
	}

	/// Returns the number of bytes required to write this object to a stream.
	cmemlink::size_type cmemlink::stream_size(void) const noexcept
	{
		const written_size_type sz(size());
		return Align(stream_size_of(sz) + sz, stream_align_of(sz));
	}

	/// Writes the data to file \p "filename".
	void cmemlink::write_file(const char* filename, int mode) const
	{
		fstream f;
		f.exceptions(fstream::allbadbits);
		f.open(filename, fstream::out | fstream::trunc, mode);
		f.write(data(), readable_size());
		f.close();
	}



}