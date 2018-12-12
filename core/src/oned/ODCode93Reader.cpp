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
#include "TextDecoder.h"
#include "ZXContainerAlgorithms.h"

#include <array>

namespace ZXing {

namespace OneD {

// Note that 'abcd' are dummy characters in place of control characters.
static const char ALPHABET_STRING[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%abcd*";
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

static_assert(Length(ALPHABET_STRING) - 1 == Length(CHARACTER_ENCODINGS), "table size mismatch");

static const int ASTERISK_ENCODING = CHARACTER_ENCODINGS[47];

static const char PERCENTAGE_MAPPING[26] = {
	'A' - 38, 'B' - 38, 'C' - 38, 'D' - 38, 'E' - 38,	// %A to %E map to control codes ESC to USep
	'F' - 11, 'G' - 11, 'H' - 11, 'I' - 11, 'J' - 11,	// %F to %J map to ; < = > ?
	'K' + 16, 'L' + 16, 'M' + 16, 'N' + 16, 'O' + 16,	// %K to %O map to [ \ ] ^ _
	'P' + 43, 'Q' + 43, 'R' + 43, 'S' + 43, 'T' + 43,	// %P to %T map to { | } ~ DEL
	'\0', '@', '`',										// %U map to NUL, %V map to @, %W map to `
	127, 127, 127										// %X to %Z all map to DEL (127)
};


using CounterContainer = std::array<int, 6>;

static int
ToPattern(const CounterContainer& counters)
{
	int max = static_cast<int>(counters.size());
	int sum = Accumulate(counters, 0);
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

static BitArray::Range
FindAsteriskPattern(const BitArray& row)
{
	CounterContainer counters;

	return RowReader::FindPattern(
	    row.getNextSet(row.begin()), row.end(), counters,
	    [](BitArray::Iterator, BitArray::Iterator, const CounterContainer& counters) {
		    return ToPattern(counters) == ASTERISK_ENCODING;
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
	return 0;
}

static DecodeStatus
DecodeExtended(const std::string& encoded, std::string& decoded)
{
	size_t length = encoded.length();
	decoded.reserve(length);
	for (size_t i = 0; i < length; i++) {
		char c = encoded[i];
		if (c >= 'a' && c <= 'd') {
			if (i+1 >= length) {
				return DecodeStatus::FormatError;
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
					return DecodeStatus::FormatError;
				}
				break;
			case 'a':
				// $A to $Z map to control codes SH to SB
				if (next >= 'A' && next <= 'Z') {
					decodedChar = (char)(next - 64);
				}
				else {
					return DecodeStatus::FormatError;
				}
				break;
			case 'b':
				if (next >= 'A' && next <= 'Z') {
					decodedChar = PERCENTAGE_MAPPING[next - 'A'];
				}
				else {
					return DecodeStatus::FormatError;
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

static bool
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
	return !(total < 0 || result[checkPosition] != ALPHABET_STRING[total % 47]);
}

static bool
CheckChecksums(const std::string& result)
{
	int length = static_cast<int>(result.length());
	return CheckOneChecksum(result, length - 2, 20) && CheckOneChecksum(result, length - 1, 15);
}


Result
Code93Reader::decodeRow(int rowNumber, const BitArray& row, std::unique_ptr<DecodingState>&) const
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

		int pattern = ToPattern(theCounters);
		if (pattern < 0) {
			return Result(DecodeStatus::NotFound);
		}
		char decodedChar = PatternToChar(pattern);
		if (decodedChar == 0) {
			return Result(DecodeStatus::NotFound);
		}
		result.push_back(decodedChar);
	} while (result.back() != '*');

	result.pop_back(); // remove asterisk

	// Should be at least one more black module
	if (range.end == row.end() || !*range.end) {
		return Result(DecodeStatus::NotFound);
	}

	if (result.length() < 2) {
		// false positive -- need at least 2 checksum digits
		return Result(DecodeStatus::NotFound);
	}

	if (!CheckChecksums(result))
		return Result(DecodeStatus::ChecksumError);

	// Remove checksum digits
	result.resize(result.length() - 2);

	std::string resultString;
	auto status = DecodeExtended(result, resultString);
	if (StatusIsError(status)) {
		return Result(status);
	}

	float right = (range.begin - row.begin()) + 0.5f * range.size();
	float ypos = static_cast<float>(rowNumber);
	return Result(TextDecoder::FromLatin1(resultString), ByteArray(), { ResultPoint(left, ypos), ResultPoint(right, ypos) }, BarcodeFormat::CODE_93);
}


} // OneD
} // ZXing
