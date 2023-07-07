/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "oned/ODCode93Writer.h"
#include "BitMatrixIO.h"

#include "gtest/gtest.h"

namespace ZXing { namespace OneD {
	std::string Code93ConvertToExtended(const std::wstring& contents);
}}

using namespace ZXing;
using namespace ZXing::OneD;

namespace {
	std::string Encode(const std::wstring& input)
	{
		auto result = ToString(Code93Writer().encode(input, 0, 0), '1', '0', false);
		return result.substr(0, result.size() - 1);	// remove the \n at the end
	}
}

TEST(ODCode93WriterTest, Encode)
{
	EXPECT_EQ(Encode(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"),
				  "000001010111101101010001101001001101000101100101001100100101100010101011010001011001"
				  "001011000101001101001000110101010110001010011001010001101001011001000101101101101001"
				  "101100101101011001101001101100101101100110101011011001011001101001101101001110101000"
				  "101001010010001010001001010000101001010001001001001001000101010100001000100101000010"
				  "10100111010101000010101011110100000");
}

TEST(ODCode93WriterTest, EncodeExtended)
{
	auto encoded = Encode(std::wstring(L"\x00\x01\x1a\x1b\x1f $%+!,09:;@AZ[_`az{\x7f", 25));
	auto expected =
		"00000" "101011110"
		"111011010" "110010110" "100100110" "110101000"	// bU aA
		"100100110" "100111010" "111011010" "110101000"	// aZ bA
		"111011010" "110010010" "111010010" "111001010"	// bE space $
		"110101110" "101110110" "111010110" "110101000"	// % + cA
		"111010110" "101011000" "100010100" "100001010"	// cL 0 9
		"111010110" "100111010" "111011010" "110001010"	// cZ bF
		"111011010" "110011010" "110101000" "100111010"	// bV A Z
		"111011010" "100011010" "111011010" "100101100"	// bK bO
		"111011010" "101101100" "100110010" "110101000"	// bW dA
		"100110010" "100111010" "111011010" "100010110"	// dZ bP
		"111011010" "110100110"	// bT
		"110100010" "110101100"	// checksum: 12 28
		"101011110" "100000";

	EXPECT_EQ(encoded, expected);
}

TEST(ODCode93WriterTest, ConvertToExtended)
{
	// non-extended chars are not changed.
	std::string src = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%";
	std::string dst = Code93ConvertToExtended(std::wstring(src.begin(), src.end()));
	EXPECT_EQ(src, dst);
}
