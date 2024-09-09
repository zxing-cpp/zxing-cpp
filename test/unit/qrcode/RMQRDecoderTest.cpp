/*
 * Copyright 2023 gitlost
*/
// SPDX-License-Identifier: Apache-2.0

#include "qrcode/QRDecoder.h"

#include "BitMatrix.h"
#include "BitMatrixIO.h"
#include "DecoderResult.h"
#include "ECI.h"

#include "gtest/gtest.h"

using namespace ZXing;
using namespace ZXing::QRCode;

TEST(RMQRDecoderTest, RMQRCodeR7x43M)
{
	const auto bitMatrix = ParseBitMatrix(
		"XXXXXXX X X X X X X XXX X X X X X X X X XXX\n"
		"X     X  X XXX  XXXXX XXX      X X XX   X X\n"
		"X XXX X X XXX X X X XXXX XXXX X  X XXXXXXXX\n"
		"X XXX X  XX    XXXXX   XXXXXX   X X   X   X\n"
		"X XXX X   XX  XXX   XXXXXXX  X X  XX  X X X\n"
		"X     X XXXXX XXX XXX XXXXX    XXXXXX X   X\n"
		"XXXXXXX X X X X X X XXX X X X X X X X XXXXX\n",
		'X', false);

	const auto result = Decode(bitMatrix);
	EXPECT_TRUE(result.isValid());
	EXPECT_EQ(result.content().text(TextMode::Plain), "ABCDEFG");
}

TEST(RMQRDecoderTest, RMQRCodeR7x43MError6Bits)
{
	const auto bitMatrix = ParseBitMatrix(
		"XXXXXXX X X X X X X XXX X X X X X X X X XXX\n"
		"X     X  X XXX  XXXXX XXX      X X XX   X X\n"
		"X XXX X X XXX   X X XXXX XXXX XX X XXXXXXXX\n" // 2
		"X XXX X  XX    XXXXX X XXXXXX   X X   X   X\n" // 3
		"X XXX X   XX  XXX   XXXXXXX  X X XXX  X X X\n" // 5
		"X     X XXXXX XXX XXX XXXX X   XXXXXX X   X\n" // 6
		"XXXXXXX X X X X X X XXX X X X X X X X XXXXX\n",
		'X', false);

	const auto result = Decode(bitMatrix);
	EXPECT_EQ(Error::Checksum, result.error());
	EXPECT_EQ(result.text(), L"LSZ2EFJ");
	EXPECT_EQ(result.content().text(TextMode::Plain), "LSZ2EFJ");
}

TEST(RMQRDecoderTest, RMQRCodeR7x139H)
{
	const auto bitMatrix = ParseBitMatrix(
		"XXXXXXX X X X X X X X X X XXX X X X X X X X X X X X X XXX X X X X X X X X X X X X XXX X X X X X X X X X X X X XXX X X X X X X X X X X X XXX\n"
		"X     X XX XXX X X   X  X X XX XX  X   X X XXX XX  XXXX XXX XX  XX XX  X     XX X X X XXX  X   XX   XX   XX X X XX  X XX XXXX  X    X     X\n"
		"X XXX X    X  XXXXX   X  XXXXX        X X XXX XX    X XXX X XX XXX XX X XXX  X X XXXX   X   XXXXXXX X XX      XXX   X     X  X  XXX X XXXXX\n"
		"X XXX X  XXXX   X   XX X X    XX  XX  X XX  XX    X XXX XX X XX  X XX  X X   XX  X  X XXX  X  X      X X X X  X XX X   XX   XX   X    X   X\n"
		"X XXX X XXXX XXXXX X  X XXXXXX XX X XXXX  X    XXXX X XXX  XXXX  X XXXXXXX   XXX XXXXXX X  X XX  X     XXX  X XXXXXXXXX X XXXX  X   X X X X\n"
		"X     X X   XX  XX X  X  XX X X X XXXX X X   X XX X XXX X  X  X X X  XXX   XX   XXX X  X XX XXXX  XX X X  X   X XXXXX  XXX XX      X XX   X\n"
		"XXXXXXX X X X X X X X X X XXX X X X X X X X X X X X X XXX X X X X X X X X X X X X XXX X X X X X X X X X X X X XXX X X X X X X X X X X XXXXX\n",
		'X', false);

	const auto result = Decode(bitMatrix);
	EXPECT_TRUE(result.isValid());
	EXPECT_EQ(result.content().text(TextMode::Plain), "1234567890,ABCDEFGHIJKLMOPQRSTUVW");
}

TEST(RMQRDecoderTest, RMQRCodeR9x59H)
{
	const auto bitMatrix = ParseBitMatrix(
		"XXXXXXX X X X X X XXX X X X X X X X X XXX X X X X X X X XXX\n"
		"X     X    X  XXXXX XXX X  X XXXXXXXX X X  X    X XXXX  X X\n"
		"X XXX X XX XXX  X XXX XXXX  X         XXXXXXX  X XXXXX X  X\n"
		"X XXX X XXXX X XX X   XX   XXXX XX  XX   X  X  X XXX     X \n"
		"X XXX X    X    X XX XXXXXX X X XX   X XX   X X XXXX  XXXXX\n"
		"X     X X  X  X  X  XXX X X   X   XX  X XXXX XX  X X  X   X\n"
		"XXXXXXX  XXXXX  XXXXXX X XX XXX X    XXXX  X    X  X XX X X\n"
		"          XXX  XXXX XX XXX    X XXXXXXX X XX XXX  XX XX   X\n"
		"XXX X X X X X X X XXX X X X X X X X X XXX X X X X X X XXXXX\n",
		'X', false);

	const auto result = Decode(bitMatrix);
	EXPECT_TRUE(result.isValid());
	EXPECT_EQ(result.content().text(TextMode::Plain), "ABCDEFGHIJKLMN");
}

TEST(RMQRDecoderTest, RMQRCodeR9x77M)
{
	const auto bitMatrix = ParseBitMatrix(
		"XXXXXXX X X X X X X X X XXX X X X X X X X X X X X XXX X X X X X X X X X X XXX\n"
		"X     X  XXX XX XXX   XXX XXXX XXX XX X XXXXXXXXX X XXX  XXXX X XXXX XX XXX X\n"
		"X XXX X X  X X  XXX  X XXXX  XX  XX  X XX XX      XXX XXXX X X XX   X  X XX X\n"
		"X XXX X X   X XXXXXX  X   XX XXXX X  XXX X XX X  XX  XX XX X XXX X X XXX  XX \n"
		"X XXX X     XXXX  X X   XXXX XXXX XX     XXX X XX XXXXXX X X     XXX XX XXXXX\n"
		"X     X  X X XX XXX    X  X  XX   X X    XX XXX X X   X  X  X    XX XXXXX   X\n"
		"XXXXXXX    X XX   XX X  XXXX X  X X     X  X  XX  XXX  X XX     X  XXX XX X X\n"
		"         X XXXXX       XX X XXXXXX XX   XXXXX     X XX     XX   XXXXX XXX   X\n"
		"XXX X X X X X X X X X X XXX X X X X X X X X X X X XXX X X X X X X X X X XXXXX\n",
		'X', false);

	const auto result = Decode(bitMatrix);
	EXPECT_TRUE(result.isValid());
	EXPECT_EQ(result.content().text(TextMode::Plain), "__ABCDEFGH__1234567890___ABCDEFGHIJK");
}

TEST(RMQRDecoderTest, RMQRCodeR11x27H)
{
	const auto bitMatrix = ParseBitMatrix(
		"XXXXXXX X X X X X X X X XXX\n"
		"X     X  XX        X  X X X\n"
		"X XXX X    X  XX X   X   XX\n"
		"X XXX X XXXX XX X  XXXXXX  \n"
		"X XXX X  X X XX  XX   XXX X\n"
		"X     X XXX  X XX  XXXX  X \n"
		"XXXXXXX     X   XX  X XXXXX\n"
		"           X   X   X  X   X\n"
		"XXXX  X   X X XX XXXXXX X X\n"
		"X XX XXXXXX XXX  XXXX X   X\n"
		"XXX X X X X X X X X X XXXXX\n",
		'X', false);

	const auto result = Decode(bitMatrix);
	EXPECT_TRUE(result.isValid());
	EXPECT_EQ(result.content().text(TextMode::Plain), "ABCDEF");
}

TEST(RMQRDecoderTest, RMQRCodeR13x27M_ECI)
{
	const auto bitMatrix = ParseBitMatrix(
		"XXXXXXX X X X X X X X X XXX\n"
		"X     X    XX XX XXX   XX X\n"
		"X XXX X XX  X  XX XX XXX  X\n"
		"X XXX X  XX X XX X X   XX  \n"
		"X XXX X XXXXXXX X X      XX\n"
		"X     X   XX X  XXX  XX XX \n"
		"XXXXXXX   X   X X    X  XXX\n"
		"        XXX XX X  XX   XXX \n"
		"XXX XX XX X  X XX XX  XXXXX\n"
		" XXX  X    X X    X   X   X\n"
		"X XX X  X   XX X XX X X X X\n"
		"X   X   X  X X X X    X   X\n"
		"XXX X X X X X X X X X XXXXX\n",
		'X', false);

	const auto result = Decode(bitMatrix);
	EXPECT_TRUE(result.isValid());
	EXPECT_EQ(result.content().text(TextMode::Plain), "AB貫12345AB");
	EXPECT_TRUE(result.content().hasECI);
	EXPECT_EQ(result.content().encodings[0].eci, ECI::Shift_JIS);
	EXPECT_EQ(result.content().symbology.toString(), "]Q1");
}

TEST(RMQRDecoderTest, RMQRCodeR15x59H_GS1)
{
	const auto bitMatrix = ParseBitMatrix(
		"XXXXXXX X X X X X XXX X X X X X X X X XXX X X X X X X X XXX\n"
		"X     X   XXX XXX X XXXXX      XX XXX X X   X X X X   XXX X\n"
		"X XXX X XXX XX X  XXX XXX X  X   XXX XXXXX  XX      XXX  XX\n"
		"X XXX X X     X XX  X X     XXX X  X    X  XXXXX XX XXX    \n"
		"X XXX X XX   XXX  XX   X X X    XX  XX XX XXX XXXX X   XXXX\n"
		"X     X X  X X X     X  XXX XXX  XXXX X XXX XX    X  X     \n"
		"XXXXXXX  X  XXX  XXXX X    XX XXXX X   X XX   XXX XXXXX   X\n"
		"        X XXX     X    XXXXX     X   XX        XXXX   XX X \n"
		"XX  XX X X   X XXXXX   XX X X XX    XX X   XX X X     XX  X\n"
		" XX XX X   XXXXXX    XXX       XX  X X   XX  XXX   X X XXX \n"
		"X X    XX   XXXXXXXXXX XX X  X   XX XX XX X  XXXX XX XXXXXX\n"
		"  XX X XX X XXX   X  X X    XXX X XXX   X X  XXX   XXXX   X\n"
		"XXXX   X  X XX    XXX X  X X    XX  XXXXX XX  X  XX XXX X X\n"
		"X  X   X  XX    XXX XXXXXXX XXX  X  XXX XX  X   X  X XX   X\n"
		"XXX X X X X X X X XXX X X X X X X X X XXX X X X X X X XXXXX\n",
		'X', false);

	const auto result = Decode(bitMatrix);
	EXPECT_TRUE(result.isValid());
	EXPECT_TRUE(result.content().type() == ContentType::GS1);
	EXPECT_EQ(result.content().text(TextMode::HRI), "(01)09524000059109(21)12345678p901(10)1234567p(17)231120");
}

TEST(RMQRDecoderTest, RMQRCodeR17x99H)
{
	const auto bitMatrix = ParseBitMatrix(
		"XXXXXXX X X X X X X X XXX X X X X X X X X X X X XXX X X X X X X X X X X X XXX X X X X X X X X X XXX\n"
		"X     X   X XXXXX XXX X X  X XX X X  XX  XXXXX  X XX   X XX XXX X X XX X  X XXX     X X XX   X  X X\n"
		"X XXX X X X   XXX     XXX X XXX XXX     X  X XX XXXX X  X X  X      XXX   XXXXX X    X XX X XX  X X\n"
		"X XXX X   XX X  XX X    X   XX   X  XXXX X  XXXXX  X  X    XX X XXX XX X X       X  X   XXXXXX X   \n"
		"X XXX X    X XX  X X X X X  X   X X  XXX    XX XXXXXX X    X   XXX  X XXXXXX X   X X X X X X  XX  X\n"
		"X     X XX  X   X XXXXX  XX   X XXX  X XX   X X    XXX X  XXX  XXX X  XXXX  XX     X X X XX   XXXX \n"
		"XXXXXXX X XX X      XX X X  XXX XX  X XXXX    X  X  XXX X X XX X XXXX  XX  X   X        X XX X XXXX\n"
		"        XX XX XX XX  X  XX  X    X  X XXX XX    X     X  XXX     XXXX  XX X X  X      X XX XX  XXX \n"
		"XX       X XXX  X   X XXXX XXX XXXXX  XXX  XXX   X X X  X   X  XXX X  XX  XX X   X X  X  XX  X  XXX\n"
		" X   XXXXX X  X   XXXXX X  XX       X XX XXXX   X     X XXXXX X XX X  XX  X XX   X XX           XX \n"
		"X XX XX   X  XX   XXX  XX XXXXXX X  XXXXX  XX    XXXX  X X X   X XXXX  XX  X   X  XXXXXX    XX  X X\n"
		" XXX XX  XXX  XX  XX X   X X XX  X X X X XX   XXX XXXX      X XX  XXX X X X XXXX    XXXXX  X XXX   \n"
		"X  X  XX    X      XX XX  XX X X XX  X    X X XX XXXXXXXX X XX XX  X   X   X X X XX X X XXXXXXXXXXX\n"
		"    X X    X XX    X X   X XX XXXX    X XXX  X XX X X X   X X  XXX XXXXX    XX X X  X XXXXX X X   X\n"
		"XXXX XX XX   X  XXXX XXXX  X XX    X  XX  XX XX XXXX XXX X      X XX XX X XXXX   X XXX  XX X XX X X\n"
		"X XXX XX  XXX X X X XXX X  XXX   X XXXX  XX     X X  XXXXX X XX X  X X X  X X X X XXXX     XXXX   X\n"
		"XXX X X X X X X X X X XXX X X X X X X X X X X X XXX X X X X X X X X X X X XXX X X X X X X X X XXXXX\n",
		'X', false);

	const auto result = Decode(bitMatrix);
	EXPECT_TRUE(result.isValid());
	EXPECT_EQ(result.content().text(TextMode::Plain), "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890________________________");
}
