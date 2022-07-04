/*
* Copyright 2022 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "Error.h"

#include "gtest/gtest.h"

using namespace ZXing;

TEST(ErrorTest, Default)
{
	Error e;

	EXPECT_FALSE(e);
	EXPECT_EQ(e.type(), Error::Type::None);
	EXPECT_EQ(e.msg().empty(), true);
	EXPECT_EQ(e.location().empty(), true);
}

TEST(ErrorTest, Empty)
{
	Error e = ChecksumError();

	EXPECT_TRUE(e);
	EXPECT_EQ(e.type(), Error::Type::Checksum);
	EXPECT_EQ(e.type(), Error::Checksum);
	EXPECT_EQ(e, Error::Checksum);
	EXPECT_EQ(Error::Checksum, e);
	EXPECT_EQ(e.msg().empty(), true);
	EXPECT_EQ(e.location().empty(), false);
}

TEST(ErrorTest, WithMsg)
{
	Error e = FormatError("something is wrong"); int line = __LINE__;

	EXPECT_TRUE(e);
	EXPECT_EQ(e, Error::Format);
	EXPECT_EQ(e.msg(), "something is wrong");
	EXPECT_EQ(e.location(), "ErrorTest.cpp:" + std::to_string(line));
}
