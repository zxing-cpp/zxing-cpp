/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "QRFormatInformation.h"

#include "BitHacks.h"
#include "ZXAlgorithms.h"

#include <array>

namespace ZXing::QRCode {

static const int FORMAT_INFO_MASK_QR = 0x5412;

/**
* See ISO 18004:2006, Annex C, Table C.1
*/
static const std::array<std::pair<int, int>, 32> FORMAT_INFO_DECODE_LOOKUP = {{
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
}};

static const std::array<std::pair<int, int>, 32> FORMAT_INFO_DECODE_LOOKUP_MICRO = {{
	{0x4445, 0x00},
	{0x4172, 0x01},
	{0x4E2B, 0x02},
	{0x4B1C, 0x03},
	{0x55AE, 0x04},
	{0x5099, 0x05},
	{0x5FC0, 0x06},
	{0x5AF7, 0x07},
	{0x6793, 0x08},
	{0x62A4, 0x09},
	{0x6DFD, 0x0A},
	{0x68CA, 0x0B},
	{0x7678, 0x0C},
	{0x734F, 0x0D},
	{0x7C16, 0x0E},
	{0x7921, 0x0F},
	{0x06DE, 0x10},
	{0x03E9, 0x11},
	{0x0CB0, 0x12},
	{0x0987, 0x13},
	{0x1735, 0x14},
	{0x1202, 0x15},
	{0x1D5B, 0x16},
	{0x186C, 0x17},
	{0x2508, 0x18},
	{0x203F, 0x19},
	{0x2F66, 0x1A},
	{0x2A51, 0x1B},
	{0x34E3, 0x1C},
	{0x31D4, 0x1D},
	{0x3E8D, 0x1E},
	{0x3BBA, 0x1F},
}};

static uint32_t MirrorBits(uint32_t bits)
{
	return BitHacks::Reverse(bits) >> 17;
}

static FormatInformation FindBestFormatInfo(int mask, const std::array<std::pair<int, int>, 32> lookup,
											const std::vector<uint32_t>& bits)
{
	FormatInformation fi;

	// Some QR codes apparently do not apply the XOR mask. Try without and with additional masking.
	for (auto mask : {0, mask})
		for (int bitsIndex = 0; bitsIndex < Size(bits); ++bitsIndex)
			for (const auto& [pattern, index] : lookup) {
				// Find the int in lookup with fewest bits differing
				if (int hammingDist = BitHacks::CountBitsSet((bits[bitsIndex] ^ mask) ^ pattern); hammingDist < fi.hammingDistance) {
					fi.index = index;
					fi.hammingDistance = hammingDist;
					fi.bitsIndex = bitsIndex;
				}
			}

	return fi;
}

/**
* @param formatInfoBits1 format info indicator, with mask still applied
* @param formatInfoBits2 second copy of same info; both are checked at the same time to establish best match
*/
FormatInformation FormatInformation::DecodeQR(uint32_t formatInfoBits1, uint32_t formatInfoBits2)
{
	// maks out the 'Dark Module' for mirrored and non-mirrored case (see Figure 25 in ISO/IEC 18004:2015)
	uint32_t mirroredFormatInfoBits2 = MirrorBits(((formatInfoBits2 >> 1) & 0b111111110000000) | (formatInfoBits2 & 0b1111111));
	formatInfoBits2 = ((formatInfoBits2 >> 1) & 0b111111100000000) | (formatInfoBits2 & 0b11111111);
	auto fi = FindBestFormatInfo(FORMAT_INFO_MASK_QR, FORMAT_INFO_DECODE_LOOKUP,
								 {formatInfoBits1, formatInfoBits2, MirrorBits(formatInfoBits1), mirroredFormatInfoBits2});

	// Use bits 3/4 for error correction, and 0-2 for mask.
	fi.ecLevel = ECLevelFromBits((fi.index >> 3) & 0x03);
	fi.dataMask = static_cast<uint8_t>(fi.index & 0x07);
	fi.isMirrored = fi.bitsIndex > 1;

	return fi;
}

/**
 * @param formatInfoBits format info indicator, with mask still applied
 */
FormatInformation FormatInformation::DecodeMQR(uint32_t formatInfoBits)
{
	// We don't use the additional masking (with 0x4445) to work around potentially non complying MicroQRCode encoders
	auto fi = FindBestFormatInfo(0, FORMAT_INFO_DECODE_LOOKUP_MICRO, {formatInfoBits, MirrorBits(formatInfoBits)});

	constexpr uint8_t BITS_TO_VERSION[] = {1, 2, 2, 3, 3, 4, 4, 4};

	// Bits 2/3/4 contain both error correction level and version, 0/1 contain mask.
	fi.ecLevel = ECLevelFromBits((fi.index >> 2) & 0x07, true);
	fi.dataMask = static_cast<uint8_t>(fi.index & 0x03);
	fi.microVersion = BITS_TO_VERSION[(fi.index >> 2) & 0x07];
	fi.isMirrored = fi.bitsIndex == 1;

	return fi;
}

} // namespace ZXing::QRCode
