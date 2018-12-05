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
#include "gtest/gtest.h"
#include "BitMatrix.h"
#include "BitMatrixUtility.h"
#include "oned/ODEAN13Writer.h"

using namespace ZXing;
using namespace ZXing::OneD;

namespace {
	std::string Encode(const std::wstring& input, size_t length)
	{
		auto result = Utility::ToString(EAN13Writer().encode(input, (int)length, 0), '1', '0', false);
		return result.substr(0, result.size() - 1);	// remove the \n at the end
	}
}

TEST(ODEAN13WriterTest, Encode1)
{
	std::wstring toEncode = L"5901234123457";
	std::string expected = "00001010001011010011101100110010011011110100111010101011001101101100100001010111001001110100010010100000";
	EXPECT_EQ(Encode(toEncode, expected.length()), expected);
}

TEST(ODEAN13WriterTest, AddChecksumAndEncode)
{
	std::wstring toEncode = L"590123412345";
	std::string expected = "00001010001011010011101100110010011011110100111010101011001101101100100001010111001001110100010010100000";
	EXPECT_EQ(Encode(toEncode, expected.length()), expected);
}

TEST(ODEAN13WriterTest, EncodeIllegalCharacters)
{
	EXPECT_THROW({ Encode(L"5901234123abc", 100); }, std::invalid_argument);
}
