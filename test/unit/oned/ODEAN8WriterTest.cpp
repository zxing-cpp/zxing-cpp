/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2009 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "oned/ODEAN8Writer.h"
#include "BitMatrixIO.h"

#include "gtest/gtest.h"

using namespace ZXing;
using namespace ZXing::OneD;

namespace {
	std::string Encode(const std::string& input)
	{
		auto result = ToString(EAN8Writer().encode(input, 0, 0), '1', '0', false);
		return result.substr(0, result.size() - 1);	// remove the \n at the end
	}
}

TEST(ODEAN8WriterTest, Encode1)
{
	std::string toEncode = "96385074";
	std::string expected = "0000101000101101011110111101011011101010100111011100101000100101110010100000";
	EXPECT_EQ(Encode(toEncode), expected);
}

TEST(ODEAN8WriterTest, AddChecksumAndEncode)
{
	std::string toEncode = "9638507";
	std::string expected = "0000101000101101011110111101011011101010100111011100101000100101110010100000";
	EXPECT_EQ(Encode(toEncode), expected);
}

TEST(ODEAN8WriterTest, EncodeIllegalCharacters)
{
	EXPECT_THROW({ Encode("96385abc"); }, std::invalid_argument);
}
