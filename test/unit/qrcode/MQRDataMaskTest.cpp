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

#include "qrcode/MQRDataMask.h"

#include "BitMatrix.h"

#include "gtest/gtest.h"
#include <functional>

using namespace ZXing;
using namespace ZXing::MicroQRCode;

namespace {

void TestMaskAcrossDimensions(int maskIndex, std::function<bool(int, int)> condition)
{
	for (int version = 1; version <= 4; version++) {
		int dimension = 9 + 2 * version;
		BitMatrix bits(dimension);

		for (int i = 0; i < dimension; i++)
			for (int j = 0; j < dimension; j++)
				EXPECT_EQ(GetMaskedBit(bits, j, i, maskIndex), condition(i, j)) << "(" << i << ',' << j << ')';
	}
}

} // namespace

TEST(MQRDataMaskTest, Mask0)
{
	TestMaskAcrossDimensions(0, [](int i, int) { return i % 2 == 0; });
}

TEST(MQRDataMaskTest, Mask1)
{
	TestMaskAcrossDimensions(1, [](int i, int j) { return (i / 2 + j / 3) % 2 == 0; });
}

TEST(MQRDataMaskTest, Mask2)
{
	TestMaskAcrossDimensions(2, [](int i, int j) { return ((i * j) % 2 + (i * j) % 3) % 2 == 0; });
}

TEST(MQRDataMaskTest, Mask3)
{
	TestMaskAcrossDimensions(3, [](int i, int j) { return ((i + j) % 2 + (i * j) % 3) % 2 == 0; });
}
