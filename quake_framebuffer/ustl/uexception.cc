// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "uexception.h"
#include "ustring.h"
#include "mistream.h"
#include "sostream.h"
#include "strmsize.h"
#include "uspecial.h"

namespace ustl {

//----------------------------------------------------------------------

/// \brief Returns a descriptive error message. fmt="%s"
/// Overloads of this functions must set nullptr as the default fmt
/// argument and handle that case to provide a default format string
/// in case the user does not have a localized one. The format
/// string should be shown in the documentation to not require
/// translators to look through code. Also, this function must
/// not throw anything, so you must wrap memory allocation routines
/// (like string::format, for instance) in a try{}catch(...){} block.
///
void exception::info (string& msgbuf, const char*) const noexcept
{
    try { msgbuf.format ("%s", what()); } catch (...) {} // Ignore all exceptions
}

/// Reads the exception from stream \p is.
void exception::read (istream& is)
{
    uint32_t stmSize = 0;
    xfmt_t fmt = xfmt_Exception;
    is >> fmt >> stmSize >> _backtrace;
    assert (fmt == _format && "The saved exception is of a different type.");
    assert ((stmSize + 8) - exception::stream_size() <= is.remaining() && "The saved exception data is corrupt.");
    _format = fmt;
}

/// Writes the exception into stream \p os as an IFF chunk.
void exception::write (ostream& os) const
{
    os << _format << uint32_t(stream_size() - 8) << _backtrace;
}

/// Writes the exception as text into stream \p os.
void exception::text_write (ostringstream& os) const noexcept
{
    try {
	string buf;
	info (buf);
	os << buf;
    } catch (...) {}
}

//----------------------------------------------------------------------

#if HAVE_CXXABI_H && WANT_NAME_DEMANGLING
extern "C" char* __cxa_demangle (const char* mangled_name, char* output_buffer, size_t* length, int* status);
#endif

/// \brief Uses C++ ABI call, if available to demangle the contents of \p buf.
///
/// The result is written to \p buf, with the maximum size of \p bufSize, and
/// is zero-terminated. The return value is \p buf.
///
const char* demangle_type_name (char* buf, size_t bufSize, size_t* pdmSize) noexcept
{
    size_t bl = strlen (buf);
#if HAVE_CXXABI_H && WANT_NAME_DEMANGLING
    char dmname [256];
    size_t sz = VectorSize(dmname);
    int bFailed;
    __cxa_demangle (buf, dmname, &sz, &bFailed);
    if (!bFailed) {
	bl = min (strlen (dmname), bufSize - 1);
	memcpy (buf, dmname, bl);
	buf[bl] = 0;
    }
#else
    bl = min (bl, bufSize);
#endif
    if (pdmSize)
	*pdmSize = bl;
    return buf;
}

#if WITHOUT_LIBSTDCPP
} // namespace ustl
namespace std {
#endif

/// Initializes the empty object. \p nBytes is the size of the attempted allocation.
bad_alloc::bad_alloc (size_t nBytes) noexcept
: ustl::exception()
,_bytesRequested (nBytes)
{
    set_format (ustl::xfmt_BadAlloc);
}

/// Returns a descriptive error message. fmt="failed to allocate %d bytes"
void bad_alloc::info (ustl::string& msgbuf, const char* fmt) const noexcept
{
    if (!fmt) fmt = "failed to allocate %d bytes";
    try { msgbuf.format (fmt, _bytesRequested); } catch (...) {}
}

/// Reads the exception from stream \p is.
void bad_alloc::read (ustl::istream& is)
{
    ustl::exception::read (is);
    is >> _bytesRequested;
}

/// Writes the exception into stream \p os.
void bad_alloc::write (ustl::ostream& os) const
{
    ustl::exception::write (os);
    os << _bytesRequested;
}

/// Returns the size of the written exception.
size_t bad_alloc::stream_size (void) const noexcept
{
    return ustl::exception::stream_size() + ustl::stream_size_of(_bytesRequested);
}

} // namespace std
