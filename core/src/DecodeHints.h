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

#include "BarcodeFormat.h"

#include <vector>
#include <string>

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

class DecodeHints
{
	bool _tryHarder : 1;
	bool _tryRotate : 1;
	bool _isPure : 1;
	bool _tryCode39ExtendedMode : 1;
	bool _assumeCode39CheckDigit : 1;
	bool _assumeGS1 : 1;
	bool _returnCodabarStartEnd : 1;
	Binarizer _binarizer : 2;

	BarcodeFormats _formats = BarcodeFormat::NONE;
	std::string _characterSet;
	std::vector<int> _allowedLengths;
	std::vector<int> _allowedEanExtensions;

public:
	// bitfields don't get default initialized to 0.
	DecodeHints()
		: _tryHarder(1), _tryRotate(1), _isPure(0), _tryCode39ExtendedMode(0), _assumeCode39CheckDigit(0),
		  _assumeGS1(0), _returnCodabarStartEnd(0), _binarizer(Binarizer::LocalAverage)
	{}

#define ZX_PROPERTY(TYPE, GETTER, SETTER) \
	inline TYPE GETTER() const noexcept { return _##GETTER; } \
	inline DecodeHints& SETTER(TYPE v) { return _##GETTER = std::move(v), *this; }

	/// Specify a set of BarcodeFormats that should be searched for, the default is all supported formats.
	ZX_PROPERTY(BarcodeFormats, formats, setFormats)

	/// Spend more time to try to find a barcode; optimize for accuracy, not speed.
	ZX_PROPERTY(bool, tryHarder, setTryHarder)

	/// Also try detecting code in 90, 180 and 270 degree rotated images.
	ZX_PROPERTY(bool, tryRotate, setTryRotate)

	/// Binarizer to use internally when using the ReadBarcode function
	ZX_PROPERTY(Binarizer, binarizer, setBinarizer)

	/// Set to true if the input contains nothing but a perfectly aligned barcode (generated image)
	ZX_PROPERTY(bool, isPure, setIsPure)

	/// Specifies what character encoding to use when decoding, where applicable.
	ZX_PROPERTY(std::string, characterSet, setCharacterSet)

	/// Allowed lengths of encoded data -- reject anything else..
	ZX_PROPERTY(std::vector<int>, allowedLengths, setAllowedLengths)

	/// If true, the Code-39 reader will try to read extended mode.
	ZX_PROPERTY(bool, tryCode39ExtendedMode, setTryCode39ExtendedMode)

	/// Assume Code-39 codes employ a check digit.
	ZX_PROPERTY(bool, assumeCode39CheckDigit, setAssumeCode39CheckDigit)

	/**
	* Assume the barcode is being processed as a GS1 barcode, and modify behavior as needed.
	* For example this affects FNC1 handling for Code 128 (aka GS1-128).
	*/
	ZX_PROPERTY(bool, assumeGS1, setAssumeGS1)

	/**
	* If true, return the start and end digits in a Codabar barcode instead of stripping them. They
	* are alpha, whereas the rest are numeric. By default, they are stripped, but this causes them
	* to not be.
	*/
	ZX_PROPERTY(bool, returnCodabarStartEnd, setReturnCodabarStartEnd)

	/**
	* Allowed extension lengths for EAN or UPC barcodes. Other formats will ignore this.
	* Maps to an {@code int[]} of the allowed extension lengths, for example [2], [5], or [2, 5].
	* If it is optional to have an extension, do not set this hint. If this is set,
	* and a UPC or EAN barcode is found but an extension is not, then no result will be returned
	* at all.
	*/
	ZX_PROPERTY(std::vector<int>, allowedEanExtensions, setAllowedEanExtensions)

#undef ZX_PROPERTY

	bool hasFormat(BarcodeFormat f) const noexcept { return _formats.testFlag(f); }
	bool hasNoFormat() const noexcept { return _formats.testFlag(BarcodeFormat::NONE); }

	[[deprecated]] DecodeHints& setPossibleFormats(const std::vector<BarcodeFormat>& formats)
	{
		_formats.clear();
		for (auto f : formats)
			_formats |= f;
		return *this;
	}
};

} // ZXing
