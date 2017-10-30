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
#include "gtest/gtest.h"
#include "qrcode/QRFormatInformation.h"

using namespace ZXing;
using namespace ZXing::QRCode;


static const int MASKED_TEST_FORMAT_INFO = 0x2BED;
static const int UNMASKED_TEST_FORMAT_INFO = MASKED_TEST_FORMAT_INFO ^ 0x5412;

TEST(QRFormatInformationTest, Decode)
{
    // Normal case
    FormatInformation expected = FormatInformation::DecodeFormatInformation(MASKED_TEST_FORMAT_INFO, MASKED_TEST_FORMAT_INFO);
    EXPECT_TRUE(expected.isValid());
    EXPECT_EQ(0x07, expected.dataMask());
	EXPECT_EQ(ErrorCorrectionLevel::Quality, expected.errorCorrectionLevel());
    // where the code forgot the mask!
	EXPECT_EQ(expected, FormatInformation::DecodeFormatInformation(UNMASKED_TEST_FORMAT_INFO, MASKED_TEST_FORMAT_INFO));
}

TEST(QRFormatInformationTest, DecodeWithBitDifference)
{
    FormatInformation expected = FormatInformation::DecodeFormatInformation(MASKED_TEST_FORMAT_INFO, MASKED_TEST_FORMAT_INFO);
    // 1,2,3,4 bits difference
	EXPECT_EQ(expected, FormatInformation::DecodeFormatInformation(MASKED_TEST_FORMAT_INFO ^ 0x01, MASKED_TEST_FORMAT_INFO ^ 0x01));
	EXPECT_EQ(expected, FormatInformation::DecodeFormatInformation(MASKED_TEST_FORMAT_INFO ^ 0x03, MASKED_TEST_FORMAT_INFO ^ 0x03));
	EXPECT_EQ(expected, FormatInformation::DecodeFormatInformation(MASKED_TEST_FORMAT_INFO ^ 0x07, MASKED_TEST_FORMAT_INFO ^ 0x07));
	EXPECT_TRUE(!FormatInformation::DecodeFormatInformation(MASKED_TEST_FORMAT_INFO ^ 0x0F, MASKED_TEST_FORMAT_INFO ^ 0x0F).isValid());
}

TEST(QRFormatInformationTest, DecodeWithMisread)
{
    FormatInformation expected = FormatInformation::DecodeFormatInformation(MASKED_TEST_FORMAT_INFO, MASKED_TEST_FORMAT_INFO);
	EXPECT_EQ(expected, FormatInformation::DecodeFormatInformation(MASKED_TEST_FORMAT_INFO ^ 0x03, MASKED_TEST_FORMAT_INFO ^ 0x0F));
}
