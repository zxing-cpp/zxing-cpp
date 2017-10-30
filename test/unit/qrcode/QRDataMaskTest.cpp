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
#include "gtest/gtest.h"
#include "qrcode/QRDataMask.h"
#include "BitMatrix.h"
#include <functional>

using namespace ZXing;
using namespace ZXing::QRCode;

namespace {

	void TestMask(const DataMask& mask, int dimension, std::function<bool(int, int)> condition) {
		BitMatrix bits(dimension);
		mask.unmaskBitMatrix(bits, dimension);
		for (int i = 0; i < dimension; i++) {
			for (int j = 0; j < dimension; j++) {
				EXPECT_EQ(bits.get(j, i), condition(i, j)) << "(" << i << ',' << j << ')';
			}
		}
	}

	void TestMaskAcrossDimensions(int reference, std::function<bool(int, int)> condition) {
		DataMask mask(reference);
		for (int version = 1; version <= 40; version++) {
			int dimension = 17 + 4 * version;
			TestMask(mask, dimension, condition);
		}
	}

}

TEST(QRDataMaskTest, Mask0)
{
	TestMaskAcrossDimensions(0, [](int i, int j) { return (i + j) % 2 == 0; });
}

TEST(QRDataMaskTest, Mask1)
{
	TestMaskAcrossDimensions(1, [](int i, int j) { return i % 2 == 0; });
}

TEST(QRDataMaskTest, Mask2)
{
	TestMaskAcrossDimensions(2, [](int i, int j) { return j % 3 == 0; });
}

TEST(QRDataMaskTest, Mask3)
{
	TestMaskAcrossDimensions(3, [](int i, int j) { return (i + j) % 3 == 0; });
}

TEST(QRDataMaskTest, Mask4)
{
	TestMaskAcrossDimensions(4, [](int i, int j) { return (i / 2 + j / 3) % 2 == 0; });
}

TEST(QRDataMaskTest, Mask5)
{
	TestMaskAcrossDimensions(5, [](int i, int j) { return (i * j) % 2 + (i * j) % 3 == 0; });
}

TEST(QRDataMaskTest, Mask6)
{
	TestMaskAcrossDimensions(6, [](int i, int j) { return ((i * j) % 2 + (i * j) % 3) % 2 == 0; });
}

TEST(QRDataMaskTest, Mask7)
{
	TestMaskAcrossDimensions(7, [](int i, int j) { return ((i + j) % 2 + (i * j) % 3) % 2 == 0; });
}
