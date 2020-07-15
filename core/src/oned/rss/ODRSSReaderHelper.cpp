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

#include "ODRSSReaderHelper.h"
#include "ODRSSFinderPattern.h"
#include "ZXContainerAlgorithms.h"

#include <array>

namespace ZXing {
namespace OneD {
namespace RSS {

static int combins(int n, int r)
{
	int maxDenom;
	int minDenom;
	if (n - r > r) {
		minDenom = r;
		maxDenom = n - r;
	} else {
		minDenom = n - r;
		maxDenom = r;
	}
	int val = 1;
	int j   = 1;
	for (int i = n; i > maxDenom; i--) {
		val *= i;
		if (j <= minDenom) {
			val /= j;
			j++;
		}
	}
	while (j <= minDenom) {
		val /= j;
		j++;
	}
	return val;
}

int
ReaderHelper::GetRSSvalue(const DataCounters& widths, int maxWidth, bool noNarrow)
{
	int elements = Size(widths);
	int n = Reduce(widths);
	int val = 0;
	int narrowMask = 0;
	for (int bar = 0; bar < elements - 1; bar++) {
		int elmWidth;
		for (elmWidth = 1, narrowMask |= 1 << bar; elmWidth < widths[bar]; elmWidth++, narrowMask &= ~(1 << bar)) {
			int subVal = combins(n - elmWidth - 1, elements - bar - 2);
			if (noNarrow && (narrowMask == 0) && (n - elmWidth - (elements - bar - 1) >= elements - bar - 1)) {
				subVal -= combins(n - elmWidth - (elements - bar), elements - bar - 2);
			}
			if (elements - bar - 1 > 1) {
				int lessVal = 0;
				for (int mxwElement = n - elmWidth - (elements - bar - 2); mxwElement > maxWidth; mxwElement--) {
					lessVal += combins(n - elmWidth - mxwElement - 1, elements - bar - 3);
				}
				subVal -= lessVal * (elements - 1 - bar);
			} else if (n - elmWidth > maxWidth) {
				subVal--;
			}
			val += subVal;
		}
		n -= elmWidth;
	}
	return val;
}

static bool AdjustOddEvenCounts(int numModules, DataCounters& oddCounts, DataCounters& evenCounts,
								const std::array<float, 4>& oddRoundingErrors,
								const std::array<float, 4>& evenRoundingErrors)
{
	// RSS Expanded data character is 17 modules wide
	// RSS 14 outer data character is 16 modules wide
	// RSS 14 inner data character is 15 modules wide

	int oddSum = Reduce(oddCounts);
	int evenSum = Reduce(evenCounts);
	int mismatch = oddSum + evenSum - numModules;
	bool oddParityBad = (oddSum & 1) == (numModules > 15);
	bool evenParityBad = (evenSum & 1) == (numModules < 17);

	int minSum = 4; // each data character has 4 bars and 4 spaces
	int maxSum = numModules - minSum;

	bool incrementOdd  = oddSum < minSum + (numModules == 15); // the offset is from the old code without explanation
	bool decrementOdd  = oddSum > maxSum;
	bool incrementEven = evenSum < minSum;
	bool decrementEven = evenSum > maxSum - (numModules == 15); // the offset is from the old code without explanation

	if ((mismatch == 0 && oddParityBad != evenParityBad) || (std::abs(mismatch) == 1 && oddParityBad == evenParityBad))
		return false;

	if (mismatch == 1) {
		(oddParityBad ? decrementOdd : decrementEven) = true;
	} else if (mismatch == -1) {
		(oddParityBad ? incrementOdd : incrementEven) = true;
	} else if (mismatch == 0) {
		if (oddParityBad) {
			// Both bad
			if (oddSum < evenSum) {
				incrementOdd = true;
				decrementEven = true;
			} else {
				decrementOdd = true;
				incrementEven = true;
			}
		}
		// else: Nothing to do!
	} else {
		return false;
	}

	if ((incrementOdd && decrementOdd) || (incrementEven && decrementEven))
		return false;

	if (incrementOdd)
		oddCounts[std::max_element(oddRoundingErrors.begin(), oddRoundingErrors.end()) - oddRoundingErrors.begin()] += 1;

	if (decrementOdd)
		oddCounts[std::min_element(oddRoundingErrors.begin(), oddRoundingErrors.end()) - oddRoundingErrors.begin()] -= 1;

	if (incrementEven)
		evenCounts[std::max_element(evenRoundingErrors.begin(), evenRoundingErrors.end()) - evenRoundingErrors.begin()] += 1;

	if (decrementEven)
		evenCounts[std::min_element(evenRoundingErrors.begin(), evenRoundingErrors.end()) - evenRoundingErrors.begin()] -= 1;

	return true;
}

bool ReaderHelper::ReadOddEvenElements(const BitArray& row, const FinderPattern& pattern, int numModules,
									   bool reversed, DataCounters& oddCounts, DataCounters& evenCounts)
{
	std::array<int, 8> counters = {};

	if (reversed) {
		if (!RowReader::RecordPatternInReverse(row.begin(), row.iterAt(pattern.startPos()), counters))
			return false;
	}
	else {
		if (!RowReader::RecordPattern(row.iterAt(pattern.endPos()), row.end(), counters))
			return false;
		std::reverse(counters.begin(), counters.end());
	}

	float moduleSize = static_cast<float>(Reduce(counters)) / numModules;

	// Sanity check: element width for pattern and the character should match
	float expectedElementWidth = (pattern.endPos() - pattern.startPos()) / 15.0f;
	if (std::abs(moduleSize - expectedElementWidth) / expectedElementWidth > 0.3f)
		return false;

	std::array<float, 4> oddRoundingErrors;
	std::array<float, 4> evenRoundingErrors;

	for (int i = 0; i < 8; i++) {
		float value = counters[i] / moduleSize;
		//TODO: C++17: count = std::clamp(std::lround(value), 1, 8);
		int count = std::lround(value);
		if (count < 1)
			count = 1;
		else if (count > 8)
			count = 8;
		int offset = i / 2;
		if ((i & 1) == 0) {
			oddCounts[offset] = count;
			oddRoundingErrors[offset] = value - count;
		} else {
			evenCounts[offset] = count;
			evenRoundingErrors[offset] = value - count;
		}
	}

	return AdjustOddEvenCounts(numModules, oddCounts, evenCounts, oddRoundingErrors, evenRoundingErrors);
}

} // RSS
} // OneD
} // ZXing
