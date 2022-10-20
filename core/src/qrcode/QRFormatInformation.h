/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "QRErrorCorrectionLevel.h"

#include <cstdint>

namespace ZXing::QRCode {

class FormatInformation
{
public:
	uint8_t index = 255;
	uint8_t hammingDistance = 255;
	bool isMirrored = false;
	uint8_t dataMask = 0;
	uint8_t microVersion = 0;
	uint8_t bitsIndex = 255;
	ErrorCorrectionLevel ecLevel = ErrorCorrectionLevel::Invalid;

	FormatInformation() = default;

	static FormatInformation DecodeQR(uint32_t formatInfoBits1, uint32_t formatInfoBits2);
	static FormatInformation DecodeMQR(uint32_t formatInfoBits);

	// Hamming distance of the 32 masked codes is 7, by construction, so <= 3 bits differing means we found a match
	bool isValid() const { return hammingDistance <= 3; }

	bool operator==(const FormatInformation& other) const
	{
		return dataMask == other.dataMask && ecLevel == other.ecLevel;
	}
};

} // namespace ZXing::QRCode
