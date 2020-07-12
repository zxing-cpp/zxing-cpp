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

#include "ODCodabarReader.h"
#include "Result.h"
#include "BitArray.h"
#include "DecodeHints.h"
#include "ZXContainerAlgorithms.h"

#include <array>
#include <limits>
#include <string>
#include <vector>

namespace ZXing {
namespace OneD {

// These values are critical for determining how permissive the decoding
// will be. All stripe sizes must be within the window these define, as
// compared to the average stripe size.
static const float MAX_ACCEPTABLE = 2.0f;
static const float PADDING = 1.5f;

static const char ALPHABET[] = "0123456789-$:/.+ABCD";

// These represent the encodings of characters, as patterns of wide and narrow bars. The 7 least-significant bits of
// each int correspond to the pattern of wide and narrow, with 1s representing wide and 0s representing narrow.
static const int CHARACTER_ENCODINGS[] = {
	0x03, 0x06, 0x09, 0x60, 0x12, 0x42, 0x21, 0x24, 0x30, 0x48, // 0-9
	0x0c, 0x18, 0x45, 0x51, 0x54, 0x15, 0x1A, 0x29, 0x0B, 0x0E, // -$:/.+ABCD
};

static_assert(Size(ALPHABET) - 1 == Size(CHARACTER_ENCODINGS), "table size mismatch");

// minimal number of characters that should be present (inclusing start and stop characters)
// under normal circumstances this should be set to 3, but can be set higher
// as a last-ditch attempt to reduce false positives.
static const int MIN_CHARACTER_LENGTH = 3;

// official start and end patterns
static const char* STARTEND_ENCODING = "ABCD";
// some codabar generator allow the codabar string to be closed by every
// character. This will cause lots of false positives!

// some industries use a checksum standard but this is not part of the original codabar standard
// for more information see : http://www.mecsw.com/specs/codabar.html

/**
* Records the size of all runs of white and black pixels, starting with white.
* This is just like recordPattern, except it records all the counters, and
* uses our builtin "counters" member for storage.
* @param row row to count from
*/
static bool
InitCounters(const BitArray& row, std::vector<int>& counters)
{
	// Start from the first white bit.
	auto i = row.getNextUnset(row.begin());
	if (i == row.end())
		return false;

	auto li = i;
	while ((i = row.getNextSetTo(i, !*i)) != row.end()) {
		counters.push_back(static_cast<int>(i - li));
		li = i;
	}
	counters.push_back(static_cast<int>(i - li));

	return true;
}

// Assumes that counters[position] is a bar.
static int
ToNarrowWidePattern(const std::vector<int>& counters, int position)
{
	int counterLength = Size(counters);
	int end = position + 7;
	if (end >= counterLength) {
		return -1;
	}

	int maxBar = 0;
	int minBar = std::numeric_limits<int>::max();
	for (int j = position; j < end; j += 2) {
		int currentCounter = counters[j];
		if (currentCounter < minBar) {
			minBar = currentCounter;
		}
		if (currentCounter > maxBar) {
			maxBar = currentCounter;
		}
	}
	int thresholdBar = (minBar + maxBar) / 2;

	int maxSpace = 0;
	int minSpace = std::numeric_limits<int>::max();
	for (int j = position + 1; j < end; j += 2) {
		int currentCounter = counters[j];
		if (currentCounter < minSpace) {
			minSpace = currentCounter;
		}
		if (currentCounter > maxSpace) {
			maxSpace = currentCounter;
		}
	}
	int thresholdSpace = (minSpace + maxSpace) / 2;

	int bitmask = 1 << 7;
	int pattern = 0;
	for (int i = 0; i < 7; i++) {
		int threshold = (i & 1) == 0 ? thresholdBar : thresholdSpace;
		bitmask >>= 1;
		if (counters[position + i] > threshold) {
			pattern |= bitmask;
		}
	}

	return IndexOf(CHARACTER_ENCODINGS, pattern);
}

static int
FindStartPattern(const std::vector<int>& counters)
{
	int counterLength = Size(counters);
	for (int i = 1; i < counterLength; i += 2) {
		int charOffset = ToNarrowWidePattern(counters, i);
		if (charOffset >= 0 && IndexOf(STARTEND_ENCODING, ALPHABET[charOffset]) >= 0) {
			// Look for whitespace before start pattern, >= 50% of width of start pattern
			// We make an exception if the whitespace is the first element.
			int patternSize = 0;
			for (int j = i; j < i + 7; j++) {
				patternSize += counters[j];
			}
			if (i == 1 || counters[i - 1] >= patternSize / 2) {
				return i;
			}
		}
	}
	return -1;
}

static bool
ValidatePattern(const std::vector<int>& charOffsets, const std::vector<int>& counters, int start)
{
	// First, sum up the total size of our four categories of stripe sizes;
	std::array<int, 4> sizes = {};
	std::array<int, 4> counts = {};

	// We break out of this loop in the middle, in order to handle
	// inter-character spaces properly.
	int pos = start;
	for (int index : charOffsets) {
		int pattern = CHARACTER_ENCODINGS[index];
		for (int j = 6; j >= 0; j--) {
			// Even j = bars, while odd j = spaces. Categories 2 and 3 are for
			// long stripes, while 0 and 1 are for short stripes.
			int category = (j & 1) + (pattern & 1) * 2;
			sizes[category] += counters[pos + j];
			counts[category]++;
			pattern >>= 1;
		}
		// We ignore the inter-character space - it could be of any size.
		pos += 8;
	}

	// Calculate our allowable size thresholds using fixed-point math.
	float maxes[4];
	float mins[4];
	// Define the threshold of acceptability to be the midpoint between the
	// average small stripe and the average large stripe. No stripe lengths
	// should be on the "wrong" side of that line.
	for (int i = 0; i < 2; i++) {
		mins[i] = 0.0f;  // Accept arbitrarily small "short" stripes.
		mins[i + 2] = ((float)sizes[i] / counts[i] + (float)sizes[i + 2] / counts[i + 2]) / 2.0f;
		maxes[i] = mins[i + 2];
		maxes[i + 2] = (sizes[i + 2] * MAX_ACCEPTABLE + PADDING) / counts[i + 2];
	}

	// Now verify that all of the stripes are within the thresholds.
	pos = start;
	for (int index : charOffsets) {
		int pattern = CHARACTER_ENCODINGS[index];
		for (int j = 6; j >= 0; j--) {
			// Even j = bars, while odd j = spaces. Categories 2 and 3 are for
			// long stripes, while 0 and 1 are for short stripes.
			int category = (j & 1) + (pattern & 1) * 2;
			int size = counters[pos + j];
			if (size < mins[category] || size > maxes[category]) {
				return false;
			}
			pattern >>= 1;
		}
		pos += 8;
	}
	return true;
}

CodabarReader::CodabarReader(const DecodeHints& hints)
{
	_returnStartEnd = hints.returnCodabarStartEnd();
}

Result
CodabarReader::decodeRow(int rowNumber, const BitArray& row, std::unique_ptr<DecodingState>&) const
{
	std::vector<int> counters;
	counters.reserve(80);
	if (!InitCounters(row, counters)) {
		return Result(DecodeStatus::NotFound);
	}
	int startOffset = FindStartPattern(counters);
	if (startOffset < 0) {
		return Result(DecodeStatus::NotFound);
	}

	int nextStart = startOffset;
	std::vector<int> charOffsets;
	charOffsets.reserve(20);
	do {
		int charOffset = OneD::ToNarrowWidePattern(counters, nextStart);
		if (charOffset < 0) {
			return Result(DecodeStatus::NotFound);
		}
		// Hack: We store the position in the alphabet table into a
		// StringBuilder, so that we can access the decoded patterns in
		// validatePattern. We'll translate to the actual characters later.
		charOffsets.push_back(charOffset);
		nextStart += 8;
		// Stop as soon as we see the end character.
		if (charOffsets.size() > 1 && IndexOf(STARTEND_ENCODING, ALPHABET[charOffset]) >= 0) {
			break;
		}
	} while (nextStart < Size(counters)); // no fixed end pattern so keep on reading while data is available

	// Look for whitespace after pattern:
	int trailingWhitespace = counters[nextStart - 1];
	int lastPatternSize = 0;
	for (int i = -8; i < -1; i++) {
		lastPatternSize += counters[nextStart + i];
	}

	// We need to see whitespace equal to 50% of the last pattern size,
	// otherwise this is probably a false positive. The exception is if we are
	// at the end of the row. (I.e. the barcode barely fits.)
	if (nextStart < Size(counters) && trailingWhitespace < lastPatternSize / 2) {
		return Result(DecodeStatus::NotFound);
	}

	if (!ValidatePattern(charOffsets, counters, startOffset)) {
		return Result(DecodeStatus::NotFound);
	}

	// Translate character table offsets to actual characters.
	std::string decodeRowResult;
	decodeRowResult.reserve(charOffsets.size());
	for (int index : charOffsets) {
		decodeRowResult += ALPHABET[index];
	}
	// Ensure a valid start and end character
	if (IndexOf(STARTEND_ENCODING, decodeRowResult.front()) < 0) {
		return Result(DecodeStatus::NotFound);
	}
	if (IndexOf(STARTEND_ENCODING, decodeRowResult.back()) < 0) {
		return Result(DecodeStatus::NotFound);
	}

	// remove stop/start characters character and check if a long enough string is contained
	if (Size(decodeRowResult) <= MIN_CHARACTER_LENGTH) {
		// Almost surely a false positive ( start + stop + at least 1 character)
		return Result(DecodeStatus::NotFound);
	}

	if (!_returnStartEnd) {
		decodeRowResult = decodeRowResult.substr(1, decodeRowResult.size() - 2);
	}

	int runningCount = 0;
	for (int i = 0; i < startOffset; i++) {
		runningCount += counters[i];
	}
	int xStart = runningCount;
	for (int i = startOffset; i < nextStart - 1; i++) {
		runningCount += counters[i];
	}
	int xStop = runningCount;
	return Result(decodeRowResult, rowNumber, xStart, xStop, BarcodeFormat::CODABAR);
}

// each character has 4 bars and 3 spaces
constexpr int CHAR_LEN = 7;
// quite zone is half the width of a character symbol
constexpr float QUITE_ZONE_SCALE = 0.5f;

// official start and stop symbols are "ABCD"
// some codabar generator allow the codabar string to be closed by every
// character. This will cause lots of false positives!

inline bool IsLeftGuard(const PatternView& view, int spaceInPixel)
{
	return spaceInPixel > view.sum() * QUITE_ZONE_SCALE &&
		   Contains({0x1A, 0x29, 0x0B, 0x0E}, RowReader::NarrowWideBitPattern(view));
}

Result
CodabarReader::decodePattern(int rowNumber, const PatternView& row, std::unique_ptr<DecodingState>&) const
{
	// minimal number of characters that must be present (including start, stop and checksum characters)
	// absolute minimum would be 2 (meaning 0 'content'). everything below 4 produces too many false
	// positives.
	const int minCharCount = 4;
	auto isStartOrStopSymbol = [](char c) { return 'A' <= c && c <= 'D'; };

	auto next = FindLeftGuard<CHAR_LEN>(row, minCharCount * CHAR_LEN, IsLeftGuard);
	if (!next.isValid())
		return Result(DecodeStatus::NotFound);

	int xStart = next.pixelsInFront();
	int maxInterCharacterSpace = next.sum() / 2; // spec actually says 1 narrow space, width/2 is about 4

	std::string txt;
	txt.reserve(20);
	txt += DecodeNarrowWidePattern(next, CHARACTER_ENCODINGS, ALPHABET); // read off the start pattern

	if (!isStartOrStopSymbol(txt.back()))
		return Result(DecodeStatus::NotFound);

	do {
		// check remaining input width and inter-character space
		if (!next.skipSymbol() || !next.skipSingle(maxInterCharacterSpace))
			return Result(DecodeStatus::NotFound);

		txt += DecodeNarrowWidePattern(next, CHARACTER_ENCODINGS, ALPHABET);
		if (txt.back() < 0)
			return Result(DecodeStatus::NotFound);
	} while (!isStartOrStopSymbol(txt.back()));

	// next now points to the last decoded symbol
	// check txt length and whitespace after the last char. See also FindStartPattern.
	if (Size(txt) < minCharCount || !next.hasQuiteZoneAfter(QUITE_ZONE_SCALE))
		return Result(DecodeStatus::NotFound);

	// remove stop/start characters
	if (!_returnStartEnd)
		txt = txt.substr(1, txt.size() - 2);

	int xStop = next.pixelsTillEnd();
	return Result(txt, rowNumber, xStart, xStop, BarcodeFormat::CODABAR);
}

} // OneD
} // ZXing
