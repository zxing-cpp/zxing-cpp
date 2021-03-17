/*
* Copyright 2021 gitlost
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "TextUtfEncoding.h"

#include "gtest/gtest.h"
#include <clocale>
#include <vector>

TEST(ToUtf8AngleEscapeTest, TextUtfEncoding)
{
	using namespace ZXing::TextUtfEncoding;

	bool angleEscape = true;

	char* ctype_locale = std::setlocale(LC_CTYPE, NULL);
	EXPECT_STREQ(ctype_locale, "C");

	EXPECT_EQ(ToUtf8(std::wstring(L"¶Ж"), angleEscape), std::string("<U+B6><U+0416>"));
	EXPECT_EQ(ToUtf8(std::wstring(L"☂"), angleEscape), std::string("<U+2602>"));

	std::setlocale(LC_CTYPE, "en_US.UTF-8");

	EXPECT_EQ(ToUtf8(std::wstring(L"¶Ж"), angleEscape), std::string("¶Ж"));
	EXPECT_EQ(ToUtf8(std::wstring(L"☂"), angleEscape), std::string("☂"));
	EXPECT_EQ(ToUtf8(std::wstring(L"\x01\x1F\x7F"), angleEscape), std::string("<SOH><US><DEL>"));
	EXPECT_EQ(ToUtf8(std::wstring(L"\x80\x9F"), angleEscape), std::string("<U+80><U+9F>"));
	EXPECT_EQ(ToUtf8(std::wstring(L"\xA0"), angleEscape), std::string("<U+A0>")); // NO-BREAK space (nbsp)
	EXPECT_EQ(ToUtf8(std::wstring(L"\x2007"), angleEscape), std::string("<U+2007>")); // NO-BREAK space (numsp)
	EXPECT_EQ(ToUtf8(std::wstring(L"\xFFEF"), angleEscape), std::string("<U+FFEF>")); // Was NO-BREAK space but now isn't (BOM)
	EXPECT_EQ(ToUtf8(std::wstring(L"\u0100"), angleEscape), std::string("Ā"));
	EXPECT_EQ(ToUtf8(std::wstring(L"\u1000"), angleEscape), std::string("က"));
	EXPECT_EQ(ToUtf8(std::wstring(L"\u2000"), angleEscape), std::string("<U+2000>"));
	EXPECT_EQ(ToUtf8(std::wstring(L"\uFFFD"), angleEscape), std::string("�"));
	EXPECT_EQ(ToUtf8(std::wstring(L"\uFFFF"), angleEscape), std::string("<U+FFFF>"));
	EXPECT_EQ(ToUtf8(std::wstring(L"\U00010000"), angleEscape), std::string("𐀀"));

	std::setlocale(LC_CTYPE, ctype_locale);
}
