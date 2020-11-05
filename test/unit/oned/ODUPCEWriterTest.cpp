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

#include "oned/ODUPCEWriter.h"
#include "BitMatrixIO.h"

#include "gtest/gtest.h"
#include <stdexcept>

using namespace ZXing;
using namespace ZXing::OneD;

namespace {
	std::string Encode(const std::wstring& input)
	{
		auto result = ToString(UPCEWriter().encode(input, 0, 0), '1', '0', false);
		return result.substr(0, result.size() - 1);	// remove the \n at the end
	}
}

TEST(ODUPCEWriterTest, Encode1)
{
	std::wstring toEncode = L"05096893";
	std::string expected = "000010101110010100111000101101011110110111001011101010100000";
	EXPECT_EQ(Encode(toEncode), expected);
}

TEST(ODUPCEWriterTest, EncodeSystem1)
{
	std::wstring toEncode = L"12345670";
	std::string expected = "000010100100110111101010001101110010000101001000101010100000";
	EXPECT_EQ(Encode(toEncode), expected);
}

TEST(ODUPCEWriterTest, AddChecksumAndEncode)
{
	std::wstring toEncode = L"0509689";
	std::string expected = "000010101110010100111000101101011110110111001011101010100000";
	EXPECT_EQ(Encode(toEncode), expected);
}

TEST(ODUPCEWriterTest, EncodeIllegalCharacters)
{
	EXPECT_THROW(Encode(L"05096abc"), std::invalid_argument);
}
