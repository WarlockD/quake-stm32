#include "encoding.h"

#include <climits>
#include <codecvt>
#include <locale>
#include <iostream>

#if (_MSC_VER >= 1900 /* VS 2015*/) && (_MSC_VER <= 1911 /* VS 2017 */)
//std::locale::id std::codecvt<char16_t, char, _Mbstatet>::id;
#endif

#if _MSC_VER >= 1900

std::string utf16_to_utf8(std::u16string utf16_string)
{
	std::wstring_convert<std::codecvt_utf8_utf16<int16_t>, int16_t> convert;
	auto p = reinterpret_cast<const int16_t *>(utf16_string.data());
	return convert.to_bytes(p, p + utf16_string.size());
}

#else

std::string utf16_to_utf8(std::u16string utf16_string)
{
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
	return convert.to_bytes(utf16_string);
}

#endif

static void Append16LE(std::string& str, __int16 c) {
  str.push_back(c & UCHAR_MAX);
  str.push_back((c >> 8) & UCHAR_MAX);
}


static void Append32LE(std::string& str, __int32  c) {
  Append16LE(str, c & USHRT_MAX);  
  Append16LE(str, (c >> 16) & USHRT_MAX);
}


void ConvertToUTF16(std::string& str) {
  std::wstring_convert<std::codecvt_utf8<__int16>, __int16> utf8_ucs2_cvt;


  auto str16 = utf8_ucs2_cvt.from_bytes(str);
  str.resize(0);
  for (auto c16: str16)
    Append16LE(str, c16);
}


void ConvertToUTF32(std::string& str) {
  std::wstring_convert<std::codecvt_utf8<__int32 >, __int32 > utf8_ucs4_cvt;
  auto str32 = utf8_ucs4_cvt.from_bytes(str);
  str.resize(0);
  for (auto c32: str32)
    Append32LE(str, c32);
}


void AppendUCN(std::string& str, int c) {
  std::wstring_convert<std::codecvt_utf8<__int32 >, __int32 > utf8_ucs4_cvt;
  str += utf8_ucs4_cvt.to_bytes(static_cast<__int32 >(c));
}
