/*
* Copyright 2025 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "JSON.h"

#include "gtest/gtest.h"

using namespace ZXing;

TEST(JSONTest, Value)
{
	EXPECT_EQ(JsonValue("key", "val"), R"("key":"val",)");
	EXPECT_EQ(JsonValue("key", true), R"("key":"true",)");
	EXPECT_EQ(JsonValue("key", 1), R"("key":"1",)");
}

TEST(JSONTest, GetBool)
{
	EXPECT_TRUE(JsonGetBool("key", "key"));
	EXPECT_TRUE(JsonGetBool("key:true", "key"));
	EXPECT_TRUE(JsonGetBool("key:1", "key"));
	EXPECT_TRUE(JsonGetBool("key,other", "key"));
	EXPECT_TRUE(JsonGetBool("key", "KEY"));
	EXPECT_TRUE(JsonGetBool("key1", "key1"));

	EXPECT_FALSE(JsonGetBool("", ""));
	EXPECT_FALSE(JsonGetBool("", "key"));
	EXPECT_FALSE(JsonGetBool("key:", "key"));
	EXPECT_FALSE(JsonGetBool("key:false", "key"));
	EXPECT_FALSE(JsonGetBool("key:0", "key"));
	EXPECT_FALSE(JsonGetBool("keys", "key"));
	EXPECT_FALSE(JsonGetBool("thekey", "key"));

	EXPECT_TRUE(JsonGetBool("key , other", "key"));
	EXPECT_TRUE(JsonGetBool("\"key\": \"true\"", "key"));
}

TEST(JSONTest, GetStr)
{
	EXPECT_EQ(JsonGetStr("", "key"), "");
	EXPECT_EQ(JsonGetStr("key", "key"), "");
	EXPECT_EQ(JsonGetStr("keys:abc", "key"), "");
	EXPECT_EQ(JsonGetStr("key:", "key"), "");
	EXPECT_EQ(JsonGetStr("key:abc", "key"), "abc");
	EXPECT_EQ(JsonGetStr("key:abc,", "key"), "abc");
	EXPECT_EQ(JsonGetStr("key:abc,key2", "key"), "abc");
	EXPECT_EQ(JsonGetStr("key:abc", "KEY"), "abc");

	EXPECT_EQ(JsonGetStr("\"key\": \"abc\"", "KEY"), "abc");
}
