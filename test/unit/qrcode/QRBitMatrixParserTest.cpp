/*
 * Copyright 2017 Huy Cuong Nguyen
 * Copyright 2008 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "BitMatrix.h"
#include "BitMatrixIO.h"
#include "ByteArray.h"
#include "qrcode/QRBitMatrixParser.h"
#include "qrcode/QRFormatInformation.h"
#include "qrcode/QRVersion.h"

#include "gtest/gtest.h"

using namespace ZXing;
using namespace ZXing::QRCode;

TEST(QRBitMatrixParserTest, MQRCodeM3L)
{
	const auto bitMatrix = ParseBitMatrix("XXXXXXX X X X X\n"
										  "X     X    X X \n"
										  "X XXX X XXXXXXX\n"
										  "X XXX X X X  XX\n"
										  "X XXX X    X XX\n"
										  "X     X X X X X\n"
										  "XXXXXXX  X  XX \n"
										  "         X X  X\n"
										  "XXXXXX    X X X\n"
										  "   X  XX    XXX\n"
										  "XXX XX XXXX XXX\n"
										  " X    X  XXX X \n"
										  "X XXXXX XXX X X\n"
										  " X    X  X XXX \n"
										  "XXX XX X X XXXX\n",
										  88, false);

	const auto format = ReadFormatInformation(bitMatrix);
	const auto version = ReadVersion(bitMatrix, format.type());
	EXPECT_EQ(3, version->versionNumber());
	const auto codewords = ReadCodewords(bitMatrix, *version, format);
	EXPECT_EQ(17, codewords.size());
	EXPECT_EQ(0x0, codewords[10]);
	EXPECT_EQ(0xd1, codewords[11]);
}

TEST(QRBitMatrixParserTest, MQRCodeM3M)
{
	const auto bitMatrix = ParseBitMatrix("XXXXXXX X X X X\n"
										  "X     X      XX\n"
										  "X XXX X X XX XX\n"
										  "X XXX X X X    \n"
										  "X XXX X XX XXXX\n"
										  "X     X XX     \n"
										  "XXXXXXX  X XXXX\n"
										  "        X  XXX \n"
										  "X    XX XX X  X\n"
										  "   X X     XX  \n"
										  "XX  XX  XXXXXXX\n"
										  " X    X       X\n"
										  "XX X X      X  \n"
										  "   X X    X    \n"
										  "X X XXXX    XXX\n",
										  88, false);

	const auto format = ReadFormatInformation(bitMatrix);
	const auto version = ReadVersion(bitMatrix, format.type());
	EXPECT_EQ(3, version->versionNumber());
	const auto codewords = ReadCodewords(bitMatrix, *version, format);
	EXPECT_EQ(17, codewords.size());
	EXPECT_EQ(0x0, codewords[8]);
	EXPECT_EQ(0x89, codewords[9]);
}
