/*
* Copyright 2024 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "ZXingCpp.h"

#include "Version.h"

namespace ZXing {

const std::string& Version()
{
	static std::string res = ZXING_VERSION_STR;
	return res;
}

#ifdef ZXING_EXPERIMENTAL_API

BarcodeFormats SupportedBarcodeFormats(Operation op)
{
	switch (op) {
	case Operation::Read:
		return BarcodeFormat::None
#ifdef ZXING_READERS
#ifdef ZXING_ENABLE_1D
			   | BarcodeFormat::LinearCodes
#endif
#ifdef ZXING_ENABLE_AZTEC
			   | BarcodeFormat::Aztec
#endif
#ifdef ZXING_ENABLE_DATAMATRIX
			   | BarcodeFormat::DataMatrix
#endif
#ifdef ZXING_ENABLE_MAXICODE
			   | BarcodeFormat::MaxiCode
#endif
#ifdef ZXING_ENABLE_PDF417
			   | BarcodeFormat::PDF417
#endif
#ifdef ZXING_ENABLE_QRCODE
			   | BarcodeFormat::QRCode | BarcodeFormat::MicroQRCode | BarcodeFormat::RMQRCode
#endif
#endif // ZXING_READERS
			;
	case Operation::Create:
#if defined(ZXING_WRITERS) && defined(ZXING_USE_ZINT)
		return BarcodeFormat::Any;
#else
		return BarcodeFormat::None
#ifdef ZXING_WRITERS
#ifdef ZXING_ENABLE_1D
			   | BarcodeFormat::LinearCodes
#endif
#ifdef ZXING_ENABLE_AZTEC
			   | BarcodeFormat::Aztec
#endif
#ifdef ZXING_ENABLE_DATAMATRIX
			   | BarcodeFormat::DataMatrix
#endif
#ifdef ZXING_ENABLE_MAXICODE
			   | BarcodeFormat::MaxiCode
#endif
#ifdef ZXING_ENABLE_PDF417
			   | BarcodeFormat::PDF417
#endif
#ifdef ZXING_ENABLE_QRCODE
			   | BarcodeFormat::QRCode
#endif
#endif
			;
#endif // ZXING_WRITERS
	case Operation::CreateAndRead: return SupportedBarcodeFormats(Operation::Create) & SupportedBarcodeFormats(Operation::Read);
	case Operation::CreateOrRead: return SupportedBarcodeFormats(Operation::Create) | SupportedBarcodeFormats(Operation::Read);
	}

	return {}; // unreachable code
}

#endif // ZXING_EXPERIMENTAL_API

} // namespace ZXing
