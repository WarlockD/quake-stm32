// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "fstream.h"
#include "ustdxept.h"
#include "uutility.h"
#include <stdio.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef _MSC_VER
#include <sys/ioctrl.h>
#else
#include <io.h>

#endif



#if HAVE_SYS_MMAN_H
    #include <sys/mman.h>
#endif

namespace ustl {

/// Default constructor.
fstream::fstream (void) noexcept
: ios_base()
,_fd (-1)
,_filename()
{
    exceptions (goodbit);
}

/// Opens \p filename in \p mode.
fstream::fstream (const char* filename, openmode mode)
: ios_base()
,_fd (-1)
,_filename()
{
    exceptions (goodbit);
    open (filename, mode);
}

/// Attaches to \p nfd of \p filename.
fstream::fstream (int nfd, const char* filename)
: ios_base()
,_fd (-1)
,_filename()
{
    exceptions (goodbit);
    attach (nfd, filename);
}

/// Destructor. Closes if still open, but without throwing.
fstream::~fstream (void) noexcept
{
    clear (goodbit);
    exceptions (goodbit);	// Turn off exceptions
    close();			//  so close will not throw
    assert (!(rdstate() & badbit) && "close failed in the destructor! This may lead to loss of user data. Please call close() manually and either enable exceptions or check the badbit.");
}

/// Sets state \p s and throws depending on the exception setting.
void fstream::set_and_throw (iostate s, const char* op)
{
    if (ios_base::set_and_throw (s))
	throw file_exception (op, name());
}

/// Attaches to the given \p nfd.
void fstream::attach (int nfd, const char* filename)
{
    assert (filename && "Don't do that");
    _filename = filename;
    clear (goodbit);
    if (nfd < 0)
	set_and_throw (badbit, "open");
    close();
    _fd = nfd;
}

/// Detaches from the current fd.
void fstream::detach (void) noexcept
{
    _fd = -1;
    _filename.clear();
}
#ifdef _MSC_VER
#define O_ACCMODE (_O_RDWR | _O_WRONLY)
#endif
/// Converts openmode bits into libc open flags.
int fstream::om_to_flags (openmode m) noexcept // static
{
    static const int s_OMFlags [nombits] = {
	0,		// in
	O_CREAT,	// out
	O_APPEND,	// app
	O_APPEND,	// ate
	0,		// binary
	O_TRUNC,	// trunc
#ifdef _MSC_VER
	0,
#else
	O_NONBLOCK,	// nonblock
#endif
	0,		// nocreate
#ifdef _MSC_VER
	0,
#else
	O_NOCTTY	// noctty
#endif

    };
    int flags;
    if (O_RDONLY == in-1 && O_WRONLY == out-1 && O_RDWR == (in|out)-1)
	flags = (m - 1) & O_ACCMODE;
    else
	flags = ((m&(in|out))==(in|out)) ? O_RDWR : ((m&out) ? O_WRONLY : O_RDONLY);
    for (uoff_t i = 0; i < VectorSize(s_OMFlags); ++ i)
	flags |= s_OMFlags[i] & (!(m&(1<<i))-1);
    if (m & nocreate)
	flags &= ~O_CREAT;
    return flags;
}

/// \brief Opens \p filename in the given mode.
/// \warning The string at \p filename must exist until the object is closed.
void fstream::open (const char* filename, openmode mode, int perms)
{
    int nfd = ::open (filename, om_to_flags(mode), perms);
    attach (nfd, filename);
}

/// Closes the file and throws on error.
void fstream::close (void)
{
    if (_fd < 0)
	return;	// already closed
    while (::close(_fd)) {
	if (errno != EINTR) {
	    set_and_throw (badbit | failbit, "close");
	    break;
	}
    }
    detach();
}

/// Moves the current file position to \p n.
off_t fstream::seek (off_t n, seekdir whence)
{
    off_t p = lseek (_fd, n, whence);
    if (p < 0)
	set_and_throw (failbit, "seek");
    return p;
}

/// Returns the current file position.
off_t fstream::pos (void) const noexcept
{
    return lseek (_fd, 0, SEEK_CUR);
}

/// Reads \p n bytes into \p p.
off_t fstream::read (void* p, off_t n)
{
    off_t br = 0;
    while ((br < n) & good())
	br += readsome (advance (p, br), n - br);
    return br;
}

/// Reads at most \p n bytes into \p p, stopping when it feels like it.
off_t fstream::readsome (void* p, off_t n)
{
    int brn;
    do { brn = ::read (_fd, p, n); } while ((brn < 0) & (errno == EINTR));
    if (brn > 0)
	return brn;
    else if ((brn < 0) & (errno != EAGAIN))
	set_and_throw (failbit, "read");
    else if (!brn && ios_base::set_and_throw (eofbit | failbit))
	throw stream_bounds_exception ("read", name(), pos(), n, 0);
    return 0;
}

/// Writes \p n bytes from \p p.
off_t fstream::write (const void* p, off_t n)
{
    off_t btw = n;
    while (btw) {
	const off_t bw = n - btw;
	int bwn = ::write (_fd, advance(p,bw), btw);
	if (bwn > 0)
	    btw -= bwn;
	else if (!bwn) {
	    if (ios_base::set_and_throw (eofbit | failbit))
		throw stream_bounds_exception ("write", name(), pos() - bw, n, bw);
	    break;
	} else if (errno != EINTR) {
	    if (errno != EAGAIN)
		set_and_throw (failbit, "write");
	    break;
	}
    }
    return n - btw;
}

/// Returns the file size.
off_t fstream::size (void) const
{
    struct stat st;
    st.st_size = 0;
    stat (st);
    return st.st_size;
}

/// Synchronizes the file's data and status with the disk.
void fstream::sync (void)
{
#ifdef _MSC_VER
	if(_commit(_fd))
#else
	if (_sync(_fd))
#endif
	set_and_throw (badbit | failbit, "sync");
}

/// Get the stat structure.
void fstream::stat (struct stat& rs) const
{
    if (fstat (_fd, &rs))
	throw file_exception ("stat", name());
}

/// Calls the given ioctl. Use IOCTLID macro to pass in both \p name and \p request.
int fstream::ioctl (const char* rname, int request, long argument)
{
#ifndef _MSC_VER
    int rv = ::ioctl (_fd, request, argument);
    if (rv < 0)
	set_and_throw (failbit, rname);
    return rv;
#else
	return -1;
#endif
}

/// Calls the given fcntl. Use FCNTLID macro to pass in both \p name and \p request.
int fstream::fcntl (const char* rname, int request, long argument)
{
#ifndef _MSC_VER
    int rv = ::fcntl (_fd, request, argument);
    if (rv < 0)
	set_and_throw (failbit, rname);
    return rv;
#else
	return -1;
#endif
}

void fstream::set_nonblock (bool v) noexcept
{
#ifndef _MSC_VER
    int curf = max (0, fcntl (FCNTLID (F_GETFL)));
    if (v) curf |=  O_NONBLOCK;
    else   curf &= ~O_NONBLOCK;
    fcntl (FCNTLID (F_SETFL), curf);
#endif
}

#if HAVE_SYS_MMAN_H

/// Memory-maps the file and returns a link to it.
memlink fstream::mmap (off_t n, off_t offset)
{
    void* result = ::mmap (nullptr, n, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, offset);
    if (result == MAP_FAILED)
	set_and_throw (failbit, "mmap");
    return memlink (result, n);
}

/// Unmaps a memory-mapped area.
void fstream::munmap (memlink& l)
{
    if (::munmap (l.data(), l.size()))
	set_and_throw (failbit, "munmap");
    l.unlink();
}

/// Synchronizes a memory-mapped area.
void fstream::msync (memlink& l)
{
    if (::msync (l.data(), l.size(), MS_ASYNC | MS_INVALIDATE))
	set_and_throw (failbit, "msync");
}

#endif

} // namespace ustl
