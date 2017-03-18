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

namespace ZXing {
namespace OneD {
namespace RSS {

class ReaderHelper
{
	static const float MAX_AVG_VARIANCE;
	static const float MAX_INDIVIDUAL_VARIANCE;

	static const float MIN_FINDER_PATTERN_RATIO;
	static const float MAX_FINDER_PATTERN_RATIO;

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

	template <typename C>
	static bool IsFinderPattern(const C& counters) {
		return IsFinderPattern(counters.data(), counters.size());
	}

	static int GetRSSvalue(const std::array<int, 4>& widths, int maxWidth, bool noNarrow);

private:
	static bool IsFinderPattern(const int* counters, size_t length);
};

} // RSS
} // OneD
} // ZXing
