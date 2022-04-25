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

#include "microqrcode/detector/CornerFinder.h"

#include "BitMatrixIO.h"
#include "DecodeHints.h"

#include "gtest/gtest.h"

using namespace ZXing;
using namespace ZXing::MicroQRCode;

namespace
{

BitMatrix LoadScaledCode(const int moduleSize, const int quietZone)
{
	auto bitMatrix = ParseBitMatrix(
		"XXXXXXX X X X X\n"
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
		88,
		false
	);

	// Inflate bit matrix since corner finder does not work with pure barcodes.
	return Inflate(std::move(bitMatrix), (bitMatrix.width() + 2 * quietZone) * moduleSize, (bitMatrix.height() + 2 * quietZone) * moduleSize, quietZone * moduleSize);
}

}

TEST(MicroQRCornerFinderTest, FindCorners)
{
	const int patternCenterX = 3;
	const int patternCenterY = 3;

	// Inflate bit matrix since corner finder does not work with pure barcodes.
	const int moduleSize = 12;
	const auto scaledBitMatrix = LoadScaledCode(moduleSize, 2);

	// Create expected center of finder pattern taking into account quiet zones (+2).
	const FinderPattern patternPosition{(patternCenterX + 2) * moduleSize, (patternCenterX + 2) * moduleSize, moduleSize};
	const CornerFinder finder{scaledBitMatrix, patternPosition };
	const auto corners = finder.find();

	ASSERT_EQ(4, corners.size());
	ASSERT_NEAR(2 * moduleSize, corners[0].x(), moduleSize / 4);
	ASSERT_NEAR(2 * moduleSize, corners[0].y(), moduleSize / 4);

	ASSERT_NEAR((2 + 15) * moduleSize, corners[3].x(), moduleSize / 4);
	ASSERT_NEAR((2 + 15) * moduleSize, corners[3].y(), moduleSize / 4);
 }


TEST(MicroQRCornerFinderTest, FindNoCornersInEmptyBitMatrix)
{
	BitMatrix bitMatrix{ 15, 15 };

	const int patternCenterX = 3;
	const int patternCenterY = 3;

	// Inflate bit matrix since corner finder does not work with pure barcodes.
	const int moduleSize = 12;
	const auto scaledBitMatrix = Inflate(std::move(bitMatrix), (bitMatrix.width() + 4) * moduleSize, (bitMatrix.height() + 4) * moduleSize, 2 * moduleSize);

	// Create expected center of finder pattern taking into account quiet zones (+2).
	const FinderPattern patternPosition{ (patternCenterX + 2) * moduleSize, (patternCenterX + 2) * moduleSize, moduleSize };
	const CornerFinder finder{ scaledBitMatrix, patternPosition };
	const auto corners = finder.find();

	ASSERT_EQ(0, corners.size());
}

TEST(MicroQRCornerFinderTest, FindNoCornersWithNoQuietZone)
{
	const int patternCenterX = 3;
	const int patternCenterY = 3;

	// Inflate bit matrix since corner finder does not work with pure barcodes.
	const int moduleSize = 12;
	const auto scaledBitMatrix = LoadScaledCode(moduleSize, 0);

	// Create expected center of finder pattern.
	const FinderPattern patternPosition{ patternCenterX * moduleSize, patternCenterX * moduleSize, moduleSize };
	const CornerFinder finder{ scaledBitMatrix, patternPosition };
	const auto corners = finder.find();

	ASSERT_EQ(0, corners.size());
}
