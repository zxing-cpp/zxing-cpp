/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2008 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "qrcode/QRCodecMode.h"
#include "qrcode/QRVersion.h"
#include "Error.h"

#include "gtest/gtest.h"

using namespace ZXing;
using namespace ZXing::QRCode;

TEST(QRModeTest, ForBits)
{
	ASSERT_EQ(CodecMode::TERMINATOR, CodecModeForBits(0x00, Type::Model2));
	ASSERT_EQ(CodecMode::NUMERIC, CodecModeForBits(0x01, Type::Model2));
	ASSERT_EQ(CodecMode::ALPHANUMERIC, CodecModeForBits(0x02, Type::Model2));
	ASSERT_EQ(CodecMode::BYTE, CodecModeForBits(0x04, Type::Model2));
	ASSERT_EQ(CodecMode::KANJI, CodecModeForBits(0x08, Type::Model2));
	ASSERT_THROW(CodecModeForBits(0x10, Type::Model2), Error);
}

TEST(QRModeTest, CharacterCount)
{
	// Spot check a few values
	ASSERT_EQ(10, CharacterCountBits(CodecMode::NUMERIC, *Version::Model2(5)));
	ASSERT_EQ(12, CharacterCountBits(CodecMode::NUMERIC, *Version::Model2(26)));
	ASSERT_EQ(14, CharacterCountBits(CodecMode::NUMERIC, *Version::Model2(40)));
	ASSERT_EQ(9, CharacterCountBits(CodecMode::ALPHANUMERIC, *Version::Model2(6)));
	ASSERT_EQ(8, CharacterCountBits(CodecMode::BYTE, *Version::Model2(7)));
	ASSERT_EQ(8, CharacterCountBits(CodecMode::KANJI, *Version::Model2(8)));
}

TEST(QRModeTest, MicroForBits)
{
	// M1
	ASSERT_EQ(CodecMode::NUMERIC, CodecModeForBits(0x00, Type::Micro));
	// M2
	ASSERT_EQ(CodecMode::NUMERIC, CodecModeForBits(0x00, Type::Micro));
	ASSERT_EQ(CodecMode::ALPHANUMERIC, CodecModeForBits(0x01, Type::Micro));
	// M3
	ASSERT_EQ(CodecMode::NUMERIC, CodecModeForBits(0x00, Type::Micro));
	ASSERT_EQ(CodecMode::ALPHANUMERIC, CodecModeForBits(0x01, Type::Micro));
	ASSERT_EQ(CodecMode::BYTE, CodecModeForBits(0x02, Type::Micro));
	ASSERT_EQ(CodecMode::KANJI, CodecModeForBits(0x03, Type::Micro));
	// M4
	ASSERT_EQ(CodecMode::NUMERIC, CodecModeForBits(0x00, Type::Micro));
	ASSERT_EQ(CodecMode::ALPHANUMERIC, CodecModeForBits(0x01, Type::Micro));
	ASSERT_EQ(CodecMode::BYTE, CodecModeForBits(0x02, Type::Micro));
	ASSERT_EQ(CodecMode::KANJI, CodecModeForBits(0x03, Type::Micro));

	ASSERT_THROW(CodecModeForBits(0x04, Type::Micro), Error);
}

TEST(QRModeTest, MicroCharacterCount)
{
	// Spot check a few values
	ASSERT_EQ(3, CharacterCountBits(CodecMode::NUMERIC, *Version::Micro(1)));
	ASSERT_EQ(4, CharacterCountBits(CodecMode::NUMERIC, *Version::Micro(2)));
	ASSERT_EQ(6, CharacterCountBits(CodecMode::NUMERIC, *Version::Micro(4)));
	ASSERT_EQ(3, CharacterCountBits(CodecMode::ALPHANUMERIC, *Version::Micro(2)));
	ASSERT_EQ(4, CharacterCountBits(CodecMode::BYTE, *Version::Micro(3)));
	ASSERT_EQ(4, CharacterCountBits(CodecMode::KANJI, *Version::Micro(4)));
}

TEST(QRModeTest, RMQRForBits)
{
	ASSERT_EQ(CodecMode::TERMINATOR, CodecModeForBits(0x00, Type::rMQR));
	ASSERT_EQ(CodecMode::NUMERIC, CodecModeForBits(0x01, Type::rMQR));
	ASSERT_EQ(CodecMode::ALPHANUMERIC, CodecModeForBits(0x02, Type::rMQR));
	ASSERT_EQ(CodecMode::BYTE, CodecModeForBits(0x03, Type::rMQR));
	ASSERT_EQ(CodecMode::KANJI, CodecModeForBits(0x04, Type::rMQR));
	ASSERT_EQ(CodecMode::FNC1_FIRST_POSITION, CodecModeForBits(0x05, Type::rMQR));
	ASSERT_EQ(CodecMode::FNC1_SECOND_POSITION, CodecModeForBits(0x06, Type::rMQR));
	ASSERT_EQ(CodecMode::ECI, CodecModeForBits(0x07, Type::rMQR));
	ASSERT_THROW(CodecModeForBits(0x08, Type::rMQR), Error);
}

TEST(QRModeTest, RMQRCharacterCount)
{
	// Spot check a few values
	ASSERT_EQ(7, CharacterCountBits(CodecMode::NUMERIC, *Version::rMQR(5)));
	ASSERT_EQ(8, CharacterCountBits(CodecMode::NUMERIC, *Version::rMQR(26)));
	ASSERT_EQ(9, CharacterCountBits(CodecMode::NUMERIC, *Version::rMQR(32)));
	ASSERT_EQ(5, CharacterCountBits(CodecMode::ALPHANUMERIC, *Version::rMQR(6)));
	ASSERT_EQ(5, CharacterCountBits(CodecMode::BYTE, *Version::rMQR(7)));
	ASSERT_EQ(5, CharacterCountBits(CodecMode::KANJI, *Version::rMQR(8)));
}
