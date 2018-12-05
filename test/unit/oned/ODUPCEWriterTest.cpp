/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2016 ZXing authors
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
#include "BitMatrix.h"
#include "BitMatrixUtility.h"
#include "oned/ODUPCEWriter.h"

using namespace ZXing;
using namespace ZXing::OneD;

namespace {
	std::string Encode(const std::wstring& input, size_t length)
	{
		auto result = Utility::ToString(UPCEWriter().encode(input, (int)length, 0), '1', '0', false);
		return result.substr(0, result.size() - 1);	// remove the \n at the end
	}
}

TEST(ODUPCEWriterTest, Encode1)
{
	std::wstring toEncode = L"05096893";
	std::string expected = "0000000000010101110010100111000101101011110110111001011101010100000000000";
	EXPECT_EQ(Encode(toEncode, expected.length()), expected);
}

TEST(ODUPCEWriterTest, EncodeSystem1)
{
	std::wstring toEncode = L"12345670";
	std::string expected = "0000000000010100100110111101010001101110010000101001000101010100000000000";
	EXPECT_EQ(Encode(toEncode, expected.length()), expected);
}

TEST(ODUPCEWriterTest, AddChecksumAndEncode)
{
	std::wstring toEncode = L"0509689";
	std::string expected = "0000000000010101110010100111000101101011110110111001011101010100000000000";
	EXPECT_EQ(Encode(toEncode, expected.length()), expected);
}

TEST(ODUPCEWriterTest, EncodeIllegalCharacters)
{
	EXPECT_THROW(Encode(L"05096abc", 100), std::invalid_argument);
}
