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
#include "oned/ODCode93Writer.h"

using namespace ZXing;
using namespace ZXing::OneD;

namespace {
	std::string Encode(const std::wstring& input)
	{
		auto result = Utility::ToString(Code93Writer().encode(input, 0, 0), '1', '0', false);
		return result.substr(0, result.size() - 1);	// remove the \n at the end
	}
}

TEST(ODCode93WriterTest, Encode)
{
    EXPECT_EQ(Encode(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"),
           "000001010111101101010001101001001101000101100101001100100101100010101011010001011001"
           "001011000101001101001000110101010110001010011001010001101001011001000101101101101001"
           "101100101101011001101001101100101101100110101011011001011001101001101101001110101000"
           "101001010010001010001001010000101001010001001001001001000101010100001000100101000010"
           "10100111010101000010101011110100000");
}

