/*
* Copyright 2021 gitlost
*/
// SPDX-License-Identifier: Apache-2.0

#include "CharacterSetECI.h"
#include "ECI.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

using namespace ZXing;
using namespace ZXing::CharacterSetECI;
using namespace testing;

TEST(CharacterSetECITest, Charset2ECI)
{
	EXPECT_EQ(ToInt(ToECI(CharacterSet::ISO8859_1)), 3);
	EXPECT_EQ(ToInt(ToECI(CharacterSet::ISO8859_2)), 4);
	EXPECT_EQ(ToInt(ToECI(CharacterSet::ASCII)), 27);
	EXPECT_EQ(ToInt(ToECI(CharacterSet::EUC_KR)), 30);
	EXPECT_EQ(ToInt(ToECI(CharacterSet::BINARY)), 899);
	EXPECT_EQ(ToInt(ToECI(CharacterSet::Unknown)), -1);
}

TEST(CharacterSetECITest, InitEncoding)
{
	EXPECT_EQ(InitEncoding(""), CharacterSet::ISO8859_1);
	EXPECT_EQ(InitEncoding("", CharacterSet::ISO8859_2), CharacterSet::ISO8859_2);
	EXPECT_EQ(InitEncoding("asdfasdf"), CharacterSet::ISO8859_1);
	EXPECT_EQ(InitEncoding("asdfasdf", CharacterSet::ISO8859_2), CharacterSet::ISO8859_2);
	EXPECT_EQ(InitEncoding("ISO-8859-1"), CharacterSet::ISO8859_1);
	EXPECT_EQ(InitEncoding("ISO-8859-2"), CharacterSet::ISO8859_2);
	EXPECT_EQ(InitEncoding("UTF-16BE"), CharacterSet::UnicodeBig);
	EXPECT_EQ(InitEncoding("", CharacterSet::Unknown), CharacterSet::Unknown);
}

TEST(CharacterSetECITest, OnChangeAppendReset)
{
	{
		std::string data;
		std::wstring encoded;

		// Null
		auto result = OnChangeAppendReset(3, encoded, data, CharacterSet::Unknown);
		EXPECT_EQ(result, CharacterSet::ISO8859_1);
		EXPECT_TRUE(data.empty());
		EXPECT_TRUE(encoded.empty());
	}

	{
		static const uint8_t bytes[] = { 'A', 0xE9, 'Z' };
		std::string data(reinterpret_cast<const char*>(&bytes), sizeof(bytes));
		std::wstring encoded;

		// Encoding change
		auto result = OnChangeAppendReset(3, encoded, data, CharacterSet::Unknown);
		EXPECT_EQ(result, CharacterSet::ISO8859_1);
		EXPECT_TRUE(data.empty());
		EXPECT_EQ(encoded, L"A\u00E9Z");
	}

	{
		static const uint8_t bytes[] = { 'A', 0xE9, 'Z' };
		std::string data(reinterpret_cast<const char*>(&bytes), sizeof(bytes));
		std::wstring encoded;

		// Encoding same
		auto result = OnChangeAppendReset(3, encoded, data, CharacterSet::ISO8859_1);
		EXPECT_EQ(result, CharacterSet::ISO8859_1);
		EXPECT_EQ(data, "A\xE9Z");
		EXPECT_TRUE(encoded.empty());
	}

	{
		CharacterSet result;
		static const uint8_t bytes[] = { 'A', 0xE9, 'Z' };
		std::string data(reinterpret_cast<const char*>(&bytes), sizeof(bytes));
		std::wstring encoded(L"A\u00E9Z");

		// Encoding change
		result = OnChangeAppendReset(20, encoded, data, CharacterSet::ISO8859_5);
		EXPECT_EQ(result, CharacterSet::Shift_JIS);
		EXPECT_TRUE(data.empty());
		EXPECT_EQ(encoded, L"A\u00E9ZA\u0449Z");

		static const uint8_t bytes2[] = { 'A', 0x83, 0x65, 'Z' };
		std::string data2(reinterpret_cast<const char*>(&bytes2), sizeof(bytes2));

		// Encoding same
		result = OnChangeAppendReset(20, encoded, data2, result);
		EXPECT_EQ(result, CharacterSet::Shift_JIS);
		EXPECT_THAT(data2, ElementsAreArray(bytes2, sizeof(bytes2)));
		EXPECT_EQ(encoded, L"A\u00E9ZA\u0449Z");

		// Encoding change
		result = OnChangeAppendReset(4, encoded, data2, result);
		EXPECT_EQ(result, CharacterSet::ISO8859_2);
		EXPECT_TRUE(data2.empty());
		EXPECT_EQ(encoded, L"A\u00E9ZA\u0449ZA\u30C6Z");
	}
}
