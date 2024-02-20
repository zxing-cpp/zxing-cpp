/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "oned/ODCode93Reader.h"
#include "BitArray.h"
#include "BitArrayUtility.h"
#include "ReaderOptions.h"
#include "Barcode.h"

#include "gtest/gtest.h"

using namespace ZXing;
using namespace ZXing::OneD;

static std::string Decode(std::string_view input)
{
	ReaderOptions opts;
	auto row    = Utility::ParseBitArray(input, '1');
	auto result = DecodeSingleRow(Code93Reader(opts), row.range());
	return result.text(TextMode::Plain);
}

TEST(ODCode93ReaderTest, Decode)
{
	auto expected = std::string("Code93!\n$%/+ :\x1b;[{\x7f\x00@`\x7f\x7f\x7f", 25);
	auto decoded = Decode(
		"00000010101111011010001010011001010010110010011001011001010010011001011001001010"
		"00010101010000101110101101101010001001001101001101001110010101101011101011011101"
		"01110110111010010111010110100111010111011010110101000111011010110001010111011010"
		"10001101011101101010001011011101101011010011011101101011001011011101101011001101"
		"01110110101011011001110110101011001101110110101001101101110110101001110101001100"
		"10110101000101011110100000");
	EXPECT_EQ(expected, decoded);
}
