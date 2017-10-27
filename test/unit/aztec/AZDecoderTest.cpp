#include "gtest/gtest.h"
#include "aztec/AZDecoder.h"
#include "aztec/AZDetectorResult.h"
#include "DecoderResult.h"
#include "BitMatrix.h"
#include "DecodeStatus.h"
#include "BitMatrixUtility.h"

using namespace ZXing;

TEST(AZDecoderTest, AztecResult)
{
	auto bits = Utility::ParseBitMatrix(
		"X X X X X     X X X       X X X     X X X     \n"
		"X X X     X X X     X X X X     X X X     X X \n"
		"  X   X X       X   X   X X X X     X     X X \n"
		"  X   X X     X X     X     X   X       X   X \n"
		"  X X   X X         X               X X     X \n"
		"  X X   X X X X X X X X X X X X X X X     X   \n"
		"  X X X X X                       X   X X X   \n"
		"  X   X   X   X X X X X X X X X   X X X   X X \n"
		"  X   X X X   X               X   X X       X \n"
		"  X X   X X   X   X X X X X   X   X X X X   X \n"
		"  X X   X X   X   X       X   X   X   X X X   \n"
		"  X   X   X   X   X   X   X   X   X   X   X   \n"
		"  X X X   X   X   X       X   X   X X   X X   \n"
		"  X X X X X   X   X X X X X   X   X X X   X X \n"
		"X X   X X X   X               X   X   X X   X \n"
		"  X       X   X X X X X X X X X   X   X     X \n"
		"  X X   X X                       X X   X X   \n"
		"  X X X   X X X X X X X X X X X X X X   X X   \n"
		"X     X     X     X X   X X               X X \n"
		"X   X X X X X   X X X X X     X   X   X     X \n"
		"X X X   X X X X           X X X       X     X \n"
		"X X     X X X     X X X X     X X X     X X   \n"
		"    X X X     X X X       X X X     X X X X   \n"
		, 'X', true);

	DecoderResult result = Aztec::Decoder::Decode({std::move(bits), {}, false, 30, 2});
	EXPECT_EQ(result.isValid(), true);
	EXPECT_EQ(result.text(), L"88888TTTTTTTTTTTTTTTTTTTTTTTTTTTTTT");
	EXPECT_EQ(result.rawBytes(), ByteArray({
		0xf5, 0x55, 0x55, 0x75, 0x6b, 0x5a, 0xd6, 0xb5, 0xad, 0x6b, 
		0x5a, 0xd6, 0xb5, 0xad, 0x6b, 0x5a, 0xd6, 0xb5, 0xad, 0x6b, 
		0x5a, 0xd6, 0xb0 }));
	EXPECT_EQ(result.numBits(), 180);
}

TEST(AZDecoderTest, DecodeTooManyErrors)
{
	auto bits = Utility::ParseBitMatrix(
		"X X . X . . . X X . . . X . . X X X . X . X X X X X . \n"
		"X X . . X X . . . . . X X . . . X X . . . X . X . . X \n"
		"X . . . X X . . X X X . X X . X X X X . X X . . X . . \n"
		". . . . X . X X . . X X . X X . X . X X X X . X . . X \n"
		"X X X . . X X X X X . . . . . X X . . . X . X . X . X \n"
		"X X . . . . . . . . X . . . X . X X X . X . . X . . . \n"
		"X X . . X . . . . . X X . . . . . X . . . . X . . X X \n"
		". . . X . X . X . . . . . X X X X X X . . . . . . X X \n"
		"X . . . X . X X X X X X . . X X X . X . X X X X X X . \n"
		"X . . X X X . X X X X X X X X X X X X X . . . X . X X \n"
		". . . . X X . . . X . . . . . . . X X . . . X X . X . \n"
		". . . X X X . . X X . X X X X X . X . . X . . . . . . \n"
		"X . . . . X . X . X . X . . . X . X . X X . X X . X X \n"
		"X . X . . X . X . X . X . X . X . X . . . . . X . X X \n"
		"X . X X X . . X . X . X . . . X . X . X X X . . . X X \n"
		"X X X X X X X X . X . X X X X X . X . X . X . X X X . \n"
		". . . . . . . X . X . . . . . . . X X X X . . . X X X \n"
		"X X . . X . . X . X X X X X X X X X X X X X . . X . X \n"
		"X X X . X X X X . . X X X X . . X . . . . X . . X X X \n"
		". . . . X . X X X . . . . X X X X . . X X X X . . . . \n"
		". . X . . X . X . . . X . X X . X X . X . . . X . X . \n"
		"X X . . X . . X X X X X X X . . X . X X X X X X X . . \n"
		"X . X X . . X X . . . . . X . . . . . . X X . X X X . \n"
		"X . . X X . . X X . X . X . . . . X . X . . X . . X . \n"
		"X . X . X . . X . X X X X X X X X . X X X X . . X X . \n"
		"X X X X . . . X . . X X X . X X . . X . . . . X X X . \n"
		"X X . X . X . . . X . X . . . . X X . X . . X X . . . \n"
		, 'X', true);

	DecoderResult result = Aztec::Decoder::Decode({std::move(bits), {}, true, 16, 4});
	EXPECT_EQ(result.errorCode(), DecodeStatus::FormatError);
}

TEST(AZDecoderTest, DecodeTooManyErrors2)
{
	auto bits = Utility::ParseBitMatrix(
		". X X . . X . X X . . . X . . X X X . . . X X . X X . \n"
		"X X . X X . . X . . . X X . . . X X . X X X . X . X X \n"
		". . . . X . . . X X X . X X . X X X X . X X . . X . . \n"
		"X . X X . . X . . . X X . X X . X . X X . . . . . X . \n"
		"X X . X . . X . X X . . . . . X X . . . . . X . . . X \n"
		"X . . X . . . . . . X . . . X . X X X X X X X . . . X \n"
		"X . . X X . . X . . X X . . . . . X . . . . . X X X . \n"
		". . X X X X . X . . . . . X X X X X X . . . . . . X X \n"
		"X . . . X . X X X X X X . . X X X . X . X X X X X X . \n"
		"X . . X X X . X X X X X X X X X X X X X . . . X . X X \n"
		". . . . X X . . . X . . . . . . . X X . . . X X . X . \n"
		". . . X X X . . X X . X X X X X . X . . X . . . . . . \n"
		"X . . . . X . X . X . X . . . X . X . X X . X X . X X \n"
		"X . X . . X . X . X . X . X . X . X . . . . . X . X X \n"
		"X . X X X . . X . X . X . . . X . X . X X X . . . X X \n"
		"X X X X X X X X . X . X X X X X . X . X . X . X X X . \n"
		". . . . . . . X . X . . . . . . . X X X X . . . X X X \n"
		"X X . . X . . X . X X X X X X X X X X X X X . . X . X \n"
		"X X X . X X X X . . X X X X . . X . . . . X . . X X X \n"
		". . X X X X X . X . . . . X X X X . . X X X . X . X . \n"
		". . X X . X . X . . . X . X X . X X . . . . X X . . . \n"
		"X . . . X . X . X X X X X X . . X . X X X X X . X . . \n"
		". X . . . X X X . . . . . X . . . . . X X X X X . X . \n"
		"X . . X . X X X X . X . X . . . . X . X X . X . . X . \n"
		"X . . . X X . X . X X X X X X X X . X X X X . . X X . \n"
		". X X X X . . X . . X X X . X X . . X . . . . X X X . \n"
		"X X . . . X X . . X . X . . . . X X . X . . X . X . X \n"
		, 'X', true);

	DecoderResult result = Aztec::Decoder::Decode({std::move(bits), {}, true, 16, 4});
	EXPECT_EQ(result.errorCode(), DecodeStatus::FormatError);
}
