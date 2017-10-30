/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright (C) 2014 ZXing authors
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
#include "pdf417/PDFHighLevelEncoder.h"
#include "pdf417/PDFCompaction.h"
#include "CharacterSet.h"

using namespace ZXing;
using namespace ZXing::Pdf417;

TEST(PDF417HighLevelEncoderTest, EncodeAuto)
{
	auto encoded = HighLevelEncoder::EncodeHighLevel(L"ABCD", Compaction::AUTO, CharacterSet::UTF8);
	EXPECT_EQ(encoded, std::vector<int>({ 0x39f, 0x1A, 0x385, 'A', 'B', 'C', 'D' }));
}

TEST(PDF417HighLevelEncoderTest, EncodeAutoWithSpecialChars)
{
	//Just check if this does not throw an exception
	HighLevelEncoder::EncodeHighLevel(L"1%\xA7""s ?aG$", Compaction::AUTO, CharacterSet::UTF8);
}
  
TEST(PDF417HighLevelEncoderTest, EncodeIso88591WithSpecialChars)
{
	// Just check if this does not throw an exception
	HighLevelEncoder::EncodeHighLevel(L"asdfg\xA7""asd", Compaction::AUTO, CharacterSet::ISO8859_1);
}

TEST(PDF417HighLevelEncoderTest, EncodeText)
{
    auto encoded = HighLevelEncoder::EncodeHighLevel(L"ABCD", Compaction::TEXT, CharacterSet::UTF8);
	EXPECT_EQ(encoded, std::vector<int>({ 0x39f, 0x1a, 1, '?' }));
}

TEST(PDF417HighLevelEncoderTest, EncodeNumeric)
{
	auto encoded = HighLevelEncoder::EncodeHighLevel(L"1234", Compaction::NUMERIC, CharacterSet::UTF8);
	EXPECT_EQ(encoded, std::vector<int>({ 0x39f, 0x1a, 0x386, '\f', 0x1b2 }));
}

TEST(PDF417HighLevelEncoderTest, EncodeByte)
{
	auto encoded = HighLevelEncoder::EncodeHighLevel(L"abcd", Compaction::BYTE, CharacterSet::UTF8);
	EXPECT_EQ(encoded, std::vector<int>({ 0x39f, 0x1a, 0x385, 'a', 'b', 'c', 'd' }));
}
