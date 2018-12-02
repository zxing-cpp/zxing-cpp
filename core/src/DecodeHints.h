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
#include <string>

namespace ZXing {

enum class BarcodeFormat;

class DecodeHints
{
public:

	std::vector<BarcodeFormat> possibleFormats() const;
	void setPossibleFormats(const std::vector<BarcodeFormat>& formats);

	/**
	* Spend more time to try to find a barcode; optimize for accuracy, not speed.
	*/
	bool shouldTryHarder() const {
		return getFlag(TRY_HARDER);
	}

	void setShouldTryHarder(bool v) {
		setFlag(TRY_HARDER, v);
	}

	/**
	* For 1D readers only, should try to rotate image 90 CCW if not barcode found.
	*/
	bool shouldTryRotate() const {
		return getFlag(TRY_ROTATE);
	}

	void setShouldTryRotate(bool v) {
		setFlag(TRY_ROTATE, v);
	}

	/**
	* Specifies what character encoding to use when decoding, where applicable.
	*/
	std::string characterSet() const {
		return _charset;
	}

	void setCharacterSet(const std::string& charset) {
		_charset = charset;
	}

	/**
	* Allowed lengths of encoded data -- reject anything else..
	*/
	std::vector<int> allowedLengths() const {
		return _lengths;
	}

	void setAllowLengths(const std::vector<int>& lengths) {
		_lengths = lengths;
	}

	/**
	* If true, the CODE-39 reader will try to read extended mode.
	*/
	bool shouldTryCode39ExtendedMode() const {
		return getFlag(WITH_CODE_39_EXTENDED);
	}
	void setShouldTryCode39ExtendedMode(bool v) {
		setFlag(WITH_CODE_39_EXTENDED, v);
	}

	/**
	* Assume Code 39 codes employ a check digit.
	*/
	bool shouldAssumeCode39CheckDigit() const {
		return getFlag(ASSUME_CODE_39_CHECK_DIGIT);
	}
	void setShouldAssumeCode39CheckDigit(bool v) {
		setFlag(ASSUME_CODE_39_CHECK_DIGIT, v);
	}

	/**
	* Assume the barcode is being processed as a GS1 barcode, and modify behavior as needed.
	* For example this affects FNC1 handling for Code 128 (aka GS1-128).
	*/
	bool shouldAssumeGS1() const {
		return getFlag(ASSUME_GS1);
	}
	void setShouldAssumeGS1(bool v) {
		setFlag(ASSUME_GS1, v);
	}

	/**
	* If true, return the start and end digits in a Codabar barcode instead of stripping them. They
	* are alpha, whereas the rest are numeric. By default, they are stripped, but this causes them
	* to not be.
	*/
	bool shouldReturnCodabarStartEnd() const {
		return getFlag(RETURN_CODABAR_START_END);
	}
	void setShouldReturnCodabarStartEnd(bool v) {
		setFlag(RETURN_CODABAR_START_END, v);
	}

	/**
	* Allowed extension lengths for EAN or UPC barcodes. Other formats will ignore this.
	* Maps to an {@code int[]} of the allowed extension lengths, for example [2], [5], or [2, 5].
	* If it is optional to have an extension, do not set this hint. If this is set,
	* and a UPC or EAN barcode is found but an extension is not, then no result will be returned
	* at all.
	*/
	std::vector<int> allowedEanExtensions() const {
		return _eanExts;
	}

	void setAllowedEanExtensions(const std::vector<int>& extensions) {
		_eanExts = extensions;
	}

private:
	uint32_t _flags = 0;
	std::string _charset;
	std::vector<int> _lengths;
	std::vector<int> _eanExts;

	enum HintFlag
	{
		TRY_HARDER = 24,
		TRY_ROTATE,
		WITH_CODE_39_EXTENDED,
		ASSUME_CODE_39_CHECK_DIGIT,
		ASSUME_GS1,
		RETURN_CODABAR_START_END,
	};

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
