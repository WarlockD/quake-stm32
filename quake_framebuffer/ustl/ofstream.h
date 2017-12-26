// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#pragma once
#include "sistream.h"
#include "sostream.h"
#include "fstream.h"

namespace ustl {

/// \class ofstream fdostream.h ustl.h
/// \ingroup DeviceStreams
/// \brief A string stream that writes to an fd. Implements cout and cerr.
class ofstream : public ostringstream {
public:
			ofstream (void);
    explicit		ofstream (int ofd);
    explicit		ofstream (const char* filename, openmode mode = out);
    virtual		~ofstream (void) noexcept;
    inline void		open (const char* filename, openmode mode = out) { _file.open (filename, mode); clear (_file.rdstate()); }
    void		close (void);
    inline bool		is_open (void) const		{ return _file.is_open(); }
    inline iostate	exceptions (iostate v)		{ ostringstream::exceptions(v); return _file.exceptions(v); }
    inline void		setstate (iostate v)		{ ostringstream::setstate(v); _file.setstate(v); }
    inline void		clear (iostate v = goodbit)	{ ostringstream::clear(v); _file.clear(v); }
    inline off_t	tellp (void) const		{ return _file.tellp() + ostringstream::tellp(); }
    inline int		fd (void) const			{ return _file.fd(); }
    inline void		stat (struct stat& rs) const	{ _file.stat (rs); }
    inline void		set_nonblock (bool v = true)	{ _file.set_nonblock (v); }
    inline int		ioctl (const char* rname, int request, long argument = 0)	{ return _file.ioctl (rname, request, argument); }
    inline int		ioctl (const char* rname, int request, int argument)		{ return _file.ioctl (rname, request, argument); }
    inline int		ioctl (const char* rname, int request, void* argument)		{ return _file.ioctl (rname, request, argument); }
    ofstream&		seekp (off_t p, seekdir d = beg);
    virtual ostream&	flush (void) override;
    virtual size_type	overflow (size_type n = 1) override;
private:
    fstream		_file;
};

/// \class ifstream fdostream.h ustl.h
/// \ingroup DeviceStreams
/// \brief A string stream that reads from an fd. Implements cin.
class ifstream : public istringstream {
public:
			ifstream (void);
    explicit		ifstream (int ifd);
    explicit		ifstream (const char* filename, openmode mode = in);
    inline void		open (const char* filename, openmode mode = in)	{ _file.open (filename, mode); clear (_file.rdstate()); }
    inline void		close (void)			{ _file.close(); clear (_file.rdstate()); }
    inline bool		is_open (void) const		{ return _file.is_open(); }
    inline iostate	exceptions (iostate v)		{ istringstream::exceptions(v); return _file.exceptions(v); }
    inline void		setstate (iostate v)		{ istringstream::setstate(v); _file.setstate(v); }
    inline void		clear (iostate v = goodbit)	{ istringstream::clear(v); _file.clear(v); }
    inline off_t	tellg (void) const		{ return _file.tellg() - remaining(); }
    inline int		fd (void) const			{ return _file.fd(); }
    inline void		stat (struct stat& rs) const	{ _file.stat (rs); }
    inline void		set_nonblock (bool v = true)	{ _file.set_nonblock (v); }
    void		set_buffer_size (size_type sz);
    ifstream&		putback (char c)		{ ungetc(); _buffer[pos()] = c; return *this; }
    inline int		ioctl (const char* rname, int request, long argument = 0)	{ return _file.ioctl (rname, request, argument); }
    inline int		ioctl (const char* rname, int request, int argument)		{ return _file.ioctl (rname, request, argument); }
    inline int		ioctl (const char* rname, int request, void* argument)		{ return _file.ioctl (rname, request, argument); }
    ifstream&		seekg (off_t p, seekdir d = beg);
    int			sync (void);
    virtual size_type	underflow (size_type n = 1) override;
private:
    string		_buffer;
    fstream		_file;
};

extern ofstream cout, cerr;
extern ifstream cin;

} // namespace ustl
