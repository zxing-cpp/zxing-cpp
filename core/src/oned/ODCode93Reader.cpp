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

#include "ODCode93Reader.h"
#include "Result.h"
#include "BitArray.h"
#include "ZXNumeric.h"
#include "ZXContainerAlgorithms.h"

#include <array>
#include <string>

namespace ZXing {

namespace OneD {

// Note that 'abcd' are dummy characters in place of control characters.
static const char ALPHABET[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%abcd*";

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

static_assert(Size(ALPHABET) - 1 == Size(CHARACTER_ENCODINGS), "table size mismatch");

static const int ASTERISK_ENCODING = 0x15E;

using CounterContainer = std::array<int, 6>;

static int
ToPattern(const CounterContainer& counters)
{
	// each bar/space is 1-4 modules wide, the sum of all is 9 modules wide
	int sum = Reduce(counters);
	int pattern = 0;
	for (int i = 0, count = (int)counters.size(); i < count; ++i) {
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
		checkSum += weight * IndexOf(ALPHABET, result[i]);
		if (++weight > weightMax) {
			weight = 1;
		}
	}
	return result[checkPosition] == ALPHABET[checkSum % 47];
}

static bool
CheckChecksums(const std::string& result)
{
	int length = Size(result);
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

	int xStart = static_cast<int>(range.begin - row.begin());
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

		result += ALPHABET[i];
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

	int xStop = static_cast<int>(range.end - row.begin() - 1);
	return Result(result, rowNumber, xStart, xStop, BarcodeFormat::CODE_93);
}

constexpr int CHAR_LEN = 6;
constexpr int CHAR_SUM = 9;
constexpr auto START_PATTERN = FixedPattern<CHAR_LEN, CHAR_SUM>{1, 1, 1, 1, 4, 1};
// quite zone is half the width of a character symbol
constexpr float QUITE_ZONE_SCALE = 0.5f;

Result Code93Reader::decodePattern(int rowNumber, const PatternView &row, std::unique_ptr<DecodingState> &) const
{
	// minimal number of characters that must be present (including start, stop, checksum and 1 payload characters)
	int minCharCount = 5;
	auto isStartOrStopSymbol = [](char c) { return c == '*'; };
	auto decodePattern       = [](const PatternView& view) {
        return LookupBitPattern(OneToFourBitPattern<CHAR_LEN, CHAR_SUM>(view), CHARACTER_ENCODINGS, ALPHABET);
	};

	auto next = FindLeftGuard(row, minCharCount * CHAR_LEN, START_PATTERN, QUITE_ZONE_SCALE * CHAR_SUM);
	if (!next.isValid())
		return Result(DecodeStatus::NotFound);

	if (!isStartOrStopSymbol(decodePattern(next))) // read off the start pattern
		return Result(DecodeStatus::NotFound);

	int xStart = next.pixelsInFront();

	std::string txt;
	txt.reserve(20);

	do {
		// check remaining input width
		if (!next.skipSymbol())
			return Result(DecodeStatus::NotFound);

		txt += decodePattern(next);
		if (txt.back() < 0)
			return Result(DecodeStatus::NotFound);
	} while (!isStartOrStopSymbol(txt.back()));

	txt.pop_back(); // remove asterisk

	if (Size(txt) < minCharCount - 2)
		return Result(DecodeStatus::NotFound);

	// check termination bar (is present and not wider than about 2 modules) and quite zone
	next = next.subView(0, CHAR_LEN + 1);
	if (!next.isValid() || next[CHAR_LEN] > next.sum(CHAR_LEN) / 4 || !next.hasQuiteZoneAfter(QUITE_ZONE_SCALE))
		return Result(DecodeStatus::NotFound);

	if (!CheckChecksums(txt))
		return Result(DecodeStatus::ChecksumError);

	// Remove checksum digits
	txt.resize(txt.size() - 2);

	if (!DecodeExtendedCode39AndCode93(txt, "abcd"))
		return Result(DecodeStatus::FormatError);

	int xStop = next.pixelsTillEnd();
	return Result(txt, rowNumber, xStart, xStop, BarcodeFormat::CODE_93);
}


} // OneD
} // ZXing
