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
	EXPECT_EQ(ToUtf8(L"\u00B6\u0416", angleEscape), "<U+B6><U+0416>");
#else
	EXPECT_EQ(ToUtf8(L"\u00B6\u0416", angleEscape), "¶Ж");
#endif
	EXPECT_EQ(ToUtf8(L"\u2602", angleEscape), "<U+2602>");

#ifndef _WIN32
	std::setlocale(LC_CTYPE, "en_US.UTF-8");
	EXPECT_STREQ(std::setlocale(LC_CTYPE, NULL), "en_US.UTF-8");
#else
	std::setlocale(LC_CTYPE, ".utf8");
	EXPECT_TRUE(std::string(std::setlocale(LC_CTYPE, NULL)).find("utf8") != std::string::npos);
#endif

	EXPECT_EQ(ToUtf8(L"\u00B6\u0416", angleEscape), "¶Ж");
#ifndef _WIN32
	EXPECT_EQ(ToUtf8(L"\u2602", angleEscape), "☂");
#else
	EXPECT_EQ(ToUtf8(L"\u2602", angleEscape), "<U+2602>");
#endif
	EXPECT_EQ(ToUtf8(L"\x01\x1F\x7F", angleEscape), "<SOH><US><DEL>");
	EXPECT_EQ(ToUtf8(L"\x80\x9F", angleEscape), "<U+80><U+9F>");
	EXPECT_EQ(ToUtf8(L"\xA0", angleEscape), "<U+A0>"); // NO-BREAK space (nbsp)
	EXPECT_EQ(ToUtf8(L"\x2007", angleEscape), "<U+2007>"); // NO-BREAK space (numsp)
	EXPECT_EQ(ToUtf8(L"\xFFEF", angleEscape), "<U+FFEF>"); // Was NO-BREAK space but now isn't (BOM)
	EXPECT_EQ(ToUtf8(L"\u0100", angleEscape), "Ā");
	EXPECT_EQ(ToUtf8(L"\u1000", angleEscape), "က");
	EXPECT_EQ(ToUtf8(L"\u2000", angleEscape), "<U+2000>"); // Space char (nqsp)
#ifndef _WIN32
	EXPECT_EQ(ToUtf8(L"\uFFFD", angleEscape), "�");
#else
	EXPECT_EQ(ToUtf8(L"\uFFFD", angleEscape), "<U+FFFD>");
#endif
	EXPECT_EQ(ToUtf8(L"\uFFFF", angleEscape), "<U+FFFF>");
#ifndef __APPLE__
	EXPECT_EQ(ToUtf8(L"\U00010000", angleEscape), "𐀀");
#else
	EXPECT_EQ(ToUtf8(L"\U00010000", angleEscape), "<U+10000>");
#endif
	EXPECT_EQ(ToUtf8(L"\xD800Z", angleEscape), "<U+D800>Z"); // Unpaired high surrogate
	EXPECT_EQ(ToUtf8(L"A\xDC00", angleEscape), "A<U+DC00>"); // Unpaired low surrogate

	std::setlocale(LC_CTYPE, ctype_locale.c_str());
}
