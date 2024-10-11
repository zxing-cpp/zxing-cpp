/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2013 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "aztec/AZEncoder.h"
#include "BitArray.h"
#include "BitArrayUtility.h"
#include "BitMatrixIO.h"

#include "gtest/gtest.h"
#include <algorithm>
#include <stdexcept>

namespace ZXing {
	namespace Aztec {
		void GenerateModeMessage(bool compact, int layers, int messageSizeInWords, BitArray& modeMessage);
		void StuffBits(const BitArray& bits, int wordSize, BitArray& out);
	}
}

using namespace ZXing;

namespace {
	
	void TestEncode(const std::string& data, bool compact, int layers, const BitMatrix& expected) {
		Aztec::EncodeResult aztec = Aztec::Encoder::Encode(data, 33, Aztec::Encoder::DEFAULT_AZTEC_LAYERS);
		EXPECT_EQ(aztec.compact, compact) << "Unexpected symbol format (compact)";
		EXPECT_EQ(aztec.layers, layers) << "Unexpected nr. of layers";
		EXPECT_EQ(aztec.matrix, expected) << "encode() failed";
	}

	std::string StripSpaces(std::string str) {
#ifdef __cpp_lib_erase_if
		std::erase_if(str, isspace);
#else
		str.erase(std::remove_if(str.begin(), str.end(), isspace), str.end());
#endif
		return str;
	}

	void TestModeMessage(bool compact, int layers, int words, const std::string& expected) {
		BitArray bits;
		Aztec::GenerateModeMessage(compact, layers, words, bits);
		auto expectedBits = Utility::ParseBitArray(StripSpaces(expected));
		EXPECT_EQ(bits, expectedBits) << "generateModeMessage() failed";
	}

	void TestStuffBits(int wordSize, const std::string& bits, const std::string& expected) {
		BitArray in = Utility::ParseBitArray(StripSpaces(bits));
		BitArray expectedBits = Utility::ParseBitArray(StripSpaces(expected));
		BitArray stuffed;
		Aztec::StuffBits(in, wordSize, stuffed);
		EXPECT_EQ(stuffed, expectedBits) << "stuffBits() failed for input string: " + bits;
	}

}

TEST(AZEncoderTest, GenerateModeMessage)
{
	TestModeMessage(true, 2, 29, ".X .XXX.. ...X XX.. ..X .XX. .XX.X");
	TestModeMessage(true, 4, 64, "XX XXXXXX .X.. ...X ..XX .X.. XX..");
	TestModeMessage(false, 21, 660, "X.X.. .X.X..X..XX .XXX ..X.. .XXX. .X... ..XXX");
	TestModeMessage(false, 32, 4096, "XXXXX XXXXXXXXXXX X.X. ..... XXX.X ..X.. X.XXX");
}

TEST(AZEncoderTest, StuffBits)
{
	TestStuffBits(5, ".X.X. X.X.X .X.X.",
		".X.X. X.X.X .X.X.");
	TestStuffBits(5, ".X.X. ..... .X.X",
		".X.X. ....X ..X.X");
	TestStuffBits(3, "XX. ... ... ..X XXX .X. ..",
		"XX. ..X ..X ..X ..X .XX XX. .X. ..X");
	TestStuffBits(6, ".X.X.. ...... ..X.XX",
		".X.X.. .....X. ..X.XX XXXX.");
	TestStuffBits(6, ".X.X.. ...... ...... ..X.X.",
		".X.X.. .....X .....X ....X. X.XXXX");
	TestStuffBits(6, ".X.X.. XXXXXX ...... ..X.XX",
		".X.X.. XXXXX. X..... ...X.X XXXXX.");
	TestStuffBits(6,
		"...... ..XXXX X..XX. .X.... .X.X.X .....X .X.... ...X.X .....X ....XX ..X... ....X. X..XXX X.XX.X",
		".....X ...XXX XX..XX ..X... ..X.X. X..... X.X... ....X. X..... X....X X..X.. .....X X.X..X XXX.XX .XXXXX");
}

TEST(AZEncoderTest, Encode1)
{
	TestEncode(
		"This is an example Aztec symbol for Wikipedia.",
		true, 3, ParseBitMatrix(
			"X     X X       X     X X     X     X         \n"
			"X         X     X X     X   X X   X X       X \n"
			"X X   X X X X X   X X X                 X     \n"
			"X X                 X X   X       X X X X X X \n"
			"    X X X   X   X     X X X X         X X     \n"
			"  X X X   X X X X   X     X   X     X X   X   \n"
			"        X X X X X     X X X X   X   X     X   \n"
			"X       X   X X X X X X X X X X X     X   X X \n"
			"X   X     X X X               X X X X   X X   \n"
			"X     X X   X X   X X X X X   X X   X   X X X \n"
			"X   X         X   X       X   X X X X       X \n"
			"X       X     X   X   X   X   X   X X   X     \n"
			"      X   X X X   X       X   X     X X X     \n"
			"    X X X X X X   X X X X X   X X X X X X   X \n"
			"  X X   X   X X               X X X   X X X X \n"
			"  X   X       X X X X X X X X X X X X   X X   \n"
			"  X X   X       X X X   X X X       X X       \n"
			"  X               X   X X     X     X X X     \n"
			"  X   X X X   X X   X   X X X X   X   X X X X \n"
			"    X   X   X X X   X   X   X X X X     X     \n"
			"        X               X                 X   \n"
			"        X X     X   X X   X   X   X       X X \n"
			"  X   X   X X       X   X         X X X     X \n"
			, 'X', true)
	);
}

TEST(AZEncoderTest, Encode2)
{
	TestEncode(
		"Aztec Code is a public domain 2D matrix barcode symbology"
		" of nominally square symbols built on a square grid with a "
		"distinctive square bullseye pattern at their center.",
		false, 6, ParseBitMatrix(
			"        X X     X X     X     X     X   X X X         X   X         X   X X       \n"
			"  X       X X     X   X X   X X       X             X     X   X X   X           X \n"
			"  X   X X X     X   X   X X     X X X   X   X X               X X       X X     X \n"
			"X X X             X   X         X         X     X     X   X     X X       X   X   \n"
			"X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X \n"
			"    X X   X   X   X X X               X       X       X X     X X   X X       X   \n"
			"X X     X       X       X X X X   X   X X       X   X X   X       X X   X X   X   \n"
			"  X       X   X     X X   X   X X   X X   X X X X X X   X X           X   X   X X \n"
			"X X   X X   X   X X X X   X X X X X X X X   X   X       X X   X X X X   X X X     \n"
			"  X       X   X     X       X X     X X   X   X   X     X X   X X X   X     X X X \n"
			"  X   X X X   X X       X X X         X X           X   X   X   X X X   X X     X \n"
			"    X     X   X X     X X X X     X   X     X X X X   X X   X X   X X X     X   X \n"
			"X X X   X             X         X X X X X   X   X X   X   X   X X   X   X   X   X \n"
			"          X       X X X   X X     X   X           X   X X X X   X X               \n"
			"  X     X X   X   X       X X X X X X X X X X X X X X X   X   X X   X   X X X     \n"
			"    X X                 X   X                       X X   X       X         X X X \n"
			"        X   X X   X X X X X X   X X X X X X X X X   X     X X           X X X X   \n"
			"          X X X   X     X   X   X               X   X X     X X X   X X           \n"
			"X X     X     X   X   X   X X   X   X X X X X   X   X X X X X X X       X   X X X \n"
			"X X X X       X       X   X X   X   X       X   X   X     X X X     X X       X X \n"
			"X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X \n"
			"    X     X       X         X   X   X       X   X   X     X   X X                 \n"
			"        X X     X X X X X   X   X   X X X X X   X   X X X     X X X X   X         \n"
			"X     X   X   X         X   X   X               X   X X   X X   X X X     X   X   \n"
			"  X   X X X   X   X X   X X X   X X X X X X X X X   X X         X X     X X X X   \n"
			"    X X   X   X   X X X     X                       X X X   X X   X   X     X     \n"
			"    X X X X   X         X   X X X X X X X X X X X X X X   X       X X   X X   X X \n"
			"            X   X   X X       X X X X X     X X X       X       X X X         X   \n"
			"X       X         X   X X X X   X     X X     X X     X X           X   X       X \n"
			"X     X       X X X X X     X   X X X X   X X X     X       X X X X   X   X X   X \n"
			"  X X X X X               X     X X X   X       X X   X X   X X X X     X X       \n"
			"X             X         X   X X   X X     X     X     X   X   X X X X             \n"
			"    X   X X       X     X       X   X X X X X X   X X   X X X X X X X X X   X   X \n"
			"    X         X X   X       X     X   X   X       X     X X X     X       X X X X \n"
			"X     X X     X X X X X X             X X X   X               X   X     X     X X \n"
			"X   X X     X               X X X X X     X X     X X X X X X X X     X   X   X X \n"
			"X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X \n"
			"X           X     X X X X     X     X         X         X   X       X X   X X X   \n"
			"X   X   X X   X X X   X         X X     X X X X     X X   X   X     X   X       X \n"
			"      X     X     X     X X     X   X X   X X   X         X X       X       X   X \n"
			"X       X           X   X   X     X X   X               X     X     X X X         \n"
			, 'X', true)
	);
}

TEST(AZEncoderTest, UserSpecifiedLayers)
{
	std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	Aztec::EncodeResult aztec;
	aztec = Aztec::Encoder::Encode(alphabet, 25, -2);
	EXPECT_EQ(aztec.layers, 2);
	EXPECT_TRUE(aztec.compact);

	aztec = Aztec::Encoder::Encode(alphabet, 25, 32);
	EXPECT_EQ(aztec.layers, 32);
	EXPECT_FALSE(aztec.compact);

	EXPECT_THROW({Aztec::Encoder::Encode(alphabet, 25, 33);}, std::invalid_argument );
	EXPECT_THROW({Aztec::Encoder::Encode(alphabet, 25, -1);}, std::invalid_argument );
}

TEST(AZEncoderTest, BorderCompact4Case)
{
	// Compact(4) con hold 608 bits of information, but at most 504 can be data.  Rest must
	// be error correction
	std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	// encodes as 26 * 5 * 4 = 520 bits of data
	std::string alphabet4 = alphabet + alphabet + alphabet + alphabet;
	EXPECT_THROW({Aztec::Encoder::Encode(alphabet4, 0, -4);}, std::invalid_argument );

	// If we just try to encode it normally, it will go to a non-compact 4 layer
	auto aztec = Aztec::Encoder::Encode(alphabet4, 0, Aztec::Encoder::DEFAULT_AZTEC_LAYERS);
	EXPECT_FALSE(aztec.compact);
	EXPECT_EQ(aztec.layers, 4);

	// But shortening the string to 100 bytes (500 bits of data), compact works fine, even if we
	// include more error checking.
	aztec = Aztec::Encoder::Encode(alphabet4.substr(0, 100), 10, Aztec::Encoder::DEFAULT_AZTEC_LAYERS);
	EXPECT_TRUE(aztec.compact);
	EXPECT_EQ(aztec.layers, 4);
}


TEST(AZEncoderTest, Rune)
{
	{
		Aztec::EncodeResult aztec = Aztec::Encoder::Encode("\x19", 0, Aztec::Encoder::AZTEC_RUNE_LAYERS);
		
		EXPECT_EQ(aztec.layers, 0);
		EXPECT_EQ(aztec.matrix, ParseBitMatrix(
			"X X X   X X     X   X \n"
			"X X X X X X X X X X X \n"
			"  X               X X \n"
			"  X   X X X X X   X X \n"
			"  X   X       X   X   \n"
			"X X   X   X   X   X X \n"
			"X X   X       X   X X \n"
			"X X   X X X X X   X   \n"
			"X X               X X \n"
			"  X X X X X X X X X X \n"
			"    X     X           \n"
		));
	}
	{
		Aztec::EncodeResult aztec = Aztec::Encoder::Encode("\xFF", 0, Aztec::Encoder::AZTEC_RUNE_LAYERS);
		
		EXPECT_EQ(aztec.layers, 0);
		EXPECT_EQ(aztec.matrix, ParseBitMatrix(
			"X X   X   X   X     X \n"
			"X X X X X X X X X X X \n"
			"  X               X X \n"
			"X X   X X X X X   X X \n"
			"X X   X       X   X X \n"
			"  X   X   X   X   X X \n"
			"  X   X       X   X   \n"
			"X X   X X X X X   X X \n"
			"X X               X   \n"
			"  X X X X X X X X X X \n"
			"    X X     X X X     \n"
		));
	}
	{
		Aztec::EncodeResult aztec = Aztec::Encoder::Encode("\x44", 0, Aztec::Encoder::AZTEC_RUNE_LAYERS);

		std::cout << ToString(aztec.matrix, 'X', ' ', true);
	}

	
	
}