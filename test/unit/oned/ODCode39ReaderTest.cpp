/*
* Copyright 2022 gitlost
*/
// SPDX-License-Identifier: Apache-2.0

#include "oned/ODCode39Reader.h"

#include "DecodeHints.h"
#include "Result.h"

#include "gtest/gtest.h"

using namespace ZXing;
using namespace ZXing::OneD;

// Helper to call decodePattern()
static Result parse(PatternRow row, DecodeHints hints = {})
{
	Code39Reader reader(hints);

	row.insert(row.begin(), { 0, 1, 2, 1, 1, 2, 1, 2, 1, 1, 0 });
	row.insert(row.end(), { 0, 1, 2, 1, 1, 2, 1, 2, 1, 1, 0 });

	std::unique_ptr<RowReader::DecodingState> state;
	PatternView next(row);
	return reader.decodePattern(0, next, state);
}

TEST(ODCode39ReaderTest, SymbologyIdentifier)
{
	{
		// Plain "A"
		PatternRow row({ 2, 1, 1, 1, 1, 2, 1, 1, 2 });
		auto result = parse(row);
		EXPECT_EQ(result.symbologyIdentifier(), "]A0");
		EXPECT_EQ(result.text(), "A");
	}
	{
		// "A" with checksum
		PatternRow row({ 2, 1, 1, 1, 1, 2, 1, 1, 2, 0, 2, 1, 1, 1, 1, 2, 1, 1, 2 });
		auto result = parse(row, DecodeHints().setValidateCode39CheckSum(true));
		EXPECT_EQ(result.symbologyIdentifier(), "]A3");
		EXPECT_EQ(result.text(), "A");

		result = parse(row);
		EXPECT_EQ(result.symbologyIdentifier(), "]A0");
		EXPECT_EQ(result.text(), "AA");
	}
	{
		// Extended "a"
		PatternRow row({ 1, 2, 1, 1, 1, 2, 1, 2, 1, 0, 2, 1, 1, 1, 1, 2, 1, 1, 2 });
		auto result = parse(row, DecodeHints().setTryCode39ExtendedMode(true));
		EXPECT_EQ(result.symbologyIdentifier(), "]A4");
		EXPECT_EQ(result.text(), "a");

		result = parse(row);
		EXPECT_EQ(result.symbologyIdentifier(), "]A0");
		EXPECT_EQ(result.text(), "+A");
	}
	{
		// Extended "a" with checksum
		PatternRow row({ 1, 2, 1, 1, 1, 2, 1, 2, 1, 0, 2, 1, 1, 1, 1, 2, 1, 1, 2, 0, 2, 1, 1, 2, 1, 1, 2, 1, 1 });
		auto result = parse(row, DecodeHints().setTryCode39ExtendedMode(true).setValidateCode39CheckSum(true));
		EXPECT_EQ(result.symbologyIdentifier(), "]A7");
		EXPECT_EQ(result.text(), "a");

		result = parse(row);
		EXPECT_EQ(result.symbologyIdentifier(), "]A0");
		EXPECT_EQ(result.text(), "+A8");
	}
}
