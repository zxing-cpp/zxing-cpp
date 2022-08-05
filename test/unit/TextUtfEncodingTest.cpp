/*
* Copyright 2021 gitlost
*/
// SPDX-License-Identifier: Apache-2.0

#include "TextUtfEncoding.h"

#include "gtest/gtest.h"
#include <clocale>
#include <vector>

TEST(TextUtfEncodingTest, EscapeNonGraphical)
{
	using namespace ZXing::TextUtfEncoding;
	auto escape = [](const wchar_t* str) { return ToUtf8(EscapeNonGraphical(str)); };

	EXPECT_EQ(escape(L"\u00B6\u0416"), "¶Ж");
	EXPECT_EQ(escape(L"\x01\x1F\x7F"), "<SOH><US><DEL>");
	EXPECT_EQ(escape(L"\x80\x9F"), "<U+80><U+9F>");
	EXPECT_EQ(escape(L"\xA0"), "<U+A0>"); // NO-BREAK space (nbsp)
	EXPECT_EQ(escape(L"\x2007"), "<U+2007>"); // NO-BREAK space (numsp)
	EXPECT_EQ(escape(L"\xFFEF"), "<U+FFEF>"); // Was NO-BREAK space but now isn't (BOM)
	EXPECT_EQ(escape(L"\u2000"), "<U+2000>"); // Space char (nqsp)
	EXPECT_EQ(escape(L"\uFFFD"), "<U+FFFD>");
	EXPECT_EQ(escape(L"\uFFFF"), "<U+FFFF>");
	EXPECT_EQ(escape(L"\xD800Z"), "<U+D800>Z"); // Unpaired high surrogate
	EXPECT_EQ(escape(L"A\xDC00"), "A<U+DC00>"); // Unpaired low surrogate
}

TEST(TextUtfEncodingTest, FromUtf8)
{
	using namespace ZXing::TextUtfEncoding;

	EXPECT_EQ(FromUtf8(u8"\U00010000"), L"\U00010000");
	EXPECT_EQ(FromUtf8(u8"\U00010FFF"), L"\U00010FFF");
	EXPECT_EQ(FromUtf8("A\xE8\x80\xBFG"), L"A\u803FG"); // U+803F

//	EXPECT_EQ(FromUtf8("A\xE8\x80\xBF\x80G"), L"A\u803FG"); // Bad UTF-8 (extra continuation byte)
//	EXPECT_EQ(FromUtf8("A\xE8\x80\xC0G"), L"AG");           // Bad UTF-8 (non-continuation byte)
//	EXPECT_EQ(FromUtf8("A\xE8\x80G"), L"AG");               // Bad UTF-8 (missing continuation byte)
//	EXPECT_EQ(FromUtf8("A\xE8G"), L"AG");                   // Bad UTF-8 (missing continuation bytes)
//	EXPECT_EQ(FromUtf8("A\xED\xA0\x80G"), L"AG");           // Bad UTF-8 (unpaired high surrogate U+D800)
}
