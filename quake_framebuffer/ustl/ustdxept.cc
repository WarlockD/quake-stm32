// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "ustdxept.h"
#include "mistream.h"
#include "mostream.h"
#include "strmsize.h"
#include "uiosfunc.h"
#include "uspecial.h"

namespace ustl {

	//----------------------------------------------------------------------

	/// \p arg contains a description of the error.
	error_message::error_message(const char* arg) noexcept
		: exception()
		, _arg()
	{
		try { _arg = arg; }
		catch (...) {}
		set_format(xfmt_ErrorMessage);
	}

	/// Virtual destructor
	error_message::~error_message(void) noexcept
	{
	}

	/// Returns a descriptive error message. fmt="%s: %s"
	void error_message::info(string& msgbuf, const char* fmt) const noexcept
	{
		if (!fmt) fmt = "%s: %s";
		try { msgbuf.format(fmt, name(), _arg.c_str()); }
		catch (...) {}
	}

	/// Reads the object from stream \p is.
	void error_message::read(istream& is)
	{
		exception::read(is);
		is >> _arg >> ios::align();
	}

	/// Writes the object to stream \p os.
	void error_message::write(ostream& os) const
	{
		exception::write(os);
		os << _arg << ios::align();
	}

	/// Returns the size of the written object.
	size_t error_message::stream_size(void) const noexcept
	{
		return exception::stream_size() + Align(stream_size_of(_arg));
	}

	//----------------------------------------------------------------------

	/// Initializes the empty object. \p operation is the function that returned the error code.
	system_error::system_error(const char* operation) noexcept
#if HAVE_CPP14
		: system_error(*::_errno(), system_category(), operation)
#else
		: runtime_error(strerror(errno))
		, _operation(operation)
		, _errno(errno)
#endif
	{
		set_format(xfmt_SystemError);
	}

	/// Reads the exception from stream \p is.
	void system_error::read(istream& is)
	{
		runtime_error::read(is);
		is >> _operation >> ios::align() >> _errno >> ios::align();
	}

	/// Writes the exception into stream \p os.
	void system_error::write(ostream& os) const
	{
		runtime_error::write(os);
		os << _operation << ios::align() << _errno << ios::align();
	}

	/// Returns the size of the written exception.
	size_t system_error::stream_size(void) const noexcept
	{
		return runtime_error::stream_size() +
			Align(stream_size_of(_errno)) +
			Align(stream_size_of(_operation));
	}

	//----------------------------------------------------------------------

	/// Initializes the empty object. \p operation is the function that returned the error code.
	file_exception::file_exception(const char* operation, const char* filename) noexcept
		: system_error(operation)
	{
		memset(_filename, 0, VectorSize(_filename));
		set_format(xfmt_FileException);
		if (filename) {
			strncpy(_filename, filename, VectorSize(_filename));
			_filename[VectorSize(_filename) - 1] = 0;
		}
	}

	/// Returns a descriptive error message. fmt="%s %s: %s"
	void file_exception::info(string& msgbuf, const char* fmt) const noexcept
	{
		if (!fmt) fmt = "%s %s: %s";
		try { msgbuf.format(fmt, Operation(), Filename(), _arg.c_str()); }
		catch (...) {}
	}

	/// Reads the exception from stream \p is.
	void file_exception::read(istream& is)
	{
		system_error::read(is);
		string filename;
		is >> filename;
		is.align();
		strncpy(_filename, filename.c_str(), VectorSize(_filename));
		_filename[VectorSize(_filename) - 1] = 0;
	}

	/// Writes the exception into stream \p os.
	void file_exception::write(ostream& os) const
	{
		system_error::write(os);
		os << string(_filename);
		os.align();
	}

	/// Returns the size of the written exception.
	size_t file_exception::stream_size(void) const noexcept
	{
		return system_error::stream_size() +
			Align(stream_size_of(string(_filename)));
	}

	//----------------------------------------------------------------------

	/// Initializes the empty object. \p operation is the function that returned the error code.
	stream_bounds_exception::stream_bounds_exception(const char* operation, const char* type, uoff_t offset, size_t expected, size_t remaining) noexcept
		: system_error(operation)
		, _typeName(type)
		, _offset(offset)
		, _expected(expected)
		, _remaining(remaining)
	{
		set_format(xfmt_StreamBoundsException);
	}

	/// Returns a descriptive error message. fmt="%s stream %s: @%u: expected %u, available %u";
	void stream_bounds_exception::info(string& msgbuf, const char* fmt) const noexcept
	{
		char typeName[256];
		strncpy(typeName, _typeName, VectorSize(typeName));
		typeName[VectorSize(typeName) - 1] = 0;
		if (!fmt) fmt = "%s stream %s: @0x%X: need %u bytes, have %u";
		try { msgbuf.format(fmt, demangle_type_name(VectorBlock(typeName)), Operation(), _offset, _expected, _remaining); }
		catch (...) {}
	}

	/// Reads the exception from stream \p is.
	void stream_bounds_exception::read(istream& is)
	{
		system_error::read(is);
		is >> _typeName >> _offset >> _expected >> _remaining;
	}

	/// Writes the exception into stream \p os.
	void stream_bounds_exception::write(ostream& os) const
	{
		system_error::write(os);
		os << _typeName << _offset << _expected << _remaining;
	}

	/// Returns the size of the written exception.
	size_t stream_bounds_exception::stream_size(void) const noexcept
	{
		return system_error::stream_size() +
			stream_size_of(_typeName) +
			stream_size_of(_offset) +
			stream_size_of(_expected) +
			stream_size_of(_remaining);
	}

}
