#pragma once
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

	static FormatInformation DecodeFormatInformation(int maskedFormatInfo1, int maskedFormatInfo2);

	ErrorCorrectionLevel errorCorrectionLevel() const {
		return _errorCorrectionLevel;
	}

	uint8_t dataMask() const {
		return _dataMask;
	}

	bool isValid() const { return _errorCorrectionLevel != ErrorCorrectionLevel::Invalid; }

	bool operator==(const FormatInformation& other) const {
		return _dataMask == other._dataMask && _errorCorrectionLevel == other._errorCorrectionLevel;
	}

private:
	ErrorCorrectionLevel _errorCorrectionLevel = ErrorCorrectionLevel::Invalid;
	uint8_t _dataMask = 0;

	FormatInformation(int formatInfo);

	static FormatInformation DoDecodeFormatInformation(int maskedFormatInfo1, int maskedFormatInfo2);
};

} // QRCode
} // ZXing
