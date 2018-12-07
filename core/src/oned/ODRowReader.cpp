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
#include "Result.h"
#include "BitArray.h"

#include <cmath>
#include <algorithm>
#include <numeric>

namespace ZXing {
namespace OneD {

Result
RowReader::decodeSingleRow(int rowNumber, const BitArray& row) const
{
	std::unique_ptr<DecodingState> state;
	return decodeRow(rowNumber, row, state);
}

/**
* Determines how closely a set of observed counts of runs of black/white values matches a given
* target pattern. This is reported as the ratio of the total variance from the expected pattern
* proportions across all pattern elements, to the length of the pattern.
*
* @param counters observed counters
* @param pattern expected pattern
* @param maxIndividualVariance The most any counter can differ before we give up
* @return ratio of total variance between counters and pattern compared to total pattern size
*/
float
RowReader::PatternMatchVariance(const int *counters, const int* pattern, size_t length, float maxIndividualVariance)
{
	int total = std::accumulate(counters, counters+length, 0);
	int patternLength = std::accumulate(pattern, pattern+length, 0);
	if (total < patternLength) {
		// If we don't even have one pixel per unit of bar width, assume this is too small
		// to reliably match, so fail:
		return std::numeric_limits<float>::max();
	}

	float unitBarWidth = (float)total / patternLength;
	maxIndividualVariance *= unitBarWidth;

	float totalVariance = 0.0f;
	for (size_t x = 0; x < length; ++x) {
		float variance = std::abs(counters[x] - pattern[x] * unitBarWidth);
		if (variance > maxIndividualVariance) {
			return std::numeric_limits<float>::max();
		}
		totalVariance += variance;
	}
	return totalVariance / total;
}

} // OneD
} // ZXing
