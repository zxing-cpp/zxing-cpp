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

#include "MQRFormatInformationFactory.h"

#include "BitHacks.h"
#include "MQRErrorCorrectionLevelFactory.h"

#include <array>

namespace ZXing::MicroQRCode {

static const int FORMAT_INFO_MASK_QR = 0x4445;

/**
 * See ISO 18004:2006, Annex C, Table C.1
 */
static const std::array<int, 2> FORMAT_INFO_DECODE_LOOKUP[] = {
	{0x4445, 0x00}, {0x4172, 0x01}, {0x4E2B, 0x02}, {0x4B1C, 0x03}, {0x55AE, 0x04}, {0x5099, 0x05}, {0x5FC0, 0x06},
	{0x5AF7, 0x07}, {0x6793, 0x08}, {0x62A4, 0x09}, {0x6DFD, 0x0A}, {0x68CA, 0x0B}, {0x7678, 0x0C}, {0x734F, 0x0D},
	{0x7C16, 0x0E}, {0x7921, 0x0F}, {0x06DE, 0x10}, {0x03E9, 0x11}, {0x0CB0, 0x12}, {0x0987, 0x13}, {0x1735, 0x14},
	{0x1202, 0x15}, {0x1D5B, 0x16}, {0x186C, 0x17}, {0x2508, 0x18}, {0x203F, 0x19}, {0x2F66, 0x1A}, {0x2A51, 0x1B},
	{0x34E3, 0x1C}, {0x31D4, 0x1D}, {0x3E8D, 0x1E}, {0x3BBA, 0x1F},
};

/**
 * @param maskedFormatInfo format info indicator, with mask still applied
 * @return information about the format it specifies, or {@code null}
 * if doesn't seem to match any known pattern
 */
QRCode::FormatInformation DecodeFormatInformation(uint32_t maskedFormatInfo)
{
	// Find the int in FORMAT_INFO_DECODE_LOOKUP with fewest bits differing
	int bestDifference = 32;
	int bestFormatInfo = -1;

	// Some QR codes apparently do not apply the XOR mask. Try without and with additional masking.
	// NB: This approach has been copied from QR codes and assumed that it also applies for micro QR codes.
	// But by checking masked and unmasked it removes the effectiveness of the Hamming distance comparison
	// because the mask allows 5 bits to be different.
	for (auto mask : {0, FORMAT_INFO_MASK_QR}) {
		uint32_t bits = maskedFormatInfo ^ mask;
		for (auto& [pattern, decodedInfo] : FORMAT_INFO_DECODE_LOOKUP)
			if (int bitsDifference = BitHacks::CountBitsSet(bits ^ pattern); bitsDifference < bestDifference) {
				bestFormatInfo = decodedInfo;
				bestDifference = bitsDifference;
			}
	}

	// Hamming distance of the 32 masked codes is 7, by construction, so <= 3 bits
	// differing means we found a match
	if (bestDifference <= 3)
		return {ECLevelFromBits((bestFormatInfo >> 2) & 0x07), static_cast<uint8_t>(bestFormatInfo & 0x03)};

	return {};
}

} // namespace ZXing::MicroQRCode
