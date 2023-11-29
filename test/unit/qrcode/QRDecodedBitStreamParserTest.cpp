/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2008 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "BitArray.h"
#include "ByteArray.h"
#include "DecoderResult.h"
#include "qrcode/QRErrorCorrectionLevel.h"
#include "qrcode/QRVersion.h"

#include "gtest/gtest.h"

namespace ZXing {
	namespace QRCode {
		DecoderResult DecodeBitStream(ByteArray&& bytes, const Version& version, ErrorCorrectionLevel ecLevel);
	}
}

using namespace ZXing;
using namespace ZXing::QRCode;

TEST(QRDecodedBitStreamParserTest, SimpleByteMode)
{
	BitArray ba;
	ba.appendBits(0x04, 4); // Byte mode
	ba.appendBits(0x03, 8); // 3 bytes
	ba.appendBits(0xF1, 8);
	ba.appendBits(0xF2, 8);
	ba.appendBits(0xF3, 8);
	auto result = DecodeBitStream(ba.toBytes(), *Version::Model2(1), ErrorCorrectionLevel::Medium).text();
	EXPECT_EQ(L"\xF1\xF2\xF3", result);
}

TEST(QRDecodedBitStreamParserTest, SimpleSJIS)
{
	BitArray ba;
	ba.appendBits(0x04, 4); // Byte mode
	ba.appendBits(0x04, 8); // 4 bytes
	ba.appendBits(0xA1, 8);
	ba.appendBits(0xA2, 8);
	ba.appendBits(0xA3, 8);
	ba.appendBits(0xD0, 8);
	auto result = DecodeBitStream(ba.toBytes(), *Version::Model2(1), ErrorCorrectionLevel::Medium).text();
	EXPECT_EQ(L"\uff61\uff62\uff63\uff90", result);
}

TEST(QRDecodedBitStreamParserTest, ECI)
{
	BitArray ba;
	ba.appendBits(0x07, 4); // ECI mode
	ba.appendBits(0x02, 8); // ECI 2 = CP437 encoding
	ba.appendBits(0x04, 4); // Byte mode
	ba.appendBits(0x03, 8); // 3 bytes
	ba.appendBits(0xA1, 8);
	ba.appendBits(0xA2, 8);
	ba.appendBits(0xA3, 8);
	auto result = DecodeBitStream(ba.toBytes(), *Version::Model2(1), ErrorCorrectionLevel::Medium).text();
	EXPECT_EQ(L"\xED\xF3\xFA", result);
}

TEST(QRDecodedBitStreamParserTest, Hanzi)
{
	BitArray ba;
	ba.appendBits(0x0D, 4); // Hanzi mode
	ba.appendBits(0x01, 4); // Subset 1 = GB2312 encoding
	ba.appendBits(0x01, 8); // 1 characters
	ba.appendBits(0x03C1, 13);
	auto result = DecodeBitStream(ba.toBytes(), *Version::Model2(1), ErrorCorrectionLevel::Medium).text();
	EXPECT_EQ(L"\u963f", result);
}

TEST(QRDecodedBitStreamParserTest, HanziLevel1)
{
	BitArray ba;
	ba.appendBits(0x0D, 4); // Hanzi mode
	ba.appendBits(0x01, 4); // Subset 1 = GB2312 encoding
	ba.appendBits(0x01, 8); // 1 characters
	// A5A2 (U+30A2) => A5A2 - A1A1 = 401, 4*60 + 01 = 0181
	ba.appendBits(0x0181, 13);

	auto result = DecodeBitStream(ba.toBytes(), *Version::Model2(1), ErrorCorrectionLevel::Medium).text();
	EXPECT_EQ(L"\u30a2", result);
}

TEST(QRDecodedBitStreamParserTest, SymbologyIdentifier)
{
	const Version& version = *Version::Model2(1);
	const ErrorCorrectionLevel ecLevel = ErrorCorrectionLevel::Medium;
	DecoderResult result;

	// Plain "ANUM(1) A"
	result = DecodeBitStream({0x20, 0x09, 0x40}, version, ecLevel);
	EXPECT_EQ(result.symbologyIdentifier(), "]Q1");
	EXPECT_EQ(result.text(), L"A");

	// GS1 "FNC1(1st) NUM(4) 2001"
	result = DecodeBitStream({0x51, 0x01, 0x0C, 0x81, 0x00}, version, ecLevel);
	EXPECT_EQ(result.symbologyIdentifier(), "]Q3");
	EXPECT_EQ(result.text(), L"2001"); // "(20)01"

	// GS1 "NUM(4) 2001 FNC1(1st) 301" - FNC1(1st) can occur anywhere (this actually violates the specification)
	result = DecodeBitStream({0x10, 0x10, 0xC8, 0x15, 0x10, 0x0D, 0x2D, 0x00}, version, ecLevel);
	EXPECT_EQ(result.symbologyIdentifier(), "]Q3");
	EXPECT_EQ(result.text(), L"2001301"); // "(20)01(30)1"

	// AIM "FNC1(2nd) 99 (0x63) ANUM(1) A"
	result = DecodeBitStream({0x96, 0x32, 0x00, 0x94, 0x00}, version, ecLevel);
	EXPECT_EQ(result.symbologyIdentifier(), "]Q5");
	EXPECT_EQ(result.text(), L"99A");

	// AIM "BYTE(1) A FNC1(2nd) 99 (0x63) BYTE(1) B" - FNC1(2nd) can occur anywhere
	// Disabled this test, since this violates the specification and the code does support it anymore
//	result = DecodeBitStream({0x40, 0x14, 0x19, 0x63, 0x40, 0x14, 0x20, 0x00}, version, ecLevel, "");
//	EXPECT_EQ(result.symbologyIdentifier(), "]Q5");
//	EXPECT_EQ(result.text(), L"99AB"); // Application Indicator prefixed to data

	// AIM "FNC1(2nd) A (100 + 61 = 0xA5) ANUM(1) B"
	result = DecodeBitStream({0x9A, 0x52, 0x00, 0x96, 0x00}, version, ecLevel);
	EXPECT_EQ(result.symbologyIdentifier(), "]Q5");
	EXPECT_EQ(result.text(), L"AB");

	// AIM "FNC1(2nd) a (100 + 97 = 0xC5) ANUM(1) B"
	result = DecodeBitStream({0x9C, 0x52, 0x00, 0x96, 0x00}, version, ecLevel);
	EXPECT_EQ(result.symbologyIdentifier(), "]Q5");
	EXPECT_EQ(result.text(), L"aB");

	// Bad AIM Application Indicator "FNC1(2nd) @ (0xA4) ANUM(1) B"
	result = DecodeBitStream({0x9A, 0x42, 0x00, 0x96, 0x00}, version, ecLevel);
	EXPECT_FALSE(result.isValid());
}

TEST(QRDecodedBitStreamParserTest, GS1PercentGS)
{
	const Version& version = *Version::Model2(1);
	const ErrorCorrectionLevel ecLevel = ErrorCorrectionLevel::Quality;
	DecoderResult result;

	// GS1 "FNC1(1st) A(11) 9112%%%2012 (9112%<FNC1>2012)"
	result = DecodeBitStream({0x52, 0x05, 0x99, 0x60, 0x5F, 0xB5, 0x35, 0x80, 0x01, 0x08, 0x00, 0xEC, 0x11}, version, ecLevel);
	EXPECT_EQ(result.content().text(TextMode::Plain), "9112%\x1D" "2012");
	EXPECT_EQ(result.content().text(TextMode::HRI), "(91)12%(20)12");
}
