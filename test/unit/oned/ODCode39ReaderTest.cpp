/*
* Copyright 2022 gitlost
*/
// SPDX-License-Identifier: Apache-2.0

#include "oned/ODCode39Reader.h"

#include "ReaderOptions.h"
#include "Barcode.h"

#include "gtest/gtest.h"

using namespace ZXing;
using namespace ZXing::OneD;

// Helper to call decodePattern()
static Barcode parse(PatternRow row, ReaderOptions opts = {})
{
	Code39Reader reader(opts);

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
		auto result = parse(row);
		EXPECT_EQ(result.symbologyIdentifier(), "]A1");
		EXPECT_EQ(result.text(), "AA");
	}
	{
		// Extended "a"
		PatternRow row({ 1, 2, 1, 1, 1, 2, 1, 2, 1, 0, 2, 1, 1, 1, 1, 2, 1, 1, 2 });
		auto result = parse(row);
		EXPECT_EQ(result.symbologyIdentifier(), "]A4");
		EXPECT_EQ(result.text(), "a");
	}
	{
		// Extended "a" with checksum
		PatternRow row({ 1, 2, 1, 1, 1, 2, 1, 2, 1, 0, 2, 1, 1, 1, 1, 2, 1, 1, 2, 0, 2, 1, 1, 2, 1, 1, 2, 1, 1 });
		auto result = parse(row);
		EXPECT_EQ(result.symbologyIdentifier(), "]A5");
		EXPECT_EQ(result.text(), "a8");
	}
}
