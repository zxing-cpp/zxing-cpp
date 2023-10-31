/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
* Copyright 2023 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "QRFormatInformation.h"

#include "BitHacks.h"
#include "ZXAlgorithms.h"

#include <array>

namespace ZXing::QRCode {

static uint32_t MirrorBits(uint32_t bits)
{
	return BitHacks::Reverse(bits) >> 17;
}

static FormatInformation FindBestFormatInfo(const std::vector<uint32_t>& masks, const std::vector<uint32_t>& bits)
{
	// See ISO 18004:2015, Annex C, Table C.1
	constexpr uint32_t MODEL2_MASKED_PATTERNS[] = {
		0x5412, 0x5125, 0x5E7C, 0x5B4B, 0x45F9, 0x40CE, 0x4F97, 0x4AA0, 0x77C4, 0x72F3, 0x7DAA, 0x789D, 0x662F, 0x6318, 0x6C41, 0x6976,
		0x1689, 0x13BE, 0x1CE7, 0x19D0, 0x0762, 0x0255, 0x0D0C, 0x083B, 0x355F, 0x3068, 0x3F31, 0x3A06, 0x24B4, 0x2183, 0x2EDA, 0x2BED,
	};

	FormatInformation fi;

	for (auto mask : masks)
		for (int bitsIndex = 0; bitsIndex < Size(bits); ++bitsIndex)
			for (uint32_t pattern : MODEL2_MASKED_PATTERNS) {
				// 'unmask' the pattern first to get the original 5-data bits + 10-ec bits back
				pattern ^= FORMAT_INFO_MASK_MODEL2;
				// Find the pattern with fewest bits differing
				if (int hammingDist = BitHacks::CountBitsSet((bits[bitsIndex] ^ mask) ^ pattern);
					hammingDist < fi.hammingDistance) {
					fi.mask = mask; // store the used mask to discriminate between types/models
					fi.data = pattern >> 10; // drop the 10 BCH error correction bits
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
	// Some (Model2) QR codes apparently do not apply the XOR mask. Try with (standard) and without (quirk) masking.
	auto fi = FindBestFormatInfo({FORMAT_INFO_MASK_MODEL2, 0, FORMAT_INFO_MASK_MODEL1},
								 {formatInfoBits1, formatInfoBits2, MirrorBits(formatInfoBits1), mirroredFormatInfoBits2});

	// Use bits 3/4 for error correction, and 0-2 for mask.
	fi.ecLevel = ECLevelFromBits((fi.data >> 3) & 0x03);
	fi.dataMask = static_cast<uint8_t>(fi.data & 0x07);
	fi.isMirrored = fi.bitsIndex > 1;

	return fi;
}

/**
 * @param formatInfoBits format info indicator, with mask still applied
 */
FormatInformation FormatInformation::DecodeMQR(uint32_t formatInfoBits)
{
	auto fi = FindBestFormatInfo({FORMAT_INFO_MASK_MICRO}, {formatInfoBits, MirrorBits(formatInfoBits)});

	constexpr uint8_t BITS_TO_VERSION[] = {1, 2, 2, 3, 3, 4, 4, 4};

	// Bits 2/3/4 contain both error correction level and version, 0/1 contain mask.
	fi.ecLevel = ECLevelFromBits((fi.data >> 2) & 0x07, true);
	fi.dataMask = static_cast<uint8_t>(fi.data & 0x03);
	fi.microVersion = BITS_TO_VERSION[(fi.data >> 2) & 0x07];
	fi.isMirrored = fi.bitsIndex == 1;

	return fi;
}

} // namespace ZXing::QRCode
