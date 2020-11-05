/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2009 ZXing authors
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

#include "oned/ODEAN8Writer.h"
#include "BitMatrixIO.h"

#include "gtest/gtest.h"

using namespace ZXing;
using namespace ZXing::OneD;

namespace {
	std::string Encode(const std::wstring& input)
	{
		auto result = ToString(EAN8Writer().encode(input, 0, 0), '1', '0', false);
		return result.substr(0, result.size() - 1);	// remove the \n at the end
	}
}

TEST(ODEAN8WriterTest, Encode1)
{
	std::wstring toEncode = L"96385074";
	std::string expected = "0000101000101101011110111101011011101010100111011100101000100101110010100000";
	EXPECT_EQ(Encode(toEncode), expected);
}

TEST(ODEAN8WriterTest, AddChecksumAndEncode)
{
	std::wstring toEncode = L"9638507";
    std::string expected = "0000101000101101011110111101011011101010100111011100101000100101110010100000";
	EXPECT_EQ(Encode(toEncode), expected);
}

TEST(ODEAN8WriterTest, EncodeIllegalCharacters)
{
	EXPECT_THROW({ Encode(L"96385abc"); }, std::invalid_argument);
}
