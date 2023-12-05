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
static const int MASKED_TEST_FORMAT_INFO2 = ((0x2BED << 1) & 0b1111111000000000) | 0b100000000 | (0x2BED & 0b11111111); // insert the 'Dark Module'
static const int UNMASKED_TEST_FORMAT_INFO = MASKED_TEST_FORMAT_INFO ^ 0x5412;
static const int MICRO_MASKED_TEST_FORMAT_INFO = 0x3BBA;
static const int RMQR_MASKED_TEST_FORMAT_INFO = 0x20137;
static const int RMQR_MASKED_TEST_FORMAT_INFO_SUB = 0x1F1FE;

static void DoFormatInformationTest(const int formatInfo, const uint8_t expectedMask, const ErrorCorrectionLevel& expectedECL)
{
	FormatInformation parsedFormat = FormatInformation::DecodeMQR(formatInfo);
	EXPECT_TRUE(parsedFormat.isValid());
	EXPECT_EQ(expectedMask, parsedFormat.dataMask);
	EXPECT_EQ(expectedECL, parsedFormat.ecLevel);
}

// Helper for rMQR to unset `numBits` number of bits
static uint32_t RMQRUnsetBits(uint32_t formatInfoBits, int numBits)
{
	for (int i = 0; i < 18 && numBits; i++) {
		if (formatInfoBits & (1 << i)) {
			formatInfoBits ^= 1 << i;
			numBits--;
		}
	}
	return formatInfoBits;
}

TEST(QRFormatInformationTest, Decode)
{
	// Normal case
	FormatInformation expected = FormatInformation::DecodeQR(MASKED_TEST_FORMAT_INFO, MASKED_TEST_FORMAT_INFO2);
	EXPECT_TRUE(expected.isValid());
	EXPECT_EQ(0x07, expected.dataMask);
	EXPECT_EQ(ErrorCorrectionLevel::Quality, expected.ecLevel);
	// where the code forgot the mask!
	EXPECT_EQ(expected, FormatInformation::DecodeQR(UNMASKED_TEST_FORMAT_INFO, MASKED_TEST_FORMAT_INFO2));
}

TEST(QRFormatInformationTest, DecodeWithBitDifference)
{
	FormatInformation expected = FormatInformation::DecodeQR(MASKED_TEST_FORMAT_INFO, MASKED_TEST_FORMAT_INFO2);
	// 1,2,3,4 bits difference
	EXPECT_EQ(expected, FormatInformation::DecodeQR(MASKED_TEST_FORMAT_INFO ^ 0x01, MASKED_TEST_FORMAT_INFO2 ^ 0x01));
	EXPECT_EQ(expected, FormatInformation::DecodeQR(MASKED_TEST_FORMAT_INFO ^ 0x03, MASKED_TEST_FORMAT_INFO2 ^ 0x03));
	EXPECT_EQ(expected, FormatInformation::DecodeQR(MASKED_TEST_FORMAT_INFO ^ 0x07, MASKED_TEST_FORMAT_INFO2 ^ 0x07));
	auto unexpected = FormatInformation::DecodeQR(MASKED_TEST_FORMAT_INFO ^ 0x0F, MASKED_TEST_FORMAT_INFO2 ^ 0x0F);
	EXPECT_FALSE(expected == unexpected);
	EXPECT_FALSE(unexpected.isValid() && unexpected.type() == Type::Model2);
}

TEST(QRFormatInformationTest, DecodeWithMisread)
{
	FormatInformation expected = FormatInformation::DecodeQR(MASKED_TEST_FORMAT_INFO, MASKED_TEST_FORMAT_INFO2);
	EXPECT_EQ(expected, FormatInformation::DecodeQR(MASKED_TEST_FORMAT_INFO ^ 0x03, MASKED_TEST_FORMAT_INFO2 ^ 0x0F));
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
//	static const int MICRO_UNMASKED_TEST_FORMAT_INFO = MICRO_MASKED_TEST_FORMAT_INFO ^ 0x4445;
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

TEST(QRFormatInformationTest, DecodeRMQR)
{
	// Normal case
	FormatInformation expected = FormatInformation::DecodeRMQR(RMQR_MASKED_TEST_FORMAT_INFO, RMQR_MASKED_TEST_FORMAT_INFO_SUB);
	EXPECT_TRUE(expected.isValid());
	EXPECT_EQ(4, expected.dataMask);
	EXPECT_EQ(ErrorCorrectionLevel::High, expected.ecLevel);
	EXPECT_EQ(FORMAT_INFO_MASK_RMQR, expected.mask);
	// Not catered for: where the code forgot the mask!
}

TEST(QRFormatInformationTest, DecodeRMQRWithBitDifference)
{
	FormatInformation expected = FormatInformation::DecodeRMQR(RMQR_MASKED_TEST_FORMAT_INFO, RMQR_MASKED_TEST_FORMAT_INFO_SUB);
	EXPECT_EQ(expected.ecLevel, ErrorCorrectionLevel::High);
	// 1,2,3,4,5 bits difference
	EXPECT_EQ(expected, FormatInformation::DecodeRMQR(RMQRUnsetBits(RMQR_MASKED_TEST_FORMAT_INFO, 1), RMQRUnsetBits(RMQR_MASKED_TEST_FORMAT_INFO_SUB, 1)));
	EXPECT_EQ(expected, FormatInformation::DecodeRMQR(RMQRUnsetBits(RMQR_MASKED_TEST_FORMAT_INFO, 2), RMQRUnsetBits(RMQR_MASKED_TEST_FORMAT_INFO_SUB, 2)));
	EXPECT_EQ(expected, FormatInformation::DecodeRMQR(RMQRUnsetBits(RMQR_MASKED_TEST_FORMAT_INFO, 3), RMQRUnsetBits(RMQR_MASKED_TEST_FORMAT_INFO_SUB, 3)));
	EXPECT_EQ(expected, FormatInformation::DecodeRMQR(RMQRUnsetBits(RMQR_MASKED_TEST_FORMAT_INFO, 4), RMQRUnsetBits(RMQR_MASKED_TEST_FORMAT_INFO_SUB, 4)));
	auto unexpected = FormatInformation::DecodeRMQR(RMQRUnsetBits(RMQR_MASKED_TEST_FORMAT_INFO, 5), RMQRUnsetBits(RMQR_MASKED_TEST_FORMAT_INFO_SUB, 5));
	EXPECT_FALSE(expected == unexpected);
	EXPECT_FALSE(unexpected.isValid());
	EXPECT_TRUE(unexpected.type() == Type::rMQR); // Note `mask` (used to determine type) set regardless
}

TEST(QRFormatInformationTest, DecodeRMQRWithMisread)
{
	FormatInformation expected = FormatInformation::DecodeRMQR(RMQR_MASKED_TEST_FORMAT_INFO, RMQR_MASKED_TEST_FORMAT_INFO_SUB);
	{
		auto actual = FormatInformation::DecodeRMQR(RMQRUnsetBits(RMQR_MASKED_TEST_FORMAT_INFO, 2), RMQRUnsetBits(RMQR_MASKED_TEST_FORMAT_INFO_SUB, 4));
		EXPECT_EQ(expected, actual);
		EXPECT_EQ(actual.mask, FORMAT_INFO_MASK_RMQR);
	}
	{
		auto actual = FormatInformation::DecodeRMQR(RMQRUnsetBits(RMQR_MASKED_TEST_FORMAT_INFO, 5), RMQRUnsetBits(RMQR_MASKED_TEST_FORMAT_INFO_SUB, 4));
		EXPECT_EQ(expected, actual);
		EXPECT_EQ(actual.mask, FORMAT_INFO_MASK_RMQR_SUB);
	}
}
