/*
* Copyright 2022 gitlost
*/
// SPDX-License-Identifier: Apache-2.0

#include "oned/ODDataBarReader.h"

#include "ReaderOptions.h"
#include "Barcode.h"

#include "gtest/gtest.h"

using namespace ZXing;
using namespace ZXing::OneD;

// Helper to call decodePattern()
static Barcode parse(PatternRow row, ReaderOptions opts = {})
{
	DataBarReader reader(opts);

	row.insert(row.begin(), { 1, 1 }); // Left guard
	row.insert(row.end(), { 1, 1 }); // Right guard

	std::unique_ptr<RowReader::DecodingState> state;
	PatternView next(row);
	return reader.decodePattern(0, next, state);
}

TEST(ODDataBarReaderTest, Composite)
{
	{
		// With 2D linkage flag (GS1 Composite) in checksum
		PatternRow row = { 2, 3, 1, 2, 1, 2, 4, 1, 3, 3, 7, 1, 1, 3, 1, 2, 1, 1, 1, 4, 2, 4, 1, 1, 2, 3, 1, 1, 2, 1, 1, 2, 8, 3, 3, 2, 2, 1, 4, 1, 1, 2 };
		auto result = parse(row);
		EXPECT_TRUE(result.isValid());
		EXPECT_EQ(result.text(), "01234567890128");
	}
}
