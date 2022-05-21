/*
* Copyright 2018 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "BitHacks.h"

#include "gtest/gtest.h"
#include <vector>

TEST(BitHackTest, BitHacks)
{
	using namespace ZXing::BitHacks;

	EXPECT_EQ(NumberOfLeadingZeros(0), 32);
	EXPECT_EQ(NumberOfLeadingZeros(1), 31);
	EXPECT_EQ(NumberOfLeadingZeros(0xffffffff), 0);

//	EXPECT_EQ(NumberOfTrailingZeros(0), 32);
	EXPECT_EQ(NumberOfTrailingZeros(1), 0);
	EXPECT_EQ(NumberOfTrailingZeros(2), 1);
	EXPECT_EQ(NumberOfTrailingZeros(0xffffffff), 0);

	EXPECT_EQ(Reverse(0), 0);
	EXPECT_EQ(Reverse(1), 0x80000000);
	EXPECT_EQ(Reverse(0xffffffff), 0xffffffff);
	EXPECT_EQ(Reverse(0xff00ff00), 0x00ff00ff);

	EXPECT_EQ(CountBitsSet(0), 0);
	EXPECT_EQ(CountBitsSet(1), 1);
	EXPECT_EQ(CountBitsSet(2), 1);
	EXPECT_EQ(CountBitsSet(0xffffffff), 32);
	EXPECT_EQ(CountBitsSet(0x11111111), 8);

	EXPECT_EQ(HighestBitSet(0x1), 0);
	EXPECT_EQ(HighestBitSet(0xffffffff), 31);
	EXPECT_EQ(HighestBitSet(0x1F), 4);

	using V = std::vector<uint32_t>;
	auto checkReverse = [](V&& v1, int p, V&& v2) {
		Reverse(v1, p);
		EXPECT_EQ(v1, v2);
	};

	checkReverse(V{1}, 0, V{0x80000000});
	checkReverse(V{0, 1}, 0, V{0x80000000, 0});
	checkReverse(V{0, 1}, 31, V{1, 0});
	checkReverse(V{0xffffffff, 0}, 16, V{0xffff0000, 0xffff});
}
