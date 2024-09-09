/*
 * Copyright 2017 Huy Cuong Nguyen
 * Copyright 2008 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "qrcode/QRDecoder.h"

#include "BitMatrix.h"
#include "BitMatrixIO.h"
#include "DecoderResult.h"

#include "gtest/gtest.h"

using namespace ZXing;
using namespace ZXing::QRCode;

TEST(MQRDecoderTest, MQRCodeM3L)
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

	const auto result = Decode(bitMatrix);
	EXPECT_TRUE(result.isValid());
}

TEST(MQRDecoderTest, MQRCodeM3M)
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

	const auto result = Decode(bitMatrix);
	EXPECT_TRUE(result.isValid());
}

TEST(MQRDecoderTest, MQRCodeM1)
{
	const auto bitMatrix = ParseBitMatrix("XXXXXXX X X\n"
										  "X     X    \n"
										  "X XXX X XXX\n"
										  "X XXX X  XX\n"
										  "X XXX X   X\n"
										  "X     X XX \n"
										  "XXXXXXX X  \n"
										  "        X  \n"
										  "XX     X   \n"
										  " X  XXXXX X\n"
										  "X  XXXXXX X\n",
										  88, false);
	const auto result = Decode(bitMatrix);
	EXPECT_TRUE(result.isValid());
	EXPECT_EQ(L"123", result.text());
}

TEST(MQRDecoderTest, MQRCodeM1Error4Bits)
{
	const auto bitMatrix = ParseBitMatrix("XXXXXXX X X\n"
										  "X     X  XX\n"
										  "X XXX X X  \n"
										  "X XXX X  XX\n"
										  "X XXX X   X\n"
										  "X     X XX \n"
										  "XXXXXXX X  \n"
										  "        X  \n"
										  "XX     X   \n"
										  " X  XXXXXX \n"
										  "X  XXXXXXX \n",
										  88, false);
	const auto result = Decode(bitMatrix);
	EXPECT_EQ(Error::Checksum, result.error());
	EXPECT_EQ(result.text(), L"6350");
}

TEST(MQRDecoderTest, MQRCodeM4)
{
	const auto bitMatrix = ParseBitMatrix("XXXXXXX X X X X X\n"
										  "X     X XX X   XX\n"
										  "X XXX X  X  X  XX\n"
										  "X XXX X XX  XX XX\n"
										  "X XXX X  X  XXXXX\n"
										  "X     X XX      X\n"
										  "XXXXXXX XX  X  XX\n"
										  "         X  XX XX\n"
										  "X  X XXX    X XXX\n"
										  " XX  X  XX XX  X \n"
										  "XX  XXXX X XX  XX\n"
										  "    XX XX X XX XX\n"
										  "XXX XXX XXX XX XX\n"
										  "  X X   X   XX  X\n"
										  "X X XX   XXXXX   \n"
										  "  X X X X   X    \n"
										  "X   XXXXXXX X X X\n",
										  88, false);
	const auto result = Decode(bitMatrix);
	EXPECT_TRUE(result.isValid());
}
