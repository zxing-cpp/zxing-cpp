/*
 * Copyright 2017 Huy Cuong Nguyen
 * Copyright 2007 ZXing authors
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

#include "qrcode/MQRFormatInformationFactory.h"
#include "qrcode/QRErrorCorrectionLevel.h"

#include "gtest/gtest.h"

using namespace ZXing::QRCode;

namespace ZXing {

namespace MicroQRCode {

static const int MASKED_TEST_FORMAT_INFO = 0x3BBA;
static const int UNMASKED_TEST_FORMAT_INFO = MASKED_TEST_FORMAT_INFO ^ 0x4445;

namespace {

void DoFormatInformationTest(const int formatInfo, const uint8_t expectedMask, const ErrorCorrectionLevel& expectedECL)
{
	FormatInformation parsedFormat = DecodeFormatInformation(formatInfo);
	EXPECT_TRUE(parsedFormat.isValid());
	EXPECT_EQ(expectedMask, parsedFormat.dataMask());
	EXPECT_EQ(expectedECL, parsedFormat.errorCorrectionLevel());
}

} // namespace

TEST(MQRFormatInformationTest, Decode)
{
	// Normal cases.
	DoFormatInformationTest(0x4445, 0x0, ErrorCorrectionLevel::Low);
	DoFormatInformationTest(0x4172, 0x1, ErrorCorrectionLevel::Low);
	DoFormatInformationTest(0x5fc0, 0x2, ErrorCorrectionLevel::Low);
	DoFormatInformationTest(0x5af7, 0x3, ErrorCorrectionLevel::Low);
	DoFormatInformationTest(0x6793, 0x0, ErrorCorrectionLevel::Medium);
	DoFormatInformationTest(0x62a4, 0x1, ErrorCorrectionLevel::Medium);
	DoFormatInformationTest(0x3e8d, 0x2, ErrorCorrectionLevel::Quality);
	DoFormatInformationTest(MASKED_TEST_FORMAT_INFO, 0x3, ErrorCorrectionLevel::Quality);

	// where the code forgot the mask!
	DoFormatInformationTest(UNMASKED_TEST_FORMAT_INFO, 0x3, ErrorCorrectionLevel::Quality);
}

// This doesn't work as expected because the implementation of the decode tries with
// and without the mask (0x4445).  This effectively adds a tolerance of 5 bits to the Hamming
// distance calculation.
TEST(MQRFormatInformationTest, DecodeWithBitDifference)
{
	FormatInformation expected = DecodeFormatInformation(MASKED_TEST_FORMAT_INFO);

	// 1,2,3 bits difference
	EXPECT_EQ(expected, DecodeFormatInformation(MASKED_TEST_FORMAT_INFO ^ 0x01));
	EXPECT_EQ(expected, DecodeFormatInformation(MASKED_TEST_FORMAT_INFO ^ 0x03));
	EXPECT_EQ(expected, DecodeFormatInformation(MASKED_TEST_FORMAT_INFO ^ 0x07));

	// Bigger bit differences can return valid FormatInformation objects but the data mask and error
	// correction levels do not match.
	EXPECT_TRUE(DecodeFormatInformation(MASKED_TEST_FORMAT_INFO ^ 0x0f).isValid());
	EXPECT_NE(expected.dataMask(), DecodeFormatInformation(MASKED_TEST_FORMAT_INFO ^ 0x0f).dataMask());
	EXPECT_NE(expected.errorCorrectionLevel(),
			  DecodeFormatInformation(MASKED_TEST_FORMAT_INFO ^ 0x0f).errorCorrectionLevel());
}

} // namespace MicroQRCode

} // namespace ZXing
