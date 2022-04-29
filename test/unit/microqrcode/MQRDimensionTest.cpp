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
	ASSERT_EQ(11, RoundUpDimension(-1));
	ASSERT_EQ(11, RoundUpDimension(11));
	ASSERT_EQ(13, RoundUpDimension(12));
	ASSERT_EQ(13, RoundUpDimension(13));
	ASSERT_EQ(15, RoundUpDimension(14));
	ASSERT_EQ(15, RoundUpDimension(15));
	ASSERT_EQ(17, RoundUpDimension(16));
	ASSERT_EQ(17, RoundUpDimension(20));
}

TEST(MicroQRDimensionTest, ComputeRoundOff)
{
	ASSERT_EQ(11, RoundOffDimension(11));
	ASSERT_EQ(13, RoundOffDimension(12));
	ASSERT_EQ(13, RoundOffDimension(13));
	ASSERT_EQ(13, RoundOffDimension(14));
	ASSERT_EQ(15, RoundOffDimension(15));
}
