// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2016 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "stdtest.h"

#if HAVE_CPP11
void TestChrono (void)
{
    // How to time a block of code with system_clock_ms
    auto mspt1 = chrono::system_clock_ms::now();
    // ... timed code here
    cout << chrono::system_clock_ms::now() - mspt1 << endl;

    // Timing with steady_clock
    auto steadypt1 = chrono::steady_clock::now();
    // ... timed code here
    auto steadypt2 = chrono::steady_clock::now();
    cout << chrono::duration_cast<chrono::milliseconds> (steadypt2 - steadypt1) << endl;

    // Compile-time rational arithmetic type
    cout << "\n2/6 + 4/15 = " << ratio_add<ratio<2,6>,ratio<4,15>>::type() << endl;
    cout << "2/6 / 4/15 = " << ratio_divide<ratio<2,6>,ratio<4,15>>::type() << endl;
    cout << "2/6 > 4/15 = " << ratio_greater<ratio<2,6>,ratio<4,15>>::value << endl;

    // Example code for chrono::duration from cppreference.com
    using shakes = chrono::duration<int, ratio<1, 100000000>>;
    using jiffies = chrono::duration<int, centi>;
    using microfortnights = chrono::duration<float, ratio<12096,10000>>;
    using nanocenturies = chrono::duration<float, ratio<3155,1000>>;

    chrono::seconds sec(1);

    // integer scale conversion with no precision loss: no cast
    cout << "\n1 second is:\n"
	<< chrono::microseconds(sec).count() << " microseconds\n"
	<< shakes(sec).count() << " shakes\n"
	<< jiffies(sec).count() << " jiffies\n"
	// integer scale conversion with precision loss: requires a cast
	<< chrono::duration_cast<chrono::minutes>(sec).count() << " minutes\n";

    // floating-point scale conversion: no cast
    cout.format ("%.5f microfortnights\n%.6f nanocenturies\n\n",
	    microfortnights(sec).count(), nanocenturies(sec).count());

    // Example code for chrono::time_point
    chrono::time_point<chrono::high_resolution_clock, chrono::seconds> tps (chrono::seconds(4));
    // implicit cast, no precision loss
    chrono::time_point<chrono::high_resolution_clock, chrono::milliseconds> tpms (tps);
    cout << tpms << endl;
    tpms = chrono::time_point<chrono::high_resolution_clock, chrono::milliseconds> (chrono::milliseconds(5756));
    // explicit cast, 5756 truncated to 5000
    cout << chrono::time_point_cast<chrono::seconds>(tpms) << endl;

    // Excercise the clock code
    auto nowsc = chrono::system_clock::now();
    auto nowhr = chrono::high_resolution_clock::now();
    auto nowhrsc = chrono::time_point_cast<chrono::system_clock::duration>(nowhr);
    if (nowsc.time_since_epoch() <= nowhrsc.time_since_epoch())
	cout << "system_clock::now() <= high_resolution_clock::now()\n";
    auto nowschr = chrono::time_point_cast<chrono::high_resolution_clock::duration>(nowsc);
    if (nowhr.time_since_epoch() >= nowschr.time_since_epoch())
	cout << "high_resolution_clock::now() >= system_clock::now()\n";

    // Exercise duration arithmetic
    nowhr -= nowhr.time_since_epoch();
    nowhr += chrono::hours(2);
    nowhr += chrono::milliseconds(42);
    cout << chrono::time_point_cast<chrono::milliseconds> (nowhr) << endl;
    tps = chrono::time_point_cast<chrono::seconds> (nowhr);
    cout << tps << endl;
    cout << chrono::time_point_cast<chrono::milliseconds> (tps) << endl;
}
#else
void TestChrono (void)
{
    static const char* c_StdPaths[] = { "test/chron.std", "../../test/chron.std", "chron.std" };
    string so;
    for (unsigned i = 0; i < VectorSize(c_StdPaths); ++i)
	if (0 == access (c_StdPaths[i], R_OK))
	    so.read_file (c_StdPaths[i]);
    cout << so;
}
#endif

StdTestMain (TestChrono)
