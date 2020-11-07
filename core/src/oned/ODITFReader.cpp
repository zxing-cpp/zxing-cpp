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

#include "ODITFReader.h"

#include "BitArray.h"
#include "DecodeHints.h"
#include "Result.h"
#include "ZXContainerAlgorithms.h"

#include <array>

namespace ZXing::OneD {

/** Valid ITF lengths. Anything longer than the largest value is also allowed. */
static const std::array<int, 5> DEFAULT_ALLOWED_LENGTHS = { 6, 8, 10, 12, 14 };

ITFReader::ITFReader(const DecodeHints& hints) :
	_allowedLengths(hints.allowedLengths())
{
	if (_allowedLengths.empty()) {
		_allowedLengths.assign(DEFAULT_ALLOWED_LENGTHS.begin(), DEFAULT_ALLOWED_LENGTHS.end());
	}
}

constexpr auto START_PATTERN_ = FixedPattern<4, 4>{1, 1, 1, 1};
constexpr auto STOP_PATTERN_1 = FixedPattern<3, 4>{2, 1, 1};
constexpr auto STOP_PATTERN_2 = FixedPattern<3, 5>{3, 1, 1};

Result ITFReader::decodePattern(int rowNumber, const PatternView& row, std::unique_ptr<DecodingState>&) const
{
	const int minCharCount = 6;
	const int minQuiteZone = 10;

	auto next = FindLeftGuard(row, 4 + minCharCount/2 + 3, START_PATTERN_, minQuiteZone);
	if (!next.isValid())
		return Result(DecodeStatus::NotFound);

	std::string txt;
	txt.reserve(20);

	constexpr int weights[] = {1, 2, 4, 7, 0};
	int xStart = next.pixelsInFront();
	next = next.subView(4, 10);

	while (next.index() < row.size() - (10 + 3)) {
		const auto threshold = NarrowWideThreshold(next);
		if (!threshold.isValid())
			break;

		BarAndSpace<int> digits, numWide;
		for (int i = 0; i < 10; ++i) {
			if (next[i] > threshold[i] * 2)
				break;
			numWide[i] += next[i] > threshold[i];
			digits[i] += weights[i/2] * (next[i] > threshold[i]);
		}

		if (numWide.bar != 2 || numWide.space != 2)
			break;

		for (int i = 0; i < 2; ++i)
			txt.push_back((char)('0' + (digits[i] == 11 ? 0 : digits[i])));

		next.skipSymbol();
	}

	next = next.subView(0, 3);

	if (Size(txt) < minCharCount)
		return Result(DecodeStatus::NotFound);

	if (!IsRightGuard(next, STOP_PATTERN_1, minQuiteZone) && !IsRightGuard(next, STOP_PATTERN_2, minQuiteZone))
		return Result(DecodeStatus::NotFound);

	int xStop = next.pixelsTillEnd();
	return Result(txt, rowNumber, xStart, xStop, BarcodeFormat::ITF);
}

} // namespace ZXing::OneD
