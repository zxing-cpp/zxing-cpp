/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2010 ZXing authors
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
#include "oned/ODUPCAWriter.h"

using namespace ZXing;
using namespace ZXing::OneD;

namespace {
	std::string Encode(const std::wstring& input, size_t length)
	{
		auto result = Utility::ToString(UPCAWriter().encode(input, (int)length, 0), '1', '0', false);
		return result.substr(0, result.size() - 1);	// remove the \n at the end
	}
}

TEST(ODUPCAWriterTest, Encode1)
{
	std::wstring toEncode = L"485963095124";
	std::string expected = "00001010100011011011101100010001011010111101111010101011100101110100100111011001101101100101110010100000";
	EXPECT_EQ(Encode(toEncode, expected.length()), expected);
}

TEST(ODUPCAWriterTest, AddChecksumAndEncode)
{
	std::wstring toEncode = L"12345678901";
	std::string expected = "00001010011001001001101111010100011011000101011110101010001001001000111010011100101100110110110010100000";
	EXPECT_EQ(Encode(toEncode, expected.length()), expected);
}

