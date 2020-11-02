/*
* Copyright 2020 Axel Waggershauser
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

#include "ODDataBarCommon.h"

namespace ZXing {
namespace OneD {

// apparently the spec calls numbers at even indices 'odd'!?!
constexpr int odd = 0, evn = 1;

template <typename T>
struct OddEven
{
	T odd = {}, evn = {};
	T& operator[](int i) { return i & 1 ? evn : odd; }
};

bool ReadDataCharacterRaw(const PatternView& view, int numModules, bool reversed, Array4I& oddPattern,
						  Array4I& evnPattern)
{
	OddEven<Array4I&> res = {oddPattern, evnPattern};
	OddEven<Array4F> rem;

	float moduleSize = static_cast<float>(view.sum(8)) / numModules;
	auto* iter = view.data() + reversed * 7;
	int inc = reversed ? -1 : 1;

	for (int i = 0; i < 8; ++i, iter += inc) {
		float v = *iter / moduleSize;
		res[i % 2][i / 2] = int(v + .5f);
		rem[i % 2][i / 2] = v - res[i % 2][i / 2];
	}

	// DataBarExpanded data character is 17 modules wide
	// DataBar outer   data character is 16 modules wide
	// DataBar inner   data character is 15 modules wide

	int minSum = 4; // each data character has 4 bars and 4 spaces
	int maxSum = numModules - minSum;
	int oddSum = Reduce(res.odd);
	int evnSum = Reduce(res.evn);
	int sumErr = oddSum + evnSum - numModules;
	// sum < min -> negative error; sum > max -> positive error
	int oddSumErr = std::min(0, oddSum - (minSum + (numModules == 15))) + std::max(0, oddSum - maxSum);
	int evnSumErr = std::min(0, evnSum - minSum) + std::max(0, evnSum - (maxSum - (numModules == 15)));

	int oddParityErr = (oddSum & 1) == (numModules > 15);
	int evnParityErr = (evnSum & 1) == (numModules < 17);

	if ((sumErr == 0 && oddParityErr != evnParityErr) || (std::abs(sumErr) == 1 && oddParityErr == evnParityErr) ||
		std::abs(sumErr) > 1 || std::abs(oddSumErr) > 1 || std::abs(evnSumErr) > 1)
		return {};

	if (sumErr == -1) {
		oddParityErr *= -1;
		evnParityErr *= -1;
	} else if (sumErr == 0 && oddParityErr != 0) {
		// both parity errors are 1 -> flip one of them
		(oddSum < evnSum ? oddParityErr : evnParityErr) *= -1;
	}

	// check if parity and sum errors have opposite signs
	if (oddParityErr * oddSumErr < 0 || evnParityErr * evnSumErr < 0)
		return {};

	for (int i : {odd, evn}) {
		int err = i == odd ? (oddSumErr | oddParityErr) : (evnSumErr | evnParityErr);
		int mi = err < 0 ? std::max_element(rem[i].begin(), rem[i].end()) - rem[i].begin()
						 : std::min_element(rem[i].begin(), rem[i].end()) - rem[i].begin();
		res[i][mi] -= err;
	};

	return true;
}

} // OneD
} // ZXing
