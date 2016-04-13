#pragma once
#include "qrcode/ErrorCorrectionLevel.h"

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
	FormatInformation();

	bool decode(int maskedFormatInfo1, int maskedFormatInfo2);

	ErrorCorrectionLevel errorCorrectionLevel() const {
		return _errorCorrectionLevel;
	}

	uint8_t dataMask() const {
		return _dataMask;
	}

private:
	ErrorCorrectionLevel _errorCorrectionLevel;
	uint8_t _dataMask;

	void set(int formatInfo);
	bool doDecode(int maskedFormatInfo1, int maskedFormatInfo2);
};

} // QRCode
} // ZXing
