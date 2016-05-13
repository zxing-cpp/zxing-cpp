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

#include "oned/ODCode93Reader.h"
#include "Result.h"
#include "BitArray.h"
#include "ZXNumeric.h"
#include "TextCodec.h"

#include <array>

namespace ZXing {

namespace OneD {

static const int SYMBOL_COUNT = 48;

// Note that 'abcd' are dummy characters in place of control characters.
static const char* ALPHABET_STRING = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%abcd*";
//static final char[] ALPHABET = ALPHABET_STRING.toCharArray();

/**
* These represent the encodings of characters, as patterns of wide and narrow bars.
* The 9 least-significant bits of each int correspond to the pattern of wide and narrow.
*/
static const int CHARACTER_ENCODINGS[] = {
	0x114, 0x148, 0x144, 0x142, 0x128, 0x124, 0x122, 0x150, 0x112, 0x10A, // 0-9
	0x1A8, 0x1A4, 0x1A2, 0x194, 0x192, 0x18A, 0x168, 0x164, 0x162, 0x134, // A-J
	0x11A, 0x158, 0x14C, 0x146, 0x12C, 0x116, 0x1B4, 0x1B2, 0x1AC, 0x1A6, // K-T
	0x196, 0x19A, 0x16C, 0x166, 0x136, 0x13A, // U-Z
	0x12E, 0x1D4, 0x1D2, 0x1CA, 0x16E, 0x176, 0x1AE, // - - %
	0x126, 0x1DA, 0x1D6, 0x132, 0x15E, // Control chars? $-*
};

static const int ASTERISK_ENCODING = CHARACTER_ENCODINGS[47];

typedef std::array<int, 6> CounterContainer;

static int ToPattern(const CounterContainer& counters)
{
	int max = static_cast<int>(counters.size());
	int sum = 0;
	for (int counter : counters) {
		sum += counter;
	}
	int pattern = 0;
	for (int i = 0; i < max; i++) {
		int scaled = RoundToNearest(counters[i] * 9.0f / sum);
		if (scaled < 1 || scaled > 4) {
			return -1;
		}
		if ((i & 0x01) == 0) {
			for (int j = 0; j < scaled; j++) {
				pattern = (pattern << 1) | 0x01;
			}
		}
		else {
			pattern <<= scaled;
		}
	}
	return pattern;
}

static ErrorStatus
FindAsteriskPattern(const BitArray& row, int& outPatternStart, int& outPatternEnd)
{
	int width = row.size();
	int offset = row.getNextSet(0);
	CounterContainer theCounters = {};
	int patternStart = offset;
	bool isWhite = false;
	int patternLength = static_cast<int>(theCounters.size());
	int counterPosition = 0;
	auto bitIter = row.iterAt(offset);
	for (; offset < width; ++offset, ++bitIter) {
		if (*bitIter ^ isWhite) {
			theCounters[counterPosition]++;
		}
		else {
			if (counterPosition == patternLength - 1) {
				if (ToPattern(theCounters) == ASTERISK_ENCODING) {
					outPatternStart = patternStart;
					outPatternEnd = offset;
					return ErrorStatus::NoError;
				}
				patternStart += theCounters[0] + theCounters[1];
				std::copy(theCounters.begin() + 2, theCounters.end(), theCounters.begin());
				theCounters[patternLength - 2] = 0;
				theCounters[patternLength - 1] = 0;
				counterPosition--;
			}
			else {
				counterPosition++;
			}
			theCounters[counterPosition] = 1;
			isWhite = !isWhite;
		}
	}
	return ErrorStatus::NotFound;
}

static char
PatternToChar(int pattern)
{
	for (int i = 0; i < SYMBOL_COUNT; i++) {
		if (CHARACTER_ENCODINGS[i] == pattern) {
			return ALPHABET_STRING[i];
		}
	}
	return 0;
}

static ErrorStatus
DecodeExtended(const std::string& encoded, std::string& decoded)
{
	size_t length = encoded.length();
	decoded.reserve(length);
	for (size_t i = 0; i < length; i++) {
		char c = encoded[i];
		if (c >= 'a' && c <= 'd') {
			if (i+1 >= length) {
				return ErrorStatus::FormatError;
			}
			char next = encoded[i + 1];
			char decodedChar = '\0';
			switch (c) {
			case 'd':
				// +A to +Z map to a to z
				if (next >= 'A' && next <= 'Z') {
					decodedChar = (char)(next + 32);
				}
				else {
					return ErrorStatus::FormatError;
				}
				break;
			case 'a':
				// $A to $Z map to control codes SH to SB
				if (next >= 'A' && next <= 'Z') {
					decodedChar = (char)(next - 64);
				}
				else {
					return ErrorStatus::FormatError;
				}
				break;
			case 'b':
				if (next >= 'A' && next <= 'E') {
					// %A to %E map to control codes ESC to USep
					decodedChar = (char)(next - 38);
				}
				else if (next >= 'F' && next <= 'J') {
					// %F to %J map to ; < = > ?
					decodedChar = (char)(next - 11);
				}
				else if (next >= 'K' && next <= 'O') {
					// %K to %O map to [ \ ] ^ _
					decodedChar = (char)(next + 16);
				}
				else if (next >= 'P' && next <= 'S') {
					// %P to %S map to { | } ~
					decodedChar = (char)(next + 43);
				}
				else if (next >= 'T' && next <= 'Z') {
					// %T to %Z all map to DEL (127)
					decodedChar = 127;
				}
				else {
					return ErrorStatus::FormatError;
				}
				break;
			case 'c':
				// /A to /O map to ! to , and /Z maps to :
				if (next >= 'A' && next <= 'O') {
					decodedChar = (char)(next - 32);
				}
				else if (next == 'Z') {
					decodedChar = ':';
				}
				else {
					return ErrorStatus::FormatError;
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
	return ErrorStatus::NoError;
}

static int IndexOf(const char* str, char c)
{
	auto s = strchr(str, c);
	return s != nullptr ? static_cast<int>(s - str) : -1;
}

bool
CheckOneChecksum(const std::string& result, int checkPosition, int weightMax)
{
	int weight = 1;
	int total = 0;
	for (int i = checkPosition - 1; i >= 0; i--) {
		total += weight * IndexOf(ALPHABET_STRING, result[i]);
		if (++weight > weightMax) {
			weight = 1;
		}
	}
	if (total < 0 || result[checkPosition] != ALPHABET_STRING[total % 47]) {
		return false;
	}
	return true;
}

static ErrorStatus
CheckChecksums(const std::string& result) 
{
	int length = static_cast<int>(result.length());
	if (CheckOneChecksum(result, length - 2, 20) && CheckOneChecksum(result, length - 1, 15)) {
		return ErrorStatus::NoError;
	}
	return ErrorStatus::ChecksumError;
}


Result
Code93Reader::decodeRow(int rowNumber, const BitArray& row, std::unique_ptr<DecodingState>& state) const
{
	int patternStart, patternEnd;
	ErrorStatus status = FindAsteriskPattern(row, patternStart, patternEnd);
	if (StatusIsError(status)) {
		return Result(status);
	}
	// Read off white space    
	int nextStart = row.getNextSet(patternEnd);
	int end = row.size();

	CounterContainer theCounters = {};
	std::string result;
	result.reserve(20);

	char decodedChar;
	int lastStart;
	do {
		status = RecordPattern(row, nextStart, theCounters);
		if (StatusIsError(status)) {
			return Result(status);
		}
		int pattern = ToPattern(theCounters);
		if (pattern < 0) {
			return Result(ErrorStatus::NotFound);
		}
		decodedChar = PatternToChar(pattern);
		if (decodedChar == 0) {
			return Result(ErrorStatus::NotFound);
		}
		result.push_back(decodedChar);
		lastStart = nextStart;
		for (int counter : theCounters) {
			nextStart += counter;
		}
		// Read off white space
		nextStart = row.getNextSet(nextStart);
	} while (decodedChar != '*');
	
	result.resize(result.length() - 1); // remove asterisk

	int lastPatternSize = 0;
	for (int counter : theCounters) {
		lastPatternSize += counter;
	}

	// Should be at least one more black module
	if (nextStart == end || !row.get(nextStart)) {
		return Result(ErrorStatus::NotFound);
	}

	if (result.length() < 2) {
		// false positive -- need at least 2 checksum digits
		return Result(ErrorStatus::NotFound);
	}

	status = CheckChecksums(result);
	if (StatusIsError(status)) {
		return Result(status);
	}
	// Remove checksum digits
	result.resize(result.length() - 2);

	std::string resultString;
	status = DecodeExtended(result, resultString);
	if (StatusIsError(status)) {
		return Result(status);
	}

	float left = 0.5f * static_cast<float>(patternStart + patternEnd);
	float right = static_cast<float>(lastStart) + 0.5f * static_cast<float>(lastPatternSize);
	float ypos = static_cast<float>(rowNumber);
	return Result(TextCodec::FromLatin1(resultString), ByteArray(), { ResultPoint(left, ypos), ResultPoint(right, ypos) }, BarcodeFormat::CODE_93);
}


} // OneD
} // ZXing
