/*
* Copyright 2022 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "ZXAlgorithms.h"

#include "gtest/gtest.h"

using namespace ZXing;

TEST(ZXAlgorithmsTest, ToDigit)
{
	EXPECT_EQ(ToDigit(0), '0');
	EXPECT_EQ(ToDigit(9), '9');

	EXPECT_THROW(ToDigit(-1), Error);
	EXPECT_THROW(ToDigit(11), Error);
}

TEST(ZXAlgorithmsTest, ToString)
{
	EXPECT_EQ(ToString(0, 1), "0");
	EXPECT_EQ(ToString(0, 2), "00");
	EXPECT_EQ(ToString(1, 3), "001");
	EXPECT_EQ(ToString(99, 2), "99");

	EXPECT_THROW(ToString(-1, 2), Error);
	EXPECT_THROW(ToString(111, 2), Error);
}

TEST(ZXAlgorithmsTest, UpdateMinMax)
{
	int m = 10, M = 0;
	UpdateMinMax(m, M, 5);
	EXPECT_EQ(m, 5);
	EXPECT_EQ(M, 5);

	UpdateMinMax(m, M, 2);
	EXPECT_EQ(m, 2);
	EXPECT_EQ(M, 5);

	m = M = 1;
	UpdateMinMax(m, M, 0);
	EXPECT_EQ(m, 0);
	EXPECT_EQ(M, 1);

	UpdateMinMax(m, M, 2);
	EXPECT_EQ(m, 0);
	EXPECT_EQ(M, 2);
}
