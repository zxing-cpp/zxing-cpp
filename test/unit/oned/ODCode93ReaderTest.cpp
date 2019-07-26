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
#include "BitArray.h"
#include "Result.h"
#include "BitArrayUtility.h"
#include "oned/ODCode93Reader.h"

using namespace ZXing;
using namespace ZXing::OneD;

namespace {
	std::wstring Decode(const std::string& input)
	{
		Code93Reader sut;
		auto row = Utility::ParseBitArray(input, '1');
		auto result = sut.decodeSingleRow(0, row);
		return result.text();
	}
}

TEST(ODCode93ReaderTest, Decode)
{
	auto expected = std::wstring(L"Code93!\n$%/+ :\x1b;[{\x7f\x00@`\x7f\x7f\x7f", 25);
	auto decoded = Decode(
		"00000010101111011010001010011001010010110010011001011001010010011001011001001010"
		"00010101010000101110101101101010001001001101001101001110010101101011101011011101"
		"01110110111010010111010110100111010111011010110101000111011010110001010111011010"
		"10001101011101101010001011011101101011010011011101101011001011011101101011001101"
		"01110110101011011001110110101011001101110110101001101101110110101001110101001100"
		"101101010001010111101111");
	EXPECT_EQ(expected, decoded);
}
