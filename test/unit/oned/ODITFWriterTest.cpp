/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "oned/ODITFWriter.h"
#include "BitMatrixIO.h"

#include "gtest/gtest.h"
#include <stdexcept>

using namespace ZXing;
using namespace ZXing::OneD;

namespace {
	std::string Encode(const std::wstring& input)
	{
		auto result = ToString(ITFWriter().encode(input, 0, 0), '1', '0', false);
		return result.substr(0, result.size() - 1);	// remove the \n at the end
	}
}

TEST(ODITFWriterTest, Encode)
{
	EXPECT_EQ(Encode(L"00123456789012"),
				  "0000010101010111000111000101110100010101110001110111010001010001110100011"
				  "100010101000101011100011101011101000111000101110100010101110001110100000");
}

TEST(ODITFWriterTest, EncodeIllegalCharacters)
{
	EXPECT_THROW({ Encode(L"00123456789abc"); }, std::invalid_argument);
}
