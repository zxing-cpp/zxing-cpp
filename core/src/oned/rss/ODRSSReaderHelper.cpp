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

#include "oned/rss/ODRSSReaderHelper.h"
#include "ZXContainerAlgorithms.h"

#include <limits>

namespace ZXing {
namespace OneD {
namespace RSS {

const float ReaderHelper::MAX_AVG_VARIANCE = 0.2f;
const float ReaderHelper::MAX_INDIVIDUAL_VARIANCE = 0.45f;

const float ReaderHelper::MIN_FINDER_PATTERN_RATIO = 9.5f / 12.0f;
const float ReaderHelper::MAX_FINDER_PATTERN_RATIO = 12.5f / 14.0f;


bool
ReaderHelper::IsFinderPattern(const int* counters, size_t length)
{
	int firstTwoSum = counters[0] + counters[1];
	int sum = firstTwoSum + counters[2] + counters[3];
	float ratio = (float)firstTwoSum / (float)sum;
	if (ratio >= MIN_FINDER_PATTERN_RATIO && ratio <= MAX_FINDER_PATTERN_RATIO) {
		// passes ratio test in spec, but see if the counts are unreasonable
		auto minmax = std::minmax_element(counters, counters + length);
		return *minmax.second < 10 * *minmax.first;
	}
	return false;
}

static int combins(int n, int r)
{
	int maxDenom;
	int minDenom;
	if (n - r > r) {
		minDenom = r;
		maxDenom = n - r;
	}
	else {
		minDenom = n - r;
		maxDenom = r;
	}
	int val = 1;
	int j = 1;
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
ReaderHelper::GetRSSvalue(const std::array<int, 4>& widths, int maxWidth, bool noNarrow)
{
	int elements = static_cast<int>(widths.size());
	int n = Accumulate(widths, 0);
	int val = 0;
	int narrowMask = 0;
	for (int bar = 0; bar < elements - 1; bar++) {
		int elmWidth;
		for (elmWidth = 1, narrowMask |= 1 << bar;
			elmWidth < widths[bar];
			elmWidth++, narrowMask &= ~(1 << bar)) {
			int subVal = combins(n - elmWidth - 1, elements - bar - 2);
			if (noNarrow && (narrowMask == 0) &&
				(n - elmWidth - (elements - bar - 1) >= elements - bar - 1)) {
				subVal -= combins(n - elmWidth - (elements - bar),
					elements - bar - 2);
			}
			if (elements - bar - 1 > 1) {
				int lessVal = 0;
				for (int mxwElement = n - elmWidth - (elements - bar - 2);
					mxwElement > maxWidth; mxwElement--) {
					lessVal += combins(n - elmWidth - mxwElement - 1,
						elements - bar - 3);
				}
				subVal -= lessVal * (elements - 1 - bar);
			}
			else if (n - elmWidth > maxWidth) {
				subVal--;
			}
			val += subVal;
		}
		n -= elmWidth;
	}
	return val;
}

} // RSS
} // OneD
} // ZXing
