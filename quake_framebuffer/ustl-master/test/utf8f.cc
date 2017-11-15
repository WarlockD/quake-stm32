// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "stdtest.h"

static void Widen (const string& str, vector<wchar_t>& result)
{
    result.clear();
    result.resize (str.length());
    copy (str.utf8_begin(), str.utf8_end(), result.begin());
}

static void DumpWchars (const vector<wchar_t>& v)
{
    foreach (vector<wchar_t>::const_iterator, i, v)
	cout.format (" %u", uint32_t(*i));
}

void TestUTF8 (void)
{
    cout << "Generating Unicode characters ";
    vector<wchar_t> srcChars;
    srcChars.resize (0xFFFF);
    iota (srcChars.begin(), srcChars.end(), 0);
    cout.format ("%zu - %zu\n", size_t(srcChars[0]), size_t(srcChars.back()));

    cout << "Encoding to utf8.\n";
    string encoded;
    encoded.reserve (srcChars.size() * 4);
    copy (srcChars, utf8out (back_inserter(encoded)));

    for (unsigned i = 0, ci = 0; i < encoded.size();) {
	unsigned seqb = Utf8SequenceBytes(encoded[i]), cntb = Utf8Bytes(ci);
	if (seqb != cntb)
	    cout.format ("Char %x encoded in %u bytes instead of %u\n", ci, seqb, cntb);
	unsigned char h1 = 0xff << (8-seqb), m1 = 0xff << (7-seqb);
	if (ci <= 0x7f) {
	    h1 = 0;
	    m1 = 0x80;
	}
	if ((encoded[i] & m1) != h1)
	    cout.format ("Char %x has incorrect encoded header %02x instead of %02hhx\n", ci, encoded[i] & m1, h1);
	++i;
	for (unsigned j = 1; j < seqb; ++j, ++i)
	    if ((encoded[i] & 0xC0) != 0x80)
		cout.format ("Char %x has incorrect intrabyte %u: %02hhx\n", ci, j, encoded[i]);
	++ci;
    }

    cout << "Decoding back.\n";
    vector<wchar_t> decChars;
    Widen (encoded, decChars);

    cout.format ("Comparing.\nsrc = %zu chars, encoded = %zu chars, decoded = %zu\n", srcChars.size(), encoded.size(), decChars.size());
    size_t nDiffs = 0;
    for (uoff_t i = 0; i < min (srcChars.size(), decChars.size()); ++ i) {
	if (srcChars[i] != decChars[i]) {
	    cout.format ("%u != %u\n", uint32_t(srcChars[i]), uint32_t(decChars[i]));
	    ++ nDiffs;
	}
    }
    cout.format ("%zu differences between src and decoded.\n", nDiffs);

    cout << "Testing wide character string::insert\n";
    string ws ("1234567890", 10);

    ws.insert (ws.find('1'), 1, wchar_t(1234));
    static const wchar_t c_WChars[2] = { 3456, 4567 };
    ws.insert (ws.find('3'), VectorRange(c_WChars), 2);
    ws.insert (ws.find('3'), 1, wchar_t(2345));
    ws.insert (ws.size(), 1, wchar_t(5678));
    cout.format ("Values[%zu]:", ws.length());
    for (string::utf8_iterator j = ws.utf8_begin(); j < ws.utf8_end(); ++ j)
	cout.format (" %u", uint32_t(*j));
    cout << endl;

    cout << "Character offsets:";
    for (string::utf8_iterator k = ws.utf8_begin(); k < ws.utf8_end(); ++ k)
	cout.format (" %zu", distance (ws.begin(), k.base()));
    cout << endl;

    cout.format ("Erasing character %zu: ", ws.length() - 1);
    ws.erase (ws.wiat(ws.length() - 1), ws.end());
    Widen (ws, decChars);
    DumpWchars (decChars);
    cout << endl;

    cout << "Erasing 2 characters after '2': ";
    ws.erase (ws.find('2')+1, Utf8Bytes(VectorRange(c_WChars)));
    Widen (ws, decChars);
    DumpWchars (decChars);
    cout << endl;
}

StdTestMain (TestUTF8)
