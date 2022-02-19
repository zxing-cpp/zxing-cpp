/*
* Copyright 2021 gitlost
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

#include "oned/ODCode128Reader.h"

#include "Result.h"

#include "gtest/gtest.h"

using namespace ZXing;
using namespace ZXing::OneD;

// Helper to call decodePattern()
static Result parse(const int startPattern, PatternRow row)
{
	if (startPattern == 'A') {
		row.insert(row.begin(), { 0, 2, 1, 1, 4, 1, 2 });
	} else if (startPattern == 'B') {
		row.insert(row.begin(), { 0, 2, 1, 1, 2, 1, 4 });
	} else if (startPattern == 'C') {
		row.insert(row.begin(), { 0, 2, 1, 1, 2, 3, 2 });
	}
	row.insert(row.end(), { 2, 3, 3, 1, 1, 1, 2, 0 }); // Stop pattern

	std::unique_ptr<Code128Reader::DecodingState> state;
	Code128Reader reader;
	PatternView next(row);
	return reader.decodePattern(0, next, state);
}

TEST(ODCode128ReaderTest, SymbologyIdentifier)
{
	{
		// Plain "2001"
		PatternRow row({ 2, 2, 1, 2, 3, 1, 2, 2, 2, 1, 2, 2, 3, 1, 1, 2, 2, 2 });
		auto result = parse('C', row);
		EXPECT_EQ(result.symbologyIdentifier(), "]C0");
		EXPECT_EQ(result.text(), L"2001");
	}

	{
		// GS1 "(20)01"
		PatternRow row({ 4, 1, 1, 1, 3, 1, 2, 2, 1, 2, 3, 1, 2, 2, 2, 1, 2, 2, 1, 3, 2, 1, 3, 1 });
		auto result = parse('C', row);
		EXPECT_EQ(result.symbologyIdentifier(), "]C1");
		EXPECT_EQ(result.text(), L"2001");
	}

	{
		// AIM "A FNC1 B"
		PatternRow row({ 1, 1, 1, 3, 2, 3, 4, 1, 1, 1, 3, 1, 1, 3, 1, 1, 2, 3, 2, 1, 2, 3, 2, 1 });
		auto result = parse('B', row);
		EXPECT_EQ(result.symbologyIdentifier(), "]C2");
		EXPECT_EQ(result.text(), L"AB");
	}

	{
		// AIM "z FNC1 B"
		PatternRow row({ 2, 1, 4, 1, 2, 1, 4, 1, 1, 1, 3, 1, 1, 3, 1, 1, 2, 3, 4, 2, 1, 2, 1, 1 });
		auto result = parse('B', row);
		EXPECT_EQ(result.symbologyIdentifier(), "]C2");
		EXPECT_EQ(result.text(), L"zB");
	}

	{
		// AIM "99 FNC1 A"
		PatternRow row({ 1, 1, 3, 1, 4, 1, 4, 1, 1, 1, 3, 1, 1, 1, 4, 1, 3, 1, 1, 1, 1, 3, 2, 3, 1, 2, 3, 1, 2, 2 });
		auto result = parse('C', row);
		EXPECT_EQ(result.symbologyIdentifier(), "]C2");
		EXPECT_EQ(result.text(), L"99A");
	}

	{
		// Bad AIM Application Indicator "? FNC1 B"
		PatternRow row({ 2, 1, 2, 3, 2, 1, 4, 1, 1, 1, 3, 1, 1, 3, 1, 1, 2, 3, 3, 2, 2, 2, 1, 1 });
		auto result = parse('B', row);
		EXPECT_EQ(result.symbologyIdentifier(), "]C0"); // Just ignoring, not giving FormatError
		EXPECT_EQ(result.text(), L"?\u001DB");
	}
}

TEST(ODCode128ReaderTest, ReaderInit)
{
	{
		// Null
		PatternRow row({ 1, 1, 1, 1, 4, 3, 1, 3, 1, 1, 4, 1 });
		auto result = parse('C', row);
		EXPECT_FALSE(result.readerInit());
		EXPECT_EQ(result.text(), L"92");
	}

	{
		// Set (FNC3 first)
		PatternRow row({ 1, 1, 4, 3, 1, 1, 1, 1, 3, 1, 4, 1, 1, 1, 1, 1, 4, 3, 3, 3, 1, 1, 2, 1 });
		auto result = parse('B', row);
		EXPECT_TRUE(result.readerInit());
		EXPECT_EQ(result.text(), L"92");
	}

	{
		// Set (FNC3 between "9" and "2" )
		PatternRow row({ 3, 2, 1, 1, 2, 2, 1, 1, 4, 3, 1, 1, 2, 2, 3, 2, 1, 1, 1, 2, 1, 4, 2, 1 });
		auto result = parse('B', row);
		EXPECT_TRUE(result.readerInit());
		EXPECT_EQ(result.text(), L"92");
	}
}
