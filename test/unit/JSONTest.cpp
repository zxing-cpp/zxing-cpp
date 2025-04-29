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
	EXPECT_EQ(JsonValue("key", true), R"("key":true,)");
	EXPECT_EQ(JsonValue("key", 1), R"("key":1,)");
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
	EXPECT_EQ(JsonGetStr("{\"key\": true}", "key"), "true"); // JSON
	EXPECT_EQ(JsonGetStr("{'key': True}", "key"), "True");   // Python
}

TEST(JSONTest, GetBool)
{
	EXPECT_TRUE(JsonGet<bool>("key", "key"));
	EXPECT_TRUE(JsonGet<bool>("key:true", "key"));
	EXPECT_TRUE(JsonGet<bool>("key:1", "key"));
	EXPECT_TRUE(JsonGet<bool>("key,other", "key"));
	EXPECT_TRUE(JsonGet<bool>("key", "KEY"));
	EXPECT_TRUE(JsonGet<bool>("key1", "key1"));

	EXPECT_FALSE(JsonGet<bool>("", ""));
	EXPECT_FALSE(JsonGet<bool>("", "key"));
	EXPECT_FALSE(JsonGet<bool>("key:", "key"));
	EXPECT_FALSE(JsonGet<bool>("key:false", "key"));
	EXPECT_FALSE(JsonGet<bool>("key:0", "key"));
	EXPECT_FALSE(JsonGet<bool>("keys", "key"));
	EXPECT_FALSE(JsonGet<bool>("thekey", "key"));

	EXPECT_TRUE(JsonGet<bool>("key , other", "key"));
	EXPECT_TRUE(JsonGet<bool>("\"key\": \"true\"", "key"));
	EXPECT_TRUE(JsonGet<bool>("{\"key\": true}", "key")); // JSON
	EXPECT_TRUE(JsonGet<bool>("{'key': True'}", "key"));  // Python
}

TEST(JSONTest, GetInt)
{
	EXPECT_FALSE(JsonGet<int>("key:", "key"));
	EXPECT_THROW(JsonGet<int>("key:false", "key"), std::invalid_argument);

	EXPECT_EQ(JsonGet<int>("key:1", "key").value(), 1);
	EXPECT_EQ(JsonGet<int>("{\"key\": 2}", "key").value(), 2); // JSON
	EXPECT_EQ(JsonGet<int>("{'key': 1}", "key").value(), 1);  // Python
}
