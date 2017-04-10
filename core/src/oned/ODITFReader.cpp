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

#include "oned/ODITFReader.h"
#include "Result.h"
#include "BitArray.h"
#include "DecodeHints.h"
#include "TextDecoder.h"
#include "ZXContainerAlgorithms.h"

#include <array>

namespace ZXing {

namespace OneD {

static const float MAX_AVG_VARIANCE = 0.38f;
static const float MAX_INDIVIDUAL_VARIANCE = 0.78f;

static const int W = 3; // Pixel width of a wide line
static const int N = 1; // Pixed width of a narrow line

/** Valid ITF lengths. Anything longer than the largest value is also allowed. */
static const std::array<int, 5> DEFAULT_ALLOWED_LENGTHS = { 6, 8, 10, 12, 14 };

/**
* Start/end guard pattern.
*
* Note: The end pattern is reversed because the row is reversed before
* searching for the END_PATTERN
*/
static const std::array<int, 4> START_PATTERN = { N, N, N, N };
static const std::array<int, 3> END_PATTERN_REVERSED = { N, N, W };

/**
* Patterns of Wide / Narrow lines to indicate each digit
*/
static const std::array<std::array<int, 5>, 10> PATTERNS = {
	N, N, W, W, N, // 0
	W, N, N, N, W, // 1
	N, W, N, N, W, // 2
	W, W, N, N, N, // 3
	N, N, W, N, W, // 4
	W, N, W, N, N, // 5
	N, W, W, N, N, // 6
	N, N, N, W, W, // 7
	W, N, N, W, N, // 8
	N, W, N, W, N,  // 9
};

/**
* Attempts to decode a sequence of ITF black/white lines into single
* digit.
*
* @param counters the counts of runs of observed black/white/black/... values
* @return The decoded digit
* @throws NotFoundException if digit cannot be decoded
*/
static bool DecodeDigit(const std::array<int, 5>& counters, int* outCode)
{
	float bestVariance = MAX_AVG_VARIANCE; // worst variance we'll accept
	int bestMatch = -1;
	for (size_t i = 0; i < PATTERNS.size(); i++) {
		auto& pattern = PATTERNS[i];
		float variance = RowReader::PatternMatchVariance(counters, pattern, MAX_INDIVIDUAL_VARIANCE);
		if (variance < bestVariance) {
			bestVariance = variance;
			bestMatch = static_cast<int>(i);
		}
	}
	if (bestMatch >= 0) {
		*outCode = bestMatch;
		return true;
	}
	return false;
}

/**
* @param row          row of black/white values to search
* @param payloadStart offset of start pattern
* @param resultString {@link StringBuilder} to append decoded chars to
* @throws NotFoundException if decoding could not complete successfully
*/
static bool DecodeMiddle(BitArray::Iterator begin, BitArray::Iterator end, std::string* resultString)
{
	// Digits are interleaved in pairs - 5 black lines for one digit, and the 5
	// interleaved white lines for the second digit.
	// Therefore, need to scan 10 lines and then
	// split these into two arrays
	std::array<int, 10> counterDigitPair = {};
	std::array<int, 5> counterBlack = {};
	std::array<int, 5> counterWhite = {};

	while (begin != end) {

		// Get 10 runs of black/white.
		auto range = RowReader::RecordPattern(begin, end, counterDigitPair);
		if (!range)
			return false;

		// Split them into each array
		for (int k = 0; k < 5; k++) {
			int twoK = 2 * k;
			counterBlack[k] = counterDigitPair[twoK];
			counterWhite[k] = counterDigitPair[twoK + 1];
		}

		int bestMatch = 0;
		if (!DecodeDigit(counterBlack, &bestMatch))
			return false;

		resultString->push_back((char)('0' + bestMatch));

		if (!DecodeDigit(counterWhite, &bestMatch))
			return false;

		resultString->push_back((char)('0' + bestMatch));

		begin = range.end;
	}
	return true;
}

/**
* @param row       row of black/white values to search
* @param pattern   pattern of counts of number of black and white pixels that are
*                  being searched for as a pattern
* @return start/end horizontal offset of guard pattern, as an array of two
*         ints
* @throws NotFoundException if pattern is not found
*/
template <typename Container>
static BitArray::Range
FindGuardPattern(const BitArray& row, const Container& pattern)
{
	Container counters;

	return RowReader::FindPattern(
	    row.getNextSet(row.begin()), row.end(), counters,
	    [&row, &pattern](BitArray::Iterator begin, BitArray::Iterator end, const Container& counters) {
		    if (!(RowReader::PatternMatchVariance(counters, pattern, MAX_INDIVIDUAL_VARIANCE) < MAX_AVG_VARIANCE))
			    return false;

		    // The start & end patterns must be pre/post fixed by a quiet zone. This
		    // zone must be at least 10 times the width of a narrow line.  Scan back until
		    // we either get to the start of the barcode or match the necessary number of
		    // quiet zone pixels.
		    // ref: http://www.barcode-1.net/i25code.html

			// Determine the width of a narrow line in pixels. Both the start and the end pattern
			// start with 2 narrow lines.
			int narrowLineWidth = (counters[0] + counters[1]) / 2;
			int quietZoneWidth = 10 * narrowLineWidth;
		    return row.hasQuiteZone(begin, -quietZoneWidth);
	    });
}

/**
* Identify where the start of the middle / payload section starts.
*
* @param row row of black/white values to search
* @return Array, containing index of start of 'start block' and end of
*         'start block'
* @throws NotFoundException
*/
static BitArray::Range DecodeStart(const BitArray& row)
{
	return FindGuardPattern(row, START_PATTERN);
}

/**
* Identify where the end of the middle / payload section ends.
*
* @param row row of black/white values to search
* @return Array, containing index of start of 'end block' and end of 'end
*         block'
* @throws NotFoundException
*/
static BitArray::Range DecodeEnd(const BitArray& row)
{
	BitArray revRow;
	row.copyTo(revRow);
	// For convenience, reverse the row and then
	// search from 'the start' for the end block
	revRow.reverse();
	auto range = FindGuardPattern(revRow, END_PATTERN_REVERSED);

	// Now recalculate the indices of where the 'endblock' starts & stops to accommodate
	// the reversed nature of the search
	return {row.iterAt(row.size() - (range.end - revRow.begin())),
	        row.iterAt(row.size() - (range.begin - revRow.begin()))};
}

ITFReader::ITFReader(const DecodeHints& hints) :
	_allowedLengths(hints.allowedLengths())
{
	if (_allowedLengths.empty()) {
		_allowedLengths.assign(DEFAULT_ALLOWED_LENGTHS.begin(), DEFAULT_ALLOWED_LENGTHS.end());
	}
}

Result
ITFReader::decodeRow(int rowNumber, const BitArray& row, std::unique_ptr<DecodingState>& state) const
{
	// Find out where the Middle section (payload) starts & ends
	auto startRange = DecodeStart(row);
	if (!startRange)
		return Result(DecodeStatus::NotFound);

	auto endRange = DecodeEnd(row);
	if (!endRange)
		return Result(DecodeStatus::NotFound);

	std::string result;
	result.reserve(20);
	if (!DecodeMiddle(startRange.end, endRange.begin, &result))
		return Result(DecodeStatus::NotFound);

	// To avoid false positives with 2D barcodes (and other patterns), make
	// an assumption that the decoded string must be a 'standard' length if it's short
	int length = static_cast<int>(result.length());
	if (!_allowedLengths.empty() && !Contains(_allowedLengths, length)) {
		int maxAllowedLength = *std::max_element(_allowedLengths.begin(), _allowedLengths.end());
		if (length < maxAllowedLength)
			return Result(DecodeStatus::FormatError);
	}

	float x1 = startRange.end - row.begin();
	float x2 = endRange.begin - row.begin();
	float ypos = static_cast<float>(rowNumber);

	return Result(TextDecoder::FromLatin1(result), ByteArray(), { ResultPoint(x1, ypos), ResultPoint(x2, ypos) }, BarcodeFormat::ITF);
}

} // OneD
} // ZXing
