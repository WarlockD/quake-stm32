// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#pragma once
#include "uexception.h"
#include "ustring.h"
#include "uerror.h"

namespace ustl {

enum {
    xfmt_ErrorMessage	= xfmt_BadAlloc+1,
    xfmt_LogicError	= xfmt_ErrorMessage,
    xfmt_RuntimeError	= xfmt_ErrorMessage,
    xfmt_SystemError,
    xfmt_FileException	= 13,
    xfmt_StreamBoundsException
};

/// \class logic_error ustdxept.h ustl.h
/// \ingroup Exceptions
///
/// \brief Logic errors represent problems in the internal logic of the program.
///
class error_message : public exception {
public:
    explicit		error_message (const char* arg) noexcept;
    virtual		~error_message (void) noexcept;
    inline virtual const char*	what (void) const noexcept override { return _arg.c_str(); }
    inline virtual const char*	name (void) const noexcept { return "error"; }
    virtual void	info (string& msgbuf, const char* fmt = nullptr) const noexcept override;
    virtual void	read (istream& is) override;
    virtual void	write (ostream& os) const override;
    virtual size_t	stream_size (void) const noexcept override;
protected:
    string		_arg;
};

/// \class logic_error ustdxept.h ustl.h
/// \ingroup Exceptions
///
/// \brief Logic errors represent problems in the internal logic of the program.
///
class logic_error : public error_message {
public:
    inline explicit		logic_error (const char* arg) noexcept : error_message (arg) {}
    inline virtual const char*	name (void) const noexcept override { return "logic error"; }
};

/// \class domain_error ustdxept.h ustl.h
/// \ingroup Exceptions
///
/// \brief Reports domain errors ("domain" is in the mathematical sense)
///
class domain_error : public logic_error {
public:
    inline explicit		domain_error (const char* arg) noexcept : logic_error (arg) {}
    inline virtual const char*	name (void) const noexcept override { return "domain error"; }
};

/// \class invalid_argument ustdxept.h ustl.h
/// \ingroup Exceptions
///
/// \brief Reports an invalid argument to a function.
///
class invalid_argument : public logic_error {
public:
    inline explicit		invalid_argument (const char* arg) noexcept : logic_error (arg) {}
    inline virtual const char*	name (void) const noexcept override { return "invalid argument"; }
};

/// \class length_error ustdxept.h ustl.h
/// \ingroup Exceptions
///
/// \brief Reports when an object exceeds its allowed size.
///
class length_error : public logic_error {
public:
    inline explicit		length_error (const char* arg) noexcept : logic_error (arg) {} 
    inline virtual const char*	name (void) const noexcept override { return "length error"; }
};

/// \class out_of_range ustdxept.h ustl.h
/// \ingroup Exceptions
///
/// \brief Reports arguments with values out of allowed range.
///
class out_of_range : public logic_error {
public:
    inline explicit		out_of_range (const char* arg) noexcept : logic_error (arg) {}
    inline virtual const char*	name (void) const noexcept override { return "out of range"; }
};

/// \class runtime_error ustdxept.h ustl.h
/// \ingroup Exceptions
///
/// \brief Reports errors that are dependent on the data being processed.
///
class runtime_error : public error_message {
public:
    inline explicit		runtime_error (const char* arg) noexcept : error_message (arg) {}
    inline virtual const char*	name (void) const noexcept override { return "runtime error"; }
};

/// \class range_error ustdxept.h ustl.h
/// \ingroup Exceptions
///
/// \brief Reports data that does not fall within the permitted range.
///
class range_error : public runtime_error {
public:
    inline explicit		range_error (const char* arg) noexcept : runtime_error (arg) {}
    inline virtual const char*	name (void) const noexcept override { return "range error"; }
};

/// \class overflow_error ustdxept.h ustl.h
/// \ingroup Exceptions
///
/// \brief Reports arithmetic overflow.
///
class overflow_error : public runtime_error {
public:
    inline explicit		overflow_error (const char* arg) noexcept : runtime_error (arg) {}
    inline virtual const char*	name (void) const noexcept override { return "overflow error"; }
};

/// \class underflow_error ustdxept.h ustl.h
/// \ingroup Exceptions
///
/// \brief Reports arithmetic underflow.
///
class underflow_error : public runtime_error {
public:
    inline explicit		underflow_error (const char* arg) noexcept : runtime_error (arg) {}
    inline virtual const char*	name (void) const noexcept override { return "underflow error"; }
};

/// \class system_error uexception.h ustl.h
/// \ingroup Exceptions
///
/// \brief Thrown when a libc function returns an error.
///
/// Contains an errno and description. This is a uSTL extension.
///
class system_error : public runtime_error {
public:
    explicit		system_error (const char* operation) noexcept;
    inline virtual const char*	what (void) const noexcept override { return "system error"; }
    inline virtual const char*	name (void) const noexcept override { return _operation.c_str(); }
    virtual void	read (istream& is) override;
    virtual void	write (ostream& os) const override;
    virtual size_t	stream_size (void) const noexcept override;
    inline int		Errno (void) const	{ return _errno; }
    inline const char*	Operation (void) const	{ return _operation.c_str(); }
#if HAVE_CPP14
			system_error (error_code ec, const char* operation) noexcept
			    : runtime_error (ec.message()),_operation(operation),_errno(ec.value()) {}
    inline              system_error (error_code ec, const string& operation) noexcept
			    : system_error (ec, operation.c_str()) {}
    inline              system_error (int ec, const error_category& ecat, const char* operation)
			    : system_error (error_code(ec,ecat), operation) {}
    inline              system_error (int ec, const error_category& ecat, const string& operation)
			    : system_error (ec, ecat, operation.c_str()) {}
    inline auto		code (void) const	{ return error_code (_errno, system_category()); }
#endif
private:
    string		_operation;	///< Name of the failed operation.
    int			_errno;		///< Error code returned by the failed operation.
};

typedef system_error libc_exception;

/// \class file_exception uexception.h ustl.h
/// \ingroup Exceptions
///
/// \brief File-related exceptions.
///
/// Contains the file name. This is a uSTL extension.
///
class file_exception : public system_error {
public:
			file_exception (const char* operation, const char* filename) noexcept;
    inline virtual const char* what (void) const noexcept override { return "file error"; }
    virtual void	info (string& msgbuf, const char* fmt = nullptr) const noexcept override;
    virtual void	read (istream& is) override;
    virtual void	write (ostream& os) const override;
    virtual size_t	stream_size (void) const noexcept override;
    inline const char*	Filename (void) const	{ return _filename; }
private:
    char		_filename [PATH_MAX];	///< Name of the file causing the error.
};

/// \class stream_bounds_exception uexception.h ustl.h
/// \ingroup Exceptions
///
/// \brief Stream bounds checking.
///
/// Only thrown in debug builds unless you say otherwise in config.h
/// This is a uSTL extension.
///
class stream_bounds_exception : public system_error {
public:
			stream_bounds_exception (const char* operation, const char* type, uoff_t offset, size_t expected, size_t remaining) noexcept;
    inline virtual const char*	what (void) const noexcept override { return "stream bounds exception"; }
    virtual void	info (string& msgbuf, const char* fmt = nullptr) const noexcept override;
    virtual void	read (istream& is) override;
    virtual void	write (ostream& os) const override;
    virtual size_t	stream_size (void) const noexcept override;
    inline const char*	TypeName (void) const	{ return _typeName; }
    inline uoff_t	Offset (void) const	{ return _offset; }
    inline size_t	Expected (void) const	{ return _expected; }
    inline size_t	Remaining (void) const	{ return _remaining; }
private:
    const char*		_typeName;
    uoff_t		_offset;
    size_t		_expected;
    size_t		_remaining;
};

} // namespace ustl
