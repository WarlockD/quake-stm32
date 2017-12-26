// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#pragma once
#if HAVE_CPP14
#include "uttraits.h"

//----------------------------------------------------------------------
namespace ustl {

class error_code;
class error_condition;

template <typename T> struct is_error_code_enum : public std::false_type { };
template <typename T> struct is_error_condition_enum : public std::false_type {};

//{{{ errc enum of error codes for error_code and error_condition ------

enum class errc {
    address_family_not_supported	= EAFNOSUPPORT,
    address_in_use			= EADDRINUSE,
    address_not_available		= EADDRNOTAVAIL,
    already_connected			= EISCONN,
    argument_list_too_long		= E2BIG,
    argument_out_of_domain		= EDOM,
    bad_address				= EFAULT,
    bad_file_descriptor			= EBADF,
    bad_message				= EBADMSG,
    broken_pipe				= EPIPE,
    connection_aborted			= ECONNABORTED,
    connection_already_in_progress	= EALREADY,
    connection_refused			= ECONNREFUSED,
    connection_reset			= ECONNRESET,
    cross_device_link			= EXDEV,
    destination_address_required	= EDESTADDRREQ,
    device_or_resource_busy		= EBUSY,
    directory_not_empty			= ENOTEMPTY,
    executable_format_error		= ENOEXEC,
    file_exists				= EEXIST,
    file_too_large			= EFBIG,
    filename_too_long			= ENAMETOOLONG,
    function_not_supported		= ENOSYS,
    host_unreachable			= EHOSTUNREACH,
    identifier_removed			= EIDRM,
    illegal_byte_sequence		= EILSEQ,
    inappropriate_io_control_operation	= ENOTTY,
    interrupted				= EINTR,
    invalid_argument			= EINVAL,
    invalid_seek			= ESPIPE,
    io_error				= EIO,
    is_a_directory			= EISDIR,
    message_size			= EMSGSIZE,
    network_down			= ENETDOWN,
    network_reset			= ENETRESET,
    network_unreachable			= ENETUNREACH,
    no_buffer_space			= ENOBUFS,
    no_child_process			= ECHILD,
    no_link				= ENOLINK,
    no_lock_available			= ENOLCK,
    no_message_available		= ENODATA,
    no_message				= ENOMSG,
    no_protocol_option			= ENOPROTOOPT,
    no_space_on_device			= ENOSPC,
    no_stream_resources			= ENOSR,
    no_such_device_or_address		= ENXIO,
    no_such_device			= ENODEV,
    no_such_file_or_directory		= ENOENT,
    no_such_process			= ESRCH,
    not_a_directory			= ENOTDIR,
    not_a_socket			= ENOTSOCK,
    not_a_stream			= ENOSTR,
    not_connected			= ENOTCONN,
    not_enough_memory			= ENOMEM,
    not_supported			= EINVAL,
    operation_canceled			= ECANCELED,
    operation_in_progress		= EINPROGRESS,
    operation_not_permitted		= EPERM,
    operation_not_supported		= EOPNOTSUPP,
    operation_would_block		= EAGAIN,
    owner_dead				= EOWNERDEAD,
    permission_denied			= EACCES,
    protocol_error			= EPROTO,
    protocol_not_supported		= EPROTONOSUPPORT,
    read_only_file_system		= EROFS,
    resource_deadlock_would_occur	= EDEADLK,
    resource_unavailable_try_again	= EAGAIN,
    result_out_of_range			= ERANGE,
    state_not_recoverable		= ENOTRECOVERABLE,
    stream_timeout			= ETIME,
    text_file_busy			= ETXTBSY,
    timed_out				= ETIMEDOUT,
    too_many_files_open_in_system	= ENFILE,
    too_many_files_open			= EMFILE,
    too_many_links			= EMLINK,
    too_many_symbolic_link_levels	= ELOOP,
    value_too_large			= EOVERFLOW,
    wrong_protocol_type			= EPROTOTYPE
};

template<> struct is_error_code_enum<errc> : public std::true_type {};
template<> struct is_error_condition_enum<errc> : public std::true_type {};

error_code make_error_code (errc) noexcept;
error_condition make_error_condition (errc) noexcept;

//}}}-------------------------------------------------------------------
//{{{ error_category

class error_category {
public:
    constexpr			error_category (void) noexcept = default;
    error_category&		operator= (const error_category&) = delete;
    inline constexpr auto	name (void) const	{ return "system"; }
    inline auto			message (int ec) const	{ return strerror(ec); }
    inline error_condition	default_error_condition (int ec) const noexcept;
    inline constexpr bool	equivalent (int ec, const error_condition& cond) const noexcept;
    inline constexpr bool	equivalent (const error_code& ec, int c) const noexcept;
    inline constexpr bool	operator== (const error_category&) const { return true; }
    inline constexpr bool	operator!= (const error_category&) const { return false; }
};

inline constexpr auto system_category (void)	{ return error_category(); }
inline constexpr auto generic_category (void)	{ return error_category(); }

//}}}-------------------------------------------------------------------
//{{{ error_code

class error_code {
public:
    inline constexpr		error_code (void) : _ec(0) { }
    inline constexpr		error_code (int ec, const error_category&) : _ec(ec) {}
    template <typename ErrorCodeEnum,
		typename = typename enable_if<is_error_code_enum<ErrorCodeEnum>::value>::type>
    inline constexpr		error_code (ErrorCodeEnum e) : error_code (make_error_code(e)) {}
    inline void			assign (int ec, const error_category&) noexcept	{ _ec = ec; }
    template <typename ErrorCodeEnum>
    inline typename std::enable_if<is_error_code_enum<ErrorCodeEnum>::value, error_code&>::type
				operator= (ErrorCodeEnum e)	{ return *this = make_error_code(e); }
    inline void			clear (void)			{ assign (0, system_category()); }
    inline constexpr auto	value (void) const		{ return _ec; }
    inline constexpr auto	category (void) const		{ return system_category(); }
    inline error_condition	default_error_condition (void) const noexcept;
    inline auto			message (void) const		{ return category().message (value()); }
    inline constexpr explicit		operator bool (void) const	{ return _ec; }
    inline void			read (istream& is);
    inline void			write (ostream& os) const;
    inline constexpr bool	operator== (const error_code& v) const	{ return category() == v.category() && value() == v.value(); }
    inline constexpr bool	operator== (const error_condition& v) const noexcept;
private:
    int				_ec;
};

inline error_code make_error_code (errc e) noexcept
    { return error_code (static_cast<int>(e), system_category()); }

//}}}-------------------------------------------------------------------
//{{{ error_condition

class error_condition {
public:
    inline constexpr		error_condition (void) : _ec(0) { }
    inline constexpr		error_condition (int ec, const error_category&) : _ec(ec) {}
    template <typename ErrorConditionEnum,
		typename = typename enable_if<is_error_condition_enum<ErrorConditionEnum>::value>::type>
    inline constexpr		error_condition (ErrorConditionEnum e) : error_condition (make_error_condition(e)) {}
    inline void			assign (int ec, const error_category&) noexcept	{ _ec = ec; }
    template <typename ErrorConditionEnum>
    inline typename std::enable_if<is_error_condition_enum<ErrorConditionEnum>::value, error_condition&>::type
				operator=(ErrorConditionEnum e)	{ return *this = make_error_condition(e); }
    inline void			clear (void)			{ assign (0, generic_category()); }
    inline constexpr auto	value (void) const		{ return _ec; }
    inline constexpr auto	category (void) const		{ return generic_category(); }
    inline auto			message (void) const		{ return category().message (value()); }
    inline constexpr explicit	operator bool (void) const	{ return value(); }
    inline void			read (istream& is);
    inline void			write (ostream& os) const;
    inline constexpr bool	operator== (const error_condition& v) const
				    { return category() == v.category() && value() == v.value(); }
    inline constexpr bool	operator== (const error_code& v) const
				    { return category() == v.category() && value() == v.value(); }
private:
    int				_ec;
};

inline error_condition make_error_condition (errc e) noexcept
    { return error_condition (static_cast<int>(e), generic_category()); }

error_condition	error_category::default_error_condition (int ec) const noexcept
    { return error_condition (ec, *this); }
constexpr bool error_category::equivalent (int ec, const error_condition& cond) const noexcept
    { return *this == cond.category() && ec == cond.value(); }
constexpr bool error_category::equivalent (const error_code& ec, int c) const noexcept
    { return *this == ec.category() && c == ec.value(); }

error_condition	error_code::default_error_condition (void) const noexcept
    { return error_condition (value(), category()); }
constexpr bool error_code::operator== (const error_condition& v) const noexcept
    { return category() == v.category() && value() == v.value(); }

//}}}-------------------------------------------------------------------
} // namespace ustl
#endif // HAVE_CPP14
