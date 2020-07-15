#pragma once
/*
* Copyright 2016 Nu-book Inc.
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
#include "../ODRowReader.h"
#include "Pattern.h"

#include <array>
#include <algorithm>

namespace ZXing {
namespace OneD {
namespace RSS {

class FinderPattern;

using FinderCounters = std::array<int, 4>;
using DataCounters = std::array<int, 4>;

template<bool EXPANDED>
inline bool IsFinderPattern(const PatternView& v)
{
	//  a,b,c,d,e, g | sum(a..e) = 15
	//  ------------
	//  1,1,2,1,1, 1
	//	| | |
	//	3,8,9

	// TODO: not checking the guard (g) might allow scanning rotated extended codes

	auto isFinderPattern = [](int a, int b, int c, int d, int e, int g) {
		int w = 2 * (b + c), n = d + e;
		return w > 9 * n && w < 13 * n && std::max(b, c) < 10 * std::min(d, e) && a < 4 * d && 4 * a > n &&
			   g * 4 < 3 * n;
	};

	int guardLeft = v[-10];
	int guardRight = v[13];

	// with GS1 Expanded Stacked codes, we don't know whether the guard element is left or right
	if (EXPANDED)
		guardLeft = guardRight = std::min(guardLeft, guardRight);

	return isFinderPattern(v[-1], v[0], v[1], v[2], v[3], guardLeft) ||
		   isFinderPattern(v[4], v[3], v[2], v[1], v[0], guardRight);
}

template<bool EXPANDED>
inline PatternView FindFinderPattern(const PatternView& row)
{
	const int minNumElems = 10; // number of elements left and right of a finder pattern, including the guard
	auto window = row.subView(minNumElems, 5);
	for (auto end = row.end() - minNumElems; window.data() < end; window.skipPair())
		if (IsFinderPattern<EXPANDED>(window))
			return window;

	return {};
}

class ReaderHelper
{
	static constexpr float MAX_AVG_VARIANCE = 0.2f;
	static constexpr float MAX_INDIVIDUAL_VARIANCE = 0.45f;

	static constexpr float MIN_FINDER_PATTERN_RATIO = 9.5f / 12.0f;
	static constexpr float MAX_FINDER_PATTERN_RATIO = 12.5f / 14.0f;

public:
	template <typename C, typename P>
	static int ParseFinderValue(const C& counters, const P& finderPatterns)
	{
		for (size_t value = 0; value < finderPatterns.size(); ++value) {
			if (RowReader::PatternMatchVariance(counters, finderPatterns[value], MAX_INDIVIDUAL_VARIANCE) < MAX_AVG_VARIANCE) {
				return static_cast<int>(value);
			}
		}
		return -1;
	}

	static bool IsFinderPatternExtended( const FinderCounters& counters, bool reversed )
	{
		int sumA = counters[0] + counters[1];
		int sumB = counters[2] + counters[3];
		float sum = static_cast<float>(sumA + sumB);
		float ratio = (reversed ? sumB : sumA) / sum;
		if (ratio >= MIN_FINDER_PATTERN_RATIO && ratio <= MAX_FINDER_PATTERN_RATIO) {
			// passes ratio test in spec, but see if the counts are unreasonable
			auto minmax = std::minmax_element(counters.begin(), counters.end());
			return *minmax.second < 10 * *minmax.first;
		}
		return false;
	}

	static bool IsFinderPattern( const FinderCounters& counters )
	{
		// Rhe RSS14 finder pattern is 5 counts long, the FINDER_PATTERNS array contains only the first 4
		// of those. The 5th is '1' (same as the forth). The 4 counters passed here are 2nd to 5th.
		// The first 2 of those 4 is 10 to 12 times as wide as both of the last two.
		int a = counters[0] + counters[1];
		int b = counters[2];
		int c = counters[3];

		return a > 8 * b && a < 14 * b && a > 8 * c && a < 14 * c;
	}

	static int GetRSSvalue(const DataCounters& widths, int maxWidth, bool noNarrow);

	static bool ReadOddEvenElements(const BitArray& row, const FinderPattern& pattern, int numModules, bool reversed,
									DataCounters& oddCounts, DataCounters& evenCounts);
};

} // RSS
} // OneD
} // ZXing
