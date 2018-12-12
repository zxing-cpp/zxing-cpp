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
#include "TextDecoder.h"
#include "ZXContainerAlgorithms.h"

#include <algorithm>
#include <array>

namespace ZXing {

namespace OneD {

static const char ALPHABET_STRING[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%";

// Note this lacks '*' compared to ALPHABET_STRING
static const char CHECK_DIGIT_STRING[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%";

/**
* These represent the encodings of characters, as patterns of wide and narrow bars.
* The 9 least-significant bits of each int correspond to the pattern of wide and narrow,
* with 1s representing "wide" and 0s representing narrow.
*/
static const int CHARACTER_ENCODINGS[] = {
	0x034, 0x121, 0x061, 0x160, 0x031, 0x130, 0x070, 0x025, 0x124, 0x064, // 0-9
	0x109, 0x049, 0x148, 0x019, 0x118, 0x058, 0x00D, 0x10C, 0x04C, 0x01C, // A-J
	0x103, 0x043, 0x142, 0x013, 0x112, 0x052, 0x007, 0x106, 0x046, 0x016, // K-T
	0x181, 0x0C1, 0x1C0, 0x091, 0x190, 0x0D0, 0x085, 0x184, 0x0C4, 0x0A8, // U-$
	0x0A2, 0x08A, 0x02A // /-%
};

static_assert(Length(ALPHABET_STRING) - 1 == Length(CHARACTER_ENCODINGS), "table size mismatch");


static const char PERCENTAGE_MAPPING[26] = {
	'A' - 38, 'B' - 38, 'C' - 38, 'D' - 38, 'E' - 38,	// %A to %E map to control codes ESC to USep
	'F' - 11, 'G' - 11, 'H' - 11, 'I' - 11, 'J' - 11,	// %F to %J map to ; < = > ?
	'K' + 16, 'L' + 16, 'M' + 16, 'N' + 16, 'O' + 16,	// %K to %O map to [ \ ] ^ _
	'P' + 43, 'Q' + 43, 'R' + 43, 'S' + 43, 'T' + 43,	// %P to %T map to { | } ~ DEL
	'\0', '@', '`',										// %U map to NUL, %V map to @, %W map to `
	127, 127, 127										// %X to %Z all map to DEL (127)
};


static const int ASTERISK_ENCODING = 0x094;

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
		    return ToNarrowWidePattern(counters) == ASTERISK_ENCODING &&
		           row.hasQuiteZone(begin, - (end - begin) / 2);
	    });
}

static char
PatternToChar(int pattern)
{
	for (int i = 0; i < Length(CHARACTER_ENCODINGS); i++) {
		if (CHARACTER_ENCODINGS[i] == pattern) {
			return ALPHABET_STRING[i];
		}
	}
	if (pattern == ASTERISK_ENCODING) {
		return '*';
	}
	return 0;
}

/** Decode the extended string in place. Return false if FormatError occured. */
static bool
DecodeExtended(std::string& encoded)
{
	size_t length = encoded.length();
	std::string decoded;
	decoded.reserve(length);
	for (size_t i = 0; i < length; i++) {
		int c = (unsigned char)encoded[i];
		if (c == '+' || c == '$' || c == '%' || c == '/') {
			if (i+1 >= length) {
				return false;
			}
			int next = (unsigned char)encoded[i + 1];
			char decodedChar = '\0';
			switch (c) {
				case '+':
					// +A to +Z map to a to z
					if (next >= 'A' && next <= 'Z') {
						decodedChar = (char)(next + 32);
					}
					else {
						return false;
					}
					break;
				case '$':
					// $A to $Z map to control codes SH to SB
					if (next >= 'A' && next <= 'Z') {
						decodedChar = (char)(next - 64);
					}
					else {
						return false;
					}
					break;
				case '%':
					if (next >= 'A' && next <= 'Z') {
						decodedChar = PERCENTAGE_MAPPING[next - 'A'];
					}
					else {
						return false;
					}
					break;
				case '/':
					// /A to /O map to ! to , and /Z maps to :
					if (next >= 'A' && next <= 'O') {
						decodedChar = (char)(next - 32);
					}
					else if (next == 'Z') {
						decodedChar = ':';
					}
					else {
						return false;
					}
					break;
			}
			decoded.push_back(decodedChar);
			// bump up i again since we read two characters
			i++;
		}
		else {
			decoded.push_back((char)c);
		}
	}

	encoded = decoded;
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

	float left = (range.begin - row.begin()) + 0.5f * range.size();
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

		char decodedChar = PatternToChar(pattern);
		if (decodedChar == 0) {
			return Result(DecodeStatus::NotFound);
		}
		result += decodedChar;
	} while (result.back() != '*');

	result.pop_back(); // remove asterisk

	// If 50% of last pattern size, following last pattern, is not whitespace, fail
	// (but if it's whitespace to the very end of the image, that's OK)
	if (!row.hasQuiteZone(range.end, range.size()/2))
		return Result(DecodeStatus::NotFound);

	if (_usingCheckDigit) {
		int max = static_cast<int>(result.length()) - 1;
		int total = 0;
		for (int i = 0; i < max; i++) {
			total += IndexOf(ALPHABET_STRING, result[i]);
		}
		if (total < 0 || result[max] != ALPHABET_STRING[total % (Length(ALPHABET_STRING)-1)]) {
			return Result(DecodeStatus::ChecksumError);
		}
		result.resize(max);
	}

	if (result.empty()) {
		// false positive
		return Result(DecodeStatus::NotFound);
	}

	if (_extendedMode && !DecodeExtended(result))
		return Result(DecodeStatus::FormatError);

	float right = (range.begin - row.begin()) + 0.5f * range.size();
	float ypos = static_cast<float>(rowNumber);
	return Result(TextDecoder::FromLatin1(result), ByteArray(), { ResultPoint(left, ypos), ResultPoint(right, ypos) }, BarcodeFormat::CODE_39);
}



} // OneD
} // ZXing
