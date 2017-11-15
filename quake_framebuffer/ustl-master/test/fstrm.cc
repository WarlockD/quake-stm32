// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "stdtest.h"

void TestFStream (void)
{
    fstream fs ("test/fstrm.std", ios::in | ios::nocreate);
    if (!fs && !(fs.open("fstrm.std", ios::in | ios::nocreate),fs))
	cout << "Failed to open fstrm.std" << endl;
    string buf;
    buf.resize (fs.size());
    if (buf.size() != 71)
	cout << "fstream.size() returned " << buf.size() << endl;
    fs.read (buf.begin(), buf.size());
    cout << buf;
    fs.close();
}

StdTestMain (TestFStream)
