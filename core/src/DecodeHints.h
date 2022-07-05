/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
* Copyright 2020 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "BarcodeFormat.h"

#include <string>
#include <utility>
#include <vector>

namespace ZXing {

/**
 * @brief The Binarizer enum
 *
 * Specify which algorithm to use for the grayscale to binary transformation.
 * The difference is how to get to a threshold value T which results in a bit
 * value R = L <= T.
 */
enum class Binarizer : unsigned char // needs to unsigned for the bitfield below to work, uint8_t fails as well
{
	LocalAverage,    ///< T = average of neighboring pixels for matrix and GlobalHistogram for linear (HybridBinarizer)
	GlobalHistogram, ///< T = valley between the 2 largest peaks in the histogram (per line in linear case)
	FixedThreshold,  ///< T = 127
	BoolCast,        ///< T = 0, fastest possible
};

enum class EanAddOnSymbol : unsigned char // see above
{
	Ignore,  ///< Ignore any Add-On symbol during read/scan
	Read,    ///< Read EAN-2/EAN-5 Add-On symbol if found
	Require, ///< Require EAN-2/EAN-5 Add-On symbol to be present
};

class DecodeHints
{
	bool _tryHarder                : 1;
	bool _tryRotate                : 1;
	bool _tryDownscale             : 1;
	bool _isPure                   : 1;
	bool _tryCode39ExtendedMode    : 1;
	bool _validateCode39CheckSum   : 1;
	bool _validateITFCheckSum      : 1;
	bool _returnCodabarStartEnd    : 1;
	bool _returnErrors             : 1;
	EanAddOnSymbol _eanAddOnSymbol : 2;
	Binarizer _binarizer           : 2;

	uint8_t _minLineCount        = 2;
	uint8_t _maxNumberOfSymbols  = 0xff;
	uint8_t _downscaleFactor     = 3;
	uint16_t _downscaleThreshold = 500;
	BarcodeFormats _formats      = BarcodeFormat::None;
	std::string _characterSet;

public:
	// bitfields don't get default initialized to 0 before c++20
	DecodeHints()
		: _tryHarder(1),
		  _tryRotate(1),
		  _tryDownscale(1),
		  _isPure(0),
		  _tryCode39ExtendedMode(0),
		  _validateCode39CheckSum(0),
		  _validateITFCheckSum(0),
		  _returnCodabarStartEnd(0),
		  _returnErrors(0),
		  _eanAddOnSymbol(EanAddOnSymbol::Ignore),
		  _binarizer(Binarizer::LocalAverage)
	{}

#define ZX_PROPERTY(TYPE, GETTER, SETTER) \
	TYPE GETTER() const noexcept { return _##GETTER; } \
	DecodeHints& SETTER(TYPE v)& { return _##GETTER = std::move(v), *this; } \
	DecodeHints&& SETTER(TYPE v)&& { return _##GETTER = std::move(v), std::move(*this); }

	/// Specify a set of BarcodeFormats that should be searched for, the default is all supported formats.
	ZX_PROPERTY(BarcodeFormats, formats, setFormats)

	/// Spend more time to try to find a barcode; optimize for accuracy, not speed.
	ZX_PROPERTY(bool, tryHarder, setTryHarder)

	/// Also try detecting code in 90, 180 and 270 degree rotated images.
	ZX_PROPERTY(bool, tryRotate, setTryRotate)

	/// Also try detecting code in downscaled images (depending on image size).
	ZX_PROPERTY(bool, tryDownscale, setTryDownscale)

	/// Binarizer to use internally when using the ReadBarcode function
	ZX_PROPERTY(Binarizer, binarizer, setBinarizer)

	/// Set to true if the input contains nothing but a single perfectly aligned barcode (generated image)
	ZX_PROPERTY(bool, isPure, setIsPure)

	/// Image size ( min(width, height) ) threshold at which to start downscaled scanning
	// WARNING: this API is experimental and may change/disappear
	ZX_PROPERTY(uint16_t, downscaleThreshold, setDownscaleThreshold)

	/// Scale factor used during downscaling, meaningful values are 2, 3 and 4
	// WARNING: this API is experimental and may change/disappear
	ZX_PROPERTY(uint8_t, downscaleFactor, setDownscaleFactor)

	/// The number of scan lines in a linear barcode that have to be equal to accept the result, default is 2
	ZX_PROPERTY(uint8_t, minLineCount, setMinLineCount)

	/// The maximum number of symbols (barcodes) to detect / look for in the image with ReadBarcodes
	ZX_PROPERTY(uint8_t, maxNumberOfSymbols, setMaxNumberOfSymbols)

	/// Specifies fallback character set to use instead of auto-detecting it (when applicable)
	ZX_PROPERTY(std::string, characterSet, setCharacterSet)

	/// If true, the Code-39 reader will try to read extended mode.
	ZX_PROPERTY(bool, tryCode39ExtendedMode, setTryCode39ExtendedMode)

	/// Assume Code-39 codes employ a check digit and validate it.
	ZX_PROPERTY(bool, validateCode39CheckSum, setValidateCode39CheckSum)

	/// Assume ITF codes employ a GS1 check digit and validate it.
	ZX_PROPERTY(bool, validateITFCheckSum, setValidateITFCheckSum)

	/// If true, return the start and end chars in a Codabar barcode instead of stripping them.
	ZX_PROPERTY(bool, returnCodabarStartEnd, setReturnCodabarStartEnd)

	/// If true, return the barcodes with errors as well (e.g. checksum errors, see @Result::error())
	ZX_PROPERTY(bool, returnErrors, setReturnErrors)

	/// Specify whether to ignore, read or require EAN-2/5 add-on symbols while scanning EAN/UPC codes
	ZX_PROPERTY(EanAddOnSymbol, eanAddOnSymbol, setEanAddOnSymbol)

#undef ZX_PROPERTY

	/// NOTE: used to affect FNC1 handling for Code 128 (aka GS1-128) but behavior now based on position of FNC1.
	[[deprecated]] bool assumeGS1() const noexcept { return true; }
	[[deprecated]] DecodeHints& setAssumeGS1(bool v [[maybe_unused]]) { return *this; }

	/// NOTE: has not been in effect since at least 1.2 and no one noticed.
	[[deprecated]] std::vector<int> allowedLengths() const noexcept { return {}; }
	[[deprecated]] DecodeHints& setAllowedLengths(const std::vector<int> v [[maybe_unused]]) { return *this; }

	/// NOTE: use validateCode39CheckSum
	[[deprecated]] bool assumeCode39CheckDigit() const noexcept { return validateCode39CheckSum(); }
	[[deprecated]] DecodeHints& setAssumeCode39CheckDigit(bool v) & { return setValidateCode39CheckSum(v); }

	bool hasFormat(BarcodeFormats f) const noexcept { return _formats.testFlags(f) || _formats.empty(); }
	[[deprecated]] bool hasNoFormat() const noexcept { return _formats.empty(); }
};

} // ZXing
