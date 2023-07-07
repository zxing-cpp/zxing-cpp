/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright (C) 2014 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "CharacterSet.h"
#include "pdf417/PDFCompaction.h"
#include "pdf417/PDFHighLevelEncoder.h"

#include "gtest/gtest.h"

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

TEST(PDF417HighLevelEncoderTest, EncodeByteBINARYECI)
{
	auto encoded = HighLevelEncoder::EncodeHighLevel(L"\u00E9", Compaction::BYTE, CharacterSet::BINARY);
	EXPECT_EQ(encoded, std::vector<int>({ 927, 899, 901, 0xe9 }));
}

TEST(PDF417HighLevelEncoderTest, EncodeByteUnknown)
{
	EXPECT_THROW(HighLevelEncoder::EncodeHighLevel(L"\u00E9", Compaction::BYTE, CharacterSet::Unknown), std::invalid_argument);
}
