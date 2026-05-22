/*
 * Copyright 2026 Axel Waggershauser
 */
// SPDX-License-Identifier: Apache-2.0

#include "ODTelepenReader.h"

#include "BarcodeData.h"
#include "SymbologyIdentifier.h"
#include "ZXAlgorithms.h"

#include <cstdint>
#include <ranges>

#ifndef PRINT_DEBUG
#define printf(...){}
#endif

namespace ZXing::OneD {

static std::string DecodeNumeric(std::string_view encoded)
{
	std::string decoded;
	decoded.reserve(encoded.size() * 2);
	bool inAlphaTail = false;

	for (uint8_t codeword : encoded) {
		// 16 is the "shift to alpha tail" codeword,
		// see https://advanova.co.uk/wp-content/uploads/2022/05/Barcode-Symbology-information-and-History.pdf
		if (!inAlphaTail && codeword == 16)
			inAlphaTail = true;
		else if (inAlphaTail)
			decoded += codeword;
		else if (17 <= codeword && codeword < 27) {
			decoded += ToDigit(codeword - 17);
			decoded += 'X';
		}
		else if (27 <= codeword && codeword < 127)
			decoded += ToString(codeword - 27, 2);
		else
			return {};
	}

	return decoded;
}

BarcodeData TelepenReader::decodePattern(int rowNumber, PatternView& next, std::unique_ptr<RowReader::DecodingState>&) const
{
	constexpr int minCharCount = 1; // TODO
	constexpr int minQuietZone = 5; // spec requires 10
	constexpr int minCharLength = 16 / 3;
	constexpr auto startPattern = FixedPattern<12, 16>{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3};
	constexpr auto endPattern = FixedPattern<11, 15>{3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1};

#if 0 // use fast 1:1:1:1 start pattern plausibility check, then E2E check for the whole start pattern
	next = FindLeftGuard<12>(next, 2 * 12 + minCharCount * minCharLength, [=](const PatternView& view, int spaceInPixel) {
		// find min/max of 4 consecutive bars/spaces and make sure they are close together
		auto mB = view[0], mS = view[1], MB = view[2], MS = view[3];
		if (mB > MB)
			std::swap(mB, MB);
		if (mS > MS)
			std::swap(mS, MS);
		return MB <= mB * 4 / 3 + 1 && MS <= mS * 4 / 3 + 1 && MB < 2 * mS + 1 && MS < 2 * mB + 1
			   && IsPattern<true>(view, startPattern, spaceInPixel, minQuietZone);
	});
#else
	next = FindLeftGuard<true>(next, 2 * 12 + minCharCount * minCharLength, startPattern, minQuietZone);
#endif
	if (!next.isValid())
		return {};

	int xStart = next.pixelsInFront();

	// get threshold from start pattern
	auto threshold = NarrowWideThreshold(next);
	if (!threshold.isValid())
		return {};

	next = next.subView(startPattern.size(), endPattern.size());
	std::string raw;
	while (next.isValid() && !IsRightGuard<true>(next, endPattern, minQuietZone)) {
		BitArray ba;
		bool inBlock = false;
		BarAndSpace<int> wSum, wNum, nSum, nNum;
		while (next.isValid() && ba.size() < 8) {
			if (next[0] > threshold[0] * 3 || next[1] > threshold[1] * 3)
				return {};
			BarAndSpace<bool> wide = {next[0] > threshold[0], next[1] > threshold[1]};
			if (!wide.bar && !wide.space)     // narrow bar, narrow space
				ba.appendBit(1);
			else if (!wide.bar && wide.space) // narrow bar, wide space
				// trailing 10 | leading 01
				ba.appendBits(std::exchange(inBlock, !inBlock) ? 0b10 : 0b01, 2);
			else if (wide.bar && !wide.space) // wide bar, narrow space
				ba.appendBits(0b00, 2);
			else if (wide.bar && wide.space)  // wide bar, wide space
				ba.appendBits(0b010, 3);

			for (int i = 0; i < 2; ++i) {
				if (next[i] > threshold[i]) {
					wSum[i] += next[i];
					++wNum[i];
				} else {
					nSum[i] += next[i];
					++nNum[i];
				}
			}

			next.skipPair();
		}
		if (ba.size() != 8 || std::ranges::count(ba, false) % 2 != 0) // even parity check
			return {};
		ba.reverse();
		raw += ToInt<char>(ba) & 0x7f; // drop the parity bit

		printf("line: %d, threshold: %2d, %2d, wSum: %2d, %2d, wNum: %d, %d, nSum: %2d, %2d, nNum: %d, %d -> %c ends at: %d\n", rowNumber,
			   threshold[0], threshold[1], wSum[0], wSum[1], wNum[0], wNum[1], nSum[0], nSum[1], nNum[0], nNum[1],
			   raw.back(), next.pixelsInFront());

		for (int i = 0; i < 2; ++i)
			threshold[i] = wNum[i] && nNum[i] ? (wSum[i] / wNum[i] + nSum[i] / nNum[i]) / 2
						   : nNum[i]          ? 2 * nSum[i] / nNum[i]
											  : threshold[i];
	}

	if (raw.size() < minCharCount + 1 || !IsRightGuard<true>(next, endPattern, minQuietZone))
		return {};

	auto txt = raw.substr(0, raw.size() - 1); // drop checksum character
	auto checkSum = (127 - (Reduce(txt, 0) % 127)) % 127;
	Error error = checkSum != raw.back() ? Error::Checksum : Error{};

	SymbologyIdentifier symbologyIdentifier = {'B', '0'};

	if (readNumeric && (!readAlpha || std::ranges::any_of(txt, [](uint8_t c) { return c < 32; }))) {
		if (auto decoded = DecodeNumeric(txt); !decoded.empty()) {
			// see ISO/IEC 15424:2025 4.4.3
			symbologyIdentifier.modifier = std::ranges::any_of(txt, [](uint8_t c) { return c == 16; }) ? '2' : '1';
			txt = std::move(decoded);
		}
		else if (!readAlpha)
			return {};
	}

	int xStop = next.pixelsTillEnd();
	auto format = symbologyIdentifier.modifier == '1' ? BarcodeFormat::TelepenNumeric : BarcodeFormat::TelepenAlpha;
	printf("line: %d, raw: %s, txt: %s, checksum: %d\n", rowNumber, raw.c_str(), txt.c_str(), checkSum);

	return LinearBarcode(format, txt, rowNumber, xStart, xStop, symbologyIdentifier, error);
}

} // namespace ZXing::OneD
