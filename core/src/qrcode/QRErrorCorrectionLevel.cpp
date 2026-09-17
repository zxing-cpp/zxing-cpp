/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "QRErrorCorrectionLevel.h"
#include "ZXAlgorithms.h"

#include <cassert>

namespace ZXing::QRCode {

std::string ToString(ErrorCorrectionLevel l)
{
	assert(l != ErrorCorrectionLevel::Invalid);
	return EnumToString(l, {"L", "M", "Q", "H", nullptr});
}

ErrorCorrectionLevel ECLevelFromString(const char* str)
{
	switch (str[0]) {
	case 'L': return ErrorCorrectionLevel::Low;
	case 'M': return ErrorCorrectionLevel::Medium;
	case 'Q': return ErrorCorrectionLevel::Quality;
	case 'H': return ErrorCorrectionLevel::High;
	default:  return ErrorCorrectionLevel::Invalid;
	}
}

ErrorCorrectionLevel ECLevelFromBits(int bits, const bool isMicro)
{
	using enum ErrorCorrectionLevel;
	if (isMicro) {
		constexpr ErrorCorrectionLevel LEVEL_FOR_BITS[] = {Low, Low, Medium, Low, Medium, Low, Medium, Quality};
		return LEVEL_FOR_BITS[bits & 0x07];
	}
	constexpr ErrorCorrectionLevel LEVEL_FOR_BITS[] = {Medium, Low, High, Quality};
	return LEVEL_FOR_BITS[bits & 0x3];
}

int BitsFromECLevel(ErrorCorrectionLevel l)
{
	assert(l != ErrorCorrectionLevel::Invalid);
	static const int BITS[] = {1, 0, 3, 2, -1};
	return BITS[(int)l];
}

} // namespace ZXing::QRCode
