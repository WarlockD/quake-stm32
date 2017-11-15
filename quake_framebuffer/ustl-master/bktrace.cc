// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2006 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "bktrace.h"
#include "sostream.h"
#include "mistream.h"
#if HAVE_EXECINFO_H
    #include <execinfo.h>
#else
    static inline int backtrace (void**, int)			{ return 0; }
    static inline char** backtrace_symbols (void* const*, int)	{ return nullptr; }
#endif

namespace ustl {

/// Default constructor. The backtrace is obtained here.
CBacktrace::CBacktrace (void) noexcept
:_symbols (nullptr)
,_nFrames (0)
,_symbolsSize (0)
{
    _nFrames = backtrace (VectorBlock (_addresses));
    GetSymbols();
}

/// Copy constructor.
CBacktrace::CBacktrace (const CBacktrace& v) noexcept
:_symbols (nullptr)
,_nFrames (0)
,_symbolsSize (0)
{
    operator= (v);
}

/// Copy operator.
const CBacktrace& CBacktrace::operator= (const CBacktrace& v) noexcept
{
    memcpy (_addresses, v._addresses, sizeof(_addresses));
    _symbols = strdup (v._symbols);
    _nFrames = v._nFrames;
    _symbolsSize = v._symbolsSize;
    return *this;
}

/// Converts a string returned by backtrace_symbols into readable form.
static size_t ExtractAbiName (const char* isym, char* nmbuf) noexcept
{
    // Prepare the demangled name, if possible
    size_t nmSize = 0;
    if (isym) {
	// Copy out the name; the strings are: "file(function+0x42) [0xAddress]"
	const char* mnStart = strchr (isym, '(');
	if (++mnStart == (const char*)(1))
	    mnStart = isym;
	const char* mnEnd = strchr (isym, '+');
	const char* isymEnd = isym + strlen (isym);
	if (!mnEnd)
	    mnEnd = isymEnd;
	nmSize = min (size_t (distance (mnStart, mnEnd)), 255U);
	memcpy (nmbuf, mnStart, nmSize);
    }
    nmbuf[nmSize] = 0;
    // Demangle
    demangle_type_name (nmbuf, 255, &nmSize);
    return nmSize;
}

/// Tries to get symbol information for the addresses.
void CBacktrace::GetSymbols (void) noexcept
{
    char** symbols = backtrace_symbols (_addresses, _nFrames);
    if (!symbols)
	return;
    char nmbuf [256];
    size_t symSize = 1;
    for (uoff_t i = 0; i < _nFrames; ++ i)
	symSize += ExtractAbiName (symbols[i], nmbuf) + 1;
    if ((_symbols = (char*) calloc (symSize, 1))) {
	for (uoff_t i = 0; _symbolsSize < symSize - 1; ++ i) {
	    size_t sz = ExtractAbiName (symbols[i], nmbuf);
	    memcpy (_symbols + _symbolsSize, nmbuf, sz);
	    _symbolsSize += sz + 1;
	    _symbols [_symbolsSize - 1] = '\n';
	}
    }
    free (symbols);
}

#if SIZE_OF_LONG == 8
    #define ADDRESS_FMT	"%16p  "
#else
    #define ADDRESS_FMT	"%8p  "
#endif

/// Prints the backtrace to \p os.
void CBacktrace::text_write (ostringstream& os) const
{
    const char *ss = _symbols, *se;
    for (uoff_t i = 0; i < _nFrames; ++ i) {
	os.format (ADDRESS_FMT, _addresses[i]);
	se = strchr (ss, '\n') + 1;
	os.write (ss, distance (ss, se));
	ss = se;
    }
}

/// Reads the object from stream \p is.
void CBacktrace::read (istream& is)
{
    assert (is.aligned (stream_align_of (_addresses[0])) && "Backtrace object contains pointers and must be void* aligned");
    is >> _nFrames >> _symbolsSize;
    nfree (_symbols);
    _symbols = (char*) malloc (_symbolsSize + 1);
    is.read (_symbols, _symbolsSize);
    _symbols [_symbolsSize] = 0;
    is.align();
    is.read (_addresses, _nFrames * sizeof(void*));
}

/// Writes the object to stream \p os.
void CBacktrace::write (ostream& os) const
{
    assert (os.aligned (stream_align_of (_addresses[0])) && "Backtrace object contains pointers and must be void* aligned");
    os << _nFrames << _symbolsSize;
    os.write (_symbols, _symbolsSize);
    os.align();
    os.write (_addresses, _nFrames * sizeof(void*));
}

/// Returns the size of the written object.
size_t CBacktrace::stream_size (void) const
{
    return Align (stream_size_of (_nFrames) +
		   stream_size_of (_symbolsSize) +
		   _nFrames * sizeof(void*) +
		   _symbolsSize);
}

} // namespace ustl
