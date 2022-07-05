/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Flags.h"

#include <string>

namespace ZXing {

/**
* Enumerates barcode formats known to this package.
*/
enum class BarcodeFormat
{
	// The values are an implementation detail. The c++ use-case (ZXing::Flags) could have been designed such that it
	// would not have been necessary to explicitly set the values to single bit constants. This has been done to ease
	// the interoperability with C-like interfaces, the python and the Qt wrapper.
	None            = 0,         ///< Used as a return value if no valid barcode has been detected
	Aztec           = (1 << 0),  ///< Aztec
	Codabar         = (1 << 1),  ///< Codabar
	Code39          = (1 << 2),  ///< Code39
	Code93          = (1 << 3),  ///< Code93
	Code128         = (1 << 4),  ///< Code128
	DataBar         = (1 << 5),  ///< GS1 DataBar, formerly known as RSS 14
	DataBarExpanded = (1 << 6),  ///< GS1 DataBar Expanded, formerly known as RSS EXPANDED
	DataMatrix      = (1 << 7),  ///< DataMatrix
	EAN8            = (1 << 8),  ///< EAN-8
	EAN13           = (1 << 9),  ///< EAN-13
	ITF             = (1 << 10), ///< ITF (Interleaved Two of Five)
	MaxiCode        = (1 << 11), ///< MaxiCode
	PDF417          = (1 << 12), ///< PDF417
	QRCode          = (1 << 13), ///< QR Code
	UPCA            = (1 << 14), ///< UPC-A
	UPCE            = (1 << 15), ///< UPC-E
	MicroQRCode     = (1 << 16), ///< Micro QR Code

	LinearCodes = Codabar | Code39 | Code93 | Code128 | EAN8 | EAN13 | ITF | DataBar | DataBarExpanded | UPCA | UPCE,
	MatrixCodes = Aztec | DataMatrix | MaxiCode | PDF417 | QRCode | MicroQRCode,
	Any         = LinearCodes | MatrixCodes,

	// Deprecated names, kept for compatibility at the moment
	NONE [[deprecated]]         = None,
	AZTEC [[deprecated]]        = Aztec,
	CODABAR [[deprecated]]      = Codabar,
	CODE_39 [[deprecated]]      = Code39,
	CODE_93 [[deprecated]]      = Code93,
	CODE_128 [[deprecated]]     = Code128,
	DATA_MATRIX [[deprecated]]  = DataMatrix,
	EAN_8 [[deprecated]]        = EAN8,
	EAN_13 [[deprecated]]       = EAN13,
	MAXICODE [[deprecated]]     = MaxiCode,
	PDF_417 [[deprecated]]      = PDF417,
	QR_CODE [[deprecated]]      = QRCode,
	RSS_14 [[deprecated]]       = DataBar,
	RSS_EXPANDED [[deprecated]] = DataBarExpanded,
	UPC_A [[deprecated]]        = UPCA,
	UPC_E [[deprecated]]        = UPCE,
	OneDCodes [[deprecated]]    = LinearCodes,
	TwoDCodes [[deprecated]]    = MatrixCodes,

	_max                        = MicroQRCode, ///> implementation detail, don't use
};

ZX_DECLARE_FLAGS(BarcodeFormats, BarcodeFormat)

const char* ToString(BarcodeFormat format);
std::string ToString(BarcodeFormats formats);

/**
 * @brief Parse a string into a BarcodeFormat. '-' and '_' are optional.
 * @return None if str can not be parsed as a valid enum value
 */
BarcodeFormat BarcodeFormatFromString(const std::string& str);

/**
 * @brief Parse a string into a set of BarcodeFormats.
 * Separators can be (any combination of) '|', ',' or ' '.
 * Underscores are optional and input can be lower case.
 * e.g. "EAN-8 qrcode, Itf" would be parsed into [EAN8, QRCode, ITF].
 * @throws std::invalid_parameter if the string can not be fully parsed.
 */
BarcodeFormats BarcodeFormatsFromString(const std::string& str);

} // ZXing
