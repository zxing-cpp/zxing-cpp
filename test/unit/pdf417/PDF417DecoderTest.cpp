/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "DecoderResult.h"
#include "pdf417/PDFDecoder.h"
#include "pdf417/PDFDecoderResultExtra.h"

#include "gtest/gtest.h"

namespace ZXing::Pdf417 {
int DecodeMacroBlock(const std::vector<int>& codewords, int codeIndex, DecoderResultExtra& resultMetadata);
}

using namespace ZXing;
using namespace ZXing::Pdf417;

/**
* Tests the first sample given in ISO/IEC 15438:2015(E) - Annex H.4
*/
TEST(PDF417DecoderTest, StandardSample1)
{
	std::vector<int> sampleCodes = { 20, 928, 111, 100, 17, 53, 923, 1, 111, 104, 923, 3, 64, 416, 34, 923, 4, 258, 446, 67,
		// we should never reach these
		1000, 1000, 1000 };

	DecoderResultExtra resultMetadata;
	DecodeMacroBlock(sampleCodes, 2, resultMetadata);

	EXPECT_EQ(0, resultMetadata.segmentIndex());
	EXPECT_EQ("017053", resultMetadata.fileId());
	EXPECT_EQ(false, resultMetadata.isLastSegment());
	EXPECT_EQ(4, resultMetadata.segmentCount());
	EXPECT_EQ("CEN BE", resultMetadata.sender());
	EXPECT_EQ("ISO CH", resultMetadata.addressee());

	auto optionalData = resultMetadata.optionalData();
	EXPECT_EQ(1, optionalData.front()) << "first element of optional array should be the first field identifier";
	EXPECT_EQ(67, optionalData.back()) << "last element of optional array should be the last codeword of the last field";

	auto result = Decode(sampleCodes);

	EXPECT_EQ(0, result.structuredAppend().index);
	EXPECT_EQ("017053", result.structuredAppend().id);
	EXPECT_EQ(4, result.structuredAppend().count);
}

/**
* Tests the second given in ISO/IEC 15438:2015(E) - Annex H.4
*/
TEST(PDF417DecoderTest, StandardSample2)
{
	std::vector<int> sampleCodes = { 11, 928, 111, 103, 17, 53, 923, 1, 111, 104, 922,
		// we should never reach these
		1000, 1000, 1000 };

	DecoderResultExtra resultMetadata;
	DecodeMacroBlock(sampleCodes, 2, resultMetadata);

	EXPECT_EQ(3, resultMetadata.segmentIndex());
	EXPECT_EQ("017053", resultMetadata.fileId());
	EXPECT_EQ(true, resultMetadata.isLastSegment());
	EXPECT_EQ(4, resultMetadata.segmentCount());
	EXPECT_EQ("", resultMetadata.sender());
	EXPECT_EQ("", resultMetadata.addressee());

	auto optionalData = resultMetadata.optionalData();
	EXPECT_EQ(1, optionalData.front()) << "first element of optional array should be the first field identifier";
	EXPECT_EQ(104, optionalData.back()) << "last element of optional array should be the last codeword of the last field";

	auto result = Decode(sampleCodes);

	EXPECT_EQ(3, result.structuredAppend().index);
	EXPECT_EQ("017053", result.structuredAppend().id);
	EXPECT_EQ(4, result.structuredAppend().count);
}

/**
* Tests the example given in ISO/IEC 15438:2015(E) - Annex H.6
*/
TEST(PDF417DecoderTest, StandardSample3)
{
	std::vector<int> sampleCodes = { 7, 928, 111, 100, 100, 200, 300 };

	DecoderResultExtra resultMetadata;
	DecodeMacroBlock(sampleCodes, 2, resultMetadata);

	EXPECT_EQ(0, resultMetadata.segmentIndex());
	EXPECT_EQ("100200300", resultMetadata.fileId());
	EXPECT_EQ(-1, resultMetadata.segmentCount());

	auto result = Decode(sampleCodes);

	EXPECT_EQ(0, result.structuredAppend().index);
	EXPECT_EQ("100200300", result.structuredAppend().id);
	EXPECT_EQ(0, result.structuredAppend().count);
}

TEST(PDF417DecoderTest, SampleWithFilename)
{
	std::vector<int> sampleCodes = { 23, 477, 928, 111, 100, 0, 252, 21, 86, 923, 0, 815, 251, 133, 12, 148, 537, 593,
		599, 923, 1, 111, 102, 98, 311, 355, 522, 920, 779, 40, 628, 33, 749, 267, 506, 213, 928, 465, 248, 493, 72,
		780, 699, 780, 493, 755, 84, 198, 628, 368, 156, 198, 809, 19, 113 };

	DecoderResultExtra resultMetadata;
	DecodeMacroBlock(sampleCodes, 3, resultMetadata);

	EXPECT_EQ(0, resultMetadata.segmentIndex());
	EXPECT_EQ("000252021086", resultMetadata.fileId());
	EXPECT_EQ(false, resultMetadata.isLastSegment());
	EXPECT_EQ(2, resultMetadata.segmentCount());
	EXPECT_EQ("", resultMetadata.sender());
	EXPECT_EQ("", resultMetadata.addressee());
	EXPECT_EQ("filename.txt", resultMetadata.fileName());

	auto result = Decode(sampleCodes);

	EXPECT_EQ(0, result.structuredAppend().index);
	EXPECT_EQ("000252021086", result.structuredAppend().id);
	EXPECT_EQ(2, result.structuredAppend().count);
}

TEST(PDF417DecoderTest, SampleWithNumericValues)
{
	std::vector<int> sampleCodes = { 25, 477, 928, 111, 100, 0, 252, 21, 86, 923, 2, 2, 0, 1, 0, 0, 0, 923, 5, 130,
		923, 6, 1, 500, 13 };

	DecoderResultExtra resultMetadata;
	DecodeMacroBlock(sampleCodes, 3, resultMetadata);

	EXPECT_EQ(0, resultMetadata.segmentIndex());
	EXPECT_EQ("000252021086", resultMetadata.fileId());
	EXPECT_EQ(false, resultMetadata.isLastSegment());

	EXPECT_EQ(180980729000000L, resultMetadata.timestamp());
	EXPECT_EQ(30, resultMetadata.fileSize());
	EXPECT_EQ(260013, resultMetadata.checksum());
	EXPECT_EQ(-1, resultMetadata.segmentCount());

	auto result = Decode(sampleCodes);

	EXPECT_EQ(0, result.structuredAppend().index);
	EXPECT_EQ("000252021086", result.structuredAppend().id);
	EXPECT_EQ(0, result.structuredAppend().count);
}

TEST(PDF417DecoderTest, SampleWithMacroTerminatorOnly)
{
	std::vector<int> sampleCodes = { 7, 477, 928, 222, 198, 0, 922 };

	DecoderResultExtra resultMetadata;
	DecodeMacroBlock(sampleCodes, 3, resultMetadata);

	EXPECT_EQ(99998, resultMetadata.segmentIndex());
	EXPECT_EQ("000", resultMetadata.fileId());
	EXPECT_EQ(true, resultMetadata.isLastSegment());
	EXPECT_EQ(-1, resultMetadata.segmentCount());

	auto result = Decode(sampleCodes);

	EXPECT_EQ(99998, result.structuredAppend().index);
	EXPECT_EQ("000", result.structuredAppend().id);
	EXPECT_EQ(99999, result.structuredAppend().count);
}

// Shorthand to decode and return text
static std::wstring decode(const std::vector<int>& codewords)
{
	return Decode(codewords).text();
}

// Shorthand to decode and return isValid
static bool valid(const std::vector<int>& codewords)
{
	return Decode(codewords).isValid();
}

TEST(PDF417DecoderTest, TextCompactionSimple)
{
	// ISO/IEC 15438:2015 Figure 1
	EXPECT_EQ(decode({ 16, 453, 178, 121, 236, 858, 834, 361, 431, 426, 746, 828, 570, 393, 17, 119 }),
		L"PDF417 Symbology Standard");
	EXPECT_EQ(decode({ 16, 453, 178, 121, 237, 807, 564, 361, 431, 426, 746, 828, 570, 393, 17, 119 }),
		L"PDF417 Symbology Standard");

	// Alpha
	EXPECT_EQ(decode({ 15, 1, 63, 125, 187, 249, 311, 373, 435, 497, 559, 621, 683, 745, 809 }),
		L"ABCDEFGHIJKLMNOPQRSTUVWXYZ ");

	// Lower
	EXPECT_EQ(decode({ 15, 810, 32, 94, 156, 218, 280, 342, 404, 466, 528, 590, 652, 714, 776 }),
		L"abcdefghijklmnopqrstuvwxyz ");

	// Mixed
	EXPECT_EQ(decode({ 15, 840, 32, 94, 156, 311, 373, 435, 497, 559, 621, 683, 746, 218, 299 }),
		L"0123456&\r\t,:#-.$/+%*=^ 789");

	// Punctuation
	EXPECT_EQ(decode({ 16, 865, 1, 63, 125, 187, 849, 311, 373, 435, 497, 559, 621, 683, 745, 809 }),
		L";<>@[\\]_'~!\r\t,:\n-.$/\"|*()?{");

	// Alpha Punctuation Lower Mixed
	EXPECT_EQ(decode({ 27, 1, 865, 807, 896, 782, 855, 626, 807, 94, 865, 807, 896, 808, 776, 839, 176, 808, 32, 776,
		839, 806, 208, 776, 839, 806, 239 }),
		L"AB{}  C#+  de{}  {}F  12{}  G{}  H");
	EXPECT_EQ(decode({ 25, 1, 896, 897, 806, 88, 470, 836, 783, 148, 776, 839, 806, 896, 897, 178, 806, 32, 776, 839,
		806, 209, 809, 836, 787 }),
		L"AB{}  C#+  de{}  {}F  12{}  G{}  H");
}

TEST(PDF417DecoderTest, TextCompactionShiftByte)
{
	// Alpha ShiftByte Alpha
	EXPECT_EQ(decode({ 7, 0, 0, 913, 233, 0, 0 }), L"AAAA\u00E9AAAA");

	// Alpha ShiftByte Alpha(PS) (Padding)
	EXPECT_EQ(decode({ 8, 0, 0, 913, 233, 0, 0, 29 }), L"AAAA\u00E9AAAAA");

	// Alpha(PS) ShiftByte Alpha (Section 5.4.2.4 (b) (1) PS ignored)
	EXPECT_EQ(decode({ 8, 0, 0, 29, 913, 233, 0, 0 }), L"AAAAA\u00E9AAAA");

	// Alpha(PS) ShiftByte Lower(PS) (Padding)
	EXPECT_EQ(decode({ 10, 0, 0, 29, 913, 233, 810, 0, 0, 29 }), L"AAAAA\u00E9aaaaaa");

	// Lower ShiftByte Lower
	EXPECT_EQ(decode({ 9, 810, 0, 0, 913, 233, 0, 0, 0 }), L"aaaaa\u00E9aaaaaa");

	// Lower(PS) ShiftByte Lower (Section 5.4.2.4 (b) (1) PS ignored)
	EXPECT_EQ(decode({ 10, 810, 0, 0, 29, 913, 233, 0, 0, 0 }), L"aaaaaa\u00E9aaaaaa");

	// Mixed ShiftByte Mixed
	EXPECT_EQ(decode({ 9, 840, 0, 0, 913, 233, 0, 0, 0 }), L"00000\u00E9000000");

	// Mixed(PS) ShiftByte Mixed (Section 5.4.2.4 (b) (1) PS ignored)
	EXPECT_EQ(decode({ 8, 840, 0, 29, 913, 233, 0, 0, }), L"0000\u00E90000");

	// Punctuation ShiftByte Punctuation
	EXPECT_EQ(decode({ 8, 865, 0, 0, 913, 233, 0, 0 }), L";;;;\u00E9;;;;");

	// Punctuation(AL) ShiftByte (Alpha) (Section 5.4.2.4 (b) (2) AL not ignored)
	EXPECT_EQ(decode({ 9, 865, 0, 0, 29, 913, 233, 0, 0 }), L";;;;;\u00E9AAAA");

	// Punctuation(AL) ShiftByte Punctuation(AL) (Padding)
	EXPECT_EQ(decode({ 11, 865, 0, 0, 29, 913, 233, 865, 0, 0, 29 }), L";;;;;\u00E9;;;;;");

	// Punctuation(AL) ShiftByte Lower
	EXPECT_EQ(decode({ 10, 865, 0, 0, 29, 913, 233, 810, 0, 0 }), L";;;;;\u00E9aaaaa");

	// ShiftByte (first position, which defaults to Text Compaction)
	EXPECT_EQ(decode({ 5, 913, 255, 775, 775 }), L"\u00FFZZZZ");

	// Byte ShiftByte (ShiftByte can only occur in Text Compaction)
	EXPECT_FALSE(valid({ 6, 901, 255, 255, 913, 255 }));

	// Numeric ShiftByte (ShiftByte can only occur in Text Compaction)
	EXPECT_FALSE(valid({ 7, 902, 171, 209, 268, 913, 255 }));

	// Text, Numeric, Byte, ShiftByte
	EXPECT_FALSE(valid({ 18, 1, 63, 125, 902, 17, 110, 836, 811, 223, 901, 127, 127, 127, 127, 913, 255, 775 }));

	// Text, Numeric, ShiftByte
	EXPECT_FALSE(valid({ 13, 1, 63, 125, 902, 17, 110, 836, 811, 223, 913, 255, 775 }));
}

TEST(PDF417DecoderTest, ByteCompaction)
{
	// Byte (901)
	EXPECT_EQ(decode({ 12, 901, 213, 598, 413, 118, 87, 127, 127, 127, 127, 127 }),
		L"\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F");

	// Byte6 (924) (mod 6 == 0)
	EXPECT_EQ(decode({ 12, 924, 213, 598, 413, 118, 87, 213, 598, 413, 118, 87 }),
		L"\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F");

	// 924/901
	EXPECT_EQ(decode({ 13, 924, 213, 598, 413, 118, 87, 901, 127, 127, 127, 127, 127 }),
		L"\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F");
	EXPECT_EQ(decode({ 17, 924, 213, 598, 413, 118, 87, 213, 598, 413, 118, 87, 901, 127, 127, 901, 127 }),
		L"\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F");

	// Multiple 901s
	EXPECT_EQ(decode({ 15, 901, 127, 127, 127, 127, 127, 901, 127, 127, 127, 127, 901, 127, 127 }),
		L"\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F");
	EXPECT_EQ(decode({ 13, 901, 213, 598, 413, 118, 87, 127, 901, 127, 127, 127, 127 }),
		L"\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F");
	EXPECT_EQ(decode({ 13, 901, 213, 598, 413, 118, 87, 127, 127, 127, 127, 901, 127 }),
		L"\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F");
	EXPECT_EQ(decode({ 13, 901, 213, 598, 413, 118, 87, 127, 127, 127, 127, 127, 901 }),
		L"\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F");
	EXPECT_EQ(decode({ 14, 901, 213, 598, 413, 118, 87, 127, 127, 127, 127, 901, 901, 127 }),
		L"\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F");
	EXPECT_EQ(decode({ 15, 901, 213, 598, 413, 118, 87, 127, 127, 127, 127, 901, 901, 127, 901 }),
		L"\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F");
	EXPECT_EQ(decode({ 17, 901, 213, 598, 413, 118, 87, 127, 127, 127, 127, 127, 901, 127, 127, 127, 127 }),
		L"\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F");
}

TEST(PDF417DecoderTest, NumericCompaction)
{
	// 43 consecutive
	EXPECT_EQ(decode({ 17, 902, 49, 98, 103, 675, 30, 186, 631, 467, 409, 266, 246, 677, 536, 811, 223 }),
		L"1234567890123456789012345678901234567890123");

	// 44 consecutive
	EXPECT_EQ(decode({ 17, 902, 491, 81, 137, 450, 302, 67, 15, 174, 492, 862, 667, 475, 869, 12, 434 }),
		L"12345678901234567890123456789012345678901234");

	// 45 consecutive
	EXPECT_EQ(decode({ 18, 902, 491, 81, 137, 450, 302, 67, 15, 174, 492, 862, 667, 475, 869, 12, 434, 15 }),
		L"123456789012345678901234567890123456789012345");

	// 87 consecutive
	EXPECT_EQ(decode({ 32, 902, 491, 81, 137, 450, 302, 67, 15, 174, 492, 862, 667, 475, 869, 12, 434, 68, 482, 582,
		185, 641, 663, 847, 653, 803, 879, 734, 537, 34, 771, 667 }),
		L"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567");

	// 88 consecutive
	EXPECT_EQ(decode({ 32, 902, 491, 81, 137, 450, 302, 67, 15, 174, 492, 862, 667, 475, 869, 12, 434, 685, 326, 422,
		57, 117, 339, 377, 238, 839, 698, 145, 870, 348, 517, 378 }),
		L"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678");

	// 89 consecutive
	EXPECT_EQ(decode({ 33, 902, 491, 81, 137, 450, 302, 67, 15, 174, 492, 862, 667, 475, 869, 12, 434, 685, 326, 422,
		57, 117, 339, 377, 238, 839, 698, 145, 870, 348, 517, 378, 19 }),
		L"12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789");
}

TEST(PDF417DecoderTest, CompactionCombos)
{
	// Text, Byte, Numeric, Text
	EXPECT_EQ(decode({ 19, 1, 63, 125, 901, 127, 127, 127, 127, 902, 17, 110, 836, 811, 223, 900, 652, 714, 779 }),
		L"ABCDEF\x7F\x7F\x7F\x7F" L"1234567890123VWXYZ");

	// Text, Numeric, Byte, Text
	EXPECT_EQ(decode({ 19, 1, 63, 125, 902, 17, 110, 836, 811, 223, 901, 127, 127, 127, 127, 900, 652, 714, 779 }),
		L"ABCDEF1234567890123\x7F\x7F\x7F\x7FVWXYZ");

	// Byte, Text, Numeric, Text
	EXPECT_EQ(decode({ 19, 901, 127, 127, 127, 900, 1, 63, 125, 902, 17, 110, 836, 811, 223, 900, 652, 714, 779 }),
		L"\x7F\x7F\x7F" L"ABCDEF1234567890123VWXYZ");

	// Byte, Numeric, Text
	EXPECT_EQ(decode({ 17, 901, 127, 127, 127, 127, 127, 902, 17, 110, 836, 811, 223, 900, 652, 714, 779 }),
		L"\x7F\x7F\x7F\x7F\x7F" L"1234567890123VWXYZ");

	// Numeric, Text, Byte, Text
	EXPECT_EQ(decode({ 19, 902, 17, 110, 836, 811, 223, 900, 1, 63, 125, 901, 127, 127, 127, 900, 652, 714, 779 }),
		L"1234567890123ABCDEF\x7F\x7F\x7FVWXYZ");

	// Numeric, Byte, Text
	EXPECT_EQ(decode({ 18, 902, 17, 110, 836, 811, 223, 901, 127, 127, 127, 900, 1, 63, 125, 652, 714, 779 }),
		L"1234567890123\x7F\x7F\x7F" L"ABCDEFVWXYZ");
}

TEST(PDF417DecoderTest, ECISingleText)
{
	// ECI 3 "Aé"
	EXPECT_EQ(decode({ 7, 927, 3, 900, 29, 913, 233 }), L"A\u00E9"); // ECI Text ShiftByte
	EXPECT_EQ(decode({ 7, 900, 927, 3, 29, 913, 233 }), L"A\u00E9"); // Text ECI ShiftByte
	EXPECT_EQ(decode({ 6, 927, 3, 29, 913, 233 }), L"A\u00E9"); // ECI (Text) ShiftByte
	EXPECT_EQ(decode({ 6, 927, 3, 901, 65, 233 }), L"A\u00E9"); // ECI Byte
	EXPECT_EQ(decode({ 6, 29, 913, 927, 3, 233 }), L"A\u00E9"); // (Text) ShiftByte ECI

	// ECI 9 "Aβ"
	EXPECT_EQ(decode({ 7, 927, 9, 900, 29, 913, 226 }), L"A\u03B2"); // ECI Text ShiftByte
	EXPECT_EQ(decode({ 7, 900, 927, 9, 29, 913, 226 }), L"A\u03B2"); // Text ECI ShiftByte
	EXPECT_EQ(decode({ 6, 927, 9, 29, 913, 226 }), L"A\u03B2"); // ECI (Text) ShiftByte
	EXPECT_EQ(decode({ 6, 927, 9, 901, 65, 226 }), L"A\u03B2"); // ECI Byte
	EXPECT_EQ(decode({ 6, 29, 913, 927, 9, 226 }), L"A\u03B2"); // (Text) ShiftByte ECI

	// "AB" ShiftByte ECI 9 "β"
	EXPECT_EQ(decode({ 6, 1, 913, 927, 9, 226 }), L"AB\u03B2");
}

TEST(PDF417DecoderTest, ECISingleByte)
{
	// ECI 20 Byte "点茗"
	EXPECT_EQ(decode({ 8, 927, 20, 901, 147, 95, 228, 170 }), L"\u70B9\u8317");

	// Byte ECI 20 "点茗"
	EXPECT_EQ(decode({ 8, 901, 927, 20, 147, 95, 228, 170 }), L"\u70B9\u8317");

	// ECI 20 Byte "点茗A"
	EXPECT_EQ(decode({ 9, 927, 20, 901, 147, 95, 228, 170, 65 }), L"\u70B9\u8317A");

	// Byte ECI 20 "点茗A"
	EXPECT_EQ(decode({ 9, 901, 927, 20, 147, 95, 228, 170, 65 }), L"\u70B9\u8317A");

	// ECI 20 Byte6 "点茗テ"
	EXPECT_EQ(decode({ 9, 927, 20, 924, 246, 877, 166, 106, 797 }), L"\u70B9\u8317\u30C6");

	// Byte6 ECI 20 "点茗テ"
	EXPECT_EQ(decode({ 9, 924, 927, 20, 246, 877, 166, 106, 797 }), L"\u70B9\u8317\u30C6");

	// Byte6 ECI 20 (not allowed inside 5-codeword batch)
	EXPECT_FALSE(valid({ 9, 924, 246, 877, 166, 106, 927, 20, 797 }));

	// Byte ECI 20 "点茗テA"
	EXPECT_EQ(decode({ 10, 901, 927, 20, 246, 877, 166, 106, 797, 65 }), L"\u70B9\u8317\u30C6A");
}

TEST(PDF417DecoderTest, ECISingleNumeric)
{
	// ECIs allowed anywhere in Numeric Compaction

	// Numeric ECI 20 Numeric(15)
	EXPECT_EQ(decode({ 19, 902, 927, 20, 491, 81, 137, 450, 302, 67, 15, 174, 492, 862, 667, 475, 869, 12, 434 }),
		L"12345678901234567890123456789012345678901234");

	// Numeric(1) ECI 20 Numeric(14)
	EXPECT_EQ(decode({ 19, 902, 11, 927, 20, 485, 624, 192, 160, 521, 439, 324, 737, 864, 136, 732, 282, 410, 12 }),
		L"123456789012345678901234567890123456789012");

	// Numeric(4) ECI 20 Numeric(11) Byte(ShiftJIS) "点茗"
	EXPECT_EQ(decode({ 24, 902, 154, 98, 332, 101, 927, 20, 354, 63, 496, 448, 236, 148, 354, 624, 335, 600, 123,
		901, 147, 95, 228, 170 }),
		L"1234567890123456789012345678901234567890123\u70B9\u8317");

	// Numeric(11) ECI 25 Numeric(4) Byte(UnicodeBig) "AĀ" (U+0100)
	// (ASCII values of "3456789012" as UTF-16 "343536373839303132" (CJK compatibility block)
	EXPECT_EQ(decode({ 24, 902, 322, 183, 750, 813, 535, 621, 854, 718, 783, 621, 112, 927, 25, 18, 413, 287, 712,
		901, 0, 'A', 1, 0 }),
		L"12345678901234567890123456789012\u3334\u3536\u3738\u3930\u3132A\u0100");
}

TEST(PDF417DecoderTest, ECIMultipleTextByte)
{
	// Text "ABCDEFG" ECI 9 Byte "αβ" ECI 3 "áA"
	EXPECT_EQ(decode({ 15, 1, 63, 125, 209, 927, 9, 901, 225, 226, 927, 3, 901, 225, 65 }),
		L"ABCDEFG\u03B1\u03B2\u00E1A");
	EXPECT_EQ(decode({ 14, 1, 63, 125, 209, 927, 9, 901, 225, 226, 927, 3, 225, 65 }), L"ABCDEFG\u03B1\u03B2\u00E1A");

	// Text "ABCDEFG" ECI 9 Byte6 "αβγδεζ" ECI 3 "áA" ECI 7 "жзи"
	EXPECT_EQ(decode({ 24, 1, 63, 125, 209, 927, 9, 924, 378, 492, 165, 708, 390, 927, 3, 901, 225, 65, 927, 7, 901,
		214, 215, 216 }),
		L"ABCDEFG\u03B1\u03B2\u03B3\u03B4\u03B5\u03B6\u00E1A\u0436\u0437\u0438");

	// "AB" ShiftByte ECI 9 "β" ShiftByte ECI 7 "ж"
	EXPECT_EQ(decode({ 10, 1, 913, 927, 9, 226, 913, 927, 7, 214 }), L"AB\u03B2\u0436");
}

TEST(PDF417DecoderTest, ECIMultipleByte)
{
	// Byte "AB" ECI 9 Byte "αβ" ECI 3 "á"
	EXPECT_EQ(decode({ 13, 901, 65, 66, 927, 9, 901, 225, 226, 927, 3, 901, 225 }), L"AB\u03B1\u03B2\u00E1");
	// Byte "AB" ECI 9 "αβ" ECI 3 "á"
	EXPECT_EQ(decode({ 11, 901, 65, 66, 927, 9, 225, 226, 927, 3, 225 }), L"AB\u03B1\u03B2\u00E1");

	// Byte ECI 20 "点茗" ECI 9 "α"
	EXPECT_EQ(decode({ 11, 901, 927, 20, 147, 95, 228, 170, 927, 9, 225 }), L"\u70B9\u8317\u03B1");

	// Byte ECI 20 "点茗" ECI 810899 ECI 9 ECI 811799 "α"
	EXPECT_EQ(decode({ 16, 901, 927, 20, 147, 95, 228, 170, 926, 899, 899, 927, 9, 925, 0, 225 }),
		L"\u70B9\u8317\u03B1");

	// Byte6 ECI 20 "点茗テ" ECI 22 Byte "ђ"
	EXPECT_EQ(decode({ 13, 924, 927, 20, 246, 877, 166, 106, 797, 927, 22, 901, 0x90 }), L"\u70B9\u8317\u30C6\u0452");

	// Byte ECI 20 "点茗テ" ECI 9 "α" ECI 22 "ђ"
	EXPECT_EQ(decode({ 15, 901, 927, 20, 246, 877, 166, 106, 797, 927, 9, 225, 927, 22, 0x90 }),
		L"\u70B9\u8317\u30C6\u03B1\u0452");

	// ECI 10 Byte ECI 20 "点茗テ" ECI 30 ECI 29 "齄膀赧" ECI 8 ECI 9 "α" ECI 810898 ECI 22 "ђ" ECI 4 Text ShiftByte
	// "Ź" ECI 811800
	EXPECT_EQ(decode({ 37, 927, 10, 901, 927, 20, 246, 877, 166, 106, 797, 927, 30, 927, 29, 415, 537, 357, 329, 194,
		927, 8, 927, 9, 225, 926, 899, 898, 927, 22, 0x90, 927, 4, 900, 913, 0xAC, 925, 1 }),
		L"\u70B9\u8317\u30C6\u9F44\u8180\u8D67\u03B1\u0452\u0179");
}

TEST(PDF417DecoderTest, ECIMultipleNumeric)
{
	// Numeric(5) ECI 16 ECI 20 Numeric(10) Text(ShiftJIS) "AB点"
	EXPECT_EQ(decode({ 25, 902, 171, 209, 269, 12, 434, 927, 20, 404, 629, 775, 441, 213, 222, 288, 513, 400, 123,
		900, 1, 913, 147, 913, 95 }),
		L"1234567890123456789012345678901234567890123AB\u70B9");

	// Numeric(6) ECI 16 Numeric(4) ECI 20 Numeric(5) Byte(ShiftJIS) "AB点" ECI 26 "Θ"
	EXPECT_EQ(decode({ 31, 902, 190, 232, 498, 813, 782, 767, 927, 16, 259, 248, 517, 378, 927, 20, 289, 700, 317, 21,
		112, 901, 'A', 'B', 147, 95, 927, 26, 0xCE, 901, 0x98 }),
		L"123456789012345678901234567890123456789012AB\u70B9\u0398");

	// Numeric(10) ECI 16 ECI 25 Numeric(5) Byte6(UnicodeBig) "AĀŁ" ECI 26 Byte "Θ"
	EXPECT_EQ(decode({ 32, 902, 289, 885, 405, 732, 212, 109, 679, 286, 885, 289, 927, 16, 927, 25, 289, 700, 317, 21,
		112, 924, 0, 382, 878, 524, 177, 927, 26, 901, 0xCE, 0x98 }),
		L"12345678901234567890123456789\u3930\u3132\u3334\u3536\u3738\u3930\u3132A\u0100\u0141\u0398");
}

TEST(PDF417DecoderTest, ECIInvalid)
{
	EXPECT_EQ(decode({ 4, 927, 901, 0 }), L""); // non-charset ECI > 899 -> empty text result
	EXPECT_EQ(Decode({4, 927, 901, 0}).content().bytes, ByteArray("AA")); // non-charset ECI > 899 -> ignored in binary result
	EXPECT_EQ(decode({ 3, 0, 927 }), L"AA"); // Malformed ECI at end silently ignored
}

TEST(PDF417DecoderTest, ECIMacroOptionalNumeric)
{
	// Check that ECI 25 (UnicodeBig) in numeric field (resulting in "\u3x3x" codepoints) still parses

	// File Size is "1234567890" ECI 25 "12345" ("\u3132\u3334\x35", the final odd byte gets dropped on UTF-16
	// conversion)
	std::vector<int> sampleCodes = { 19, 477, 928, 111, 100, 0, 252, 21, 86, 923, 5, 15, 369, 753, 190, 927, 25, 124,
		745 };

	DecoderResultExtra resultMetadata;
	DecodeMacroBlock(sampleCodes, 3, resultMetadata);

	EXPECT_EQ(0, resultMetadata.segmentIndex());
	EXPECT_EQ("000252021086", resultMetadata.fileId());
	EXPECT_EQ(false, resultMetadata.isLastSegment());

	EXPECT_EQ(1234567890, resultMetadata.fileSize());
	EXPECT_EQ(-1, resultMetadata.segmentCount());
}

TEST(PDF417DecoderTest, ECIGeneralPurpose)
{
	// 2-byte
	EXPECT_EQ(decode({ 5, 926, 10, 10, 0 }), L"AA"); // All General Purpose ECIs silently ignored
	EXPECT_TRUE(valid({ 4, 0, 926, 10 })); // Malformed ECI at end silently ignored
	EXPECT_TRUE(valid({ 3, 0, 926 })); // Malformed ECI at end silently ignored
}

TEST(PDF417DecoderTest, ECIUserDefined)
{
	// 1-byte
	EXPECT_EQ(decode({ 4, 925, 10, 0 }), L"AA"); // All User Defined ECIs silently ignored
	EXPECT_TRUE(valid({ 3, 0, 925 })); // Malformed ECI at end silently ignored
}

TEST(PDF417DecoderTest, ReaderInit)
{
	// Null
	EXPECT_FALSE(Decode({2, 0}).readerInit());
	EXPECT_EQ(decode({ 2, 0 }), L"AA");

	// Set
	EXPECT_TRUE(Decode({3, 921, 0}).readerInit());
	EXPECT_EQ(decode({ 3, 921, 0 }), L"AA");

	// Must be first
	EXPECT_FALSE(Decode({3, 0, 921}).readerInit());
	EXPECT_FALSE(valid({ 3, 0, 921 }));

	EXPECT_FALSE(Decode({4, 901, 65, 921}).readerInit());
	EXPECT_FALSE(valid({ 4, 901, 65, 921 }));

	EXPECT_FALSE(Decode({4, 901, 921, 65}).readerInit());
	EXPECT_FALSE(valid({ 4, 901, 921, 65 }));
}

TEST(PDF417DecoderTest, LinkageOther)
{
	EXPECT_FALSE(valid({ 3, 918, 0 })); // Not supported
	EXPECT_FALSE(valid({ 3, 0, 918 }));
}

TEST(PDF417DecoderTest, LinkageEANUCC)
{
	EXPECT_TRUE(valid({ 3, 920, 0 })); // Ignored if first codeword after length
	EXPECT_FALSE(valid({ 3, 0, 920 })); // But not elsewhere
}

TEST(PDF417DecoderTest, Reserved)
{
	EXPECT_FALSE(valid({ 3, 903, 0 })); // Not supported
	EXPECT_FALSE(valid({ 3, 0, 903 }));
}
