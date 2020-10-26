/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
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

#include "ODDataBarReader.h"

#include "ODDataBarCommon.h"
#include "rss/ODRSSReaderHelper.h"

#include "BarcodeFormat.h"
#include "Result.h"
#include "GTIN.h"
#include "TextDecoder.h"

#include <sstream>
#include <iomanip>
#include <unordered_set>

namespace ZXing {
namespace OneD {

DataBarReader::DataBarReader(const DecodeHints&) {}
DataBarReader::~DataBarReader() = default;

Result DataBarReader::decodeRow(int, const BitArray&, std::unique_ptr<RowReader::DecodingState>&) const
{
	return Result(DecodeStatus::FormatError);
}

inline bool IsFinderPattern(int a, int b, int c, int d, int e)
{
	//  a,b,c,d,e, g | sum(a..e) = 15
	//  ------------
	//  1,1,2
	//	| | |,1,1, 1
	//	3,8,9

	// use only pairs of bar+space to limit effect of poor threshold:
	// b+c can be 10, 11 or 12 modules, d+e is always 2
	int w = 2 * (b + c), n = d + e;
	// the offsets (5 and 2) are there to reduce quantization effects for small module sizes
	// TODO: review after switch to sub-pixel bar width calculation
	return (w + 5 > 9 * n) && (w - 5 < 13 * n) && (b < 5 + 9 * d) && (c < 5 + 10 * e) && (a < 2 + 4 * e) && (4 * a > n);
};

inline bool IsLeftPair(const PatternView& v)
{
	return IsFinderPattern(v[8], v[9], v[10], v[11], v[12]) && IsGuard(v[-1], v[11]) && IsCharacterPair(v, 16, 15);
}

inline bool IsRightPair(const PatternView& v)
{
	return IsFinderPattern(v[12], v[11], v[10], v[9], v[8]) && IsGuard(v[9], v[21]) && IsCharacterPair(v, 15, 16);
}

static Character ReadDataCharacter(const PatternView& view, bool outsideChar, bool rightPair)
{
	constexpr int OUTSIDE_EVEN_TOTAL_SUBSET[] = {1, 10, 34, 70, 126};
	constexpr int INSIDE_ODD_TOTAL_SUBSET[]   = {4, 20, 48, 81};
	constexpr int OUTSIDE_GSUM[]              = {0, 161, 961, 2015, 2715};
	constexpr int INSIDE_GSUM[]               = {0, 336, 1036, 1516};
	constexpr int OUTSIDE_ODD_WIDEST[]        = {8, 6, 4, 3, 1};
	constexpr int INSIDE_ODD_WIDEST[]         = {2, 4, 6, 8};

	Array4I oddPattern = {}, evnPattern = {};
	if (!ReadDataCharacterRaw(view, outsideChar ? 16 : 15, outsideChar == rightPair, oddPattern, evnPattern))
		return {};

	auto calcChecksumPortion = [](const Array4I& counts) {
		int res = 0;
		for (auto it = counts.rbegin(); it != counts.rend(); ++it)
			res = 9 * res + *it;
		return res;
	};

	int checksumPortion = calcChecksumPortion(oddPattern) + 3 * calcChecksumPortion(evnPattern);

	if (outsideChar) {
		int oddSum = Reduce(oddPattern);
		assert((oddSum & 1) == 0 && oddSum <= 12 && oddSum >= 4); // checked in ReadDataCharacterRaw
		int group = (12 - oddSum) / 2;
		int oddWidest = OUTSIDE_ODD_WIDEST[group];
		int evnWidest = 9 - oddWidest;
		int vOdd = RSS::ReaderHelper::GetRSSvalue(oddPattern, oddWidest, false);
		int vEvn = RSS::ReaderHelper::GetRSSvalue(evnPattern, evnWidest, true);
		int tEvn = OUTSIDE_EVEN_TOTAL_SUBSET[group];
		int gSum = OUTSIDE_GSUM[group];
		return {vOdd * tEvn + vEvn + gSum, checksumPortion};
	} else {
		int evnSum = Reduce(evnPattern);
		assert((evnSum & 1) == 0 && evnSum <= 12 && evnSum >= 4); // checked in ReadDataCharacterRaw
		int group = (10 - evnSum) / 2;
		int oddWidest = INSIDE_ODD_WIDEST[group];
		int evnWidest = 9 - oddWidest;
		int vOdd = RSS::ReaderHelper::GetRSSvalue(oddPattern, oddWidest, true);
		int vEvn = RSS::ReaderHelper::GetRSSvalue(evnPattern, evnWidest, false);
		int tOdd = INSIDE_ODD_TOTAL_SUBSET[group];
		int gSum = INSIDE_GSUM[group];
		return {vEvn * tOdd + vOdd + gSum, checksumPortion};
	}
}

int ParseFinderPattern(const PatternView& view, bool reversed)
{
	constexpr float MAX_AVG_VARIANCE        = 0.2f;
	constexpr float MAX_INDIVIDUAL_VARIANCE = 0.45f;
	constexpr FixedPattern<5, 15> FINDER_PATTERNS[] = {
		{3,8,2,1,1},
		{3,5,5,1,1},
		{3,3,7,1,1},
		{3,1,9,1,1},
		{2,7,4,1,1},
		{2,5,6,1,1},
		{2,3,8,1,1},
		{1,5,7,1,1},
		{1,3,9,1,1},
	};
	// TODO: c++20 constexpr inversion from FIND_PATTERN?
	constexpr FixedPattern<5, 15> REVERSED_FINDER_PATTERNS[] = {
		{1, 1, 2, 8, 3}, {1, 1, 5, 5, 3}, {1, 1, 7, 3, 3}, {1, 1, 9, 1, 3}, {1, 1, 4, 7, 2},
		{1, 1, 6, 5, 2}, {1, 1, 8, 3, 2}, {1, 1, 7, 5, 1}, {1, 1, 9, 3, 1},
	};

	return RowReader::DecodeDigit(view.subView(0, 5), reversed ? REVERSED_FINDER_PATTERNS : FINDER_PATTERNS,
								  MAX_AVG_VARIANCE, MAX_INDIVIDUAL_VARIANCE, true);
}

constexpr int PAIR_SIZE = 8 + 5 + 8;

static Pair ReadPair(const PatternView& view, bool rightPair)
{
	if (int pattern = ParseFinderPattern(view.subView(8, 5), rightPair); pattern != -1)
		if (auto outside = ReadDataCharacter(view.subView(rightPair ? 13 : 0, 8), true, rightPair))
			if (auto inside = ReadDataCharacter(view.subView(rightPair ? 0 : 13, 8), false, rightPair)) {
				// include left and right guards
				int xStart = view.pixelsInFront() - (rightPair ? 0 : view[-1] + std::min(view[-2], view[-1]));
				int xStop  = view.pixelsTillEnd() + (rightPair ? view[PAIR_SIZE] + view[PAIR_SIZE + 1] : 0);
				Character character = {1597 * outside.value + inside.value, outside.checksum + 4 * inside.checksum};
				return {character, pattern, xStart, xStop};
			}

	return {};
}

static bool CheckChecksum(Pair leftPair, Pair rightPair)
{
	int a = (leftPair.checksum + 16 * rightPair.checksum) % 79;
	int b = 9 * leftPair.finder + rightPair.finder;
	if (b > 72)
		b--;
	if (b > 8)
		b--;
	return a == b;
}

static std::string ConstructText(Pair leftPair, Pair rightPair)
{
	auto res = 4537077LL * leftPair.value + rightPair.value;
	std::ostringstream txt;
	txt << std::setw(13) << std::setfill('0') << res;
	txt << GTIN::ComputeCheckDigit(txt.str());
	return txt.str();
}

struct State : public RowReader::DecodingState
{
	std::unordered_set<Pair, PairHash> leftPairs;
	std::unordered_set<Pair, PairHash> rightPairs;
};

Result DataBarReader::decodePattern(int rowNumber, const PatternView& view,
									std::unique_ptr<RowReader::DecodingState>& state) const
{
#if 0 // non-stacked version
	auto next = view.subView(-1, PAIR_SIZE);
	// yes: the first view we test is at index 1 (black bar at 0 would be the guard pattern)
	while (next.shift(2)) {
		if (IsLeftPair(next)) {
			if (auto leftPair = ReadPair(next, false); leftPair && next.skipSymbol() && IsRightPair(next)) {
				if (auto rightPair = ReadPair(next, true); rightPair && CheckChecksum(leftPair, rightPair)) {
					return {ConstructText(leftPair, rightPair), rowNumber, leftPair.xStart, rightPair.xStop,
							BarcodeFormat::DataBar};
				}
			}
		}
	}
#else
	if (!state)
		state.reset(new State);
	auto* prevState = static_cast<State*>(state.get());

	auto next = view.subView(0, PAIR_SIZE);
	// yes: the first view we test is at index 1 (black bar at 0 would be the guard pattern)
	while (next.shift(1)) {
		if (IsLeftPair(next)) {
			if (auto leftPair = ReadPair(next, false)) {
				leftPair.rowNumber = rowNumber;
				prevState->leftPairs.insert(leftPair);
				next.skipSymbol();
				next.shift(-1);
			}
		}

		if (next.shift(1) && IsRightPair(next)) {
			if (auto rightPair = ReadPair(next, true)) {
				rightPair.rowNumber = rowNumber;
				prevState->rightPairs.insert(rightPair);
			}
		}
	}

	for (const auto& leftPair : prevState->leftPairs)
		for (const auto& rightPair : prevState->rightPairs)
			if (CheckChecksum(leftPair, rightPair))
			{
				Position p = leftPair.rowNumber == rightPair.rowNumber
								 ? Line(leftPair.rowNumber, leftPair.xStart, leftPair.xStop)
								 : Position{{leftPair.xStart, leftPair.rowNumber},
											{leftPair.xStop, leftPair.rowNumber},
											{rightPair.xStop, rightPair.rowNumber},
											{rightPair.xStart, rightPair.rowNumber}};
				return {TextDecoder::FromLatin1(ConstructText(leftPair, rightPair)), std::move(p),
						BarcodeFormat::DataBar};
			}
#endif

	return Result(DecodeStatus::NotFound);
}

} // OneD
} // ZXing
