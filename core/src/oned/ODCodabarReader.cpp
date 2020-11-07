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

#include "BitArray.h"
#include "DecodeHints.h"
#include "Result.h"
#include "ZXContainerAlgorithms.h"

#include <array>
#include <limits>
#include <string>
#include <vector>

namespace ZXing::OneD {

static const char ALPHABET[] = "0123456789-$:/.+ABCD";

// These represent the encodings of characters, as patterns of wide and narrow bars. The 7 least-significant bits of
// each int correspond to the pattern of wide and narrow, with 1s representing wide and 0s representing narrow.
static const int CHARACTER_ENCODINGS[] = {
	0x03, 0x06, 0x09, 0x60, 0x12, 0x42, 0x21, 0x24, 0x30, 0x48, // 0-9
	0x0c, 0x18, 0x45, 0x51, 0x54, 0x15, 0x1A, 0x29, 0x0B, 0x0E, // -$:/.+ABCD
};

static_assert(Size(ALPHABET) - 1 == Size(CHARACTER_ENCODINGS), "table size mismatch");

// some industries use a checksum standard but this is not part of the original codabar standard
// for more information see : http://www.mecsw.com/specs/codabar.html

CodabarReader::CodabarReader(const DecodeHints& hints)
{
	_returnStartEnd = hints.returnCodabarStartEnd();
}

// each character has 4 bars and 3 spaces
constexpr int CHAR_LEN = 7;
// quite zone is half the width of a character symbol
constexpr float QUITE_ZONE_SCALE = 0.5f;

// official start and stop symbols are "ABCD"
// some codabar generator allow the codabar string to be closed by every
// character. This will cause lots of false positives!

bool IsLeftGuard(const PatternView& view, int spaceInPixel)
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
		if (txt.back() == 0)
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
	return Result(txt, rowNumber, xStart, xStop, BarcodeFormat::Codabar);
}

} // namespace ZXing::OneD
