/*
 * Copyright 2017 Huy Cuong Nguyen
 * Copyright 2008 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "BitMatrix.h"
#include "BitMatrixIO.h"
#include "ByteArray.h"
#include "DecoderResult.h"
#include "qrcode/QRBitMatrixParser.h"
#include "qrcode/QRDataBlock.h"
#include "qrcode/QRDecoder.h"
#include "qrcode/QRFormatInformation.h"
#include "qrcode/QRVersion.h"

#include "gtest/gtest.h"

using namespace ZXing;
using namespace ZXing::QRCode;

TEST(QRBitMatrixParserTest, ECBlockCodewordCounts)
{
	const ECBlocks blocks{10, {ECB{2, 20}, ECB{1, 21}}};
	EXPECT_EQ(3, blocks.numBlocks());
	EXPECT_EQ(30, blocks.numECCodewords());
	EXPECT_EQ(61, blocks.numDataCodewords());
	EXPECT_EQ(91, blocks.numCodewords());
}

TEST(QRBitMatrixParserTest, Model1CodewordCounts)
{
	constexpr int capacities[] = {26, 46, 72, 100, 134, 170, 212, 256, 306, 358, 416, 476, 542, 610};
	FormatInformation format;
	format.mask = FORMAT_INFO_MASK_MODEL1;
	format.dataMask = 1;
	for (int number = 1; number <= 14; ++number) {
		const auto version = Version::Model1(number);
		const BitMatrix bitMatrix(17 + 4 * number);
		EXPECT_EQ(capacities[number - 1], version->totalCodewords()) << number;
		EXPECT_EQ(capacities[number - 1], ReadCodewords(bitMatrix, *version, format).size()) << number;
	}
}

TEST(QRBitMatrixParserTest, Model1LargeVersionsReachErrorCorrection)
{
	constexpr int formatPositions[][2] = {
		{0, 8}, {1, 8}, {2, 8}, {3, 8}, {4, 8}, {5, 8}, {7, 8}, {8, 8},
		{8, 7}, {8, 5}, {8, 4}, {8, 3}, {8, 2}, {8, 1}, {8, 0},
	};
	for (int number = 12; number <= 14; ++number) {
		const int dimension = 17 + 4 * number;
		BitMatrix bitMatrix(dimension);
		for (int index = 0; index < 15; ++index)
			bitMatrix.set(formatPositions[index][0], formatPositions[index][1], (0x0EC4u >> (14 - index)) & 1);
		for (int index = 0; index < 8; ++index) {
			bitMatrix.set(8, dimension - 1 - index, (0x0101u >> (15 - index)) & 1);
			bitMatrix.set(dimension - 8 + index, 8, (0x0101u >> (7 - index)) & 1);
		}
		EXPECT_EQ(Error::Checksum, Decode(bitMatrix).error()) << number;
	}
}

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
