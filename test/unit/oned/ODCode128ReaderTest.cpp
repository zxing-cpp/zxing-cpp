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

#include "DecodeHints.h"
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
	DecodeHints hints;
	Code128Reader reader(hints);
	return reader.decodePattern(0, row, state);
}

TEST(ODCode128ReaderTest, ReaderInit)
{
	{
		// Null
		PatternRow row({ 1, 1, 1, 1, 4, 3, 1, 3, 1, 1, 4, 1 });
		EXPECT_FALSE(parse('C', row).readerInit());
		EXPECT_EQ(parse('C', row).text(), L"92");
	}

	{
		// Set (FNC3 first)
		PatternRow row({ 1, 1, 4, 3, 1, 1, 1, 1, 3, 1, 4, 1, 1, 1, 1, 1, 4, 3, 3, 3, 1, 1, 2, 1 });
		EXPECT_TRUE(parse('B', row).readerInit());
		EXPECT_EQ(parse('B', row).text(), L"92");
	}

	{
		// Set (FNC3 between "9" and "2" )
		PatternRow row({ 3, 2, 1, 1, 2, 2, 1, 1, 4, 3, 1, 1, 2, 2, 3, 2, 1, 1, 1, 2, 1, 4, 2, 1 });
		EXPECT_TRUE(parse('B', row).readerInit());
		EXPECT_EQ(parse('B', row).text(), L"92");
	}
}
