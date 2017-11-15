// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2016 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "stdtest.h"

struct LoggedObj {
    LoggedObj (int v = 0)	:_v(v) { cout.format ("LoggedObj ctor %d\n", v); }
    ~LoggedObj (void)		{ cout.format ("LoggedObj dtor %d\n", _v); }
private:
    int _v;
};

#if !HAVE_CPP14
// Workarounds to print valid info when c++14 is not supported
typedef void (*pfn_scope_exit_t)(void);
static inline void PrintScopeExit (void) { cout.format ("~scope_exit\n"); }
static inline void PrintResourceDestruct (void) { cout.format ("Closing resource 4\n"); }
struct OldScopeExit {
    OldScopeExit (pfn_scope_exit_t f) : _f(f) {}
    ~OldScopeExit (void) { _f(); }
    pfn_scope_exit_t _f;
};
#endif

void TestSmartPtrs (void)
{
#if HAVE_CPP14
    auto plo = make_unique<LoggedObj> (42);
    auto ploa = make_unique<LoggedObj[]> (3);
    auto plos = make_shared<LoggedObj> (72);
    auto exprint = make_scope_exit ([]{ cout.format ("~scope_exit\n"); });
    auto uniqres = make_unique_resource (4, [](int v) { cout.format ("Closing resource %d\n", v); });
#elif HAVE_CPP11
    auto plo = unique_ptr<LoggedObj> (new LoggedObj (42));
    auto ploa = unique_ptr<LoggedObj[]> (new LoggedObj [3]);
    auto plos = shared_ptr<LoggedObj> (new LoggedObj (72));
    auto exprint = scope_exit<pfn_scope_exit_t> (PrintScopeExit);
    auto uniqres = scope_exit<pfn_scope_exit_t> (PrintResourceDestruct);
#else
    auto_ptr<LoggedObj> plo (new LoggedObj (42));
    auto_ptr<LoggedObj> ploa [3];
    for (unsigned i = 0; i < VectorSize(ploa); ++i)
	ploa[i] = new LoggedObj;
    auto_ptr<LoggedObj> plos (new LoggedObj (72));
    auto_ptr<OldScopeExit> exprint (new OldScopeExit (PrintScopeExit));
    auto_ptr<OldScopeExit> uniqres (new OldScopeExit (PrintResourceDestruct));
#endif
}

StdTestMain (TestSmartPtrs)
