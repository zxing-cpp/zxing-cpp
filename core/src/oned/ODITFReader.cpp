/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "ODITFReader.h"

#include "DecodeHints.h"
#include "GTIN.h"
#include "Result.h"
#include "ZXAlgorithms.h"

#include <array>

namespace ZXing::OneD {

constexpr auto START_PATTERN_ = FixedPattern<4, 4>{1, 1, 1, 1};
constexpr auto STOP_PATTERN_1 = FixedPattern<3, 4>{2, 1, 1};
constexpr auto STOP_PATTERN_2 = FixedPattern<3, 5>{3, 1, 1};

Result ITFReader::decodePattern(int rowNumber, PatternView& next, std::unique_ptr<DecodingState>&) const
{
	const int minCharCount = 6;
	const int minQuietZone = 10;

	next = FindLeftGuard(next, 4 + minCharCount/2 + 3, START_PATTERN_, minQuietZone);
	if (!next.isValid())
		return {};

	std::string txt;
	txt.reserve(20);

	constexpr int weights[] = {1, 2, 4, 7, 0};
	int xStart = next.pixelsInFront();
	next = next.subView(4, 10);

	while (next.isValid()) {
		const auto threshold = NarrowWideThreshold(next);
		if (!threshold.isValid())
			break;

		BarAndSpace<int> digits, numWide;
		for (int i = 0; i < 10; ++i) {
			if (next[i] > threshold[i] * 2)
				break;
			numWide[i] += next[i] > threshold[i];
			digits[i] += weights[i/2] * (next[i] > threshold[i]);
		}

		if (numWide.bar != 2 || numWide.space != 2)
			break;

		for (int i = 0; i < 2; ++i)
			txt.push_back(ToDigit(digits[i] == 11 ? 0 : digits[i]));

		next.skipSymbol();
	}

	next = next.subView(0, 3);

	if (Size(txt) < minCharCount || !next.isValid())
		return {};

	if (!IsRightGuard(next, STOP_PATTERN_1, minQuietZone) && !IsRightGuard(next, STOP_PATTERN_2, minQuietZone))
		return {};

	Error error;
	if (_hints.validateITFCheckSum() && !GTIN::IsCheckDigitValid(txt))
		error = ChecksumError();

	// Symbology identifier ISO/IEC 16390:2007 Annex C Table C.1
	// See also GS1 General Specifications 5.1.3 Figure 5.1.3-2
	SymbologyIdentifier symbologyIdentifier = {'I', '0'}; // No check character validation

	if (_hints.validateITFCheckSum() || (txt.size() == 14 && GTIN::IsCheckDigitValid(txt))) // If no hint test if valid ITF-14
		symbologyIdentifier.modifier = '1'; // Modulo 10 symbol check character validated and transmitted

	int xStop = next.pixelsTillEnd();
	return Result(txt, rowNumber, xStart, xStop, BarcodeFormat::ITF, symbologyIdentifier, error);
}

} // namespace ZXing::OneD
