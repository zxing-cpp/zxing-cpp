/*
* Copyright 2021 gitlost
*/
// SPDX-License-Identifier: Apache-2.0

#include "ByteArray.h"
#include "DecoderResult.h"

#include "gtest/gtest.h"
#include <utility>

namespace ZXing::MaxiCode::DecodedBitStreamParser {

DecoderResult Decode(ByteArray&& bytes, const int mode);

}

using namespace ZXing;

// Pad out to max data length 93 (mode 4)
static void pad(ByteArray& padded)
{
	while (padded.size() < 93 + 1) { // 93 + mode
		padded.push_back(33);
	}
}

// Helper to call Decode()
static DecoderResult parse(ByteArray bytes, const int mode)
{
	ByteArray padded;
	padded.reserve(93 + 1); // 93 + mode
	if (mode == 2) {
		// Mode 2, Postcode 152382802, Country 840, Class 001 example from ISO/IEC 16023:2000 Annex B.2
		padded = {34, 20, 45, 20, 17, 18, 2, 18, 7, 0};
	} else if (mode == 3) {
		// Mode 3, Postcode B1050, Country 056, Class 999 example from ISO/IEC 16023:2000 Annex B.1
		padded = {3, 8, 28, 13, 28, 44, 0, 14, 28, 62};
	} else {
		padded.push_back(mode);
	}
	padded.insert(padded.end(), bytes.begin(), bytes.end());
	pad(padded);
	return MaxiCode::DecodedBitStreamParser::Decode(std::move(padded), mode);
}

// Helper to return Structured Append
static StructuredAppendInfo info(ByteArray bytes, const int mode)
{
	return parse(bytes, mode).structuredAppend();
}

TEST(MCDecodeTest, StructuredAppendSymbologyIdentifier)
{
	// Null
	EXPECT_EQ(info({49}, 2).index, -1); // Mode 2
	EXPECT_EQ(info({49}, 2).count, -1);
	EXPECT_TRUE(info({49}, 2).id.empty());
	EXPECT_EQ(parse({49}, 2).symbologyIdentifier(), "]U1");

	EXPECT_EQ(info({49}, 3).index, -1); // Mode 3
	EXPECT_EQ(info({49}, 3).count, -1);
	EXPECT_TRUE(info({49}, 3).id.empty());
	EXPECT_EQ(parse({49}, 3).symbologyIdentifier(), "]U1");

	EXPECT_EQ(info({49}, 4).index, -1); // Mode 4
	EXPECT_EQ(info({49}, 4).count, -1);
	EXPECT_TRUE(info({49}, 4).id.empty());
	EXPECT_EQ(parse({49}, 4).symbologyIdentifier(), "]U0");

	EXPECT_EQ(info({49}, 5).index, -1); // Mode 5
	EXPECT_EQ(info({49}, 5).count, -1);
	EXPECT_TRUE(info({49}, 5).id.empty());
	EXPECT_EQ(parse({49}, 5).symbologyIdentifier(), "]U0");

	EXPECT_EQ(info({49}, 6).index, -1); // Mode 6
	EXPECT_EQ(info({49}, 6).count, -1);
	EXPECT_TRUE(info({49}, 6).id.empty());
//	EXPECT_TRUE(parse({49}, 6).symbologyIdentifier().empty()); // Not defined for reader initialisation/programming

	// ISO/IEC 16023:2000 4.9.1 example
	EXPECT_EQ(info({33, 22, 49}, 2).index, 2); // Mode 2 - 3rd position 1-based == index 2
	EXPECT_EQ(info({33, 22, 49}, 2).count, 7);
	EXPECT_TRUE(info({33, 22, 49}, 2).id.empty());

	EXPECT_EQ(info({33, 22, 49}, 3).index, 2); // Mode 3
	EXPECT_EQ(info({33, 22, 49}, 3).count, 7);
	EXPECT_TRUE(info({33, 22, 49}, 3).id.empty());

	EXPECT_EQ(info({33, 22, 49}, 4).index, 2); // Mode 4
	EXPECT_EQ(info({33, 22, 49}, 4).count, 7);
	EXPECT_TRUE(info({33, 22, 49}, 4).id.empty());

	EXPECT_EQ(info({33, 22, 49}, 5).index, 2); // Mode 5
	EXPECT_EQ(info({33, 22, 49}, 5).count, 7);
	EXPECT_TRUE(info({33, 22, 49}, 5).id.empty());

	EXPECT_EQ(info({33, 22, 49}, 6).index, 2); // Mode 6
	EXPECT_EQ(info({33, 22, 49}, 6).count, 7);
	EXPECT_TRUE(info({33, 22, 49}, 6).id.empty());

	// Various
	EXPECT_EQ(info({33, 007, 49}, 2).index, 0); // Mode 2
	EXPECT_EQ(info({33, 007, 49}, 2).count, 8);
	EXPECT_EQ(info({33, 007, 49}, 4).index, 0); // Mode 4
	EXPECT_EQ(info({33, 007, 49}, 4).count, 8);

	EXPECT_EQ(info({33, 067, 49}, 2).index, 6); // Mode 2
	EXPECT_EQ(info({33, 067, 49}, 2).count, 8);
	EXPECT_EQ(info({33, 067, 49}, 4).index, 6); // Mode 4
	EXPECT_EQ(info({33, 067, 49}, 4).count, 8);

	EXPECT_EQ(info({33, 077, 49}, 2).index, 7); // Mode 2
	EXPECT_EQ(info({33, 077, 49}, 2).count, 8);
	EXPECT_EQ(info({33, 077, 49}, 4).index, 7); // Mode 4
	EXPECT_EQ(info({33, 077, 49}, 4).count, 8);

	EXPECT_EQ(info({33, 001, 49}, 2).index, 0); // Mode 2
	EXPECT_EQ(info({33, 001, 49}, 2).count, 2);
	EXPECT_EQ(info({33, 001, 49}, 4).index, 0); // Mode 4
	EXPECT_EQ(info({33, 001, 49}, 4).count, 2);

	EXPECT_EQ(info({33, 011, 49}, 2).index, 1); // Mode 2
	EXPECT_EQ(info({33, 011, 49}, 2).count, 2);
	EXPECT_EQ(info({33, 011, 49}, 4).index, 1); // Mode 4
	EXPECT_EQ(info({33, 011, 49}, 4).count, 2);

	// Invalid
	EXPECT_EQ(info({33, 000, 49}, 2).index, 0); // Mode 2
	EXPECT_EQ(info({33, 000, 49}, 2).count, 0); // Count 1 set to 0
	EXPECT_EQ(info({33, 000, 49}, 4).index, 0); // Mode 4
	EXPECT_EQ(info({33, 000, 49}, 4).count, 0);

	EXPECT_EQ(info({33, 032, 49}, 2).index, 3); // Mode 2
	EXPECT_EQ(info({33, 032, 49}, 2).count, 0); // Count 3 <= index 3 so set to 0
	EXPECT_EQ(info({33, 032, 49}, 4).index, 3); // Mode 4
	EXPECT_EQ(info({33, 032, 49}, 4).count, 0);
}

TEST(MCDecodeTest, ReaderInit)
{
	// Null
	EXPECT_FALSE(parse({49}, 2).readerInit()); // Mode 2
	EXPECT_TRUE(parse({49}, 2).isValid());

	// Set
	EXPECT_TRUE(parse({49}, 6).readerInit()); // Mode 6
	EXPECT_TRUE(parse({49}, 6).isValid());
}
