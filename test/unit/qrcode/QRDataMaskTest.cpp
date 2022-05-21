/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2007 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "qrcode/QRDataMask.h"
#include "BitMatrix.h"

#include "gtest/gtest.h"
#include <functional>

using namespace ZXing;
using namespace ZXing::QRCode;

namespace {

	void TestMaskAcrossDimensionsImpl(int maskIndex, bool isMicro, const int versionMax, const int dimensionStart, const int dimensionStep, std::function<bool(int, int)> condition)
	{
		for (int version = 1; version <= versionMax; version++) {
			int dimension = dimensionStart + dimensionStep * version;
			BitMatrix bits(dimension);

			for (int i = 0; i < dimension; i++)
				for (int j = 0; j < dimension; j++)
					EXPECT_EQ(GetMaskedBit(bits, j, i, maskIndex, isMicro), condition(i, j)) << "(" << i << ',' << j << ')';
		}
	}

	void TestMaskAcrossDimensions(int maskIndex, std::function<bool(int, int)> condition)
	{
		TestMaskAcrossDimensionsImpl(maskIndex, false, 40, 17, 4, condition);
	}

	void TestMicroMaskAcrossDimensions(int maskIndex, std::function<bool(int, int)> condition)
	{
		TestMaskAcrossDimensionsImpl(maskIndex, true, 4, 9, 2, condition);
	}

}

TEST(QRDataMaskTest, Mask0)
{
	TestMaskAcrossDimensions(0, [](int i, int j) { return (i + j) % 2 == 0; });
}

TEST(QRDataMaskTest, Mask1)
{
	TestMaskAcrossDimensions(1, [](int i, int) { return i % 2 == 0; });
}

TEST(QRDataMaskTest, Mask2)
{
	TestMaskAcrossDimensions(2, [](int, int j) { return j % 3 == 0; });
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

TEST(QRDataMaskTest, MicroMask0)
{
	TestMicroMaskAcrossDimensions(0, [](int i, int) { return i % 2 == 0; });
}

TEST(QRDataMaskTest, MicroMask1)
{
	TestMicroMaskAcrossDimensions(1, [](int i, int j) { return (i / 2 + j / 3) % 2 == 0; });
}

TEST(QRDataMaskTest, MicroMask2)
{
	TestMicroMaskAcrossDimensions(2, [](int i, int j) { return ((i * j) % 2 + (i * j) % 3) % 2 == 0; });
}

TEST(QRDataMaskTest, MicroMask3)
{
	TestMicroMaskAcrossDimensions(3, [](int i, int j) { return ((i + j) % 2 + (i * j) % 3) % 2 == 0; });
}
