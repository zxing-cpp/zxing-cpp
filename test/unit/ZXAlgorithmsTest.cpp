/*
* Copyright 2022 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "ZXAlgorithms.h"

#include "gtest/gtest.h"

using namespace ZXing;

TEST(ZXAlgorithmsTest, ToString)
{
	EXPECT_EQ(ToString(0, 1), "0");
	EXPECT_EQ(ToString(0, 2), "00");
	EXPECT_EQ(ToString(1, 3), "001");
	EXPECT_EQ(ToString(99, 2), "99");

	EXPECT_THROW(ToString(-1, 2), Error);
	EXPECT_THROW(ToString(111, 2), Error);
}
