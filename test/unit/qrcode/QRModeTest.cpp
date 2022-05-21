/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2008 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "qrcode/QRCodecMode.h"
#include "qrcode/QRVersion.h"

#include "gtest/gtest.h"

using namespace ZXing;
using namespace ZXing::QRCode;

TEST(QRModeTest, ForBits)
{
    ASSERT_EQ(CodecMode::TERMINATOR, CodecModeForBits(0x00));
    ASSERT_EQ(CodecMode::NUMERIC, CodecModeForBits(0x01));
    ASSERT_EQ(CodecMode::ALPHANUMERIC, CodecModeForBits(0x02));
    ASSERT_EQ(CodecMode::BYTE, CodecModeForBits(0x04));
    ASSERT_EQ(CodecMode::KANJI, CodecModeForBits(0x08));
	ASSERT_THROW(CodecModeForBits(0x10), std::invalid_argument);
}

TEST(QRModeTest, CharacterCount)
{
    // Spot check a few values
    ASSERT_EQ(10, CharacterCountBits(CodecMode::NUMERIC, *Version::VersionForNumber(5)));
    ASSERT_EQ(12, CharacterCountBits(CodecMode::NUMERIC, *Version::VersionForNumber(26)));
    ASSERT_EQ(14, CharacterCountBits(CodecMode::NUMERIC, *Version::VersionForNumber(40)));
    ASSERT_EQ(9, CharacterCountBits(CodecMode::ALPHANUMERIC, *Version::VersionForNumber(6)));
    ASSERT_EQ(8, CharacterCountBits(CodecMode::BYTE, *Version::VersionForNumber(7)));
    ASSERT_EQ(8, CharacterCountBits(CodecMode::KANJI, *Version::VersionForNumber(8)));
}

TEST(QRModeTest, MicroForBits)
{
	// M1
	ASSERT_EQ(CodecMode::NUMERIC, CodecModeForBits(0x00, true));
	// M2
	ASSERT_EQ(CodecMode::NUMERIC, CodecModeForBits(0x00, true));
	ASSERT_EQ(CodecMode::ALPHANUMERIC, CodecModeForBits(0x01, true));
	// M3
	ASSERT_EQ(CodecMode::NUMERIC, CodecModeForBits(0x00, true));
	ASSERT_EQ(CodecMode::ALPHANUMERIC, CodecModeForBits(0x01, true));
	ASSERT_EQ(CodecMode::BYTE, CodecModeForBits(0x02, true));
	ASSERT_EQ(CodecMode::KANJI, CodecModeForBits(0x03, true));
	// M4
	ASSERT_EQ(CodecMode::NUMERIC, CodecModeForBits(0x00, true));
	ASSERT_EQ(CodecMode::ALPHANUMERIC, CodecModeForBits(0x01, true));
	ASSERT_EQ(CodecMode::BYTE, CodecModeForBits(0x02, true));
	ASSERT_EQ(CodecMode::KANJI, CodecModeForBits(0x03, true));

	ASSERT_THROW(CodecModeForBits(0x04, true), std::invalid_argument);
}

TEST(QRModeTest, MicroCharacterCount)
{
	// Spot check a few values
	ASSERT_EQ(3, CharacterCountBits(CodecMode::NUMERIC, *Version::VersionForNumber(1, true)));
	ASSERT_EQ(4, CharacterCountBits(CodecMode::NUMERIC, *Version::VersionForNumber(2, true)));
	ASSERT_EQ(6, CharacterCountBits(CodecMode::NUMERIC, *Version::VersionForNumber(4, true)));
	ASSERT_EQ(3, CharacterCountBits(CodecMode::ALPHANUMERIC, *Version::VersionForNumber(2, true)));
	ASSERT_EQ(4, CharacterCountBits(CodecMode::BYTE, *Version::VersionForNumber(3, true)));
	ASSERT_EQ(4, CharacterCountBits(CodecMode::KANJI, *Version::VersionForNumber(4, true)));
}
