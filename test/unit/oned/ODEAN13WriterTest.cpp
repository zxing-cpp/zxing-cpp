/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2009 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "oned/ODEAN13Writer.h"
#include "BitMatrixIO.h"

#include "gtest/gtest.h"
#include <stdexcept>

using namespace ZXing;
using namespace ZXing::OneD;

namespace {
	std::string Encode(const std::string& input)
	{
		auto result = ToString(EAN13Writer().encode(input, 0, 0), '1', '0', false);
		return result.substr(0, result.size() - 1);	// remove the \n at the end
	}
}

TEST(ODEAN13WriterTest, Encode1)
{
	std::string toEncode = "5901234123457";
	std::string expected = "00001010001011010011101100110010011011110100111010101011001101101100100001010111001001110100010010100000";
	EXPECT_EQ(Encode(toEncode), expected);
}

TEST(ODEAN13WriterTest, AddChecksumAndEncode)
{
	std::string toEncode = "590123412345";
	std::string expected = "00001010001011010011101100110010011011110100111010101011001101101100100001010111001001110100010010100000";
	EXPECT_EQ(Encode(toEncode), expected);
}

TEST(ODEAN13WriterTest, EncodeIllegalCharacters)
{
	EXPECT_THROW({ Encode("5901234123abc"); }, std::invalid_argument);
}
