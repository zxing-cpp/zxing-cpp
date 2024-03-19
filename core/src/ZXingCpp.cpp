/*
* Copyright 2024 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#ifdef ZXING_EXPERIMENTAL_API

#include "ZXingCpp.h"

namespace ZXing {

BarcodeFormats SupportedBarcodeFormats(Operation op)
{
	switch (op) {
	case Operation::Read:
#ifdef ZXING_READERS
		return BarcodeFormat::Any;
#else
		return BarcodeFormat::None;
#endif
	case Operation::Create:
#if defined(ZXING_WRITERS) && defined(ZXING_EXPERIMENTAL_API)
		return BarcodeFormats(BarcodeFormat::Any).setFlag(BarcodeFormat::DXFilmEdge, false);
#else
		return BarcodeFormat::None;
#endif
	case Operation::CreateAndRead: return SupportedBarcodeFormats(Operation::Create) & SupportedBarcodeFormats(Operation::Read);
	case Operation::CreateOrRead: return SupportedBarcodeFormats(Operation::Create) | SupportedBarcodeFormats(Operation::Read);
	}

	return {}; // unreachable code
}

} // namespace ZXing

#endif // ZXING_EXPERIMENTAL_API
