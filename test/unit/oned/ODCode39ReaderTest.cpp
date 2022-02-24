/*
* Copyright 2022 gitlost
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

#include "oned/ODCode39Reader.h"

#include "DecodeHints.h"
#include "Result.h"

#include "gtest/gtest.h"

using namespace ZXing;
using namespace ZXing::OneD;

// Helper to call decodePattern()
static Result parse(PatternRow row, DecodeHints hints = {})
{
	Code39Reader reader(hints);

	row.insert(row.begin(), { 0, 1, 2, 1, 1, 2, 1, 2, 1, 1, 0 });
	row.insert(row.end(), { 0, 1, 2, 1, 1, 2, 1, 2, 1, 1, 0 });

	std::unique_ptr<RowReader::DecodingState> state;
	PatternView next(row);
	return reader.decodePattern(0, next, state);
}

TEST(ODCode39ReaderTest, SymbologyIdentifier)
{
	{
		// Plain "A"
		PatternRow row({ 2, 1, 1, 1, 1, 2, 1, 1, 2 });
		auto result = parse(row);
		EXPECT_EQ(result.symbologyIdentifier(), "]A0");
		EXPECT_EQ(result.text(), L"A");
	}
	{
		// "A" with checksum
		PatternRow row({ 2, 1, 1, 1, 1, 2, 1, 1, 2, 0, 2, 1, 1, 1, 1, 2, 1, 1, 2 });
		auto result = parse(row, DecodeHints().setAssumeCode39CheckDigit(true));
		EXPECT_EQ(result.symbologyIdentifier(), "]A3");
		EXPECT_EQ(result.text(), L"A");

		result = parse(row);
		EXPECT_EQ(result.symbologyIdentifier(), "]A0");
		EXPECT_EQ(result.text(), L"AA");
	}
	{
		// Extended "a"
		PatternRow row({ 1, 2, 1, 1, 1, 2, 1, 2, 1, 0, 2, 1, 1, 1, 1, 2, 1, 1, 2 });
		auto result = parse(row, DecodeHints().setTryCode39ExtendedMode(true));
		EXPECT_EQ(result.symbologyIdentifier(), "]A4");
		EXPECT_EQ(result.text(), L"a");

		result = parse(row);
		EXPECT_EQ(result.symbologyIdentifier(), "]A0");
		EXPECT_EQ(result.text(), L"+A");
	}
	{
		// Extended "a" with checksum
		PatternRow row({ 1, 2, 1, 1, 1, 2, 1, 2, 1, 0, 2, 1, 1, 1, 1, 2, 1, 1, 2, 0, 2, 1, 1, 2, 1, 1, 2, 1, 1 });
		auto result = parse(row, DecodeHints().setTryCode39ExtendedMode(true).setAssumeCode39CheckDigit(true));
		EXPECT_EQ(result.symbologyIdentifier(), "]A7");
		EXPECT_EQ(result.text(), L"a");

		result = parse(row);
		EXPECT_EQ(result.symbologyIdentifier(), "]A0");
		EXPECT_EQ(result.text(), L"+A8");
	}
}
