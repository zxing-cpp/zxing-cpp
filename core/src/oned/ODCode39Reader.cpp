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

#include <algorithm>
#include <array>

namespace ZXing {

namespace OneD {

static const int SYMBOL_COUNT = 44;
static const char* ALPHABET_STRING = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. *$/+%";

// Note this lacks '*' compared to ALPHABET_STRING
static const char* CHECK_DIGIT_STRING = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%";
static const int CHECK_DIGIT_COUNT = SYMBOL_COUNT - 1;

/**
* These represent the encodings of characters, as patterns of wide and narrow bars.
* The 9 least-significant bits of each int correspond to the pattern of wide and narrow,
* with 1s representing "wide" and 0s representing narrow.
*/
static const int CHARACTER_ENCODINGS[] = {
	0x034, 0x121, 0x061, 0x160, 0x031, 0x130, 0x070, 0x025, 0x124, 0x064, // 0-9
	0x109, 0x049, 0x148, 0x019, 0x118, 0x058, 0x00D, 0x10C, 0x04C, 0x01C, // A-J
	0x103, 0x043, 0x142, 0x013, 0x112, 0x052, 0x007, 0x106, 0x046, 0x016, // K-T
	0x181, 0x0C1, 0x1C0, 0x091, 0x190, 0x0D0, 0x085, 0x184, 0x0C4, 0x094, // U-*
	0x0A8, 0x0A2, 0x08A, 0x02A // $-%
};

static const int ASTERISK_ENCODING = CHARACTER_ENCODINGS[39];

typedef std::array<int, 9> CounterContainer;

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

static DecodeStatus
FindAsteriskPattern(const BitArray& row, CounterContainer& counters, int& outPatternStart, int& outPatternEnd)
{
	int width = row.size();
	int offset = row.getNextSet(0);
	int counterPosition = 0;
	int patternStart = offset;
	bool isWhite = false;
	int patternLength = static_cast<int>(counters.size());
	auto bitIter = row.iterAt(offset);
	for (; offset < width; ++offset, ++bitIter) {
		if (*bitIter ^ isWhite) {
			counters[counterPosition]++;
		}
		else {
			if (counterPosition == patternLength - 1) {
				// Look for whitespace before start pattern, >= 50% of width of start pattern
				if (ToNarrowWidePattern(counters) == ASTERISK_ENCODING &&
					row.isRange(std::max(0, patternStart - ((offset - patternStart) / 2)), patternStart, false)) {
					outPatternStart = patternStart;
					outPatternEnd = offset;
					return DecodeStatus::NoError;
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
	return DecodeStatus::NotFound;
}

static char
PatternToChar(int pattern)
{
	for (size_t i = 0; i < SYMBOL_COUNT; i++) {
		if (CHARACTER_ENCODINGS[i] == pattern) {
			return ALPHABET_STRING[i];
		}
	}
	return 0;
}

static DecodeStatus
DecodeExtended(const std::string& encoded, std::string& decoded)
{
	size_t length = encoded.length();
	decoded.reserve(length);
	for (size_t i = 0; i < length; i++) {
		char c = encoded[i];
		if (c == '+' || c == '$' || c == '%' || c == '/') {
			if (i+1 >= length) {
				return DecodeStatus::FormatError;
			}
			char next = encoded[i + 1];
			char decodedChar = '\0';
			switch (c) {
			case '+':
				// +A to +Z map to a to z
				if (next >= 'A' && next <= 'Z') {
					decodedChar = (char)(next + 32);
				}
				else {
					return DecodeStatus::FormatError;
				}
				break;
			case '$':
				// $A to $Z map to control codes SH to SB
				if (next >= 'A' && next <= 'Z') {
					decodedChar = (char)(next - 64);
				}
				else {
					return DecodeStatus::FormatError;
				}
				break;
			case '%':
				// %A to %E map to control codes ESC to US
				if (next >= 'A' && next <= 'E') {
					decodedChar = (char)(next - 38);
				}
				else if (next >= 'F' && next <= 'W') {
					decodedChar = (char)(next - 11);
				}
				else {
					return DecodeStatus::FormatError;
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
					return DecodeStatus::FormatError;
				}
				break;
			}
			decoded += decodedChar;
			// bump up i again since we read two characters
			i++;
		}
		else {
			decoded += c;
		}
	}
	return DecodeStatus::NoError;
}

static int IndexOf(const char* str, char c)
{
	auto s = strchr(str, c);
	return s != nullptr ? static_cast<int>(s - str) : -1;
}

Code39Reader::Code39Reader(const DecodeHints& hints, bool extendedMode) :
	_extendedMode(extendedMode),
	_usingCheckDigit(hints.shouldAssumeCode39CheckDigit())
{
}

Result
Code39Reader::decodeRow(int rowNumber, const BitArray& row, std::unique_ptr<DecodingState>& state) const
{
	CounterContainer theCounters = {};
	std::string result;
	result.reserve(20);
	int patternStart = 0, patternEnd = 0;
	DecodeStatus status = FindAsteriskPattern(row, theCounters, patternStart, patternEnd);
	if (StatusIsError(status)) {
		return Result(status);
	}
	// Read off white space    
	int nextStart = row.getNextSet(patternEnd);
	int end = row.size();

	char decodedChar;
	int lastStart;
	do {
		status = RecordPattern(row, nextStart, theCounters);
		if (StatusIsError(status)) {
			return Result(status);
		}
		int pattern = ToNarrowWidePattern(theCounters);
		if (pattern < 0) {
			return Result(DecodeStatus::NotFound);
		}
		
		decodedChar = PatternToChar(pattern);
		if (decodedChar == 0) {
			return Result(DecodeStatus::NotFound);
		}
		result += decodedChar;
		lastStart = nextStart;
		for (int counter : theCounters) {
			nextStart += counter;
		}
		// Read off white space
		nextStart = row.getNextSet(nextStart);
	} while (decodedChar != '*');

	result.resize(result.length() - 1); // remove asterisk

	// Look for whitespace after pattern:
	int lastPatternSize = 0;
	for (int counter : theCounters) {
		lastPatternSize += counter;
	}
	int whiteSpaceAfterEnd = nextStart - lastStart - lastPatternSize;
	// If 50% of last pattern size, following last pattern, is not whitespace, fail
	// (but if it's whitespace to the very end of the image, that's OK)
	if (nextStart != end && (whiteSpaceAfterEnd * 2) < lastPatternSize) {
		return Result(DecodeStatus::NotFound);
	}

	if (_usingCheckDigit) {
		int max = static_cast<int>(result.length()) - 1;
		int total = 0;
		for (int i = 0; i < max; i++) {
			total += IndexOf(CHECK_DIGIT_STRING, result[i]);
		}
		if (total < 0 || result[max] != CHECK_DIGIT_STRING[total % CHECK_DIGIT_COUNT]) {
			return Result(DecodeStatus::ChecksumError);
		}
		result.resize(max);
	}

	if (result.empty()) {
		// false positive
		return Result(DecodeStatus::NotFound);
	}

	std::string resultString;
	if (_extendedMode) {
		status = DecodeExtended(result, resultString);
		if (StatusIsError(status)) {
			return Result(status);
		}
	}
	else {
		resultString = result;
	}

	float left = 0.5f * static_cast<float>(patternStart + patternEnd);
	float right = static_cast<float>(lastStart) + 0.5f * static_cast<float>(lastPatternSize);
	float ypos = static_cast<float>(rowNumber);
	return Result(TextDecoder::FromLatin1(resultString), ByteArray(), { ResultPoint(left, ypos), ResultPoint(right, ypos) }, BarcodeFormat::CODE_39);
}



} // OneD
} // ZXing
