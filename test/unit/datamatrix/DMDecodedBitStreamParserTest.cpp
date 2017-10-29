/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2008 ZXing authors
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
#include "DecoderResult.h"
#include "ByteArray.h"

namespace ZXing {
	namespace DataMatrix {
		namespace DecodedBitStreamParser {
			DecoderResult Decode(ByteArray&& bytes);
		}
	}
}

using namespace ZXing;

TEST(DMDecodedBitStreamParserTest, AsciiStandardDecode)
{
	// ASCII characters 0-127 are encoded as the value + 1
	ByteArray bytes = { 'a' + 1, 'b' + 1, 'c' + 1, 'A' + 1, 'B' + 1, 'C' + 1 };
	auto decodedString = DataMatrix::DecodedBitStreamParser::Decode(std::move(bytes)).text();
	EXPECT_EQ(decodedString, L"abcABC");
}

TEST(DMDecodedBitStreamParserTest, AsciiDoubleDigitDecode)
{
	// ASCII double digit (00 - 99) Numeric Value + 130
	ByteArray bytes = { 130 , 1 + 130, 98 + 130, 99 + 130 };
	auto decodedString = DataMatrix::DecodedBitStreamParser::Decode(std::move(bytes)).text();
	EXPECT_EQ(decodedString, L"00019899");
}
