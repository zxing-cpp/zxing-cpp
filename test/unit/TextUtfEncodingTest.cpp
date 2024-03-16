/*
* Copyright 2021 gitlost
*/
// SPDX-License-Identifier: Apache-2.0

#include "Utf.h"

#include "gtest/gtest.h"

using namespace ZXing;

#if __cplusplus > 201703L
std::string EscapeNonGraphical(const char8_t* utf8)
{
	return EscapeNonGraphical(reinterpret_cast<const char*>(utf8));
}
#endif

TEST(TextUtfEncodingTest, EscapeNonGraphical)
{
	EXPECT_EQ(EscapeNonGraphical(u8"\u00B6\u0416"), "¶Ж");
	EXPECT_EQ(EscapeNonGraphical(u8"\x01\x1F\x7F"), "<SOH><US><DEL>");
	EXPECT_EQ(EscapeNonGraphical(u8"\u0080\u009F"), "<U+80><U+9F>");
	EXPECT_EQ(EscapeNonGraphical(u8"\u00A0"), "<U+A0>"); // NO-BREAK space (nbsp)
	EXPECT_EQ(EscapeNonGraphical(u8"\u2007"), "<U+2007>"); // NO-BREAK space (numsp)
	EXPECT_EQ(EscapeNonGraphical(u8"\u2000"), "<U+2000>"); // Space char (nqsp)
	EXPECT_EQ(EscapeNonGraphical(u8"\uFFFD"), "<U+FFFD>");
	EXPECT_EQ(EscapeNonGraphical(u8"\uFFFF"), "<U+FFFF>");
}

TEST(TextUtfEncodingTest, FromUtf8)
{
	EXPECT_EQ(FromUtf8(u8"\U00010000"), L"\U00010000");
	EXPECT_EQ(FromUtf8(u8"\U00010FFF"), L"\U00010FFF");
	EXPECT_EQ(FromUtf8("A\xE8\x80\xBFG"), L"A\u803FG"); // U+803F

//	EXPECT_EQ(FromUtf8("A\xE8\x80\xBF\x80G"), L"A\u803FG"); // Bad UTF-8 (extra continuation byte)
//	EXPECT_EQ(FromUtf8("A\xE8\x80\xC0G"), L"AG");           // Bad UTF-8 (non-continuation byte)
//	EXPECT_EQ(FromUtf8("A\xE8\x80G"), L"AG");               // Bad UTF-8 (missing continuation byte)
//	EXPECT_EQ(FromUtf8("A\xE8G"), L"AG");                   // Bad UTF-8 (missing continuation bytes)
//	EXPECT_EQ(FromUtf8("A\xED\xA0\x80G"), L"AG");           // Bad UTF-8 (unpaired high surrogate U+D800)
}
