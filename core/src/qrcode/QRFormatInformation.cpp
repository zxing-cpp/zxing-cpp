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

#include "QRFormatInformation.h"

#include "BitHacks.h"

#include <array>

namespace ZXing::QRCode {

static const int FORMAT_INFO_MASK_QR = 0x5412;

/**
* See ISO 18004:2006, Annex C, Table C.1
*/
static const std::array<int, 2> FORMAT_INFO_DECODE_LOOKUP[] = {
	{0x5412, 0x00},
	{0x5125, 0x01},
	{0x5E7C, 0x02},
	{0x5B4B, 0x03},
	{0x45F9, 0x04},
	{0x40CE, 0x05},
	{0x4F97, 0x06},
	{0x4AA0, 0x07},
	{0x77C4, 0x08},
	{0x72F3, 0x09},
	{0x7DAA, 0x0A},
	{0x789D, 0x0B},
	{0x662F, 0x0C},
	{0x6318, 0x0D},
	{0x6C41, 0x0E},
	{0x6976, 0x0F},
	{0x1689, 0x10},
	{0x13BE, 0x11},
	{0x1CE7, 0x12},
	{0x19D0, 0x13},
	{0x0762, 0x14},
	{0x0255, 0x15},
	{0x0D0C, 0x16},
	{0x083B, 0x17},
	{0x355F, 0x18},
	{0x3068, 0x19},
	{0x3F31, 0x1A},
	{0x3A06, 0x1B},
	{0x24B4, 0x1C},
	{0x2183, 0x1D},
	{0x2EDA, 0x1E},
	{0x2BED, 0x1F},
};

FormatInformation::FormatInformation(int formatInfo)
{
	// Bits 3,4
	_errorCorrectionLevel = ECLevelFromBits((formatInfo >> 3) & 0x03);
	// Bottom 3 bits
	_dataMask = static_cast<uint8_t>(formatInfo & 0x07);
}

/**
* @param formatInfoBits1 format info indicator, with mask still applied
* @param formatInfoBits2 second copy of same info; both are checked at the same time
*  to establish best match
* @return information about the format it specifies, or {@code null}
*  if doesn't seem to match any known pattern
*/
FormatInformation
FormatInformation::DecodeFormatInformation(uint32_t formatInfoBits1, uint32_t formatInfoBits2)
{
	// Find the int in FORMAT_INFO_DECODE_LOOKUP with fewest bits differing
	int bestDifference = 32;
	int bestFormatInfo = -1;

	// Some QR codes apparently do not apply the XOR mask. Try without and with additional masking.
	for (auto mask : {0, FORMAT_INFO_MASK_QR})
		for (uint32_t bits : {formatInfoBits1 ^ mask, formatInfoBits2 ^ mask})
			for (auto& [pattern, decodedInfo] : FORMAT_INFO_DECODE_LOOKUP)
				if (int bitsDifference = BitHacks::CountBitsSet(bits ^ pattern); bitsDifference < bestDifference) {
					bestFormatInfo = decodedInfo;
					bestDifference = bitsDifference;
				}

	// Hamming distance of the 32 masked codes is 7, by construction, so <= 3 bits
	// differing means we found a match
	if (bestDifference <= 3)
		return {bestFormatInfo};

	return {};
}

} // namespace ZXing::QRCode
