/*
* Copyright 2020 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "BarcodeFormat.h"

#include "gtest/gtest.h"
#include <stdexcept>
#include <vector>

using namespace ZXing;

TEST(BarcodeFormatTest, BarcodeFormat)
{
	EXPECT_EQ(ToString(BarcodeFormat::QRCode), std::string("QRCode"));
	EXPECT_EQ(ToString(BarcodeFormat::None), std::string("None"));
	EXPECT_EQ(ToString(BarcodeFormat::DataMatrix | BarcodeFormat::EAN13), std::string("DataMatrix|EAN-13"));

	EXPECT_EQ(BarcodeFormat::EAN8, BarcodeFormatFromString("EAN_8"));
	EXPECT_EQ(BarcodeFormat::EAN8, BarcodeFormatFromString("EAN-8"));
	EXPECT_EQ(BarcodeFormat::EAN8, BarcodeFormatFromString("EAN8"));
	EXPECT_EQ(BarcodeFormat::EAN8, BarcodeFormatFromString("ean8"));
	EXPECT_EQ(BarcodeFormat::None, BarcodeFormatFromString("invalid-string"));

	EXPECT_EQ(BarcodeFormat::None, BarcodeFormatsFromString(""));

	auto formats = BarcodeFormat::EAN8 | BarcodeFormat::ITF;
	EXPECT_EQ(formats, BarcodeFormatsFromString("EAN-8,ITF"));
	EXPECT_EQ(formats, BarcodeFormatsFromString("EAN-8, ITF"));
	EXPECT_EQ(formats, BarcodeFormatsFromString("EAN-8 ITF"));
	EXPECT_EQ(formats, BarcodeFormatsFromString("ean8|itf"));

	auto f1 = std::vector<BarcodeFormat>(formats.begin(), formats.end());
	auto f2 = std::vector<BarcodeFormat>{BarcodeFormat::EAN8, BarcodeFormat::ITF};
	EXPECT_EQ(f1, f2);

	EXPECT_THROW(BarcodeFormatsFromString("ITF, invalid-string"), std::invalid_argument);
}
