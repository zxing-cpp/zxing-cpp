/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "QRErrorCorrectionLevel.h"

#include <cstdint>

namespace ZXing {
namespace QRCode {

/**
* <p>Encapsulates a QR Code's format information, including the data mask used and
* error correction level.</p>
*
* @author Sean Owen
* @see DataMask
* @see ErrorCorrectionLevel
*/
class FormatInformation
{
public:
	FormatInformation() = default;

	static FormatInformation DecodeQR(uint32_t formatInfoBits1, uint32_t formatInfoBits2);
	static FormatInformation DecodeMQR(uint32_t formatInfoBits);

	ErrorCorrectionLevel errorCorrectionLevel() const { return _errorCorrectionLevel; }

	uint8_t dataMask() const { return _dataMask; }
	uint8_t microVersion() const { return _microVersion; }

	bool isValid() const { return _errorCorrectionLevel != ErrorCorrectionLevel::Invalid; }

	bool operator==(const FormatInformation& other) const
	{
		return _dataMask == other._dataMask && _errorCorrectionLevel == other._errorCorrectionLevel;
	}

private:
	ErrorCorrectionLevel _errorCorrectionLevel = ErrorCorrectionLevel::Invalid;
	uint8_t _dataMask = 0;
	uint8_t _microVersion = 0;

	FormatInformation(const ErrorCorrectionLevel& errorCorrectionLevel, uint8_t dataMask, uint8_t microVersion = 0)
		: _errorCorrectionLevel(errorCorrectionLevel), _dataMask(dataMask), _microVersion(microVersion)
	{}
};

} // QRCode
} // ZXing
