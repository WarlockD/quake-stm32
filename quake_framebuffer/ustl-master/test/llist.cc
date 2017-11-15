// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "stdtest.h"
#include "../ulist.h"

void TestList (void)
{
    static const int c_TestNumbers[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 13, 14, 15, 16, 17, 18 };
    list<int> v;
    v.push_back (2);
    cout << v << endl;
    v.push_front (1);
    cout << v.front() << "," << v.back() << endl;
    list<int> lt (VectorRange (c_TestNumbers));
    cout << "lt: " << lt << endl;
    v.splice (v.end(), lt, lt.begin(), lt.end());
    cout << "v: " << v << ", lt: " << lt << endl;
    v.remove (9);
    cout << "remove(9): " << v << endl;
    v.reverse();
    cout << "reverse: " << v << endl;
    v.sort();
    cout << "sort: " << v << endl;
#if HAVE_CPP11
    lt.emplace_front (0);
#else
    lt.push_front (0);
#endif
    lt.insert (lt.end(), VectorRange (c_TestNumbers));
    v.merge (lt);
    cout << "merge: " << v << endl;
    v.unique();
    cout << "unique: " << v << endl;
}

StdTestMain (TestList)
