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

#include <vector>
#include <sstream>
#include <string>

#include "BarcodeFormat.h"

namespace ZXing {

class DecodeHints
{
public:

	std::vector<BarcodeFormat> possibleFormats() const;
	DecodeHints& setPossibleFormats(const std::vector<BarcodeFormat>& formats);

	bool hasFormat(BarcodeFormat f) const noexcept {
		return getFlag((int)f);
	}

	bool hasNoFormat() const noexcept {
		return (_flags & ~(0xffffffff << (int)BarcodeFormat::FORMAT_COUNT)) == 0;
	}

	/**
	* Spend more time to try to find a barcode; optimize for accuracy, not speed.
	*/
	bool tryHarder() const {
		return getFlag(TRY_HARDER);
	}

	DecodeHints& setTryHarder(bool v) {
		setFlag(TRY_HARDER, v);
		return *this;
	}

	/**
	* For 1D readers only, should try to rotate image 90 CCW if not barcode found.
	*/
	bool tryRotate() const {
		return getFlag(TRY_ROTATE);
	}

	DecodeHints& setTryRotate(bool v) {
		setFlag(TRY_ROTATE, v);
		return *this;
	}

	/**
	* Specifies what character encoding to use when decoding, where applicable.
	*/
	const std::string& characterSet() const {
		return _charset;
	}

	DecodeHints& setCharacterSet(const std::string& charset) {
		_charset = charset;
		return *this;
	}

	/**
	* Allowed lengths of encoded data -- reject anything else..
	*/
	const std::vector<int>& allowedLengths() const {
		return _lengths;
	}

	DecodeHints& setAllowLengths(const std::vector<int>& lengths) {
		_lengths = lengths;
		return *this;
	}

	/**
	* If true, the CODE-39 reader will try to read extended mode.
	*/
	bool tryCode39ExtendedMode() const {
		return getFlag(WITH_CODE_39_EXTENDED);
	}
	DecodeHints& setTryCode39ExtendedMode(bool v) {
		setFlag(WITH_CODE_39_EXTENDED, v);
		return *this;
	}

	/**
	* Assume Code 39 codes employ a check digit.
	*/
	bool assumeCode39CheckDigit() const {
		return getFlag(ASSUME_CODE_39_CHECK_DIGIT);
	}
	DecodeHints& setAssumeCode39CheckDigit(bool v) {
		setFlag(ASSUME_CODE_39_CHECK_DIGIT, v);
		return *this;
	}

	/**
	* Assume the barcode is being processed as a GS1 barcode, and modify behavior as needed.
	* For example this affects FNC1 handling for Code 128 (aka GS1-128).
	*/
	bool assumeGS1() const {
		return getFlag(ASSUME_GS1);
	}
	DecodeHints& setAssumeGS1(bool v) {
		setFlag(ASSUME_GS1, v);
		return *this;
	}

	/**
	* If true, return the start and end digits in a Codabar barcode instead of stripping them. They
	* are alpha, whereas the rest are numeric. By default, they are stripped, but this causes them
	* to not be.
	*/
	bool returnCodabarStartEnd() const {
		return getFlag(RETURN_CODABAR_START_END);
	}
	DecodeHints& setReturnCodabarStartEnd(bool v) {
		setFlag(RETURN_CODABAR_START_END, v);
		return *this;
	}

	/**
	* Allowed extension lengths for EAN or UPC barcodes. Other formats will ignore this.
	* Maps to an {@code int[]} of the allowed extension lengths, for example [2], [5], or [2, 5].
	* If it is optional to have an extension, do not set this hint. If this is set,
	* and a UPC or EAN barcode is found but an extension is not, then no result will be returned
	* at all.
	*/
	const std::vector<int>& allowedEanExtensions() const {
		return _eanExts;
	}

	DecodeHints& setAllowedEanExtensions(const std::vector<int>& extensions) {
		_eanExts = extensions;
		return *this;
	}

private:
	uint32_t _flags = 0;
	std::string _charset;
	std::vector<int> _lengths;
	std::vector<int> _eanExts;

	enum HintFlag
	{
		TRY_HARDER = static_cast<int>(BarcodeFormat::FORMAT_COUNT) + 1,
		TRY_ROTATE,
		WITH_CODE_39_EXTENDED,
		ASSUME_CODE_39_CHECK_DIGIT,
		ASSUME_GS1,
		RETURN_CODABAR_START_END,
		FLAG_COUNT
	};

	static_assert(FLAG_COUNT < 8 * sizeof(_flags), "HintFlag overflow");

	bool getFlag(int f) const {
		return (_flags & (1 << f)) != 0;
	}

	void setFlag(int f, bool v)
	{
		if (v)
			_flags |= (1 << f);
		else
			_flags &= ~(1 << f);
	}
};

} // ZXing
