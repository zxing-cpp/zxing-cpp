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

#include "oned/ODRowReader.h"

#include <array>
#include <algorithm>

namespace ZXing {
namespace OneD {
namespace RSS {

using FinderCounters = std::array<int, 4>;

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
		float sum = sumA + sumB;
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

	static int GetRSSvalue(const std::array<int, 4>& widths, int maxWidth, bool noNarrow);
};

} // RSS
} // OneD
} // ZXing
