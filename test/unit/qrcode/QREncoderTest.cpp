/*
 * Copyright 2008 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "BitArray.h"
#include "BitArrayUtility.h"
#include "BitMatrixIO.h"
#include "CharacterSet.h"
#include "TextDecoder.h"
#include "Utf.h"
#include "qrcode/QREncoder.h"
#include "qrcode/QRCodecMode.h"
#include "qrcode/QREncodeResult.h"
#include "qrcode/QRErrorCorrectionLevel.h"

#include "gtest/gtest.h"

namespace ZXing {
	namespace QRCode {
		int GetAlphanumericCode(int code);
		CodecMode ChooseMode(const std::wstring& content, CharacterSet encoding);
		void AppendModeInfo(CodecMode mode, BitArray& bits);
		void AppendLengthInfo(int numLetters, const Version& version, CodecMode mode, BitArray& bits);
		void AppendNumericBytes(const std::wstring& content, BitArray& bits);
		void AppendAlphanumericBytes(const std::wstring& content, BitArray& bits);
		void Append8BitBytes(const std::wstring& content, CharacterSet encoding, BitArray& bits);
		void AppendKanjiBytes(const std::wstring& content, BitArray& bits);
		void AppendBytes(const std::wstring& content, CodecMode mode, CharacterSet encoding, BitArray& bits);
		void TerminateBits(int numDataBytes, BitArray& bits);
		void GetNumDataBytesAndNumECBytesForBlockID(int numTotalBytes, int numDataBytes, int numRSBlocks, int blockID, int& numDataBytesInBlock, int& numECBytesInBlock);
		void GenerateECBytes(const ByteArray& dataBytes, int numEcBytesInBlock, ByteArray& ecBytes);
		BitArray InterleaveWithECBytes(const BitArray& bits, int numTotalBytes, int numDataBytes, int numRSBlocks);
	}
}

using namespace ZXing;
using namespace ZXing::QRCode;
using namespace ZXing::Utility;

namespace {
	std::wstring ShiftJISString(const std::vector<uint8_t>& bytes)
	{
		std::string str;
		TextDecoder::Append(str, bytes.data(), bytes.size(), CharacterSet::Shift_JIS);
		return FromUtf8(str);
	}

	std::string RemoveSpace(std::string s)
	{
		s.erase(std::remove(s.begin(), s.end(), ' '), s.end());
		return s;
	}
}

TEST(QREncoderTest, GetAlphanumericCode)
{
	// The first ten code points are numbers.
	for (int i = 0; i < 10; ++i) {
		EXPECT_EQ(i, GetAlphanumericCode('0' + i));
	}

	// The next 26 code points are capital alphabet letters.
	for (int i = 10; i < 36; ++i) {
		EXPECT_EQ(i, GetAlphanumericCode('A' + i - 10));
	}

	// Others are symbol letters
	EXPECT_EQ(36, GetAlphanumericCode(' '));
	EXPECT_EQ(37, GetAlphanumericCode('$'));
	EXPECT_EQ(38, GetAlphanumericCode('%'));
	EXPECT_EQ(39, GetAlphanumericCode('*'));
	EXPECT_EQ(40, GetAlphanumericCode('+'));
	EXPECT_EQ(41, GetAlphanumericCode('-'));
	EXPECT_EQ(42, GetAlphanumericCode('.'));
	EXPECT_EQ(43, GetAlphanumericCode('/'));
	EXPECT_EQ(44, GetAlphanumericCode(':'));

	// Should return -1 for other letters;
	EXPECT_EQ(-1, GetAlphanumericCode('a'));
	EXPECT_EQ(-1, GetAlphanumericCode('#'));
	EXPECT_EQ(-1, GetAlphanumericCode('\0'));
}

TEST(QREncoderTest, ChooseMode)
{
	// Numeric mode.
	EXPECT_EQ(CodecMode::NUMERIC, ChooseMode(L"0", CharacterSet::Unknown));
	EXPECT_EQ(CodecMode::NUMERIC, ChooseMode(L"0123456789", CharacterSet::Unknown));
	// Alphanumeric mode.
	EXPECT_EQ(CodecMode::ALPHANUMERIC, ChooseMode(L"A", CharacterSet::Unknown));
	EXPECT_EQ(CodecMode::ALPHANUMERIC,
			  ChooseMode(L"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:", CharacterSet::Unknown));
	// 8-bit byte mode.
	EXPECT_EQ(CodecMode::BYTE, ChooseMode(L"a", CharacterSet::Unknown));
	EXPECT_EQ(CodecMode::BYTE, ChooseMode(L"#", CharacterSet::Unknown));
	EXPECT_EQ(CodecMode::BYTE, ChooseMode(L"", CharacterSet::Unknown));
	// Kanji mode.  We used to use MODE_KANJI for these, but we stopped
	// doing that as we cannot distinguish Shift_JIS from other encodings
	// from data bytes alone.  See also comments in qrcode_encoder.h.

	// AIUE in Hiragana in Shift_JIS
	EXPECT_EQ(CodecMode::BYTE,
			  ChooseMode(ShiftJISString({0x8, 0xa, 0x8, 0xa, 0x8, 0xa, 0x8, 0xa6}), CharacterSet::Unknown));

	// Nihon in Kanji in Shift_JIS.
	EXPECT_EQ(CodecMode::BYTE, ChooseMode(ShiftJISString({0x9, 0xf, 0x9, 0x7b}), CharacterSet::Unknown));

	// Sou-Utsu-Byou in Kanji in Shift_JIS.
	EXPECT_EQ(CodecMode::BYTE, ChooseMode(ShiftJISString({0xe, 0x4, 0x9, 0x5, 0x9, 0x61}), CharacterSet::Unknown));
}

TEST(QREncoderTest, Encode)
{
	auto qrCode = Encode(L"ABCDEF", ErrorCorrectionLevel::High, CharacterSet::Unknown, 0, false, -1);
	EXPECT_EQ(qrCode.mode, CodecMode::ALPHANUMERIC);
	EXPECT_EQ(qrCode.ecLevel, ErrorCorrectionLevel::High);
	ASSERT_NE(qrCode.version, nullptr);
	EXPECT_EQ(qrCode.version->versionNumber(), 1);
	EXPECT_EQ(qrCode.maskPattern, 4);
	EXPECT_EQ(ToString(qrCode.matrix, 'X', ' ', true),
		"X X X X X X X     X   X     X X X X X X X \n"
		"X           X   X   X   X   X           X \n"
		"X   X X X   X               X   X X X   X \n"
		"X   X X X   X     X     X   X   X X X   X \n"
		"X   X X X   X     X   X     X   X X X   X \n"
		"X           X   X     X X   X           X \n"
		"X X X X X X X   X   X   X   X X X X X X X \n"
		"                X       X                 \n"
		"        X X X X   X X   X   X X       X   \n"
		"        X X   X X X     X X X X   X X   X \n"
		"X         X X     X   X       X X X   X X \n"
		"X     X X X     X X X X         X         \n"
		"  X X X X X X   X   X   X X X     X X     \n"
		"                X X       X X       X   X \n"
		"X X X X X X X   X X X X           X X     \n"
		"X           X   X X   X       X   X X X X \n"
		"X   X X X   X   X     X       X X     X X \n"
		"X   X X X   X       X X   X         X X X \n"
		"X   X X X   X     X   X       X X         \n"
		"X           X     X     X     X X       X \n"
		"X X X X X X X       X     X         X X X \n");
}

TEST(QREncoderTest, EncodeWithVersion)
{
	auto qrCode = Encode(L"ABCDEF", ErrorCorrectionLevel::High, CharacterSet::Unknown, 7, false, -1);
	ASSERT_NE(qrCode.version, nullptr);
	EXPECT_EQ(qrCode.version->versionNumber(), 7);
}

TEST(QREncoderTest, EncodeWithVersionTooSmall)
{
	EXPECT_THROW(
		Encode(L"THISMESSAGEISTOOLONGFORAQRCODEVERSION3", ErrorCorrectionLevel::High, CharacterSet::Unknown, 3, false, -1)
	, std::invalid_argument);
}

TEST(QREncoderTest, SimpleUTF8ECI)
{
	auto qrCode = Encode(L"hello", ErrorCorrectionLevel::High, CharacterSet::UTF8, 0, false, -1);
	EXPECT_EQ(qrCode.mode, CodecMode::BYTE);
	EXPECT_EQ(qrCode.ecLevel, ErrorCorrectionLevel::High);
	ASSERT_NE(qrCode.version, nullptr);
	EXPECT_EQ(qrCode.version->versionNumber(), 1);
	EXPECT_EQ(qrCode.maskPattern, 6);
	EXPECT_EQ(ToString(qrCode.matrix, 'X', ' ', true),
		"X X X X X X X       X X     X X X X X X X \n"
		"X           X       X X     X           X \n"
		"X   X X X   X   X     X X   X   X X X   X \n"
		"X   X X X   X   X       X   X   X X X   X \n"
		"X   X X X   X     X X       X   X X X   X \n"
		"X           X         X     X           X \n"
		"X X X X X X X   X   X   X   X X X X X X X \n"
		"                  X X X X                 \n"
		"      X X   X X         X         X X     \n"
		"                X X   X     X   X X X X X \n"
		"X X       X X X       X X     X   X   X X \n"
		"        X X     X           X   X X       \n"
		"  X X     X X     X X X   X X X X X X X X \n"
		"                X X X   X X X X X X X X X \n"
		"X X X X X X X   X   X       X             \n"
		"X           X     X       X       X X     \n"
		"X   X X X   X   X       X   X       X     \n"
		"X   X X X   X   X X X X   X     X   X X   \n"
		"X   X X X   X     X X X     X     X   X X \n"
		"X           X             X X   X X       \n"
		"X X X X X X X         X   X     X   X     \n");
}

TEST(QREncoderTest, SimpleBINARYECI)
{
	auto qrCode = Encode(L"\u00E9", ErrorCorrectionLevel::High, CharacterSet::BINARY, 0, false, -1);
	EXPECT_EQ(qrCode.mode, CodecMode::BYTE);
	EXPECT_EQ(qrCode.ecLevel, ErrorCorrectionLevel::High);
	ASSERT_NE(qrCode.version, nullptr);
	EXPECT_EQ(qrCode.version->versionNumber(), 1);
	EXPECT_EQ(qrCode.maskPattern, 6);
	EXPECT_EQ(ToString(qrCode.matrix, 'X', ' ', true),
		"X X X X X X X     X X X X   X X X X X X X \n"
		"X           X           X   X           X \n"
		"X   X X X   X   X X   X     X   X X X   X \n"
		"X   X X X   X   X X X X X   X   X X X   X \n"
		"X   X X X   X       X       X   X X X   X \n"
		"X           X     X     X   X           X \n"
		"X X X X X X X   X   X   X   X X X X X X X \n"
		"                    X X X                 \n"
		"      X X   X X     X             X X     \n"
		"X   X           X X       X     X   X   X \n"
		"X X       X X X X X X X     X   X X X X X \n"
		"X   X X X     X X   X     X       X X X X \n"
		"      X   X X   X       X X X       X X X \n"
		"                X   X X X     X   X   X X \n"
		"X X X X X X X   X       X X X X X     X X \n"
		"X           X     X       X   X X     X   \n"
		"X   X X X   X   X   X X X X   X     X   X \n"
		"X   X X X   X   X X   X   X X   X X X     \n"
		"X   X X X   X           X   X     X X X   \n"
		"X           X     X X X         X   X     \n"
		"X X X X X X X     X X X   X X       X     \n");
}

TEST(QREncoderTest, EncodeKanjiMode)
{
	auto qrCode = Encode(L"\u65e5\u672c", ErrorCorrectionLevel::Medium, CharacterSet::Shift_JIS, 0, false, -1);
	EXPECT_EQ(qrCode.mode, CodecMode::KANJI);
	EXPECT_EQ(qrCode.ecLevel, ErrorCorrectionLevel::Medium);
	ASSERT_NE(qrCode.version, nullptr);
	EXPECT_EQ(qrCode.version->versionNumber(), 1);
	EXPECT_EQ(qrCode.maskPattern, 0);
	EXPECT_EQ(ToString(qrCode.matrix, 'X', ' ', true),
		"X X X X X X X     X   X     X X X X X X X \n"
		"X           X   X X         X           X \n"
		"X   X X X   X     X X X X   X   X X X   X \n"
		"X   X X X   X           X   X   X X X   X \n"
		"X   X X X   X   X X X X X   X   X X X   X \n"
		"X           X     X X X     X           X \n"
		"X X X X X X X   X   X   X   X X X X X X X \n"
		"                    X                     \n"
		"X   X   X   X       X   X       X     X   \n"
		"X X   X       X   X X X   X   X   X       \n"
		"  X         X X X X X X   X X X   X   X   \n"
		"X X X     X   X       X X X   X X   X     \n"
		"  X X     X X   X X   X   X X X   X     X \n"
		"                X   X       X       X   X \n"
		"X X X X X X X           X       X     X X \n"
		"X           X       X       X       X X X \n"
		"X   X X X   X   X       X   X   X   X   X \n"
		"X   X X X   X         X   X   X   X   X   \n"
		"X   X X X   X   X   X X   X X X     X   X \n"
		"X           X         X X X   X X X   X   \n"
		"X X X X X X X   X X   X   X X X     X     \n");
}

TEST(QREncoderTest, EncodeShiftjisNumeric)
{
	auto qrCode = Encode(L"0123", ErrorCorrectionLevel::Medium, CharacterSet::Shift_JIS, 0, false, -1);
	EXPECT_EQ(qrCode.mode, CodecMode::NUMERIC);
	EXPECT_EQ(qrCode.ecLevel, ErrorCorrectionLevel::Medium);
	ASSERT_NE(qrCode.version, nullptr);
	EXPECT_EQ(qrCode.version->versionNumber(), 1);
	EXPECT_EQ(qrCode.maskPattern, 2);
	EXPECT_EQ(ToString(qrCode.matrix, 'X', ' ', true),
		"X X X X X X X     X X   X   X X X X X X X \n"
		"X           X     X     X   X           X \n"
		"X   X X X   X   X           X   X X X   X \n"
		"X   X X X   X   X   X X X   X   X X X   X \n"
		"X   X X X   X   X X   X X   X   X X X   X \n"
		"X           X   X X     X   X           X \n"
		"X X X X X X X   X   X   X   X X X X X X X \n"
		"                X X X X X                 \n"
		"X   X X X X X     X X   X   X X X X X     \n"
		"X X       X     X   X   X     X     X     \n"
		"  X X   X X X X   X X X   X     X X   X X \n"
		"X   X X   X   X     X         X X   X     \n"
		"    X     X X X       X   X     X   X     \n"
		"                X X   X X X X     X       \n"
		"X X X X X X X       X   X   X X           \n"
		"X           X   X X   X X X X     X   X   \n"
		"X   X X X   X   X   X   X     X     X     \n"
		"X   X X X   X   X X X   X     X     X     \n"
		"X   X X X   X   X X   X   X     X X X     \n"
		"X           X       X         X X   X X   \n"
		"X X X X X X X   X X   X   X     X X X     \n");
}

TEST(QREncoderTest, EncodeGS1)
{
	auto qrCode = Encode(L"100001%11171218", ErrorCorrectionLevel::High, CharacterSet::Unknown, 0, true, -1);
	EXPECT_EQ(qrCode.mode, CodecMode::ALPHANUMERIC);
	EXPECT_EQ(qrCode.ecLevel, ErrorCorrectionLevel::High);
	ASSERT_NE(qrCode.version, nullptr);
	EXPECT_EQ(qrCode.version->versionNumber(), 2);
	EXPECT_EQ(qrCode.maskPattern, 4);
	EXPECT_EQ(ToString(qrCode.matrix, 'X', ' ', true),
		"X X X X X X X     X X X X   X   X   X X X X X X X \n"
		"X           X   X X           X X   X           X \n"
		"X   X X X   X           X X X   X   X   X X X   X \n"
		"X   X X X   X     X   X     X X     X   X X X   X \n"
		"X   X X X   X       X X X       X   X   X X X   X \n"
		"X           X   X X   X X   X X     X           X \n"
		"X X X X X X X   X   X   X   X   X   X X X X X X X \n"
		"                X X   X X   X X                   \n"
		"        X X X X     X X       X X   X X       X   \n"
		"  X X   X X     X X X       X X X X X X X       X \n"
		"    X X X X X   X X X X X   X             X X X   \n"
		"X   X X X     X X X   X X X X X   X X   X X X     \n"
		"  X   X     X X X X X X     X X   X           X   \n"
		"X     X X X     X X       X X   X   X   X         \n"
		"    X     X X X   X X   X X X   X X X   X X X X   \n"
		"      X X     X     X     X X     X       X X X   \n"
		"X X   X   X X   X   X       X X X X X X X         \n"
		"                X X   X       X X       X X   X   \n"
		"X X X X X X X   X   X   X   X X X   X   X         \n"
		"X           X   X X       X   X X       X   X X   \n"
		"X   X X X   X   X X X           X X X X X X     X \n"
		"X   X X X   X             X X X     X X   X       \n"
		"X   X X X   X       X X   X   X X X   X X     X   \n"
		"X           X     X X   X X X X X   X   X X       \n"
		"X X X X X X X       X         X X     X X     X X \n");
}

TEST(QREncoderTest, EncodeGS1ModeHeaderWithECI)
{
	auto qrCode = Encode(L"hello", ErrorCorrectionLevel::High, CharacterSet::UTF8, 0, true, -1);
	EXPECT_EQ(qrCode.mode, CodecMode::BYTE);
	EXPECT_EQ(qrCode.ecLevel, ErrorCorrectionLevel::High);
	ASSERT_NE(qrCode.version, nullptr);
	EXPECT_EQ(qrCode.version->versionNumber(), 1);
	EXPECT_EQ(qrCode.maskPattern, 5);
	EXPECT_EQ(ToString(qrCode.matrix, 'X', ' ', true),
		"X X X X X X X   X   X X     X X X X X X X \n"
		"X           X     X X       X           X \n"
		"X   X X X   X   X X X       X   X X X   X \n"
		"X   X X X   X     X   X     X   X X X   X \n"
		"X   X X X   X   X   X       X   X X X   X \n"
		"X           X     X X X X   X           X \n"
		"X X X X X X X   X   X   X   X X X X X X X \n"
		"                X   X X X                 \n"
		"          X X     X X       X   X   X   X \n"
		"  X   X X     X   X X X X X X   X X X   X \n"
		"  X   X X X X   X X       X   X   X X     \n"
		"X X X X   X   X     X   X     X X X X     \n"
		"X     X     X X   X X   X   X     X     X \n"
		"                X X X X X   X   X     X   \n"
		"X X X X X X X       X X     X       X X   \n"
		"X           X   X X         X   X X X     \n"
		"X   X X X   X     X     X   X   X       X \n"
		"X   X X X   X           X X X   X X X X   \n"
		"X   X X X   X       X     X     X   X X X \n"
		"X           X     X       X X     X X X X \n"
		"X X X X X X X     X X X   X X   X     X   \n");
}

TEST(QREncoderTest, AppendModeInfo)
{
	BitArray bits;
	AppendModeInfo(CodecMode::NUMERIC, bits);
	EXPECT_EQ(ToString(bits), "...X");
}

TEST(QREncoderTest, AppendLengthInfo)
{
	BitArray bits;
	AppendLengthInfo(1, // 1 letter (1/1).
					 *Version::Model2(1), CodecMode::NUMERIC, bits);
	EXPECT_EQ(ToString(bits), RemoveSpace("........ .X")); // 10 bits.

	bits = BitArray();
	AppendLengthInfo(2, // 2 letters (2/1).
					 *Version::Model2(10), CodecMode::ALPHANUMERIC, bits);
	EXPECT_EQ(ToString(bits), RemoveSpace("........ .X.")); // 11 bits.

	bits = BitArray();
	AppendLengthInfo(255, // 255 letter (255/1).
					 *Version::Model2(27), CodecMode::BYTE, bits);
	EXPECT_EQ(ToString(bits), RemoveSpace("........ XXXXXXXX")); // 16 bits.

	bits = BitArray();
	AppendLengthInfo(512, // 512 letters (1024/2).
					 *Version::Model2(40), CodecMode::KANJI, bits);
	EXPECT_EQ(ToString(bits), RemoveSpace("..X..... ....")); // 12 bits.
}

TEST(QREncoderTest, AppendNumericBytes)
{
	// 1 = 01 = 0001 in 4 bits.
	BitArray bits;
	AppendNumericBytes(L"1", bits);
	EXPECT_EQ(ToString(bits), RemoveSpace("...X"));

	// 12 = 0xc = 0001100 in 7 bits.
	bits = BitArray();
	AppendNumericBytes(L"12", bits);
	EXPECT_EQ(ToString(bits), RemoveSpace("...XX.."));

	// 123 = 0x7b = 0001111011 in 10 bits.
	bits = BitArray();
	AppendNumericBytes(L"123", bits);
	EXPECT_EQ(ToString(bits), RemoveSpace("...XXXX. XX"));

	// 1234 = "123" + "4" = 0001111011 + 0100
	bits = BitArray();
	AppendNumericBytes(L"1234", bits);
	EXPECT_EQ(ToString(bits), RemoveSpace("...XXXX. XX.X.."));

	// Empty.
	bits = BitArray();
	AppendNumericBytes(L"", bits);
	EXPECT_EQ(ToString(bits), RemoveSpace(""));
}

TEST(QREncoderTest, AppendAlphanumericBytes)
{
	// A = 10 = 0xa = 001010 in 6 bits
	BitArray bits;
	AppendAlphanumericBytes(L"A", bits);
	EXPECT_EQ(ToString(bits), RemoveSpace("..X.X."));

	// AB = 10 * 45 + 11 = 461 = 0x1cd = 00111001101 in 11 bits
	bits = BitArray();
	AppendAlphanumericBytes(L"AB", bits);
	EXPECT_EQ(ToString(bits), RemoveSpace("..XXX..X X.X"));

	// ABC = "AB" + "C" = 00111001101 + 001100
	bits = BitArray();
	AppendAlphanumericBytes(L"ABC", bits);
	EXPECT_EQ(ToString(bits), RemoveSpace("..XXX..X X.X..XX. ."));

	// Empty.
	bits = BitArray();
	AppendAlphanumericBytes(L"", bits);
	EXPECT_EQ(ToString(bits), RemoveSpace(""));

	// Invalid data.
	EXPECT_THROW(AppendAlphanumericBytes(L"abc", bits), std::invalid_argument);
}

TEST(QREncoderTest, Append8BitBytes)
{
	// 0x61, 0x62, 0x63
	BitArray bits;
	Append8BitBytes(L"abc", CharacterSet::Unknown, bits);
	EXPECT_EQ(ToString(bits), RemoveSpace(".XX....X .XX...X. .XX...XX"));
	
	// Empty.
	bits = BitArray();
	Append8BitBytes(L"", CharacterSet::Unknown, bits);
	EXPECT_EQ(ToString(bits), RemoveSpace(""));
}

// Numbers are from page 21 of JISX0510:2004
TEST(QREncoderTest, AppendKanjiBytes)
{
	BitArray bits;
	AppendKanjiBytes(ShiftJISString({ 0x93, 0x5f }), bits);
	EXPECT_EQ(ToString(bits), RemoveSpace(".XX.XX.. XXXXX"));
	
	AppendKanjiBytes(ShiftJISString({ 0xe4, 0xaa }), bits);
	EXPECT_EQ(ToString(bits), RemoveSpace(".XX.XX.. XXXXXXX. X.X.X.X. X."));
}

TEST(QREncoderTest, AppendBytes)
{
	// Should use appendNumericBytes.
	// 1 = 01 = 0001 in 4 bits.
	BitArray bits;
	AppendBytes(L"1", CodecMode::NUMERIC, CharacterSet::Unknown, bits);
	EXPECT_EQ(ToString(bits), RemoveSpace("...X"));

	// Should use appendAlphanumericBytes.
	// A = 10 = 0xa = 001010 in 6 bits
	bits = BitArray();
	AppendBytes(L"A", CodecMode::ALPHANUMERIC, CharacterSet::Unknown, bits);
	EXPECT_EQ(ToString(bits), RemoveSpace("..X.X."));

	// Lower letters such as 'a' cannot be encoded in MODE_ALPHANUMERIC.
	bits = BitArray();
	EXPECT_THROW(AppendBytes(L"a", CodecMode::ALPHANUMERIC, CharacterSet::Unknown, bits), std::invalid_argument);

	// Should use append8BitBytes.
	// 0x61, 0x62, 0x63
	bits = BitArray();
	AppendBytes(L"abc", CodecMode::BYTE, CharacterSet::Unknown, bits);
	EXPECT_EQ(ToString(bits), RemoveSpace(".XX....X .XX...X. .XX...XX"));

	// Anything can be encoded in QRCode.MODE_8BIT_BYTE.
	AppendBytes(L"\0", CodecMode::BYTE, CharacterSet::Unknown, bits);

	// Should use appendKanjiBytes.
	// 0x93, 0x5f
	bits = BitArray();
	AppendBytes(ShiftJISString({0x93, 0x5f}), CodecMode::KANJI, CharacterSet::Unknown, bits);
	EXPECT_EQ(ToString(bits), RemoveSpace(".XX.XX.. XXXXX"));
}

TEST(QREncoderTest, TerminateBits)
{
	BitArray v;
	TerminateBits(0, v);
	EXPECT_EQ(ToString(v), RemoveSpace(""));

	v = BitArray();
	TerminateBits(1, v);
	EXPECT_EQ(ToString(v), RemoveSpace("........"));

	v = BitArray();
	v.appendBits(0, 3);
	TerminateBits(1, v);
	EXPECT_EQ(ToString(v), RemoveSpace("........"));

	v = BitArray();
	v.appendBits(0, 5);
	TerminateBits(1, v);
	EXPECT_EQ(ToString(v), RemoveSpace("........"));

	v = BitArray();
	v.appendBits(0, 8);
	TerminateBits(1, v);
	EXPECT_EQ(ToString(v), RemoveSpace("........"));

	v = BitArray();
	TerminateBits(2, v);
	EXPECT_EQ(ToString(v), RemoveSpace("........ XXX.XX.."));

	v = BitArray();
	v.appendBits(0, 1);
	TerminateBits(3, v);
	EXPECT_EQ(ToString(v), RemoveSpace("........ XXX.XX.. ...X...X"));
}

TEST(QREncoderTest, GetNumDataBytesAndNumECBytesForBlockID)
{
	int numDataBytes;
	int numEcBytes;
	// Version 1-H.
	GetNumDataBytesAndNumECBytesForBlockID(26, 9, 1, 0, numDataBytes, numEcBytes);
	EXPECT_EQ(9, numDataBytes);
	EXPECT_EQ(17, numEcBytes);

	// Version 3-H.  2 blocks.
	GetNumDataBytesAndNumECBytesForBlockID(70, 26, 2, 0, numDataBytes, numEcBytes);
	EXPECT_EQ(13, numDataBytes);
	EXPECT_EQ(22, numEcBytes);
	GetNumDataBytesAndNumECBytesForBlockID(70, 26, 2, 1, numDataBytes, numEcBytes);
	EXPECT_EQ(13, numDataBytes);
	EXPECT_EQ(22, numEcBytes);

	// Version 7-H. (4 + 1) blocks.
	GetNumDataBytesAndNumECBytesForBlockID(196, 66, 5, 0, numDataBytes, numEcBytes);
	EXPECT_EQ(13, numDataBytes);
	EXPECT_EQ(26, numEcBytes);
	GetNumDataBytesAndNumECBytesForBlockID(196, 66, 5, 4, numDataBytes, numEcBytes);
	EXPECT_EQ(14, numDataBytes);
	EXPECT_EQ(26, numEcBytes);

	// Version 40-H. (20 + 61) blocks.
	GetNumDataBytesAndNumECBytesForBlockID(3706, 1276, 81, 0, numDataBytes, numEcBytes);
	EXPECT_EQ(15, numDataBytes);
	EXPECT_EQ(30, numEcBytes);
	GetNumDataBytesAndNumECBytesForBlockID(3706, 1276, 81, 20, numDataBytes, numEcBytes);
	EXPECT_EQ(16, numDataBytes);
	EXPECT_EQ(30, numEcBytes);
	GetNumDataBytesAndNumECBytesForBlockID(3706, 1276, 81, 80, numDataBytes, numEcBytes);
	EXPECT_EQ(16, numDataBytes);
	EXPECT_EQ(30, numEcBytes);
}

// Numbers are from http://www.swetake.com/qr/qr3.html and
// http://www.swetake.com/qr/qr9.html
TEST(QREncoderTest, GenerateECBytes)
{
	ByteArray ecBytes;
	GenerateECBytes({ 32, 65, 205, 69, 41, 220, 46, 128, 236 }, 17, ecBytes);
	ByteArray expected = { 42, 159, 74, 221, 244, 169, 239, 150, 138, 70, 237, 85, 224, 96, 74, 219, 61 };
	EXPECT_EQ(ecBytes, expected);
	
	GenerateECBytes({ 67, 70, 22, 38, 54, 70, 86, 102, 118, 134, 150, 166, 182, 198, 214 }, 18, ecBytes);
	expected = { 175, 80, 155, 64, 178, 45, 214, 233, 65, 209, 12, 155, 117, 31, 140, 214, 27, 187 };
	EXPECT_EQ(ecBytes, expected);

	// High-order zero coefficient case.
	GenerateECBytes({ 32, 49, 205, 69, 42, 20, 0, 236, 17 }, 17, ecBytes);
	expected = { 0, 3, 130, 179, 194, 0, 55, 211, 110, 79, 98, 72, 170, 96, 211, 137, 213 };
	EXPECT_EQ(ecBytes, expected);
}

TEST(QREncoderTest, InterleaveWithECBytes)
{
	BitArray in;
	for (int dataByte : {32, 65, 205, 69, 41, 220, 46, 128, 236})
		in.appendBits(dataByte, 8);

	BitArray out = InterleaveWithECBytes(in, 26, 9, 1);
	std::vector<uint8_t> expected = {
		32, 65,  205, 69,  41,  220, 46,  128, 236,

		42, 159, 74,  221, 244, 169, 239, 150, 138, 70, 237, 85, 224, 96, 74, 219, 61,
	}; // Error correction bytes in second block
	ASSERT_EQ(Size(expected), out.sizeInBytes());
	EXPECT_EQ(expected, out.toBytes());

	// Numbers are from http://www.swetake.com/qr/qr8.html
	in = BitArray();
	for (int dataByte :
		 {67,  70, 22,  38,  54,  70,  86,  102, 118, 134, 150, 166, 182, 198, 214, 230, 247, 7,   23,  39,  55,
		  71,  87, 103, 119, 135, 151, 166, 22,  38,  54,  70,  86,  102, 118, 134, 150, 166, 182, 198, 214, 230,
		  247, 7,  23,  39,  55,  71,  87,  103, 119, 135, 151, 160, 236, 17,  236, 17,  236, 17,  236, 17})
		in.appendBits(dataByte, 8);

	out = InterleaveWithECBytes(in, 134, 62, 4);
	expected = {
		67,  230, 54,  55,  70,  247, 70,  71,  22,  7,   86,  87,  38,  23,  102, 103, 54,  39,  118, 119, 70,
		55,  134, 135, 86,  71,  150, 151, 102, 87,  166, 160, 118, 103, 182, 236, 134, 119, 198, 17,  150, 135,
		214, 236, 166, 151, 230, 17,  182, 166, 247, 236, 198, 22,  7,   17,  214, 38,  23,  236, 39,  17,

		175, 155, 245, 236, 80,  146, 56,  74,  155, 165, 133, 142, 64,  183, 132, 13,  178, 54,  132, 108, 45,
		113, 53,  50,  214, 98,  193, 152, 233, 147, 50,  71,  65,  190, 82,  51,  209, 199, 171, 54,  12,  112,
		57,  113, 155, 117, 211, 164, 117, 30,  158, 225, 31,  190, 242, 38,  140, 61,  179, 154, 214, 138, 147,
		87,  27,  96,  77,  47,  187, 49,  156, 214,
	}; // Error correction bytes in second block
	EXPECT_EQ(Size(expected), out.sizeInBytes());
	EXPECT_EQ(expected, out.toBytes());
}

TEST(QREncoderTest, BugInBitVectorNumBytes)
{
	// There was a bug in BitVector.sizeInBytes() that caused it to return a
	// smaller-by-one value (ex. 1465 instead of 1466) if the number of bits
	// in the vector is not 8-bit aligned.  In QRCodeEncoder::InitQRCode(),
	// BitVector::sizeInBytes() is used for finding the smallest QR Code
	// version that can fit the given data.  Hence there were corner cases
	// where we chose a wrong QR Code version that cannot fit the given
	// data.  Note that the issue did not occur with MODE_8BIT_BYTE, as the
	// bits in the bit vector are always 8-bit aligned.
	//
	// Before the bug was fixed, the following test didn't pass, because:
	//
	// - MODE_NUMERIC is chosen as all bytes in the data are '0'
	// - The 3518-byte numeric data needs 1466 bytes
	//   - 3518 / 3 * 10 + 7 = 11727 bits = 1465.875 bytes
	//   - 3 numeric bytes are encoded in 10 bits, hence the first
	//     3516 bytes are encoded in 3516 / 3 * 10 = 11720 bits.
	//   - 2 numeric bytes can be encoded in 7 bits, hence the last
	//     2 bytes are encoded in 7 bits.
	// - The version 27 QR Code with the EC level L has 1468 bytes for data.
	//   - 1828 - 360 = 1468
	// - In InitQRCode(), 3 bytes are reserved for a header.  Hence 1465 bytes
	//   (1468 -3) are left for data.
	// - Because of the bug in BitVector::sizeInBytes(), InitQRCode() determines
	//   the given data can fit in 1465 bytes, despite it needs 1466 bytes.
	// - Hence QRCodeEncoder.encode() failed and returned false.
	//   - To be precise, it needs 11727 + 4 (getMode info) + 14 (length info) =
	//     11745 bits = 1468.125 bytes are needed (i.e. cannot fit in 1468
	//     bytes).
	Encode(std::wstring(3518, L'0'), ErrorCorrectionLevel::Low, CharacterSet::Unknown, 0, false, -1);
}
