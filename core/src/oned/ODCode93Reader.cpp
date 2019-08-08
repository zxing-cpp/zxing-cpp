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
#include "ZXContainerAlgorithms.h"

#include <array>

namespace ZXing {

namespace OneD {

// Note that 'abcd' are dummy characters in place of control characters.
static const char ALPHABET_STRING[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%abcd*";

/**
* Each character consist of 3 bars and 3 spaces and is 9 modules wide in total.
* Each bar and space is from 1 to 4 modules wide.
* These represent the encodings of characters. Each module is asigned 1 bit.
* The 9 least-significant bits of each int correspond to the 9 modules in a symbol.
* Note: bit 9 (the first) is always 1, bit 1 (the last) is always 0.
*/
static const int CHARACTER_ENCODINGS[] = {
	0x114, 0x148, 0x144, 0x142, 0x128, 0x124, 0x122, 0x150, 0x112, 0x10A, // 0-9
	0x1A8, 0x1A4, 0x1A2, 0x194, 0x192, 0x18A, 0x168, 0x164, 0x162, 0x134, // A-J
	0x11A, 0x158, 0x14C, 0x146, 0x12C, 0x116, 0x1B4, 0x1B2, 0x1AC, 0x1A6, // K-T
	0x196, 0x19A, 0x16C, 0x166, 0x136, 0x13A, // U-Z
	0x12E, 0x1D4, 0x1D2, 0x1CA, 0x16E, 0x176, 0x1AE, // - - %
	0x126, 0x1DA, 0x1D6, 0x132, 0x15E, // Control chars ($)==a, (%)==b, (/)==c, (+)==d, *
};

static_assert(Length(ALPHABET_STRING) - 1 == Length(CHARACTER_ENCODINGS), "table size mismatch");

static const int ASTERISK_ENCODING = 0x15E;

using CounterContainer = std::array<int, 6>;

static int
ToPattern(const CounterContainer& counters)
{
	// each bar/space is 1-4 modules wide, the sum of all is 9 modules wide
	int sum = Accumulate(counters, 0);
	int pattern = 0;
	for (size_t i = 0; i < counters.size(); i++) {
		int scaled = (counters[i] * 9 + (sum/2)) / sum; // non-float version of RoundToNearest(counters[i] * 9.0f / sum);
		if (scaled < 1 || scaled > 4) {
			return -1;
		}
		pattern <<= scaled;
		pattern |= ~(0xffffffff << scaled) * (~i & 1);
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

static bool
CheckOneChecksum(const std::string& result, int checkPosition, int weightMax)
{
	int weight = 1;
	int checkSum = 0;
	for (int i = checkPosition - 1; i >= 0; i--) {
		checkSum += weight * IndexOf(ALPHABET_STRING, result[i]);
		if (++weight > weightMax) {
			weight = 1;
		}
	}
	return result[checkPosition] == ALPHABET_STRING[checkSum % 47];
}

static bool
CheckChecksums(const std::string& result)
{
	int length = static_cast<int>(result.length());
	return CheckOneChecksum(result, length - 2, 20) && CheckOneChecksum(result, length - 1, 15);
}

// forward declare here. see ODCode39Reader.cpp. Not put in header to not pollute the public facing API
bool DecodeExtendedCode39AndCode93(std::string& encoded, const char ctrl[4]);

Result
Code93Reader::decodeRow(int rowNumber, const BitArray& row, std::unique_ptr<DecodingState>&) const
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

		int pattern = ToPattern(theCounters);
		if (pattern < 0)
			return Result(DecodeStatus::NotFound);

		int i = IndexOf(CHARACTER_ENCODINGS, pattern);
		if (i < 0)
			return Result(DecodeStatus::NotFound);

		result += ALPHABET_STRING[i];
	} while (result.back() != '*');

	result.pop_back(); // remove asterisk

	// Should be at least one more black module
	if (range.end == row.end() || !*range.end) {
		return Result(DecodeStatus::NotFound);
	}

	// Need at least 2 checksum + 1 payload characters
	if (result.length() < 3)
		return Result(DecodeStatus::NotFound);

	if (!CheckChecksums(result))
		return Result(DecodeStatus::ChecksumError);

	// Remove checksum digits
	result.resize(result.length() - 2);

	if (!DecodeExtendedCode39AndCode93(result, "abcd"))
		return Result(DecodeStatus::FormatError);

	int xStop = range.end - row.begin() - 1;
	return Result(result, rowNumber, xStart, xStop, BarcodeFormat::CODE_93);
}


} // OneD
} // ZXing
