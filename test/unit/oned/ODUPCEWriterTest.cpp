/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "oned/ODUPCEWriter.h"
#include "BitMatrixIO.h"

#include "gtest/gtest.h"
#include <stdexcept>

using namespace ZXing;
using namespace ZXing::OneD;

namespace {
	std::string Encode(const std::string& input)
	{
		auto result = ToString(UPCEWriter().encode(input, 0, 0), '1', '0', false);
		return result.substr(0, result.size() - 1);	// remove the \n at the end
	}
}

TEST(ODUPCEWriterTest, Encode1)
{
	std::string toEncode = "05096893";
	std::string expected = "000010101110010100111000101101011110110111001011101010100000";
	EXPECT_EQ(Encode(toEncode), expected);
}

TEST(ODUPCEWriterTest, EncodeSystem1)
{
	std::string toEncode = "12345670";
	std::string expected = "000010100100110111101010001101110010000101001000101010100000";
	EXPECT_EQ(Encode(toEncode), expected);
}

TEST(ODUPCEWriterTest, AddChecksumAndEncode)
{
	std::string toEncode = "0509689";
	std::string expected = "000010101110010100111000101101011110110111001011101010100000";
	EXPECT_EQ(Encode(toEncode), expected);
}

TEST(ODUPCEWriterTest, EncodeIllegalCharacters)
{
	EXPECT_THROW(Encode("05096abc"), std::invalid_argument);
}
