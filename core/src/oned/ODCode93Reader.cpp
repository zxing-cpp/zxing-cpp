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

#include "BitArray.h"
#include "Result.h"
#include "ZXContainerAlgorithms.h"

#include <array>
#include <string>

namespace ZXing::OneD {

// Note that 'abcd' are dummy characters in place of control characters.
static const char ALPHABET[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%abcd*";

/**
* Each character consist of 3 bars and 3 spaces and is 9 modules wide in total.
* Each bar and space is from 1 to 4 modules wide.
* These represent the encodings of characters. Each module is assigned 1 bit.
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

constexpr int CHAR_LEN = 6;
constexpr int CHAR_SUM = 9;
// quite zone is half the width of a character symbol
constexpr float QUITE_ZONE_SCALE = 0.5f;

static bool IsStartGuard(const PatternView& window, int spaceInPixel)
{
	// The complete start pattern is FixedPattern<CHAR_LEN, CHAR_SUM>{1, 1, 1, 1, 4, 1}.
	// Use only the first 4 elements which results in more than a 2x speedup. This is counter-intuitive since we save at
	// most 1/3rd of the loop iterations in FindPattern. The reason might be a successful vectorization with the limited
	// pattern size that is missed otherwise. We check for the remaining 2 slots for plausibility of the 4:1 ratio.
	return IsPattern(window, FixedPattern<4, 4>{1, 1, 1, 1}, spaceInPixel, QUITE_ZONE_SCALE * 12) &&
		   window[4] > 3 * window[5] - 2 &&
		   RowReader::OneToFourBitPattern<CHAR_LEN, CHAR_SUM>(window) == ASTERISK_ENCODING;
}

Result Code93Reader::decodePattern(int rowNumber, const PatternView& row, std::unique_ptr<DecodingState>&) const
{
	// minimal number of characters that must be present (including start, stop, checksum and 1 payload characters)
	int minCharCount = 5;

	auto next = FindLeftGuard<CHAR_LEN>(row, minCharCount * CHAR_LEN, IsStartGuard);
	if (!next.isValid())
		return Result(DecodeStatus::NotFound);

	int xStart = next.pixelsInFront();

	std::string txt;
	txt.reserve(20);

	do {
		// check remaining input width
		if (!next.skipSymbol())
			return Result(DecodeStatus::NotFound);

		txt += LookupBitPattern(OneToFourBitPattern<CHAR_LEN, CHAR_SUM>(next), CHARACTER_ENCODINGS, ALPHABET);
		if (txt.back() == 0)
			return Result(DecodeStatus::NotFound);
	} while (txt.back() != '*');

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
	return Result(txt, rowNumber, xStart, xStop, BarcodeFormat::Code93);
}

} // namespace ZXing::OneD
