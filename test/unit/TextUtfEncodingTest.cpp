/*
* Copyright 2021 gitlost
*/
// SPDX-License-Identifier: Apache-2.0

#include "TextUtfEncoding.h"

#include "gtest/gtest.h"
#include <clocale>
#include <vector>

TEST(TextUtfEncodingTest, ToUtf8AngleEscape)
{
	using namespace ZXing::TextUtfEncoding;

	bool angleEscape = true;

	std::string ctype_locale(std::setlocale(LC_CTYPE, NULL)); // Use std::string to avoid assert on Windows Debug
	EXPECT_EQ(ctype_locale, std::string("C"));

#ifndef _WIN32
	EXPECT_EQ(ToUtf8(std::wstring(L"\u00B6\u0416"), angleEscape), std::string("<U+B6><U+0416>"));
#else
	EXPECT_EQ(ToUtf8(std::wstring(L"\u00B6\u0416"), angleEscape), std::string("¶Ж"));
#endif
	EXPECT_EQ(ToUtf8(std::wstring(L"\u2602"), angleEscape), std::string("<U+2602>"));

#ifndef _WIN32
	std::setlocale(LC_CTYPE, "en_US.UTF-8");
	EXPECT_STREQ(std::setlocale(LC_CTYPE, NULL), "en_US.UTF-8");
#else
	std::setlocale(LC_CTYPE, ".utf8");
	EXPECT_TRUE(std::string(std::setlocale(LC_CTYPE, NULL)).find("utf8") != std::string::npos);
#endif

	EXPECT_EQ(ToUtf8(std::wstring(L"\u00B6\u0416"), angleEscape), std::string("¶Ж"));
#ifndef _WIN32
	EXPECT_EQ(ToUtf8(std::wstring(L"\u2602"), angleEscape), std::string("☂"));
#else
	EXPECT_EQ(ToUtf8(std::wstring(L"\u2602"), angleEscape), std::string("<U+2602>"));
#endif
	EXPECT_EQ(ToUtf8(std::wstring(L"\x01\x1F\x7F"), angleEscape), std::string("<SOH><US><DEL>"));
	EXPECT_EQ(ToUtf8(std::wstring(L"\x80\x9F"), angleEscape), std::string("<U+80><U+9F>"));
	EXPECT_EQ(ToUtf8(std::wstring(L"\xA0"), angleEscape), std::string("<U+A0>")); // NO-BREAK space (nbsp)
	EXPECT_EQ(ToUtf8(std::wstring(L"\x2007"), angleEscape), std::string("<U+2007>")); // NO-BREAK space (numsp)
	EXPECT_EQ(ToUtf8(std::wstring(L"\xFFEF"), angleEscape), std::string("<U+FFEF>")); // Was NO-BREAK space but now isn't (BOM)
	EXPECT_EQ(ToUtf8(std::wstring(L"\u0100"), angleEscape), std::string("Ā"));
	EXPECT_EQ(ToUtf8(std::wstring(L"\u1000"), angleEscape), std::string("က"));
	EXPECT_EQ(ToUtf8(std::wstring(L"\u2000"), angleEscape), std::string("<U+2000>")); // Space char (nqsp)
#ifndef _WIN32
	EXPECT_EQ(ToUtf8(std::wstring(L"\uFFFD"), angleEscape), std::string("�"));
#else
	EXPECT_EQ(ToUtf8(std::wstring(L"\uFFFD"), angleEscape), std::string("<U+FFFD>"));
#endif
	EXPECT_EQ(ToUtf8(std::wstring(L"\uFFFF"), angleEscape), std::string("<U+FFFF>"));
#ifndef __APPLE__
	EXPECT_EQ(ToUtf8(std::wstring(L"\U00010000"), angleEscape), std::string("𐀀"));
#else
	EXPECT_EQ(ToUtf8(std::wstring(L"\U00010000"), angleEscape), std::string("<U+10000>"));
#endif
	EXPECT_EQ(ToUtf8(std::wstring(L"\xD800Z"), angleEscape), std::string("<U+D800>Z")); // Unpaired high surrogate
	EXPECT_EQ(ToUtf8(std::wstring(L"A\xDC00"), angleEscape), std::string("A<U+DC00>")); // Unpaired low surrogate

	std::setlocale(LC_CTYPE, ctype_locale.c_str());
}
