/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2011 ZXing authors
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
#include "oned/ODCodabarWriter.h"

using namespace ZXing;
using namespace ZXing::OneD;

namespace {
	std::string Encode(const std::wstring& input)
	{
		auto result = Utility::ToString(CodabarWriter().encode(input, 0, 0), '1', '0', false);
		return result.substr(0, result.size() - 1);	// remove the \n at the end
	}
}

TEST(ODCodaBarWriterTest, Encode)
{
	EXPECT_EQ(Encode(L"B515-3/B"),
           "00000"
           "1001001011" "0110101001" "0101011001" "0110101001" "0101001101"
           "0110010101" "01101101011" "01001001011"
           "00000");
}

TEST(ODCodaBarWriterTest, Encode2)
{
	EXPECT_EQ(Encode(L"T123T"),
           "00000"
           "1011001001" "0101011001" "0101001011" "0110010101" "01011001001"
           "00000");
}

TEST(ODCodaBarWriterTest, AltStartEnd)
{
	EXPECT_EQ(Encode(L"T123456789-$T"), Encode(L"A123456789-$A"));
}
