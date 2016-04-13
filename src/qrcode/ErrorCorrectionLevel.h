#pragma once

namespace ZXing {
namespace QRCode {

/**
* <p>See ISO 18004:2006, 6.5.1. This enum encapsulates the four error correction levels
* defined by the QR code standard.</p>
*/
enum class ErrorCorrectionLevel
{
	Medium,			// M = ~15% correction
	Low,			// L = ~7 % correction
	High,			// H = ~30% correction
	Quality,		// Q = ~25% correction
};

} // QRCode
} // ZXing
