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

#include "oned/ODRSS14Reader.h"
#include "oned/rss/ODRSSReaderHelper.h"
#include "oned/rss/ODRSSPair.h"
#include "BitArray.h"
#include "Result.h"
#include "DecodeHints.h"
#include "ZXConfig.h"

#include <list>
#include <array>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <iomanip>

namespace ZXing {
namespace OneD {

static const int OUTSIDE_EVEN_TOTAL_SUBSET[] = { 1,10,34,70,126 };
static const int INSIDE_ODD_TOTAL_SUBSET[] = { 4,20,48,81 };
static const int OUTSIDE_GSUM[] = { 0,161,961,2015,2715 };
static const int INSIDE_GSUM[] = { 0,336,1036,1516 };
static const int OUTSIDE_ODD_WIDEST[] = { 8,6,4,3,1 };
static const int INSIDE_ODD_WIDEST[] = { 2,4,6,8 };

using FinderCounters = std::array<int, 4>;

static const std::array<FinderCounters, 9> FINDER_PATTERNS = {
	3,8,2,1,
	3,5,5,1,
	3,3,7,1,
	3,1,9,1,
	2,7,4,1,
	2,5,6,1,
	2,3,8,1,
	1,5,7,1,
	1,3,9,1,
};

struct RSS14DecodingState : public RowReader::DecodingState
{
	std::list<RSS::Pair> possibleLeftPairs;
	std::list<RSS::Pair> possibleRightPairs;
};

//private final List<Pair> possibleLeftPairs;
//private final List<Pair> possibleRightPairs;
//
//public RSS14Reader() {
//	possibleLeftPairs = new ArrayList<>();
//	possibleRightPairs = new ArrayList<>();
//}

static BitArray::Range
FindFinderPattern(const BitArray& row, bool rightFinderPattern, FinderCounters& counters)
{
	return RowReader::FindPattern(
	    // Will encounter white first when searching for right finder pattern
	    row.getNextSetTo(row.begin(), !rightFinderPattern), row.end(), counters,
	    [](BitArray::Iterator, BitArray::Iterator, const FinderCounters& counters) {
		    return RSS::ReaderHelper::IsFinderPattern(counters);
	    });
}

static RSS::FinderPattern
ParseFoundFinderPattern(const BitArray& row, int rowNumber, bool right, BitArray::Range range, FinderCounters& finderCounters)
{
	if (!range || range.begin == row.begin())
		return {};

	// Actually we found elements 2-5 -> Locate element 1
	auto i = std::find(BitArray::ReverseIterator(range.begin), row.rend(), *range.begin);
	int firstCounter = range.begin - i.base();
	range.begin = i.base();

	// Make 'counters' hold 1-4
	std::copy_backward(finderCounters.begin(), finderCounters.end() - 1, finderCounters.end());
	finderCounters[0] = firstCounter;
	int value = RSS::ReaderHelper::ParseFinderValue(finderCounters, FINDER_PATTERNS);
	if (value < 0)
		return {};

	int start = range.begin - row.begin();
	int end = range.end - row.begin();
	if (right) {
		// row is actually reversed
		start = row.size() - 1 - start;
		end = row.size() - 1 - end;
	}

	return {value,
			range.begin - row.begin(),
			range.end - row.begin(),
			{ResultPoint(start, rowNumber), ResultPoint(end, rowNumber)}};
}

static bool
AdjustOddEvenCounts(bool outsideChar, int numModules, std::array<int, 4>& oddCounts, std::array<int, 4>& evenCounts,
	const std::array<float, 4>& oddRoundingErrors, const std::array<float, 4>& evenRoundingErrors)
{
	int oddSum = std::accumulate(oddCounts.begin(), oddCounts.end(), 0);
	int evenSum = std::accumulate(evenCounts.begin(), evenCounts.end(), 0);
	int mismatch = oddSum + evenSum - numModules;
	bool oddParityBad = (oddSum & 0x01) == (outsideChar ? 1 : 0);
	bool evenParityBad = (evenSum & 0x01) == 1;

	bool incrementOdd = false;
	bool decrementOdd = false;
	bool incrementEven = false;
	bool decrementEven = false;

	if (outsideChar) {
		if (oddSum > 12) {
			decrementOdd = true;
		}
		else if (oddSum < 4) {
			incrementOdd = true;
		}
		if (evenSum > 12) {
			decrementEven = true;
		}
		else if (evenSum < 4) {
			incrementEven = true;
		}
	}
	else {
		if (oddSum > 11) {
			decrementOdd = true;
		}
		else if (oddSum < 5) {
			incrementOdd = true;
		}
		if (evenSum > 10) {
			decrementEven = true;
		}
		else if (evenSum < 4) {
			incrementEven = true;
		}
	}

	/*if (mismatch == 2) {
	if (!(oddParityBad && evenParityBad)) {
	throw ReaderException.getInstance();
	}
	decrementOdd = true;
	decrementEven = true;
	} else if (mismatch == -2) {
	if (!(oddParityBad && evenParityBad)) {
	throw ReaderException.getInstance();
	}
	incrementOdd = true;
	incrementEven = true;
	} else */
	if (mismatch == 1) {
		if (oddParityBad) {
			if (evenParityBad) {
				return false;
			}
			decrementOdd = true;
		}
		else {
			if (!evenParityBad) {
				return false;
			}
			decrementEven = true;
		}
	}
	else if (mismatch == -1) {
		if (oddParityBad) {
			if (evenParityBad) {
				return false;
			}
			incrementOdd = true;
		}
		else {
			if (!evenParityBad) {
				return false;
			}
			incrementEven = true;
		}
	}
	else if (mismatch == 0) {
		if (oddParityBad) {
			if (!evenParityBad) {
				return false;
			}
			// Both bad
			if (oddSum < evenSum) {
				incrementOdd = true;
				decrementEven = true;
			}
			else {
				decrementOdd = true;
				incrementEven = true;
			}
		}
		else {
			if (evenParityBad) {
				return false;
			}
			// Nothing to do!
		}
	}
	else {
		return false;
	}

	if (incrementOdd) {
		if (decrementOdd) {
			return false;
		}
		oddCounts[std::max_element(oddRoundingErrors.begin(), oddRoundingErrors.end()) - oddRoundingErrors.begin()] += 1;
	}
	if (decrementOdd) {
		oddCounts[std::min_element(oddRoundingErrors.begin(), oddRoundingErrors.end()) - oddRoundingErrors.begin()] -= 1;
	}
	if (incrementEven) {
		if (decrementEven) {
			return false;
		}
		evenCounts[std::max_element(evenRoundingErrors.begin(), evenRoundingErrors.end()) - evenRoundingErrors.begin()] += 1;
	}
	if (decrementEven) {
		evenCounts[std::min_element(evenRoundingErrors.begin(), evenRoundingErrors.end()) - evenRoundingErrors.begin()] -= 1;
	}
	return true;
}

static RSS::DataCharacter
DecodeDataCharacter(const BitArray& row, const RSS::FinderPattern& pattern, bool outsideChar)
{
	std::array<int, 8> counters = {};

	if (outsideChar) {
		if (!RowReader::RecordPatternInReverse(row.begin(), row.iterAt(pattern.startPos()), counters))
			return {};
	}
	else {
		if (!RowReader::RecordPattern(row.iterAt(pattern.endPos() + 1), row.end(), counters))
			return {};
		std::reverse(counters.begin(), counters.end());
	}

	int numModules = outsideChar ? 16 : 15;
	float elementWidth = static_cast<float>(std::accumulate(counters.begin(), counters.end(), 0)) / static_cast<float>(numModules);

	std::array<int, 4> oddCounts;
	std::array<int, 4> evenCounts;
	std::array<float, 4> oddRoundingErrors;
	std::array<float, 4> evenRoundingErrors;

	for (int i = 0; i < 8; i++) {
		float value = (float)counters[i] / elementWidth;
		int count = (int)(value + 0.5f); // Round
		if (count < 1) {
			count = 1;
		}
		else if (count > 8) {
			count = 8;
		}
		int offset = i / 2;
		if ((i & 0x01) == 0) {
			oddCounts[offset] = count;
			oddRoundingErrors[offset] = value - count;
		}
		else {
			evenCounts[offset] = count;
			evenRoundingErrors[offset] = value - count;
		}
	}

	if (!AdjustOddEvenCounts(outsideChar, numModules, oddCounts, evenCounts, oddRoundingErrors, evenRoundingErrors)) {
		return {};
	}

	int oddSum = 0;
	int oddChecksumPortion = 0;
	for (auto it = oddCounts.rbegin(); it != oddCounts.rend(); ++it) {
		oddChecksumPortion *= 9;
		oddChecksumPortion += *it;
		oddSum += *it;
	}
	int evenChecksumPortion = 0;
	int evenSum = 0;
	for (auto it = evenCounts.rbegin(); it != evenCounts.rend(); ++it) {
		evenChecksumPortion *= 9;
		evenChecksumPortion += *it;
		evenSum += *it;
	}
	int checksumPortion = oddChecksumPortion + 3 * evenChecksumPortion;

	if (outsideChar) {
		if ((oddSum & 0x01) != 0 || oddSum > 12 || oddSum < 4) {
			return {};
		}
		int group = (12 - oddSum) / 2;
		int oddWidest = OUTSIDE_ODD_WIDEST[group];
		int evenWidest = 9 - oddWidest;
		int vOdd = RSS::ReaderHelper::GetRSSvalue(oddCounts, oddWidest, false);
		int vEven = RSS::ReaderHelper::GetRSSvalue(evenCounts, evenWidest, true);
		int tEven = OUTSIDE_EVEN_TOTAL_SUBSET[group];
		int gSum = OUTSIDE_GSUM[group];
		return {vOdd * tEven + vEven + gSum, checksumPortion};
	}
	else {
		if ((evenSum & 0x01) != 0 || evenSum > 10 || evenSum < 4) {
			return {};
		}
		int group = (10 - evenSum) / 2;
		int oddWidest = INSIDE_ODD_WIDEST[group];
		int evenWidest = 9 - oddWidest;
		int vOdd = RSS::ReaderHelper::GetRSSvalue(oddCounts, oddWidest, true);
		int vEven = RSS::ReaderHelper::GetRSSvalue(evenCounts, evenWidest, false);
		int tOdd = INSIDE_ODD_TOTAL_SUBSET[group];
		int gSum = INSIDE_GSUM[group];
		return {vEven * tOdd + vOdd + gSum, checksumPortion};
	}

}

static RSS::Pair
DecodePair(const BitArray& row, bool right, int rowNumber)
{
	FinderCounters finderCounters = {};

	auto range = FindFinderPattern(row, right, finderCounters);
	auto pattern = ParseFoundFinderPattern(row, rowNumber, right, range, finderCounters);
	if (pattern.isValid()) {
		auto outside = DecodeDataCharacter(row, pattern, true);
		if (outside.isValid()) {
			auto inside = DecodeDataCharacter(row, pattern, false);
			if (inside.isValid()) {
				return {1597 * outside.value() + inside.value(), outside.checksumPortion() + 4 * inside.checksumPortion(), pattern};
			}
		}
	}
	return {};
}

static void
AddOrTally(std::list<RSS::Pair>& possiblePairs, const RSS::Pair& pair)
{
	if (!pair.isValid()) {
		return;
	}
	for (RSS::Pair& other : possiblePairs) {
		if (other.value() == pair.value()) {
			other.incrementCount();
			return;
		}
	}
	possiblePairs.push_back(pair);
}

static bool
CheckChecksum(const RSS::Pair& leftPair, const RSS::Pair& rightPair)
{
	//int leftFPValue = leftPair.getFinderPattern().getValue();
	//int rightFPValue = rightPair.getFinderPattern().getValue();
	//if ((leftFPValue == 0 && rightFPValue == 8) ||
	//    (leftFPValue == 8 && rightFPValue == 0)) {
	//}
	int checkValue = (leftPair.checksumPortion() + 16 * rightPair.checksumPortion()) % 79;
	int targetCheckValue =
		9 * leftPair.finderPattern().value() + rightPair.finderPattern().value();
	if (targetCheckValue > 72) {
		targetCheckValue--;
	}
	if (targetCheckValue > 8) {
		targetCheckValue--;
	}
	return checkValue == targetCheckValue;
}

static Result
ConstructResult(const RSS::Pair& leftPair, const RSS::Pair& rightPair)
{
	int64_t symbolValue = 4537077 * static_cast<int64_t>(leftPair.value()) + rightPair.value();
	std::wstringstream buffer;
	buffer << std::setw(13) << std::setfill(L'0') << symbolValue;

	int checkDigit = 0;
	for (int i = 0; i < 13; i++) {
		int digit = buffer.get() - '0';
		checkDigit += (i & 0x01) == 0 ? 3 * digit : digit;
	}
	checkDigit = 10 - (checkDigit % 10);
	if (checkDigit == 10) {
		checkDigit = 0;
	}
	buffer.put((wchar_t)(checkDigit + '0'));

	auto& leftPoints = leftPair.finderPattern().points();
	auto& rightPoints = rightPair.finderPattern().points();
	return Result(buffer.str(), ByteArray(), { leftPoints[0], leftPoints[1], rightPoints[0], rightPoints[1] }, BarcodeFormat::RSS_14);
}

Result
RSS14Reader::decodeRow(int rowNumber, const BitArray& row_, std::unique_ptr<DecodingState>& state) const
{
	RSS14DecodingState* prevState = nullptr;
	if (state == nullptr) {
		state.reset(prevState = new RSS14DecodingState);
	}
	else {
#if !defined(ZX_HAVE_CONFIG)
		#error "You need to include ZXConfig.h"
#elif !defined(ZX_NO_RTTI)
		prevState = dynamic_cast<RSS14DecodingState*>(state.get());
#else
		prevState = static_cast<RSS14DecodingState*>(state.get());
#endif
	}

	if (prevState == nullptr) {
		throw std::runtime_error("Invalid state");
	}

	BitArray row = row_.copy();
	AddOrTally(prevState->possibleLeftPairs, DecodePair(row, false, rowNumber));
	row.reverse();
	AddOrTally(prevState->possibleRightPairs, DecodePair(row, true, rowNumber));
//	row.reverse();

	for (const auto& left : prevState->possibleLeftPairs) {
		if (left.count() > 1) {
			for (const auto& right : prevState->possibleRightPairs) {
				if (right.count() > 1) {
					if (CheckChecksum(left, right)) {
						return ConstructResult(left, right);
					}
				}
			}
		}
	}
	return Result(DecodeStatus::NotFound);
}

} // OneD
} // ZXing
