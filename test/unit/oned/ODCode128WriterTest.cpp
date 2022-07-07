/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2014 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "oned/ODCode128Writer.h"
#include "BitMatrixIO.h"
#include "DecodeHints.h"
#include "Result.h"
#include "oned/ODCode128Reader.h"

#include "gtest/gtest.h"

using namespace ZXing;
using namespace ZXing::OneD;

static const std::string FNC1 = "11110101110";
static const std::string FNC2 = "11110101000";
static const std::string FNC3 = "10111100010";
static const std::string FNC4A = "11101011110";
static const std::string FNC4B = "10111101110";
static const std::string START_CODE_A = "11010000100";
static const std::string START_CODE_B = "11010010000";
static const std::string START_CODE_C = "11010011100";
static const std::string SWITCH_CODE_A = "11101011110";
static const std::string SWITCH_CODE_B = "10111101110";
static const std::string QUIET_SPACE = "00000";
static const std::string STOP = "1100011101011";
static const std::string LF = "10000110010";

static std::string LineMatrixToString(const BitMatrix& matrix)
{
	auto result = ToString(matrix, '1', '0', false);
	return result.substr(0, result.size() - 1);
}

static ZXing::Result Decode(const BitMatrix &matrix)
{
	BitArray row;
	matrix.getRow(0, row);
	DecodeHints hints;
	return Code128Reader(hints).decodeSingleRow(0, row);
}

TEST(ODCode128Writer, EncodeWithFunc1)
{
	auto toEncode = L"\xf1""123";
	//                                                       "12"                           "3"          check digit 92
	auto expected = QUIET_SPACE + START_CODE_C + FNC1 + "10110011100" + SWITCH_CODE_B + "11001011100" + "10101111000" + STOP + QUIET_SPACE;

	auto actual = LineMatrixToString(Code128Writer().encode(toEncode, 0, 0));
	EXPECT_EQ(actual, expected);
}

TEST(ODCode128Writer, EncodeWithFunc2)
{
	auto toEncode = L"\xf2""123";
	//                                                       "1"            "2"             "3"          check digit 56
	auto expected = QUIET_SPACE + START_CODE_B + FNC2 + "10011100110" + "11001110010" + "11001011100" + "11100010110" + STOP + QUIET_SPACE;

	auto actual = LineMatrixToString(Code128Writer().encode(toEncode, 0, 0));
	EXPECT_EQ(actual, expected);
}

TEST(ODCode128Writer, EncodeWithFunc3)
{
	auto toEncode = L"\xf3""123";
	//                                                       "1"            "2"             "3"          check digit 51
	auto expected = QUIET_SPACE + START_CODE_B + FNC3 + "10011100110" + "11001110010" + "11001011100" + "11101000110" + STOP + QUIET_SPACE;

	auto actual = LineMatrixToString(Code128Writer().encode(toEncode, 0, 0));
	EXPECT_EQ(actual, expected);
}

TEST(ODCode128Writer, EncodeWithFunc4)
{
	auto toEncode = L"\xf4""123";
	//                                                       "1"            "2"             "3"          check digit 59
	auto expected = QUIET_SPACE + START_CODE_B + FNC4B + "10011100110" + "11001110010" + "11001011100" + "11100011010" + STOP + QUIET_SPACE;

	auto actual = LineMatrixToString(Code128Writer().encode(toEncode, 0, 0));
	EXPECT_EQ(actual, expected);
}

TEST(ODCode128Writer, EncodeWithFncsAndNumberInCodesetA)
{
	auto toEncode = L"\n" "\xf1" "\xf4" "1" "\n";
	auto expected = QUIET_SPACE + START_CODE_A + LF + FNC1 + FNC4A + "10011100110" + LF + "10101111000" + STOP + QUIET_SPACE;
	auto actual = LineMatrixToString(Code128Writer().encode(toEncode, 0, 0));
	EXPECT_EQ(actual, expected);
}

TEST(ODCode128Writer, RoundtripGS1)
{
	auto toEncode = L"\xf1" "10958" "\xf1" "17160526";
	auto expected = "10958\u001D17160526";

	auto encResult = Code128Writer().encode(toEncode, 0, 0);
	auto decResult = Decode(encResult);
	auto actual = decResult.text();
	EXPECT_EQ(actual, expected);
	EXPECT_EQ(decResult.symbologyIdentifier(), "]C1");
}

TEST(ODCode128Writer, RoundtripFNC1)
{
	auto toEncode = L"1\xf1" "0958" "\xf1" "17160526";
	auto expected = "1\u001D0958\u001D17160526";

	auto encResult = Code128Writer().encode(toEncode, 0, 0);
	auto decResult = Decode(encResult);
	auto actual = decResult.text();
	EXPECT_EQ(actual, expected);
	EXPECT_EQ(decResult.symbologyIdentifier(), "]C0");
}

TEST(ODCode128Writer, EncodeSwitchCodesetFromAToB)
{
	// start with A switch to B and back to A
	auto toEncode = std::string("\0ABab\u0010", 6);
	//                                           "\0"            "A"             "B"             Switch to B     "a"             "b"             Switch to A     "\u0010"        check digit
	auto expected = QUIET_SPACE + START_CODE_A + "10100001100" + "10100011000" + "10001011000" + SWITCH_CODE_B + "10010110000" + "10010000110" + SWITCH_CODE_A + "10100111100" + "11001110100" + STOP + QUIET_SPACE;

	auto encoded = Code128Writer().encode(toEncode, 0, 0);
	auto actual = LineMatrixToString(encoded);
	EXPECT_EQ(actual, expected);

	auto actualRoundTrip = Decode(encoded).text();
	EXPECT_EQ(actualRoundTrip, toEncode);
}

TEST(ODCode128Writer, EncodeSwitchCodesetFromBToA)
{
	// start with B switch to A and back to B
	auto toEncode = std::string("ab\0ab", 5);
	//                                           "a"             "b"             Switch to A     "\0             "Switch to B"   "a"             "b"             check digit
	auto expected = QUIET_SPACE + START_CODE_B + "10010110000" + "10010000110" + SWITCH_CODE_A + "10100001100" + SWITCH_CODE_B + "10010110000" + "10010000110" + "11010001110" + STOP + QUIET_SPACE;

	auto encoded = Code128Writer().encode(toEncode, 0, 0);
	auto actual = LineMatrixToString(encoded);
	EXPECT_EQ(actual, expected);

	auto actualRoundTrip = Decode(encoded).text();
	EXPECT_EQ(actualRoundTrip, toEncode);
}
