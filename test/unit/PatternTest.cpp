/*
* Copyright 2022 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "Pattern.h"

#include "gtest/gtest.h"

using namespace ZXing;

constexpr int N = 33;

TEST(PatternTest, AllWhite)
{
	for (int s = 1; s <= N; ++s) {
		std::vector<uint8_t> in(s, 0);
		PatternRow pr;
		GetPatternRow(Range<const uint8_t*>{in.data(), in.data() + in.size()}, pr);

		EXPECT_EQ(pr.size(), 1);
		EXPECT_EQ(pr[0], s);
	}
}

TEST(PatternTest, AllBlack)
{
	for (int s = 1; s <= N; ++s) {
		std::vector<uint8_t> in(s, 0xff);
		PatternRow pr;
		GetPatternRow(Range<const uint8_t*>{in.data(), in.data() + in.size()}, pr);

		EXPECT_EQ(pr.size(), 3);
		EXPECT_EQ(pr[0], 0);
		EXPECT_EQ(pr[1], s);
		EXPECT_EQ(pr[2], 0);
	}
}

TEST(PatternTest, BlackWhite)
{
	for (int s = 1; s <= N; ++s) {
		std::vector<uint8_t> in(N, 0);
		std::fill_n(in.data(), s, 0xff);
		PatternRow pr;
		GetPatternRow(Range<const uint8_t*>{in.data(), in.data() + in.size()}, pr);

		EXPECT_EQ(pr.size(), 3);
		EXPECT_EQ(pr[0], 0);
		EXPECT_EQ(pr[1], s);
		EXPECT_EQ(pr[2], N - s);
	}
}

TEST(PatternTest, WhiteBlack)
{
	for (int s = 0; s < N; ++s) {
		std::vector<uint8_t> in(N, 0xff);
		std::fill_n(in.data(), s, 0);
		PatternRow pr;
		GetPatternRow(Range<const uint8_t*>{in.data(), in.data() + in.size()}, pr);

		EXPECT_EQ(pr.size(), 3);
		EXPECT_EQ(pr[0], s);
		EXPECT_EQ(pr[1], N - s);
		EXPECT_EQ(pr[2], 0);
	}
}
