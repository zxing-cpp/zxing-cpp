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
#if 0 // old API
	EXPECT_EQ(ToString(BarcodeFormat::QRCode), "QRCode");
	EXPECT_EQ(ToString(BarcodeFormat::None), "None");
	EXPECT_EQ(ToString(BarcodeFormat::DataMatrix | BarcodeFormat::EAN13), "DataMatrix|EAN-13");

	EXPECT_EQ(BarcodeFormat::EAN8, BarcodeFormatFromString("EAN_8"));
	EXPECT_EQ(BarcodeFormat::EAN8, BarcodeFormatFromString("EAN-8"));
	EXPECT_EQ(BarcodeFormat::EAN8, BarcodeFormatFromString("EAN8"));
	EXPECT_EQ(BarcodeFormat::EAN8, BarcodeFormatFromString("ean8"));
	EXPECT_EQ(BarcodeFormat::None, BarcodeFormatFromString("invalid-string"));

	EXPECT_EQ(BarcodeFormat::None, BarcodeFormatsFromString(""));

	auto formats = BarcodeFormat::EAN8 | BarcodeFormat::ITF;
	EXPECT_EQ(formats.count(), 2);
	EXPECT_EQ(formats, BarcodeFormatsFromString("EAN-8,ITF"));
	EXPECT_EQ(formats, BarcodeFormatsFromString("EAN-8, ITF"));
	EXPECT_EQ(formats, BarcodeFormatsFromString("EAN-8 ITF"));
	EXPECT_EQ(formats, BarcodeFormatsFromString("ean8|itf"));
	EXPECT_EQ(formats, BarcodeFormatsFromString("[EAN-8,, ITF]"));

#else

	EXPECT_EQ(BarcodeFormat::None, BarcodeFormat(0));
	EXPECT_EQ(Name(BarcodeFormat::None), "None");

#if ZXING_ENABLE_1D
	EXPECT_EQ(Symbology(BarcodeFormat::DataBarLtd), BarcodeFormat::DataBar);
	EXPECT_EQ(Name(BarcodeFormat::DataBarLtd), "DataBar Limited");
	EXPECT_EQ(ToString(BarcodeFormat::EAN13 | BarcodeFormat::DataBarLtd), "EAN-13, DataBar Limited");

	EXPECT_EQ(BarcodeFormat::EAN8, BarcodeFormatFromString("EAN_8"));
	EXPECT_EQ(BarcodeFormat::EAN8, BarcodeFormatFromString("EAN-8"));
	EXPECT_EQ(BarcodeFormat::EAN8, BarcodeFormatFromString("EAN 8"));
	EXPECT_EQ(BarcodeFormat::EAN8, BarcodeFormatFromString("EAN8"));
	EXPECT_EQ(BarcodeFormat::EAN8, BarcodeFormatFromString("ean8"));

	EXPECT_EQ(BarcodeFormat::EANUPC, BarcodeFormatFromString("EAN/UPC"));
	EXPECT_EQ(BarcodeFormat::EANUPC, BarcodeFormatFromString("EAN / UPC"));

	EXPECT_THROW(BarcodeFormatFromString("invalid-string"), std::invalid_argument);

	EXPECT_TRUE(BarcodeFormats("").empty());

	BarcodeFormats formats = BarcodeFormat::EAN8 | BarcodeFormat::ITF;
	EXPECT_EQ(formats.size(), 2);
	EXPECT_EQ(formats, BarcodeFormats("EAN-8,ITF"));
	EXPECT_EQ(formats, BarcodeFormats("EAN-8, ITF"));
	EXPECT_EQ(formats, BarcodeFormats("ean8|itf"));
	EXPECT_EQ(formats, BarcodeFormats("[EAN-8,, ITF]"));
#endif

#endif // 0

	auto f1 = std::vector<BarcodeFormat>(formats.begin(), formats.end());
	auto f2 = std::vector<BarcodeFormat>{BarcodeFormat::EAN8, BarcodeFormat::ITF};
	EXPECT_EQ(f1, f2);

	EXPECT_THROW(BarcodeFormatsFromString("ITF, invalid-string"), std::invalid_argument);
}
