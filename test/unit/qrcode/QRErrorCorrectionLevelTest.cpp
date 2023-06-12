/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2008 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "qrcode/QRErrorCorrectionLevel.h"

#include "gtest/gtest.h"

using namespace ZXing;
using namespace ZXing::QRCode;

TEST(QRErrorCorrectionLevelTest, ForBits)
{
	EXPECT_EQ(ErrorCorrectionLevel::Medium, ECLevelFromBits(0));
	EXPECT_EQ(ErrorCorrectionLevel::Low, ECLevelFromBits(1));
	EXPECT_EQ(ErrorCorrectionLevel::High, ECLevelFromBits(2));
	EXPECT_EQ(ErrorCorrectionLevel::Quality, ECLevelFromBits(3));
}

TEST(QRErrorCorrectionLevelTest, ForMicroBits)
{
	EXPECT_EQ(ErrorCorrectionLevel::Low, ECLevelFromBits(0, true));
	EXPECT_EQ(ErrorCorrectionLevel::Low, ECLevelFromBits(1, true));
	EXPECT_EQ(ErrorCorrectionLevel::Medium, ECLevelFromBits(2, true));
	EXPECT_EQ(ErrorCorrectionLevel::Low, ECLevelFromBits(3, true));
	EXPECT_EQ(ErrorCorrectionLevel::Medium, ECLevelFromBits(4, true));
	EXPECT_EQ(ErrorCorrectionLevel::Low, ECLevelFromBits(5, true));
	EXPECT_EQ(ErrorCorrectionLevel::Medium, ECLevelFromBits(6, true));
	EXPECT_EQ(ErrorCorrectionLevel::Quality, ECLevelFromBits(7, true));

	EXPECT_EQ(ErrorCorrectionLevel::Quality, ECLevelFromBits(-1, true));
	EXPECT_EQ(ErrorCorrectionLevel::Low, ECLevelFromBits(8, true));
}

TEST(QRErrorCorrectionLevelTest, ToString)
{
	using namespace std::literals;

	EXPECT_EQ("L"s, ToString(ErrorCorrectionLevel::Low));
	EXPECT_EQ("M"s, ToString(ErrorCorrectionLevel::Medium));
	EXPECT_EQ("Q"s, ToString(ErrorCorrectionLevel::Quality));
	EXPECT_EQ("H"s, ToString(ErrorCorrectionLevel::High));
}
