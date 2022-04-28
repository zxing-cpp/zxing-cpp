/*
 * Copyright 2022 KURZ Digital Solutions GmbH & Co. KG
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

#include "microqrcode/MQRFinderPatternFinder.h"

#include "BitMatrixIO.h"
#include "DecodeHints.h"

#include "gtest/gtest.h"

using namespace ZXing;
using namespace ZXing::MicroQRCode;

namespace {

BitMatrix LoadScaledCode(const int moduleSize, const int quietZone)
{
	auto bitMatrix = ParseBitMatrix("XXXXXXX X X X X\n"
									"X     X    X X \n"
									"X XXX X XXXXXXX\n"
									"X XXX X X X  XX\n"
									"X XXX X    X XX\n"
									"X     X X X X X\n"
									"XXXXXXX  X  XX \n"
									"         X X  X\n"
									"XXXXXX    X X X\n"
									"   X  XX    XXX\n"
									"XXX XX XXXX XXX\n"
									" X    X  XXX X \n"
									"X XXXXX XXX X X\n"
									" X    X  X XXX \n"
									"XXX XX X X XXXX\n",
									88, false);

	// Inflate bit matrix since corner finder does not work with pure barcodes.
	return Inflate(std::move(bitMatrix), (bitMatrix.width() + 2 * quietZone) * moduleSize,
				   (bitMatrix.height() + 2 * quietZone) * moduleSize, quietZone * moduleSize);
}

} // namespace

TEST(MicroQRFinderPatternFinderTest, FindCodeCorners)
{
	// Inflate bit matrix since corner finder does not work with pure barcodes.
	const int moduleSize = 12;
	const auto scaledBitMatrix = LoadScaledCode(moduleSize, 2);

	DecodeHints hints;
	FinderPatternFinder finder;
	const auto corners = finder.findCorners(scaledBitMatrix, hints);
	ASSERT_EQ(4, corners.size());

	ASSERT_NEAR(2 * moduleSize, corners[0].x(), moduleSize / 4);
	ASSERT_NEAR(2 * moduleSize, corners[0].y(), moduleSize / 4);

	ASSERT_NEAR((2 + 15) * moduleSize, corners[3].x(), moduleSize / 4);
	ASSERT_NEAR((2 + 15) * moduleSize, corners[3].y(), moduleSize / 4);
}

TEST(MicroQRFinderPatternFinderTest, FindPatternCenters)
{
	// Inflate bit matrix since corner finder does not work with pure barcodes.
	const int moduleSize = 12;
	const auto scaledBitMatrix = LoadScaledCode(moduleSize, 2);

	DecodeHints hints;
	FinderPatternFinder finder;
	const auto finderPatternInfo = finder.findCenters(scaledBitMatrix, hints);
	ASSERT_TRUE(finderPatternInfo);

	const float patternCenterX = (3.5 + 2.0) * moduleSize;
	const float patternCenterY = (3.5 + 2.0) * moduleSize;

	ASSERT_EQ(moduleSize, finderPatternInfo->getActualTopLeft().getEstimatedModuleSize());
	ASSERT_EQ(4, finderPatternInfo->getActualTopLeft().getCount());
	ASSERT_NEAR(patternCenterX, finderPatternInfo->getActualTopLeft().x(), moduleSize / 4);
	ASSERT_NEAR(patternCenterY, finderPatternInfo->getActualTopLeft().y(), moduleSize / 4);

	ASSERT_EQ(1, finderPatternInfo->getFakeBottomLeft().getCount());
	ASSERT_EQ(1, finderPatternInfo->getFakeTopRight().getCount());
}

TEST(MicroQRFinderPatternFinderTest, FindNoPattern)
{
	const int moduleSize = 12;
	BitMatrix bitMatrix{15, 15};
	// Inflate bit matrix since corner finder does not work with pure barcodes.
	const auto scaledBitMatrix = Inflate(std::move(bitMatrix), (bitMatrix.width() + 4) * moduleSize,
										 (bitMatrix.height() + 4) * moduleSize, 2 * moduleSize);

	DecodeHints hints;
	FinderPatternFinder finder;
	const auto centerResult = finder.findCenters(scaledBitMatrix, hints);
	ASSERT_FALSE(centerResult);
	const auto cornersResult = finder.findCorners(scaledBitMatrix, hints);
	ASSERT_TRUE(cornersResult.empty());
}

TEST(MicroQRFinderPatternFinderTest, FindPatternRotated)
{
	const int moduleSize = 12;
	auto scaledBitMatrix = LoadScaledCode(moduleSize, 2);

	DecodeHints hints;
	std::vector<std::tuple<int, int, float, float>> expectedPositions = {
		{0, 15, 3.5f, 11.5f}, {15, 15, 11.5f, 11.5f}, {15, 0, 11.5f, 3.5f}};
	for (const auto& [cornerX, cornerY, centerX, centerY] : expectedPositions) {
		// Rotate matrix 90 degrees counter-clockwise.
		scaledBitMatrix.rotate90();

		FinderPatternFinder finder;
		const auto corners = finder.findCorners(scaledBitMatrix, hints);
		ASSERT_EQ(4, corners.size());

		ASSERT_NEAR((2 + cornerX) * moduleSize, corners[0].x(), moduleSize / 4);
		ASSERT_NEAR((2 + cornerY) * moduleSize, corners[0].y(), moduleSize / 4);

		const auto centers = finder.findCenters(scaledBitMatrix, hints);
		const float patternCenterX = (centerX + 2.0f) * moduleSize;
		const float patternCenterY = (centerY + 2.0f) * moduleSize;

		ASSERT_TRUE(centers);
		ASSERT_EQ(moduleSize, centers->getActualTopLeft().getEstimatedModuleSize());
		ASSERT_NEAR(patternCenterX, centers->getActualTopLeft().x(), moduleSize / 4);
		ASSERT_NEAR(patternCenterY, centers->getActualTopLeft().y(), moduleSize / 4);
	}
}
