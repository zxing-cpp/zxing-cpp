/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2008 ZXing authors
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

#include "qrcode/QRVersion.h"

#include "BitMatrix.h"

#include "gtest/gtest.h"

using namespace ZXing;
using namespace ZXing::QRCode;

namespace {

	void CheckVersion(const Version* version, int number, int dimension) {
		ASSERT_NE(version, nullptr);
		EXPECT_EQ(number, version->versionNumber());
		if (number > 1 && !version->isMicroQRCode()) {
			EXPECT_FALSE(version->alignmentPatternCenters().empty());
		}
		EXPECT_EQ(dimension, version->dimensionForVersion());
	}

	void DoTestVersion(int expectedVersion, int mask) {
		auto version = Version::DecodeVersionInformation(mask);
		ASSERT_NE(version, nullptr);
		EXPECT_EQ(expectedVersion, version->versionNumber());
	}

}

TEST(QRVersionTest, VersionForNumber)
{
    auto version = Version::VersionForNumber(0);
    EXPECT_EQ(version, nullptr) << "There is version with number 0";

	for (int i = 1; i <= 40; i++) {
		CheckVersion(Version::VersionForNumber(i), i, 4*i + 17);
    }
}


TEST(QRVersionTest, GetProvisionalVersionForDimension)
{
    for (int i = 1; i <= 40; i++) {
		auto prov = Version::ProvisionalVersionForDimension(4 * i + 17);
		ASSERT_NE(prov, nullptr);
		EXPECT_EQ(i, prov->versionNumber());
    }
}

TEST(QRVersionTest, DecodeVersionInformation)
{
    // Spot check
    DoTestVersion(7, 0x07C94);
    DoTestVersion(12, 0x0C762);
    DoTestVersion(17, 0x1145D);
    DoTestVersion(22, 0x168C9);
    DoTestVersion(27, 0x1B08E);
    DoTestVersion(32, 0x209D5);
}
  
TEST(QRVersionTest, MicroVersionForNumber)
{
	auto version = Version::VersionForNumber(0, true);
	EXPECT_EQ(version, nullptr) << "There is version with number 0";

	for (int i = 1; i <= 4; i++) {
		CheckVersion(Version::VersionForNumber(i, true), i, 2 * i + 9);
	}
}

TEST(QRVersionTest, GetProvisionalMicroVersionForDimension)
{
	for (int i = 1; i <= 4; i++) {
		auto prov = Version::ProvisionalVersionForDimension(2 * i + 9, true);
		ASSERT_NE(prov, nullptr);
		EXPECT_EQ(i, prov->versionNumber());
	}
}

TEST(QRVersionTest, FunctionPattern)
{
	auto testFinderPatternRegion = [](const BitMatrix& bitMatrix) {
		for (int row = 0; row < 9; row++)
			for (int col = 0; col < 9; col++)
				EXPECT_TRUE(bitMatrix.get(col, row));
	};
	for (int i = 1; i <= 4; i++) {
		const auto version = Version::VersionForNumber(i, true);
		const auto functionPattern = version->buildFunctionPattern();
		testFinderPatternRegion(functionPattern);

		// Check timing pattern areas.
		const auto dimension = version->dimensionForVersion();
		for (int row = dimension; row < functionPattern.height(); row++)
			EXPECT_TRUE(functionPattern.get(0, row));
		for (int col = dimension; col < functionPattern.width(); col++)
			EXPECT_TRUE(functionPattern.get(col, 0));
	}
}
