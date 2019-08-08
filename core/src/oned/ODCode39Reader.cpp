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

#include "oned/ODCode39Reader.h"
#include "Result.h"
#include "BitArray.h"
#include "DecodeHints.h"
#include "ZXContainerAlgorithms.h"

#include <algorithm>
#include <array>

namespace ZXing {

namespace OneD {

static const char ALPHABET_STRING[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%*";

/**
* Each character consists of 5 bars and 4 spaces, 3 of which are wide (i.e. 6 are narrow).
* Each character is followed by a narrow space. The narrow to wide ratio is between 1:2 and 1:3.
* These represent the encodings of characters, as patterns of wide and narrow bars.
* The 9 least-significant bits of each int correspond to the pattern of wide and narrow,
* with 1s representing "wide" and 0s representing "narrow".
*/
static const int CHARACTER_ENCODINGS[] = {
	0x034, 0x121, 0x061, 0x160, 0x031, 0x130, 0x070, 0x025, 0x124, 0x064, // 0-9
	0x109, 0x049, 0x148, 0x019, 0x118, 0x058, 0x00D, 0x10C, 0x04C, 0x01C, // A-J
	0x103, 0x043, 0x142, 0x013, 0x112, 0x052, 0x007, 0x106, 0x046, 0x016, // K-T
	0x181, 0x0C1, 0x1C0, 0x091, 0x190, 0x0D0, 0x085, 0x184, 0x0C4, 0x0A8, // U-$
	0x0A2, 0x08A, 0x02A, 0x094 // /-% , *
};

static_assert(Length(ALPHABET_STRING) - 1 == Length(CHARACTER_ENCODINGS), "table size mismatch");

static const int ASTERISK_ENCODING = 0x094;

static const char PERCENTAGE_MAPPING[26] = {
	'A' - 38, 'B' - 38, 'C' - 38, 'D' - 38, 'E' - 38,	// %A to %E map to control codes ESC to USep
	'F' - 11, 'G' - 11, 'H' - 11, 'I' - 11, 'J' - 11,	// %F to %J map to ; < = > ?
	'K' + 16, 'L' + 16, 'M' + 16, 'N' + 16, 'O' + 16,	// %K to %O map to [ \ ] ^ _
	'P' + 43, 'Q' + 43, 'R' + 43, 'S' + 43, 'T' + 43,	// %P to %T map to { | } ~ DEL
	'\0', '@', '`',										// %U map to NUL, %V map to @, %W map to `
	127, 127, 127										// %X to %Z all map to DEL (127)
};

using CounterContainer = std::array<int, 9>;

// For efficiency, returns -1 on failure. Not throwing here saved as many as 700 exceptions
// per image when using some of our blackbox images.
static int
ToNarrowWidePattern(const CounterContainer& counters)
{
	int numCounters = static_cast<int>(counters.size());
	int maxNarrowCounter = 0;
	int wideCounters;
	do {
		int minCounter = std::numeric_limits<int>::max();
		for (int i = 0; i < numCounters; ++i) {
			if (counters[i] < minCounter && counters[i] > maxNarrowCounter) {
				minCounter = counters[i];
			}
		}
		maxNarrowCounter = minCounter;
		wideCounters = 0;
		int totalWideCountersWidth = 0;
		int pattern = 0;
		for (int i = 0; i < numCounters; ++i) {
			if (counters[i] > maxNarrowCounter) {
				pattern |= 1 << (numCounters - 1 - i);
				wideCounters++;
				totalWideCountersWidth += counters[i];
			}
		}
		if (wideCounters == 3) {
			// Found 3 wide counters, but are they close enough in width?
			// We can perform a cheap, conservative check to see if any individual
			// counter is more than 1.5 times the average:
			for (int i = 0; i < numCounters && wideCounters > 0; ++i) {
				if (counters[i] > maxNarrowCounter) {
					wideCounters--;
					// totalWideCountersWidth = 3 * average, so this checks if counter >= 3/2 * average
					if ((counters[i] * 2) >= totalWideCountersWidth) {
						return -1;
					}
				}
			}
			return pattern;
		}
	} while (wideCounters > 3);
	return -1;
}

static BitArray::Range
FindAsteriskPattern(const BitArray& row)
{
	CounterContainer counters;

	return RowReader::FindPattern(
	    row.getNextSet(row.begin()), row.end(), counters,
	    [&row](BitArray::Iterator begin, BitArray::Iterator end, const CounterContainer& counters) {
		    // Look for whitespace before start pattern, >= 50% of width of start pattern
		    return row.hasQuiteZone(begin, - (end - begin) / 2) &&
				ToNarrowWidePattern(counters) == ASTERISK_ENCODING;
	    });
}

/** Decode the extended string in place. Return false if FormatError occured.
 * ctrl is either "$%/+" for code39 or "abcd" for code93. */
bool
DecodeExtendedCode39AndCode93(std::string& encoded, const char ctrl[4])
{
	auto out = encoded.begin();
	for (auto in = encoded.cbegin(); in != encoded.cend(); ++in) {
		char c = *in;
		if (Contains(ctrl, c)) {
			char next = *++in; // if in is one short of cend(), then next == 0
			if (next < 'A' || next > 'Z')
				return false;
			if (c == ctrl[0])
				c = next - 64; // $A to $Z map to control codes SH to SB
			else if (c == ctrl[1])
				c = PERCENTAGE_MAPPING[next - 'A'];
			else if (c == ctrl[2])
				c = next - 32; // /A to /O map to ! to , and /Z maps to :
			else
				c = next + 32; // +A to +Z map to a to z
		}
		*out++ = c;
	}
	encoded.erase(out, encoded.end());
	return true;
}


Code39Reader::Code39Reader(const DecodeHints& hints) :
	_extendedMode(hints.shouldTryCode39ExtendedMode()),
	_usingCheckDigit(hints.shouldAssumeCode39CheckDigit())
{
}

Result
Code39Reader::decodeRow(int rowNumber, const BitArray& row, std::unique_ptr<DecodingState>&) const
{
	auto range = FindAsteriskPattern(row);
	if (!range)
		return Result(DecodeStatus::NotFound);

	int xStart = range.begin - row.begin();
	CounterContainer theCounters = {};
	std::string result;
	result.reserve(20);

	do {
		// Read off white space
		range = RecordPattern(row.getNextSet(range.end), row.end(), theCounters);
		if (!range)
			return Result(DecodeStatus::NotFound);

		int pattern = ToNarrowWidePattern(theCounters);
		if (pattern < 0)
			return Result(DecodeStatus::NotFound);

		int i = IndexOf(CHARACTER_ENCODINGS, pattern);
		if (i < 0)
			return Result(DecodeStatus::NotFound);

		result += ALPHABET_STRING[i];
	} while (result.back() != '*');

	result.pop_back(); // remove asterisk

	// Require at least one payload character and a quite zone of half the last pattern size.
	if (result.size() < (_usingCheckDigit ? 2 : 1) || !row.hasQuiteZone(range.end, range.size() / 2)) {
		return Result(DecodeStatus::NotFound);
	}

	if (_usingCheckDigit) {
		auto checkDigit = result.back();
		result.pop_back();
		int checksum = TransformReduce(result, 0, [](char c) { return IndexOf(ALPHABET_STRING, c); });
		if (checkDigit != ALPHABET_STRING[checksum % 43]) {
			return Result(DecodeStatus::ChecksumError);
		}
	}

	if (_extendedMode && !DecodeExtendedCode39AndCode93(result, "$%/+"))
		return Result(DecodeStatus::FormatError);

	int xStop = range.end - row.begin() - 1;
	return Result(result, rowNumber, xStart, xStop, BarcodeFormat::CODE_39);
}



} // OneD
} // ZXing
