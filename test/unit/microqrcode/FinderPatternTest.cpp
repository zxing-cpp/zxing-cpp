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

#include "microqrcode/detector/FinderPatternInfo.h"

#include "gtest/gtest.h"

using namespace ZXing;
using namespace ZXing::MicroQRCode;

TEST(MicroQRFinderPatternTest, ConstructFinderPatternInfo)
{
	std::vector<FinderPattern> testPatterns;
	testPatterns.push_back(FinderPattern{ 5.0f, 5.0f, 13.0f });
	testPatterns.push_back(FinderPattern{ 6.0f, 6.0f, 13.0f });
	testPatterns.push_back(FinderPattern{ 7.0f, 7.0f, 13.0f });

	const FinderPatternInfo testPatternInfo{ testPatterns };
	ASSERT_EQ(5.0f, testPatternInfo.getActualTopLeft().x());
	ASSERT_EQ(5.0f, testPatternInfo.getActualTopLeft().y());
	ASSERT_EQ(1, testPatternInfo.getActualTopLeft().getCount());
	ASSERT_EQ(13.0f, testPatternInfo.getActualTopLeft().getEstimatedModuleSize());

	ASSERT_EQ(6.0f, testPatternInfo.getFakeTopRight().x());
	ASSERT_EQ(6.0f, testPatternInfo.getFakeTopRight().y());
	ASSERT_EQ(1, testPatternInfo.getFakeTopRight().getCount());
	ASSERT_EQ(13.0f, testPatternInfo.getFakeTopRight().getEstimatedModuleSize());

	ASSERT_EQ(7.0f, testPatternInfo.getFakeBottomLeft().x());
	ASSERT_EQ(7.0f, testPatternInfo.getFakeBottomLeft().y());
	ASSERT_EQ(1, testPatternInfo.getFakeBottomLeft().getCount());
	ASSERT_EQ(13.0f, testPatternInfo.getFakeBottomLeft().getEstimatedModuleSize());
}

TEST(MicroQRFinderPatternTest, CombineFinderPatterns)
{
	auto original = FinderPattern{ 5.0f, 5.0f, 13.0f };
	const auto tooFarAway = FinderPattern{ 500.0f, 500.0f, 13.0f };
	ASSERT_FALSE(original.aboutEquals(tooFarAway.getEstimatedModuleSize(), tooFarAway.y(), tooFarAway.x()));

	const auto closeEnough = FinderPattern{ 10.0f, 10.0f, 13.0f };
	ASSERT_TRUE(original.aboutEquals(closeEnough.getEstimatedModuleSize(), closeEnough.y(), closeEnough.x()));

	const auto combined = original.combineEstimate(closeEnough.y(), closeEnough.x(), closeEnough.getEstimatedModuleSize());
	ASSERT_EQ(2, combined.getCount());
	ASSERT_EQ(7.5f, combined.x());
	ASSERT_EQ(7.5f, combined.y());
	ASSERT_EQ(13.0f, combined.getEstimatedModuleSize());
}
