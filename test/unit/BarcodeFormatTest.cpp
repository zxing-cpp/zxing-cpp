/*
* Copyright 2020 Axel Waggershauser
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

#include "BarcodeFormat.h"
#include <vector>
#include <stdexcept>

using namespace ZXing;

TEST(BarcodeFormatTest, BarcodeFormat)
{
	EXPECT_EQ(ToString(BarcodeFormat::QR_CODE), std::string("QR_CODE"));
	EXPECT_EQ(ToString(BarcodeFormat::NONE), std::string("NONE"));

	EXPECT_EQ(BarcodeFormat::EAN_8, BarcodeFormatFromString("EAN_8"));
	EXPECT_EQ(BarcodeFormat::EAN_8, BarcodeFormatFromString("EAN8"));
	EXPECT_EQ(BarcodeFormat::EAN_8, BarcodeFormatFromString("ean8"));
	EXPECT_EQ(BarcodeFormat::NONE, BarcodeFormatFromString("invalid-string"));

	EXPECT_EQ(BarcodeFormat::NONE, BarcodeFormatsFromString(""));

	auto formats = BarcodeFormat::EAN_8 | BarcodeFormat::ITF;
	EXPECT_EQ(formats, BarcodeFormatsFromString("EAN_8,ITF"));
	EXPECT_EQ(formats, BarcodeFormatsFromString("EAN_8, ITF"));
	EXPECT_EQ(formats, BarcodeFormatsFromString("EAN_8 ITF"));
	EXPECT_EQ(formats, BarcodeFormatsFromString("ean8|itf"));

	auto f1 = std::vector<BarcodeFormat>(formats.begin(), formats.end());
	auto f2 = std::vector<BarcodeFormat>{BarcodeFormat::EAN_8, BarcodeFormat::ITF};
	EXPECT_EQ(f1, f2);

	EXPECT_THROW(BarcodeFormatsFromString("ITF, invalid-string"), std::invalid_argument);
}
