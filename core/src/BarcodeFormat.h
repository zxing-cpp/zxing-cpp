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

#include "ZXFlags.h"

#include <string>
#include <vector>

namespace ZXing {

/**
* Enumerates barcode formats known to this package.
*/
enum class BarcodeFormat
{
	// The values are an implementation detail. The c++ use-case (ZXing::Flags) could have been designed such that it
	// would not have been necessary to explicitly set the values to single bit constants. This has been done to ease
	// the interoperability with C-like interfaces, the python and the Qt wrapper.
	NONE              = 0,         ///< Used as a return value if no valid barcode has been detected
	AZTEC             = (1 << 0),  ///< Aztec (2D)
	CODABAR           = (1 << 1),  ///< CODABAR (1D)
	CODE_39           = (1 << 2),  ///< Code 39 (1D)
	CODE_93           = (1 << 3),  ///< Code 93 (1D)
	CODE_128          = (1 << 4),  ///< Code 128 (1D)
	DATA_MATRIX       = (1 << 5),  ///< Data Matrix (2D)
	EAN_8             = (1 << 6),  ///< EAN-8 (1D)
	EAN_13            = (1 << 7),  ///< EAN-13 (1D)
	ITF               = (1 << 8),  ///< ITF (Interleaved Two of Five) (1D)
	MAXICODE          = (1 << 9),  ///< MaxiCode (2D)
	PDF_417           = (1 << 10), ///< PDF417 (1D) or (2D)
	QR_CODE           = (1 << 11), ///< QR Code (2D)
	RSS_14            = (1 << 12), ///< RSS 14
	RSS_EXPANDED      = (1 << 13), ///< RSS EXPANDED
	UPC_A             = (1 << 14), ///< UPC-A (1D)
	UPC_E             = (1 << 15), ///< UPC-E (1D)
	UPC_EAN_EXTENSION = (1 << 16), ///< UPC/EAN extension (1D). Not a stand-alone format.

	FORMAT_COUNT = NONE,           ///> DEPRECATED: Used to count the number of formats
	_max         = UPC_EAN_EXTENSION, ///> implementation detail, don't use
};

ZX_DECLARE_FLAGS(BarcodeFormats, BarcodeFormat)

const char* ToString(BarcodeFormat format);
std::string ToString(BarcodeFormats formats);

/**
 * @brief Parse a string into a BarcodeFormat.
 * @return NONE if str can not be parsed as a valid enum value
 */
BarcodeFormat BarcodeFormatFromString(const std::string& str);

/**
 * @brief Parse a string into a set of BarcodeFormats.
 * Separators can be (any combination of) '|', ',' or ' '.
 * Underscors are optional and input can be lower case.
 * e.g. "EAN_8 qrcode, Itf" would be parsed into [EAN_8, QR_CODE, ITF].
 * @throws std::invalid_parameter Throws if the string can not be fully parsed.
 */
BarcodeFormats BarcodeFormatsFromString(const std::string& str);

} // ZXing
