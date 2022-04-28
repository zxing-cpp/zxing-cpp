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

#include "microqrcode/MQRDimension.h"

#include "gtest/gtest.h"

using namespace ZXing;
using namespace ZXing::MicroQRCode;

TEST(MicroQRDimensionTest, ComputeRoundUp)
{
	ASSERT_EQ(11, Dimension::computeRoundUp(-1));
	ASSERT_EQ(11, Dimension::computeRoundUp(11));
	ASSERT_EQ(13, Dimension::computeRoundUp(12));
	ASSERT_EQ(13, Dimension::computeRoundUp(13));
	ASSERT_EQ(15, Dimension::computeRoundUp(14));
	ASSERT_EQ(15, Dimension::computeRoundUp(15));
	ASSERT_EQ(17, Dimension::computeRoundUp(16));
	ASSERT_EQ(17, Dimension::computeRoundUp(20));
}

TEST(MicroQRDimensionTest, ComputeRoundOff)
{
	ASSERT_EQ(11, Dimension::computeRoundOff(11));
	ASSERT_EQ(13, Dimension::computeRoundOff(12));
	ASSERT_EQ(13, Dimension::computeRoundOff(13));
	ASSERT_EQ(13, Dimension::computeRoundOff(14));
	ASSERT_EQ(15, Dimension::computeRoundOff(15));
}
