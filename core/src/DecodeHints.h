#pragma once
/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
* Copyright 2020 Axel Waggershauser
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
	LocalAverage,    ///< T = average of neighboring pixels for 2D and GlobalHistogram for 1D (HybridBinarizer)
	GlobalHistogram, ///< T = valley between the 2 largest peaks in the histogram (per line in 1D case)
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
	bool _tryHarder : 1;
	bool _tryRotate : 1;
	bool _tryDownscale : 1;
	bool _isPure : 1;
	bool _tryCode39ExtendedMode : 1;
	bool _validateCode39CheckSum : 1;
	bool _validateITFCheckSum : 1;
	bool _returnCodabarStartEnd : 1;
	Binarizer _binarizer : 2;
	EanAddOnSymbol _eanAddOnSymbol : 2;

	std::string _characterSet;
	std::vector<int> _allowedLengths;
	BarcodeFormats _formats = BarcodeFormat::None;
	uint16_t _downscaleThreshold = 500;
	uint8_t _downscaleFactor = 3;
	uint8_t _minLineCount = 2;
	uint8_t _maxNumberOfSymbols = 0xff;

public:
	// bitfields don't get default initialized to 0.
	DecodeHints()
		: _tryHarder(1), _tryRotate(1), _tryDownscale(1), _isPure(0), _tryCode39ExtendedMode(0),
		  _validateCode39CheckSum(0), _validateITFCheckSum(0), _returnCodabarStartEnd(0),
		  _binarizer(Binarizer::LocalAverage), _eanAddOnSymbol(EanAddOnSymbol::Ignore)
	{}

#define ZX_PROPERTY(TYPE, GETTER, SETTER) \
	TYPE GETTER() const noexcept { return _##GETTER; } \
	DecodeHints& SETTER(TYPE v) { return _##GETTER = std::move(v), *this; }

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

	/// The number of scan lines in a 1D barcode that have to be equal to accept the result, default is 2
	ZX_PROPERTY(uint8_t, minLineCount, setMinLineCount)

	/// The maximum number of symbols (barcodes) to detect / look for in the image with ReadBarcodes
	ZX_PROPERTY(uint8_t, maxNumberOfSymbols, setMaxNumberOfSymbols)

	/// Specifies what character encoding to use when decoding, where applicable.
	ZX_PROPERTY(std::string, characterSet, setCharacterSet)

	/// Allowed lengths of encoded data -- reject anything else..
	ZX_PROPERTY(std::vector<int>, allowedLengths, setAllowedLengths)

	/// If true, the Code-39 reader will try to read extended mode.
	ZX_PROPERTY(bool, tryCode39ExtendedMode, setTryCode39ExtendedMode)

	/// Assume Code-39 codes employ a check digit and validate it.
	ZX_PROPERTY(bool, validateCode39CheckSum, setValidateCode39CheckSum)

	/// Assume ITF codes employ a GS1 check digit and validate it.
	ZX_PROPERTY(bool, validateITFCheckSum, setValidateITFCheckSum)

	/**
	* If true, return the start and end digits in a Codabar barcode instead of stripping them. They
	* are alpha, whereas the rest are numeric. By default, they are stripped, but this causes them
	* to not be.
	*/
	ZX_PROPERTY(bool, returnCodabarStartEnd, setReturnCodabarStartEnd)

	/// Specify whether to ignore, read or require EAN-2/5 add-on symbols while scanning EAN/UPC codes
	ZX_PROPERTY(EanAddOnSymbol, eanAddOnSymbol, setEanAddOnSymbol)

#undef ZX_PROPERTY
#undef ZX_PROPERTY_DEPRECATED

	/// NOTE: used to affect FNC1 handling for Code 128 (aka GS1-128) but behavior now based on position of FNC1.
	[[deprecated]] bool assumeGS1() const noexcept { return true; }
	[[deprecated]] DecodeHints& setAssumeGS1(bool v [[maybe_unused]]) { return *this; }

	/// NOTE: use validateCode39CheckSum
	[[deprecated]] bool assumeCode39CheckDigit() const noexcept { return validateCode39CheckSum(); }
	[[deprecated]] DecodeHints& setAssumeCode39CheckDigit(bool v) { return setValidateCode39CheckSum(v); }

	bool hasFormat(BarcodeFormats f) const noexcept { return _formats.testFlags(f); }
	bool hasNoFormat() const noexcept { return _formats.empty(); }
};

} // ZXing
