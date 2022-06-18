/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2007 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "qrcode/QRFormatInformation.h"

#include "gtest/gtest.h"

using namespace ZXing;
using namespace ZXing::QRCode;

static const int MASKED_TEST_FORMAT_INFO = 0x2BED;
static const int UNMASKED_TEST_FORMAT_INFO = MASKED_TEST_FORMAT_INFO ^ 0x5412;
static const int MICRO_MASKED_TEST_FORMAT_INFO = 0x3BBA;
static const int MICRO_UNMASKED_TEST_FORMAT_INFO = MICRO_MASKED_TEST_FORMAT_INFO ^ 0x4445;

static void DoFormatInformationTest(const int formatInfo, const uint8_t expectedMask, const ErrorCorrectionLevel& expectedECL)
{
	FormatInformation parsedFormat = FormatInformation::DecodeMQR(formatInfo);
	EXPECT_TRUE(parsedFormat.isValid());
	EXPECT_EQ(expectedMask, parsedFormat.dataMask);
	EXPECT_EQ(expectedECL, parsedFormat.ecLevel);
}

TEST(QRFormatInformationTest, Decode)
{
    // Normal case
    FormatInformation expected = FormatInformation::DecodeQR(MASKED_TEST_FORMAT_INFO, MASKED_TEST_FORMAT_INFO);
    EXPECT_TRUE(expected.isValid());
    EXPECT_EQ(0x07, expected.dataMask);
	EXPECT_EQ(ErrorCorrectionLevel::Quality, expected.ecLevel);
    // where the code forgot the mask!
	EXPECT_EQ(expected, FormatInformation::DecodeQR(UNMASKED_TEST_FORMAT_INFO, MASKED_TEST_FORMAT_INFO));
}

TEST(QRFormatInformationTest, DecodeWithBitDifference)
{
    FormatInformation expected = FormatInformation::DecodeQR(MASKED_TEST_FORMAT_INFO, MASKED_TEST_FORMAT_INFO);
    // 1,2,3,4 bits difference
	EXPECT_EQ(expected, FormatInformation::DecodeQR(MASKED_TEST_FORMAT_INFO ^ 0x01, MASKED_TEST_FORMAT_INFO ^ 0x01));
	EXPECT_EQ(expected, FormatInformation::DecodeQR(MASKED_TEST_FORMAT_INFO ^ 0x03, MASKED_TEST_FORMAT_INFO ^ 0x03));
	EXPECT_EQ(expected, FormatInformation::DecodeQR(MASKED_TEST_FORMAT_INFO ^ 0x07, MASKED_TEST_FORMAT_INFO ^ 0x07));
	EXPECT_TRUE(!FormatInformation::DecodeQR(MASKED_TEST_FORMAT_INFO ^ 0x0F, MASKED_TEST_FORMAT_INFO ^ 0x0F).isValid());
}

TEST(QRFormatInformationTest, DecodeWithMisread)
{
    FormatInformation expected = FormatInformation::DecodeQR(MASKED_TEST_FORMAT_INFO, MASKED_TEST_FORMAT_INFO);
	EXPECT_EQ(expected, FormatInformation::DecodeQR(MASKED_TEST_FORMAT_INFO ^ 0x03, MASKED_TEST_FORMAT_INFO ^ 0x0F));
}

TEST(QRFormatInformationTest, DecodeMicro)
{
	// Normal cases.
	DoFormatInformationTest(0x4445, 0x0, ErrorCorrectionLevel::Low);
	DoFormatInformationTest(0x4172, 0x1, ErrorCorrectionLevel::Low);
	DoFormatInformationTest(0x5fc0, 0x2, ErrorCorrectionLevel::Low);
	DoFormatInformationTest(0x5af7, 0x3, ErrorCorrectionLevel::Low);
	DoFormatInformationTest(0x6793, 0x0, ErrorCorrectionLevel::Medium);
	DoFormatInformationTest(0x62a4, 0x1, ErrorCorrectionLevel::Medium);
	DoFormatInformationTest(0x3e8d, 0x2, ErrorCorrectionLevel::Quality);
	DoFormatInformationTest(MICRO_MASKED_TEST_FORMAT_INFO, 0x3, ErrorCorrectionLevel::Quality);

	// where the code forgot the mask!
//	DoFormatInformationTest(MICRO_UNMASKED_TEST_FORMAT_INFO, 0x3, ErrorCorrectionLevel::Quality);
}

// This doesn't work as expected because the implementation of the decode tries with
// and without the mask (0x4445).  This effectively adds a tolerance of 5 bits to the Hamming
// distance calculation.
TEST(QRFormatInformationTest, DecodeMicroWithBitDifference)
{
	FormatInformation expected = FormatInformation::DecodeMQR(MICRO_MASKED_TEST_FORMAT_INFO);

	// 1,2,3 bits difference
	EXPECT_EQ(expected, FormatInformation::DecodeMQR(MICRO_MASKED_TEST_FORMAT_INFO ^ 0x01));
	EXPECT_EQ(expected, FormatInformation::DecodeMQR(MICRO_MASKED_TEST_FORMAT_INFO ^ 0x03));
	EXPECT_EQ(expected, FormatInformation::DecodeMQR(MICRO_MASKED_TEST_FORMAT_INFO ^ 0x07));

	// Bigger bit differences can return valid FormatInformation objects but the data mask and error
	// correction levels do not match.
//	EXPECT_TRUE(FormatInformation::DecodeFormatInformation(MICRO_MASKED_TEST_FORMAT_INFO ^ 0x3f).isValid());
//	EXPECT_NE(expected.dataMask(), FormatInformation::DecodeFormatInformation(MICRO_MASKED_TEST_FORMAT_INFO ^ 0x3f).dataMask());
//	EXPECT_NE(expected.errorCorrectionLevel(),
//			  FormatInformation::DecodeFormatInformation(MICRO_MASKED_TEST_FORMAT_INFO ^ 0x3f).errorCorrectionLevel());
}
