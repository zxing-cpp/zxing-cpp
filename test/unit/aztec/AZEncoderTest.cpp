/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2013 ZXing authors
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
#include "gtest/gtest.h"
#include "aztec/AZEncoder.h"
#include "BitMatrix.h"
#include "BitArray.h"
#include "BitMatrixUtility.h"
#include "BitArrayUtility.h"

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
		str.erase(std::remove_if(str.begin(), str.end(), isspace), str.end());
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
		true, 3, Utility::ParseBitMatrix(
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
		false, 6, Utility::ParseBitMatrix(
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

	try {
		aztec = Aztec::Encoder::Encode(alphabet, 25, 33);
		FAIL() << "Encode should have failed.  No such thing as 33 layers";
	}
	catch (const std::invalid_argument&) {
		// expected
	}

	try {
		aztec = Aztec::Encoder::Encode(alphabet, 25, -1);
		FAIL() << "Encode should have failed.  Text can't fit in 1-layer compact";
	}
	catch (const std::invalid_argument&) {
		// expected
	}
}

TEST(AZEncoderTest, BorderCompact4Case)
{
	// Compact(4) con hold 608 bits of information, but at most 504 can be data.  Rest must
	// be error correction
	std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	// encodes as 26 * 5 * 4 = 520 bits of data
	std::string alphabet4 = alphabet + alphabet + alphabet + alphabet;
	Aztec::EncodeResult aztec;
	try {
		aztec = Aztec::Encoder::Encode(alphabet4, 0, -4);
		FAIL() << "Encode should have failed.  Text can't fit in 1-layer compact";
	}
	catch (const std::invalid_argument&) {
		// expected
	}

	// If we just try to encode it normally, it will go to a non-compact 4 layer
	aztec = Aztec::Encoder::Encode(alphabet4, 0, Aztec::Encoder::DEFAULT_AZTEC_LAYERS);
	EXPECT_FALSE(aztec.compact);
	EXPECT_EQ(aztec.layers, 4);

	// But shortening the string to 100 bytes (500 bits of data), compact works fine, even if we
	// include more error checking.
	aztec = Aztec::Encoder::Encode(alphabet4.substr(0, 100), 10, Aztec::Encoder::DEFAULT_AZTEC_LAYERS);
	EXPECT_TRUE(aztec.compact);
	EXPECT_EQ(aztec.layers, 4);
}
