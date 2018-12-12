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

#include "oned/ODRSSExpandedReader.h"
#include "oned/rss/ODRSSReaderHelper.h"
#include "oned/rss/ODRSSExpandedBinaryDecoder.h"
#include "oned/rss/ODRSSExpandedRow.h"
#include "Result.h"
#include "BitArray.h"
#include "TextDecoder.h"
#include "ZXConfig.h"

#include <cmath>
#include <list>
#include <array>
#include <vector>
#include <numeric>
#include <algorithm>

namespace ZXing {
namespace OneD {

static const int SYMBOL_WIDEST[] = { 7, 5, 4, 3, 1 };
static const int EVEN_TOTAL_SUBSET[] = { 4, 20, 52, 104, 204 };
static const int GSUM[] = { 0, 348, 1388, 2948, 3988 };

static const std::array<std::array<int, 4>, 6> FINDER_PATTERNS = {
	1,8,4,1, // A
	3,6,4,1, // B
	3,4,6,1, // C
	3,2,8,1, // D
	2,6,5,1, // E
	2,2,9,1,  // F
};

static const std::array<std::array<int, 8>, 23> WEIGHTS = {
	1,   3,   9,   27,  81,  32,  96,  77,
	20,  60,  180, 118, 143, 7,   21,  63,
	189, 145, 13,  39,  117, 140, 209, 205,
	193, 157, 49,  147, 19,  57,  171, 91,
	62,  186, 136, 197, 169, 85,  44,  132,
	185, 133, 188, 142,  4,  12,  36,  108,
	113, 128, 173, 97,  80,  29,  87,  50,
	150, 28,  84,  41,  123, 158, 52,  156,
	46,  138, 203, 187, 139, 206, 196, 166,
	76,  17,  51,  153, 37,  111, 122, 155,
	43,  129, 176, 106, 107, 110, 119, 146,
	16,  48,  144, 10,  30,  90,  59,  177,
	109, 116, 137, 200, 178, 112, 125, 164,
	70,  210, 208, 202, 184, 130, 179, 115,
	134, 191, 151, 31,  93,  68,  204, 190,
	148, 22,  66,  198, 172, 94,  71,  2,
	6,   18,  54,  162, 64,  192, 154, 40,
	120, 149, 25,  75,  14,  42,  126, 167,
	79,  26,  78,  23,  69,  207, 199, 175,
	103, 98,  83,  38,  114, 131, 182, 124,
	161, 61,  183, 127, 170, 88,  53,  159,
	55,  165, 73,  8,   24,  72,  5,   15,
	45,  135, 194, 160, 58,  174, 100, 89,
};

static const int FINDER_PAT_A = 0;
static const int FINDER_PAT_B = 1;
static const int FINDER_PAT_C = 2;
static const int FINDER_PAT_D = 3;
static const int FINDER_PAT_E = 4;
static const int FINDER_PAT_F = 5;

static const std::array<std::vector<int>, 10> FINDER_PATTERN_SEQUENCES = { {
	{ FINDER_PAT_A, FINDER_PAT_A },
	{ FINDER_PAT_A, FINDER_PAT_B, FINDER_PAT_B },
	{ FINDER_PAT_A, FINDER_PAT_C, FINDER_PAT_B, FINDER_PAT_D },
	{ FINDER_PAT_A, FINDER_PAT_E, FINDER_PAT_B, FINDER_PAT_D, FINDER_PAT_C },
	{ FINDER_PAT_A, FINDER_PAT_E, FINDER_PAT_B, FINDER_PAT_D, FINDER_PAT_D, FINDER_PAT_F },
	{ FINDER_PAT_A, FINDER_PAT_E, FINDER_PAT_B, FINDER_PAT_D, FINDER_PAT_E, FINDER_PAT_F, FINDER_PAT_F },
	{ FINDER_PAT_A, FINDER_PAT_A, FINDER_PAT_B, FINDER_PAT_B, FINDER_PAT_C, FINDER_PAT_C, FINDER_PAT_D, FINDER_PAT_D },
	{ FINDER_PAT_A, FINDER_PAT_A, FINDER_PAT_B, FINDER_PAT_B, FINDER_PAT_C, FINDER_PAT_C, FINDER_PAT_D, FINDER_PAT_E, FINDER_PAT_E },
	{ FINDER_PAT_A, FINDER_PAT_A, FINDER_PAT_B, FINDER_PAT_B, FINDER_PAT_C, FINDER_PAT_C, FINDER_PAT_D, FINDER_PAT_E, FINDER_PAT_F, FINDER_PAT_F },
	{ FINDER_PAT_A, FINDER_PAT_A, FINDER_PAT_B, FINDER_PAT_B, FINDER_PAT_C, FINDER_PAT_D, FINDER_PAT_D, FINDER_PAT_E, FINDER_PAT_E, FINDER_PAT_F, FINDER_PAT_F },
} };


struct RSSExpandedDecodingState : public RowReader::DecodingState
{
	std::list<RSS::ExpandedRow> rows;
};


using namespace RSS;


static BitArray::Range
FindNextPair(const BitArray& row, const std::list<ExpandedPair>& previousPairs, int forcedOffset, bool startFromEven, std::array<int, 4>& counters)
{
	int rowOffset;
	if (forcedOffset >= 0) {
		rowOffset = forcedOffset;
	}
	else if (previousPairs.empty()) {
		rowOffset = 0;
	}
	else {
		rowOffset = previousPairs.back().finderPattern().endPos();
	}
	bool searchingEvenPair = previousPairs.size() % 2 != 0;
	if (startFromEven) {
		searchingEvenPair = !searchingEvenPair;
	}

	return RowReader::FindPattern(
	    // find
	    row.getNextSet(row.iterAt(rowOffset)), row.end(), counters,
	    [searchingEvenPair](BitArray::Iterator, BitArray::Iterator, std::array<int, 4>& counters) {
		    if (searchingEvenPair) {
			    std::reverse(counters.begin(), counters.end());
		    }
		    if (RSS::ReaderHelper::IsFinderPattern(counters))
				return true;
		    if (searchingEvenPair) {
			    std::reverse(counters.begin(), counters.end());
		    }
		    return false;
	    });
}

static FinderPattern
ParseFoundFinderPattern(const BitArray& row, int rowNumber, bool oddPattern, BitArray::Range range, std::array<int, 4>& counters) {
	// Actually we found elements 2-5.
	int firstCounter;

	if (oddPattern) {
		// If pattern number is odd, we need to locate element 1 *before* the current block.

		auto i = std::find(BitArray::ReverseIterator(range.begin), row.rend(), *range.begin);
		firstCounter = range.begin - i.base();
		range.begin = i.base();
	}
	else {
		// If pattern number is even, the pattern is reversed, so we need to locate element 1 *after* the current block.

		auto i = row.getNextUnset(std::next(range.end)); // +1?
		firstCounter = i - range.end;
		range.end = i;
	}

	int start = range.begin - row.begin();
	int end = range.end - row.begin();

	// Make 'counters' hold 1-4
	std::copy_backward(counters.begin(), counters.end() - 1, counters.end());
	counters[0] = firstCounter;
	int value = ReaderHelper::ParseFinderValue(counters, FINDER_PATTERNS);
	if (value < 0)
		return {};

	return {value, start, end, {ResultPoint(start, rowNumber), ResultPoint(end, rowNumber)}};
}

static bool
IsNotA1left(FinderPattern pattern, bool isOddPattern, bool leftChar)
{
	// A1: pattern.getValue is 0 (A), and it's an oddPattern, and it is a left char
	return !(pattern.value() == 0 && isOddPattern && leftChar);
}

static bool
AdjustOddEvenCounts(int numModules, std::array<int, 4>& oddCounts, std::array<int, 4>& evenCounts, const std::array<float, 4>& oddRoundingErrors, const std::array<float, 4>& evenRoundingErrors)
{
	int oddSum = std::accumulate(oddCounts.begin(), oddCounts.end(), 0);
	int evenSum = std::accumulate(evenCounts.begin(), evenCounts.end(), 0);
	int mismatch = oddSum + evenSum - numModules;
	bool oddParityBad = (oddSum & 0x01) == 1;
	bool evenParityBad = (evenSum & 0x01) == 0;

	bool incrementOdd = false;
	bool decrementOdd = false;

	if (oddSum > 13) {
		decrementOdd = true;
	}
	else if (oddSum < 4) {
		incrementOdd = true;
	}
	bool incrementEven = false;
	bool decrementEven = false;
	if (evenSum > 13) {
		decrementEven = true;
	}
	else if (evenSum < 4) {
		incrementEven = true;
	}

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
		//evenCounts[std::max_element(oddRoundingErrors.begin(), oddRoundingErrors.end()) - oddRoundingErrors.begin()] += 1;	// This is according to Java's code, but likely a bug!
		evenCounts[std::max_element(evenRoundingErrors.begin(), evenRoundingErrors.end()) - evenRoundingErrors.begin()] += 1;
	}
	if (decrementEven) {
		evenCounts[std::min_element(evenRoundingErrors.begin(), evenRoundingErrors.end()) - evenRoundingErrors.begin()] -= 1;
	}
	return true;
}

static DataCharacter
DecodeDataCharacter(const BitArray& row, const FinderPattern& pattern, bool isOddPattern, bool leftChar)
{
	std::array<int, 8> counters = {};

	if (leftChar) {
		if (!RowReader::RecordPatternInReverse(row.begin(), row.iterAt(pattern.startPos()), counters))
			return {};
	}
	else {
		if (!RowReader::RecordPattern(row.iterAt(pattern.endPos()), row.end(), counters))
			return {};
		std::reverse(counters.begin(), counters.end());
	}

	int numModules = 17; //left and right data characters have all the same length
	float elementWidth = static_cast<float>(std::accumulate(counters.begin(), counters.end(), 0)) / static_cast<float>(numModules);

	// Sanity check: element width for pattern and the character should match
	float expectedElementWidth = static_cast<float>(pattern.endPos() - pattern.startPos()) / 15.0f;
	if (std::abs(elementWidth - expectedElementWidth) / expectedElementWidth > 0.3f) {
		return {};
	}

	std::array<int, 4> oddCounts;
	std::array<int, 4> evenCounts;
	std::array<float, 4> oddRoundingErrors;
	std::array<float, 4> evenRoundingErrors;

	for (int i = 0; i < 8; i++) {
		float value = 1.0f * counters[i] / elementWidth;
		int count = (int)(value + 0.5f); // Round
		if (count < 1) {
			if (value < 0.3f) {
				return {};
			}
			count = 1;
		}
		else if (count > 8) {
			if (value > 8.7f) {
				return {};
			}
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

	if (!AdjustOddEvenCounts(numModules, oddCounts, evenCounts, oddRoundingErrors, evenRoundingErrors)) {
		return {};
	}

	int weightRowNumber = 4 * pattern.value() + (isOddPattern ? 0 : 2) + (leftChar ? 0 : 1) - 1;

	int oddSum = 0;
	int oddChecksumPortion = 0;
	for (int i = 3; i >= 0; i--) {
		if (IsNotA1left(pattern, isOddPattern, leftChar)) {
			int weight = WEIGHTS[weightRowNumber][2 * i];
			oddChecksumPortion += oddCounts[i] * weight;
		}
		oddSum += oddCounts[i];
	}
	int evenChecksumPortion = 0;
	//int evenSum = 0;
	for (int i = 3; i >= 0; i--) {
		if (IsNotA1left(pattern, isOddPattern, leftChar)) {
			int weight = WEIGHTS[weightRowNumber][2 * i + 1];
			evenChecksumPortion += evenCounts[i] * weight;
		}
		//evenSum += evenCounts[i];
	}
	int checksumPortion = oddChecksumPortion + evenChecksumPortion;

	if ((oddSum & 0x01) != 0 || oddSum > 13 || oddSum < 4) {
		return {};
	}

	int group = (13 - oddSum) / 2;
	int oddWidest = SYMBOL_WIDEST[group];
	int evenWidest = 9 - oddWidest;
	int vOdd = ReaderHelper::GetRSSvalue(oddCounts, oddWidest, true);
	int vEven = ReaderHelper::GetRSSvalue(evenCounts, evenWidest, false);
	int tEven = EVEN_TOTAL_SUBSET[group];
	int gSum = GSUM[group];
	int value = vOdd * tEven + vEven + gSum;

	return {value, checksumPortion};
}

// not private for testing
static bool
RetrieveNextPair(const BitArray& row, const std::list<ExpandedPair>& previousPairs, int rowNumber, bool startFromEven, ExpandedPair& outPair)
{
	bool isOddPattern = previousPairs.size() % 2 == 0;
	if (startFromEven) {
		isOddPattern = !isOddPattern;
	}

	FinderPattern pattern;
	bool keepFinding = true;
	int forcedOffset = -1;
	do {
		std::array<int, 4> counters = {};
		auto range = FindNextPair(row, previousPairs, forcedOffset, startFromEven, counters);
		if (!range)
			return false;

		pattern = ParseFoundFinderPattern(row, rowNumber, isOddPattern, range, counters);
		if (!pattern.isValid()) {
			// goto next bar of same color than current position
			range.begin = row.getNextSetTo(range.begin, !*range.begin);
			range.begin = row.getNextSetTo(range.begin, !*range.begin);
			forcedOffset = range.begin - row.begin();
		}
		else {
			keepFinding = false;
		}
	} while (keepFinding);

	// When stacked symbol is split over multiple rows, there's no way to guess if this pair can be last or not.
	// boolean mayBeLast = checkPairSequence(previousPairs, pattern);

	DataCharacter leftChar = DecodeDataCharacter(row, pattern, isOddPattern, true);
	if (!leftChar.isValid() || (!previousPairs.empty() && previousPairs.back().mustBeLast())) {
		return false;
	}

	DataCharacter rightChar = DecodeDataCharacter(row, pattern, isOddPattern, false);
	bool mayBeLast = true;
	outPair = ExpandedPair(leftChar, rightChar, pattern, mayBeLast);
	return true;
}

static bool
CheckChecksum(const std::list<ExpandedPair>& myPairs)
{
	if (myPairs.empty())
		return false;

	auto& firstPair = myPairs.front();
	
	if (firstPair.mustBeLast()) {
		return false;
	}

	int checksum = firstPair.rightChar().checksumPortion();
	int s = 2;

	for (auto it = ++myPairs.begin(); it != myPairs.end(); ++it) {
		checksum += it->leftChar().checksumPortion();
		s++;
		auto& currentRightChar = it->rightChar();
		if (currentRightChar.isValid()) {
			checksum += currentRightChar.checksumPortion();
			s++;
		}
	}
	checksum %= 211;
	int checkCharacterValue = 211 * (s - 4) + checksum;
	return checkCharacterValue == firstPair.leftChar().value();
}


// Returns true when one of the rows already contains all the pairs
static bool
IsPartialRow(const std::list<ExpandedPair>& pairs, const std::list<ExpandedRow>& rows) {
	for (const ExpandedRow& r : rows) {
		bool allFound = true;
		for (const ExpandedPair& p : pairs) {
			bool found = false;
			for (const ExpandedPair& pp : r.pairs()) {
				if (p == pp) {
					found = true;
					break;
				}
			}
			if (!found) {
				allFound = false;
				break;
			}
		}
		if (allFound) {
			// the row 'r' contain all the pairs from 'pairs'
			return true;
		}
	}
	return false;
}

// Remove all the rows that contains only specified pairs 
static void
RemovePartialRows(std::list<ExpandedRow>& rows, const std::list<ExpandedPair>& pairs)
{
	auto it = rows.begin();
	while (it != rows.end()) {
		//ExpandedRow r = iterator.next();
		if (it->pairs().size() == pairs.size()) {
			++it;
			continue;
		}
		bool allFound = true;
		for (const ExpandedPair& p : it->pairs()) {
			bool found = false;
			for (const ExpandedPair& pp : pairs) {
				if (p == pp) {
					found = true;
					break;
				}
			}
			if (!found) {
				allFound = false;
				break;
			}
		}
		if (allFound) {
			// 'pairs' contains all the pairs from the row 'r'
			auto remPos = it;
			++it;
			rows.erase(remPos);
		}
		else {
			++it;
		}
	}
}

static void
StoreRow(std::list<ExpandedRow>& rows, const std::list<ExpandedPair>& pairs, int rowNumber, bool wasReversed)
{
	// Discard if duplicate above or below; otherwise insert in order by row number.
	bool prevIsSame = false;
	bool nextIsSame = false;
	auto insertPos = rows.begin();
	for (; insertPos != rows.end(); ++insertPos) {
		if (insertPos->rowNumber() > rowNumber) {
			nextIsSame = insertPos->isEquivalent(pairs);
			break;
		}
		prevIsSame = insertPos->isEquivalent(pairs);
	}
	if (nextIsSame || prevIsSame) {
		return;
	}

	// When the row was partially decoded (e.g. 2 pairs found instead of 3),
	// it will prevent us from detecting the barcode.
	// Try to merge partial rows

	// Check whether the row is part of an allready detected row
	if (IsPartialRow(pairs, rows)) {
		return;
	}

	rows.insert(insertPos, ExpandedRow(pairs, rowNumber, wasReversed));

	RemovePartialRows(rows, pairs);
}

// Whether the pairs form a valid find pattern seqience,
// either complete or a prefix
static bool
IsValidSequence(const std::list<ExpandedPair>& pairs)
{
	for (auto& sequence : FINDER_PATTERN_SEQUENCES) {
		if (pairs.size() <= sequence.size() && std::equal(pairs.begin(), pairs.end(), sequence.begin(), [](const ExpandedPair& p, int seq) { return p.finderPattern().value() == seq; })) {
			return true;
		}
	}
	return false;
}

// Try to construct a valid rows sequence
// Recursion is used to implement backtracking
template <typename RowIterator>
static std::list<ExpandedPair>
CheckRows(RowIterator currentRow, RowIterator endRow, const std::list<ExpandedRow>& collectedRows)
{
	std::list<ExpandedPair> collectedPairs;
	for (const auto& collectedRow : collectedRows) {
		auto &p = collectedRow.pairs();
		collectedPairs.insert(collectedPairs.end(), p.begin(), p.end());
	}

	for (; currentRow != endRow; ++currentRow) {
		//ExpandedRow row = rows.get(i);
		std::list<ExpandedPair> result = collectedPairs;
		auto &p = currentRow->pairs();
		result.insert(result.end(), p.begin(), p.end());

		if (!IsValidSequence(result)) {
			continue;
		}

		if (CheckChecksum(result)) {
			return result;
		}

		std::list<ExpandedRow> rs = collectedRows;
		rs.push_back(*currentRow);
		auto nextRow = currentRow;
		result = CheckRows(++nextRow, endRow, rs);
		if (!result.empty()) {
			return result;
		}
	}
	return std::list<ExpandedPair>();
}

static std::list<ExpandedPair>
CheckRows(std::list<ExpandedRow>& rows, bool reverse) {
	// Limit number of rows we are checking
	// We use recursive algorithm with pure complexity and don't want it to take forever
	// Stacked barcode can have up to 11 rows, so 25 seems reasonable enough
	if (rows.size() > 25) {
		rows.clear();  // We will never have a chance to get result, so clear it
		return std::list<ExpandedPair>();
	}

	return reverse ?
		CheckRows(rows.rbegin(), rows.rend(), std::list<ExpandedRow>()) :
		CheckRows(rows.begin(), rows.end(), std::list<ExpandedRow>());
}

// Not private for testing
static std::list<ExpandedPair>
DecodeRow2Pairs(int rowNumber, const BitArray& row, bool startFromEven, std::list<ExpandedRow>& rows)
{
	std::list<ExpandedPair> pairs;
	ExpandedPair nextPair;
	while (RetrieveNextPair(row, pairs, rowNumber, startFromEven, nextPair)) {
		pairs.push_back(nextPair);
	}

	if (pairs.empty()) {
		return pairs;
	}

	// TODO: verify sequence of finder patterns as in checkPairSequence()
	if (CheckChecksum(pairs)) {
		return pairs;
	}

	bool tryStackedDecode = !rows.empty();
	bool wasReversed = false; // TODO: deal with reversed rows
	StoreRow(rows, pairs, rowNumber, wasReversed);
	if (tryStackedDecode) {
		// When the image is 180-rotated, then rows are sorted in wrong direction.
		// Try twice with both the directions.
		auto ps = CheckRows(rows, false);
		if (!ps.empty()) {
			return ps;
		}
		ps = CheckRows(rows, true);
		if (!ps.empty()) {
			return ps;
		}
	}
	return std::list<ExpandedPair>();
}

/**
* @author Pablo Orduña, University of Deusto (pablo.orduna@deusto.es)
* @author Eduardo Castillejo, University of Deusto (eduardo.castillejo@deusto.es)
*/
static BitArray
BuildBitArray(const std::list<ExpandedPair>& pairs)
{
	int charNumber = (static_cast<int>(pairs.size()) * 2) - 1;
	if (pairs.back().mustBeLast()) {
		charNumber -= 1;
	}

	BitArray result(12 * charNumber);
	int accPos = 0;
	auto it = pairs.begin();
	int firstValue = it->rightChar().value();
	for (int i = 11; i >= 0; --i) {
		if ((firstValue & (1 << i)) != 0) {
			result.set(accPos);
		}
		accPos++;
	}

	for (++it; it != pairs.end(); ++it) {
		int leftValue = it->leftChar().value();
		for (int j = 11; j >= 0; --j) {
			if ((leftValue & (1 << j)) != 0) {
				result.set(accPos);
			}
			accPos++;
		}

		if (it->rightChar().isValid()) {
			int rightValue = it->rightChar().value();
			for (int j = 11; j >= 0; --j) {
				if ((rightValue & (1 << j)) != 0) {
					result.set(accPos);
				}
				accPos++;
			}
		}
	}
	return result;
}

// Not private for unit testing
static Result
ConstructResult(const std::list<ExpandedPair>& pairs)
{
	if (pairs.empty()) {
		return Result(DecodeStatus::NotFound);
	}

	BitArray binary = BuildBitArray(pairs);
	auto resultString = ExpandedBinaryDecoder::Decode(binary);
	if (resultString.empty()) {
		return Result(DecodeStatus::NotFound);
	}

	auto& firstPoints = pairs.front().finderPattern().points();
	auto& lastPoints = pairs.back().finderPattern().points();

	return Result(TextDecoder::FromLatin1(resultString), ByteArray(), { firstPoints[0], firstPoints[1], lastPoints[0], lastPoints[1] }, BarcodeFormat::RSS_EXPANDED);
}

Result
RSSExpandedReader::decodeRow(int rowNumber, const BitArray& row, std::unique_ptr<DecodingState>& state) const
{
	RSSExpandedDecodingState* prevState = nullptr;
	if (state == nullptr) {
		state.reset(prevState = new RSSExpandedDecodingState);
	}
	else {
#if !defined(ZX_HAVE_CONFIG)
		#error "You need to include ZXConfig.h"
#elif !defined(ZX_NO_RTTI)
		prevState = dynamic_cast<RSSExpandedDecodingState*>(state.get());
#else
		prevState = static_cast<RSSExpandedDecodingState*>(state.get());
#endif
	}

	if (prevState == nullptr) {
		throw std::runtime_error("Invalid state");
	}

	// Rows can start with even pattern in case in prev rows there where odd number of patters.
	// So lets try twice
	Result r = ConstructResult(DecodeRow2Pairs(rowNumber, row, false, prevState->rows));
	if (!r.isValid()) {
		r = ConstructResult(DecodeRow2Pairs(rowNumber, row, true, prevState->rows));
	}
	return r;
}

} // OneD
} // ZXing
