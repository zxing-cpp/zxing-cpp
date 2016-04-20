/*
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
static ErrorStatus
DecodeDigit(const std::array<int, 5>& counters, int& outCode)
{
	float bestVariance = MAX_AVG_VARIANCE; // worst variance we'll accept
	int bestMatch = -1;
	for (size_t i = 0; i < PATTERNS.size(); i++) {
		auto& pattern = PATTERNS[i];
		float variance = Reader::PatternMatchVariance(counters, pattern, MAX_INDIVIDUAL_VARIANCE);
		if (variance < bestVariance) {
			bestVariance = variance;
			bestMatch = static_cast<int>(i);
		}
	}
	if (bestMatch >= 0) {
		outCode = bestMatch;
		return ErrorStatus::NoError;
	}
	return ErrorStatus::NotFound;
}

/**
* @param row          row of black/white values to search
* @param payloadStart offset of start pattern
* @param resultString {@link StringBuilder} to append decoded chars to
* @throws NotFoundException if decoding could not complete successfully
*/
static ErrorStatus DecodeMiddle(const BitArray& row, int payloadStart, int payloadEnd, std::string& resultString)
{
	// Digits are interleaved in pairs - 5 black lines for one digit, and the
	// 5
	// interleaved white lines for the second digit.
	// Therefore, need to scan 10 lines and then
	// split these into two arrays
	std::array<int, 10> counterDigitPair = {};
	std::array<int, 5> counterBlack = {};
	std::array<int, 5> counterWhite = {};

	ErrorStatus status;

	while (payloadStart < payloadEnd) {

		// Get 10 runs of black/white.
		status = Reader::RecordPattern(row, payloadStart, counterDigitPair);
		if (StatusIsError(status)) {
			return status;
		}

		// Split them into each array
		for (int k = 0; k < 5; k++) {
			int twoK = 2 * k;
			counterBlack[k] = counterDigitPair[twoK];
			counterWhite[k] = counterDigitPair[twoK + 1];
		}

		int bestMatch = 0;
		status = DecodeDigit(counterBlack, bestMatch);
		if (StatusIsError(status)) {
			return status;
		}
		resultString.push_back((char)('0' + bestMatch));
		status = DecodeDigit(counterWhite, bestMatch);
		if (StatusIsError(status)) {
			return status;
		}
		resultString.push_back((char)('0' + bestMatch));
		for (int counterDigit : counterDigitPair) {
			payloadStart += counterDigit;
		}
	}
	return ErrorStatus::NoError;
}

/**
* @param row       row of black/white values to search
* @param rowOffset position to start search
* @param pattern   pattern of counts of number of black and white pixels that are
*                  being searched for as a pattern
* @return start/end horizontal offset of guard pattern, as an array of two
*         ints
* @throws NotFoundException if pattern is not found
*/
ErrorStatus
ITFReader::FindGuardPattern(const BitArray& row, int rowOffset, const int* pattern, size_t patternLength, int& begin, int& end)
{
	std::vector<int> counters(patternLength, 0);
	int width = row.size();
	bool isWhite = false;

	int counterPosition = 0;
	int patternStart = rowOffset;
	for (int x = rowOffset; x < width; x++) {
		if (row.get(x) ^ isWhite) {
			counters[counterPosition]++;
		}
		else {
			if (counterPosition == patternLength - 1) {
				if (PatternMatchVariance(counters.data(), pattern, patternLength, MAX_INDIVIDUAL_VARIANCE) < MAX_AVG_VARIANCE) {
					begin = patternStart;
					end = x;
					return ErrorStatus::NoError;
				}
				patternStart += counters[0] + counters[1];
				std::copy(counters.begin() + 2, counters.end(), counters.begin());
				counters[patternLength - 2] = 0;
				counters[patternLength - 1] = 0;
				counterPosition--;
			}
			else {
				counterPosition++;
			}
			counters[counterPosition] = 1;
			isWhite = !isWhite;
		}
	}
	return ErrorStatus::NotFound;
}

/**
* Skip all whitespace until we get to the first black line.
*
* @param row row of black/white values to search
* @return index of the first black line.
* @throws NotFoundException Throws exception if no black lines are found in the row
*/
static ErrorStatus
SkipWhiteSpace(const BitArray& row, int& index)
{
	int width = row.size();
	index = row.getNextSet(0);
	if (index == width) {
		return ErrorStatus::NotFound;
	}
	return ErrorStatus::NoError;
}

/**
* The start & end patterns must be pre/post fixed by a quiet zone. This
* zone must be at least 10 times the width of a narrow line.  Scan back until
* we either get to the start of the barcode or match the necessary number of
* quiet zone pixels.
*
* Note: Its assumed the row is reversed when using this method to find
* quiet zone after the end pattern.
*
* ref: http://www.barcode-1.net/i25code.html
*
* @param row bit array representing the scanned barcode.
* @param startPattern index into row of the start or end pattern.
* @throws NotFoundException if the quiet zone cannot be found, a ReaderException is thrown.
*/
static ErrorStatus
ValidateQuietZone(const BitArray& row, int startPattern, int narrowLineWidth)
{

	int quietCount = narrowLineWidth * 10;  // expect to find this many pixels of quiet zone

											// if there are not so many pixel at all let's try as many as possible
	quietCount = quietCount < startPattern ? quietCount : startPattern;

	for (int i = startPattern - 1; quietCount > 0 && i >= 0; i--) {
		if (row.get(i)) {
			break;
		}
		quietCount--;
	}
	if (quietCount != 0) {
		// Unable to find the necessary number of quiet zone pixels.
		return ErrorStatus::NotFound;
	}
	return ErrorStatus::NoError;
}

/**
* Identify where the start of the middle / payload section starts.
*
* @param row row of black/white values to search
* @return Array, containing index of start of 'start block' and end of
*         'start block'
* @throws NotFoundException
*/
ErrorStatus
ITFReader::DecodeStart(const BitArray& row, int& narrowLineWidth, int& patternEnd)
{
	int endStart = 0;
	ErrorStatus status = SkipWhiteSpace(row, endStart);
	if (StatusIsOK(status)) {
		int patternStart;
		status = FindGuardPattern(row, endStart, START_PATTERN, patternStart, patternEnd);
		if (StatusIsOK(status)) {
			// Determine the width of a narrow line in pixels. We can do this by
			// getting the width of the start pattern and dividing by 4 because its
			// made up of 4 narrow lines.
			narrowLineWidth = (patternEnd - patternStart) / 4;

			status = ValidateQuietZone(row, patternStart, narrowLineWidth);
		}
	}
	return status;
}

/**
* Identify where the end of the middle / payload section ends.
*
* @param row row of black/white values to search
* @return Array, containing index of start of 'end block' and end of 'end
*         block'
* @throws NotFoundException
*/
ErrorStatus
ITFReader::DecodeEnd(const BitArray& row_, int narrowLineWidth, int& patternStart)
{
	BitArray row = row_;
	// For convenience, reverse the row and then
	// search from 'the start' for the end block
	row.reverse();
	int endStart = 0;
	ErrorStatus status = SkipWhiteSpace(row, endStart);
	if (StatusIsOK(status)) {
		int patternEnd;
		status = FindGuardPattern(row, endStart, END_PATTERN_REVERSED, patternStart, patternEnd);
		if (StatusIsOK(status)) {
			// The start & end patterns must be pre/post fixed by a quiet zone. This
			// zone must be at least 10 times the width of a narrow line.
			// ref: http://www.barcode-1.net/i25code.html
			status = ValidateQuietZone(row, patternStart, narrowLineWidth);
			if (StatusIsOK(status)) {
				// Now recalculate the indices of where the 'endblock' starts & stops to
				// accommodate
				// the reversed nature of the search
				patternStart = row.size() - patternEnd;
			}
		}
	}
	return status;
}

Result
ITFReader::decodeRow(int rowNumber, const BitArray& row, const DecodeHints* hints) const
{
	// Find out where the Middle section (payload) starts & ends
	int narrowLineWidth = -1;
	int startRangeEnd, endRangeBegin;
	ErrorStatus status = DecodeStart(row, narrowLineWidth, startRangeEnd);
	if (StatusIsOK(status))
		status = DecodeEnd(row, narrowLineWidth, endRangeBegin);

	if (StatusIsError(status)) {
		return Result(status);
	}

	std::string result;
	result.reserve(20);
	status = DecodeMiddle(row, startRangeEnd, endRangeBegin, result);
	if (StatusIsError(status)) {
		return Result(status);
	}

	std::vector<int> allowedLengths;
	if (hints != nullptr) {
		allowedLengths = hints->getIntegerList(DecodeHint::ALLOWED_LENGTHS);
	}
	if (allowedLengths.empty()) {
		allowedLengths.assign(DEFAULT_ALLOWED_LENGTHS.begin(), DEFAULT_ALLOWED_LENGTHS.end());
	}

	// To avoid false positives with 2D barcodes (and other patterns), make
	// an assumption that the decoded string must be a 'standard' length if it's short
	int length = static_cast<int>(result.length());
	bool lengthOK = false;
	int maxAllowedLength = 0;
	for (int allowedLength : allowedLengths) {
		if (length == allowedLength) {
			lengthOK = true;
			break;
		}
		if (allowedLength > maxAllowedLength) {
			maxAllowedLength = allowedLength;
		}
	}
	if (!lengthOK && length > maxAllowedLength) {
		lengthOK = true;
	}
	if (!lengthOK) {
		return Result(ErrorStatus::FormatError);
	}

	float x1 = static_cast<float>(startRangeEnd);
	float x2 = static_cast<float>(endRangeBegin);
	float ypos = static_cast<float>(rowNumber);

	return Result(result, ByteArray(), { ResultPoint(x1, ypos), ResultPoint(x2, ypos) }, BarcodeFormat::ITF);
}

} // OneD
} // ZXing
