/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2010 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "oned/ODUPCAWriter.h"
#include "BitMatrixIO.h"

#include "gtest/gtest.h"

using namespace ZXing;
using namespace ZXing::OneD;

namespace {
	std::string Encode(const std::string& input)
	{
		auto result = ToString(UPCAWriter().encode(input, 0, 0), '1', '0', false);
		return result.substr(0, result.size() - 1);	// remove the \n at the end
	}
}

TEST(ODUPCAWriterTest, Encode1)
{
	std::string toEncode = "485963095124";
	std::string expected = "00001010100011011011101100010001011010111101111010101011100101110100100111011001101101100101110010100000";
	EXPECT_EQ(Encode(toEncode), expected);
}

TEST(ODUPCAWriterTest, AddChecksumAndEncode)
{
	std::string toEncode = "12345678901";
	std::string expected = "00001010011001001001101111010100011011000101011110101010001001001000111010011100101100110110110010100000";
	EXPECT_EQ(Encode(toEncode), expected);
}

