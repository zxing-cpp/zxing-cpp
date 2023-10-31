/*
* Copyright 2022 gitlost
*/
// SPDX-License-Identifier: Apache-2.0

#include "DecoderResult.h"
#include "pdf417/PDFScanningDecoder.h"

#include "gtest/gtest.h"

using namespace ZXing;
using namespace ZXing::Pdf417;

// Shorthand for DecodeCodewords()
static DecoderResult decode(std::vector<int>& codewords)
{
	std::vector<int> erasures;
	auto result = DecodeCodewords(codewords, NumECCodeWords(0));

	return result;
}

TEST(PDF417ScanningDecoderTest, BadSymbolLengthDescriptor)
{
	{
		std::vector<int> codewords = { 4, 1, 449, 394 }; // 4 should be 2

		auto result = decode(codewords);

		EXPECT_TRUE(result.isValid());
		EXPECT_EQ(result.text(), L"AB");
		EXPECT_EQ(codewords[0], 2);
	}
	{
		std::vector<int> codewords = { 1, 1, 800, 351 }; // 1 should be 2

		auto result = decode(codewords);

		EXPECT_TRUE(result.isValid());
		EXPECT_EQ(result.text(), L"AB");
		EXPECT_EQ(codewords[0], 2);
	}
	{
		std::vector<int> codewords = { 0, 1, 917, 27 }; // 0 should be 2

		auto result = decode(codewords);

		EXPECT_TRUE(result.isValid());
		EXPECT_EQ(result.text(), L"AB");
		EXPECT_EQ(codewords[0], 2);
	}
}
