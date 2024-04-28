/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2008 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "ByteArray.h"
#include "DecoderResult.h"

#include "gtest/gtest.h"
#include <utility>

namespace ZXing::DataMatrix::DecodedBitStreamParser {

DecoderResult Decode(ByteArray&& bytes, const bool isDMRE);

}

using namespace ZXing;

// Helper to call Decode()
static DecoderResult parse(ByteArray bytes, const bool isDMRE = false)
{
	return DataMatrix::DecodedBitStreamParser::Decode(std::move(bytes), isDMRE);
}

// Shorthand to return text
static std::wstring decode(ByteArray bytes, const bool isDMRE = false)
{
	return parse(std::move(bytes), isDMRE).text();
}

// Shorthand to return symbology identifier
static std::string id(ByteArray bytes, const bool isDMRE = false)
{
	return parse(std::move(bytes), isDMRE).symbologyIdentifier();
}

// Shorthand to return Structured Append
static StructuredAppendInfo info(ByteArray bytes, const bool isDMRE = false)
{
	return parse(std::move(bytes), isDMRE).structuredAppend();
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
	EXPECT_EQ(parse({66, 250, 68}).error(), Error::Format);

	// ASCII err on invalid code word at end (currently failing)
	EXPECT_EQ(parse({66, 67, 68, 250}).error(), Error::Format);

	// ASCII accept extra (illegal) unlatch at end
	EXPECT_FALSE(parse({66, 67, 68, 254}).error());
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

TEST(DMDecodeTest, SymbologyIdentifier)
{
	// Plain
	EXPECT_EQ(id({50}), "]d1");
	EXPECT_EQ(decode({50}), L"1");

	// GS1 "FNC1 (20)01"
	EXPECT_EQ(id({232, 150, 131}), "]d2");
	EXPECT_EQ(decode({232, 150, 131}), L"2001");

	// "LatchC40 Shift2 FNC1 LatchASCII 2001" not recognized as FNC1 in first position
	EXPECT_EQ(id({230, 0x0A, 0x79, 254, 150, 131}), "]d1"); // shift2FNC1 = (1600 * 1) + (40 * 27) + 0 + 1 == 0x0A79
	EXPECT_EQ(decode({230, 0x0A, 0x79, 254, 150, 131}), L"\u001D2001");

	// AIM "A FNC1 B"
	EXPECT_EQ(id({66, 232, 67}), "]d3");
	EXPECT_EQ(decode({66, 232, 67}), L"AB");

	// AIM "9 FNC1 A"
	EXPECT_EQ(id({58, 232, 66}), "]d3");
	EXPECT_EQ(decode({58, 232, 66}), L"9A");

	// AIM "99 FNC1 A" (double digit + 130)
	EXPECT_EQ(id({99 + 130, 232, 66}), "]d3");
	EXPECT_EQ(decode({99 + 130, 232, 66}), L"99A");

	// AIM "? FNC1 A" (ISO/IEC 16022:2006 11.2 does not specify any restrictions on single first character)
	EXPECT_EQ(id({64, 232, 66}), "]d3");
	EXPECT_EQ(decode({64, 232, 66}), L"?A");

	// "LatchC40 A Shift2 FNC1 B" not recognized as FNC1 in second position
	EXPECT_EQ(id({230, 0x57, 0xC4, 254, 67}), "]d1"); // shift2FNC1 = 1600 * 14 + (40 * 1) + 27 + 1 == 0x57C4
	EXPECT_EQ(decode({230, 0x57, 0xC4, 254, 67}), L"A\u001DB");

	// "99 FNC1 A" (2 single digits before FNC1 not recognized as AIM)
	EXPECT_EQ(id({58, 58, 232, 66}), "]d1");
	EXPECT_EQ(decode({58, 58, 232, 66}), L"99\u001DA");

	// GS1 "StructuredAppend FNC1 (20)01"
	EXPECT_EQ(id({233, 42, 1, 1, 232, 150, 131}), "]d2");
	EXPECT_EQ(decode({233, 42, 1, 1, 232, 150, 131}), L"2001");

	// AIM "StructuredAppend A FNC1 B"
	EXPECT_EQ(id({233, 42, 1, 1, 66, 232, 67}), "]d3");
	EXPECT_EQ(decode({233, 42, 1, 1, 66, 232, 67}), L"AB");
}

TEST(DMDecodeTest, DMRESymbologyIdentifier)
{
	// Plain
	EXPECT_EQ(id({50}, true /*isDMRE*/), "]d7");
	EXPECT_EQ(decode({50}, true /*isDMRE*/), L"1");

	// GS1 "FNC1 (20)01"
	EXPECT_EQ(id({232, 150, 131}, true /*isDMRE*/), "]d8");
	EXPECT_EQ(decode({232, 150, 131}, true /*isDMRE*/), L"2001");

	// AIM "A FNC1 B"
	EXPECT_EQ(id({66, 232, 67}, true /*isDMRE*/), "]d9");
	EXPECT_EQ(decode({66, 232, 67}, true /*isDMRE*/), L"AB");

	// AIM "9 FNC1 A"
	EXPECT_EQ(id({58, 232, 66}, true /*isDMRE*/), "]d9");
	EXPECT_EQ(decode({58, 232, 66}, true /*isDMRE*/), L"9A");

	// AIM "99 FNC1 A" (double digit + 130)
	EXPECT_EQ(id({99 + 130, 232, 66}, true /*isDMRE*/), "]d9");
	EXPECT_EQ(decode({99 + 130, 232, 66}, true /*isDMRE*/), L"99A");

	// AIM "? FNC1 A" (ISO/IEC 16022:2006 11.2 does not specify any restrictions on single first character)
	EXPECT_EQ(id({64, 232, 66}, true /*isDMRE*/), "]d9");
	EXPECT_EQ(decode({64, 232, 66}, true /*isDMRE*/), L"?A");

	// "99 FNC1 A" (2 single digits before FNC1 not recognized as AIM)
	EXPECT_EQ(id({58, 58, 232, 66}, true /*isDMRE*/), "]d7");
	EXPECT_EQ(decode({58, 58, 232, 66}, true /*isDMRE*/), L"99\u001DA");

	// GS1 "StructuredAppend FNC1 (20)01"
	EXPECT_EQ(id({233, 42, 1, 1, 232, 150, 131}, true /*isDMRE*/), "]d8");
	EXPECT_EQ(decode({233, 42, 1, 1, 232, 150, 131}, true /*isDMRE*/), L"2001");

	// AIM "StructuredAppend A FNC1 B"
	EXPECT_EQ(id({233, 42, 1, 1, 66, 232, 67}, true /*isDMRE*/), "]d9");
	EXPECT_EQ(decode({233, 42, 1, 1, 66, 232, 67}, true /*isDMRE*/), L"AB");
}

TEST(DMDecodeTest, StructuredAppend)
{
	// Null
	EXPECT_EQ(info({50}).index, -1);
	EXPECT_EQ(info({50}).count, -1);
	EXPECT_TRUE(info({50}).id.empty());
	EXPECT_EQ(id({50}), "]d1");

	// Structured Append "233" must be first ISO 16022:2006 5.6.1
	EXPECT_FALSE(parse({50, 233, 42, 1, 1}).isValid());

	// ISO/IEC 16022:2006 5.6.2 sequence indicator example
	EXPECT_TRUE(parse({233, 42, 1, 1, 50}).isValid());
	EXPECT_EQ(info({233, 42, 1, 1, 50}).index, 2); // 1-based position 3 == index 2
	EXPECT_EQ(info({233, 42, 1, 1, 50}).count, 7);
	EXPECT_EQ(info({233, 42, 1, 1, 50}).id, "257");
	EXPECT_EQ(id({233, 42, 1, 1, 50}), "]d1");

	// Sequence indicator
	EXPECT_EQ(info({233, 0, 1, 1, 50}).index, 0);
	EXPECT_EQ(info({233, 0, 1, 1, 50}).count, 0); // Count 17 set to 0

	EXPECT_EQ(info({233, 1, 1, 1, 50}).index, 0);
	EXPECT_EQ(info({233, 1, 1, 1, 50}).count, 16);

	EXPECT_EQ(info({233, 0x81, 1, 1, 50}).index, 8);
	EXPECT_EQ(info({233, 0x81, 1, 1, 50}).count, 16);

	EXPECT_EQ(info({233, 0xFF, 1, 1, 50}).index, 15);
	EXPECT_EQ(info({233, 0xFF, 1, 1, 50}).count, 0); // Count 2 <= index so set to 0

	EXPECT_EQ(info({233, 0xF1, 1, 1, 50}).index, 15);
	EXPECT_EQ(info({233, 0xF1, 1, 1, 50}).count, 16);

	// File identification
	EXPECT_EQ(info({233, 42, 1, 12, 50}).id, "268");
	EXPECT_EQ(info({233, 42, 12, 34, 50}).id, "3106");
	EXPECT_EQ(info({233, 42, 12, 123, 50}).id, "3195");
	EXPECT_EQ(info({233, 42, 254, 254, 50}).id, "65278");
	// Values outside 1-254 allowed (i.e. tolerated)
	EXPECT_EQ(info({233, 42, 0, 0, 50}).id, "0");
	EXPECT_EQ(info({233, 42, 0, 255, 50}).id, "255");
	EXPECT_EQ(info({233, 42, 255, 0, 50}).id, "65280");
	EXPECT_EQ(info({233, 42, 255, 255, 50}).id, "65535");
}

TEST(DMDecodeTest, ReaderInit)
{
	// Null
	EXPECT_FALSE(parse({50}).readerInit());
	EXPECT_TRUE(parse({50}).isValid());

	// Reader Programming "234" must be first ISO 16022:2006 5.2.4.9
	EXPECT_FALSE(parse({50, 234}).isValid());

	// Set
	EXPECT_TRUE(parse({234, 50}).isValid());
	EXPECT_TRUE(parse({234, 50}).readerInit());

	EXPECT_FALSE(parse({235, 234, 50}).isValid());

	// Can't be used with Structured Append "233"
	EXPECT_TRUE(parse({233, 42, 1, 1, 50}).isValid()); // Null
	EXPECT_FALSE(parse({233, 42, 1, 1, 234, 50}).isValid());
}
