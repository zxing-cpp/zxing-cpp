/*
* Copyright 2021 gitlost
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "BitArray.h"
#include "BitArrayUtility.h"
#include "oned/rss/ODRSSExpandedBinaryDecoder.h"

#include "gtest/gtest.h"

using namespace ZXing;

static std::string parse(std::string bitStr)
{
	return OneD::DataBar::DecodeExpandedBits(Utility::ParseBitArray(bitStr, '1'));
}

TEST(ODRSSExpandedBinaryDecoderTest, FNC1NumericLatch)
{
	std::string result;

	// Correctly encoded (Alphanumeric FNC1 "01111" implying numeric latch)
	result = parse("0000000100110010101000010000001111011011000111110100001000000100");
	EXPECT_EQ(result, "(10)12A(422)123");

	// Incorrectly encoded (Alphanumeric FNC1 "01111" followed by numeric latch "000")
	result = parse("0000000100110010101000010000001111000011011000111110100001000000100");
	EXPECT_EQ(result, "(10)12A(422)123");

	// Correctly encoded (ISO646 FNC1 "01111" implying numeric latch)
	result = parse("0001000100110010101000000100111011010111101101100011111010000100000010000100");
	EXPECT_EQ(result, "(10)12((422)123");

	// Incorrectly encoded (ISO646 FNC1 "01111" followed by numeric latch "000")
	result = parse("0001000100110010101000000100111011010111100001101100011111010000100000010000100");
	EXPECT_EQ(result, "(10)12((422)123");
}
