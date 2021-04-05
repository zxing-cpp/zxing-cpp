/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2008 ZXing authors
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

#include "ByteArray.h"
#include "DecoderResult.h"

#include "gtest/gtest.h"
#include <utility>

namespace ZXing::DataMatrix::DecodedBitStreamParser {

DecoderResult Decode(ByteArray&& bytes, const std::string& characterSet);

}

using namespace ZXing;

static std::wstring decode(ByteArray bytes)
{
	return DataMatrix::DecodedBitStreamParser::Decode(std::move(bytes), "").text();
}

TEST(DMDecodeTest, Ascii)
{
	// ASCII characters 0-127 are encoded as the value + 1
	EXPECT_EQ(decode({'b', 'c', 'd', 'B', 'C', 'D'}), L"abcABC");

	// ASCII double digit (00 - 99) Numeric Value + 130
	EXPECT_EQ(decode({130, 131, 228, 229}), L"00019899");
}

TEST(DMDecodeTest, AsciiError)
{
	// ASCII err on invalid code word
	EXPECT_EQ(DataMatrix::DecodedBitStreamParser::Decode({66, 250, 68}, "").errorCode(), DecodeStatus::FormatError);

	// ASCII err on invalid code word at end (currently failing)
//	EXPECT_EQ(DataMatrix::DecodedBitStreamParser::Decode({66, 67, 68, 250}, "").errorCode(), DecodeStatus::FormatError);

	// ASCII accept extra (illegal) unlatch at end
	EXPECT_EQ(DataMatrix::DecodedBitStreamParser::Decode({66, 67, 68, 254}, "").errorCode(), DecodeStatus::NoError);
}

// Most of the following examples are taken from the DMHighLevelEncodeTest.cpp tests.
// For an explanation of the different cases, see there.

TEST(DMDecodeTest, C40)
{
	EXPECT_EQ(decode({230, 91, 11, 91, 11, 91, 11, 254}), L"AIMAIMAIM");
	EXPECT_EQ(decode({66, 74, 78, 66, 74, 66, 99, 129}), L"AIMAIAb");
	EXPECT_EQ(decode({230, 91, 11, 91, 11, 91, 11, 254, 235, 76}), L"AIMAIMAIM\xCB");
	EXPECT_EQ(decode({230, 91, 11, 91, 11, 91, 11, 254, 235, 108}), L"AIMAIMAIM\xEB");
	EXPECT_EQ(decode({230, 88, 88, 40, 8, 107, 147, 59, 67, 126, 206, 78, 126, 144, 121, 35, 47, 254}), L"A1B2C3D4E5F6G7H8I9J0K1L2");
	EXPECT_EQ(decode({230, 91, 11, 91, 11, 91, 11, 91, 11, 91, 11, 91, 11}), L"AIMAIMAIMAIMAIMAIM");
	EXPECT_EQ(decode({230, 91, 11, 91, 11, 91, 11, 91, 11, 91, 11, 90, 241}), L"AIMAIMAIMAIMAIMAI");
	EXPECT_EQ(decode({230, 91, 11, 91, 11, 91, 11, 91, 11, 91, 11, 254, 66}), L"AIMAIMAIMAIMAIMA");
	EXPECT_EQ(decode({230, 91, 11, 91, 11, 91, 11, 91, 11, 91, 11, 254, 66, 74, 129, 237}), L"AIMAIMAIMAIMAIMAI");
	EXPECT_EQ(decode({230, 91, 11, 91, 11, 91, 11, 66}), L"AIMAIMAIMA");
	EXPECT_EQ(decode({230, 91, 11, 91, 11, 91, 11, 91, 11, 91, 11, 91, 11, 254, 66, 74}), L"AIMAIMAIMAIMAIMAIMAI");
}

TEST(DMDecodeTest, Text)
{
	EXPECT_EQ(decode({239, 91, 11, 91, 11, 91, 11, 254}), L"aimaimaim");
	EXPECT_EQ(decode({239, 91, 11, 91, 11, 91, 11, 254, 40, 129}), L"aimaimaim'");
	EXPECT_EQ(decode({239, 91, 11, 91, 11, 87, 218, 110}), L"aimaimaIm");
	EXPECT_EQ(decode({239, 91, 11, 91, 11, 91, 11, 254, 67, 129}), L"aimaimaimB");
	EXPECT_EQ(decode({239, 91, 11, 91, 11, 91, 11, 16, 218, 236, 107, 181, 69, 254, 129, 237}), L"aimaimaim{txt}\x04");
}

TEST(DMDecodeTest, C40AndTextShiftUpper)
{
	// additional shiftUpper test: (1->shift 2, 30->upperShift, 3->' '+128==0xa0) == 2804 == 0x0af4

	EXPECT_EQ(decode({230, 0x0a, 0xf4}), L"\xA0"); // C40
	EXPECT_EQ(decode({239, 0x0a, 0xf4}), L"\xA0"); // Text
}

TEST(DMDecodeTest, X12)
{
	EXPECT_EQ(decode({238, 89, 233, 14, 192, 100, 207, 44, 31, 67}), L"ABC>ABC123>AB");
	EXPECT_EQ(decode({238, 89, 233, 14, 192, 100, 207, 44, 31, 254, 67, 68}), L"ABC>ABC123>ABC");
	EXPECT_EQ(decode({238, 89, 233, 14, 192, 100, 207, 44, 31, 96, 82, 254}), L"ABC>ABC123>ABCD");
	EXPECT_EQ(decode({238, 89, 233, 14, 192, 100, 207, 44, 31, 96, 82, 70}), L"ABC>ABC123>ABCDE");
	EXPECT_EQ(decode({238, 89, 233, 14, 192, 100, 207, 44, 31, 96, 82, 254, 70, 71, 129, 237}), L"ABC>ABC123>ABCDEF");
//	EXPECT_EQ(decode({}), L"");
}
