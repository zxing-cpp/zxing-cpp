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

#include <string>

namespace ZXing {

class EncodeHints
{
public:

	/**
	* Specifies what degree of error correction to use, for example in QR Codes.
	* The value depends on the encoder. For example for QR codes it's one of values of
	* {@link ZXing::QRCode::ErrorCorrectionLevel}.
	* For Aztec it represents the minimal percentage of error correction words.
	* For PDF417, valid values being 0 to 8.
	* In all cases, it can also be a representation of the desired value as well.
	* Note: an Aztec symbol should have a minimum of 25% EC words.
	*/
	int errorCorrection() const {
		return _ecLevel;
	}
	void setErrorCorrection(int level) {
		_ecLevel = level;
	}


	/**
	* Specifies what character encoding to use when encoding, where applicable.
	*/
	std::string characterSet() const {
		return _charset;
	}
	void setCharacterSet(const std::string& charset) {
		_charset = charset;
	}

	/**
	* Specifies margin, in pixels, to use when generating the barcode. The meaning can vary
	* by format; for example it controls margin before and after the barcode horizontally for
	* most 1D formats.
	*/
	int margin() const {
		return _margin;
	}
	void setMargin(int value) {
		_margin = value;
	}

	/**
	* Specifies whether to use compact mode for PDF417.
	*/
	bool pdf417Compact() const {
		return _pdf417Compact;
	}
	void setPDF417Compact(bool v) {
		_pdf417Compact = v;
	}

	/**
	* Specifies what compaction mode to use for PDF417 (should be one of
	* {@link ZXing::PDF417::CompactionMode}.).
	*/
	int pdf417Compaction() const {
		return _pdf417Compaction;
	}
	void setPDF417Compaction(int v) {
		_pdf417Compaction = v;
	}

	/**
	* Specifies the minimum and maximum number of rows and columns for PDF417.
	*/
	const int* pdf417Dimensions() const {
		return _pdf417Dimens;
	}
	void setPDF417Dimensions(int minCols, int maxCols, int minRows, int maxRows) {
		_pdf417Dimens[0] = minCols;
		_pdf417Dimens[1] = maxCols;
		_pdf417Dimens[2] = minRows;
		_pdf417Dimens[3] = maxRows;
	}

	/**
	* Specifies the required number of layers for an Aztec code.
	* A negative number (-1, -2, -3, -4) specifies a compact Aztec code.
	* 0 indicates to use the minimum number of layers (the default).
	* A positive number (1, 2, .. 32) specifies a normal (non-compact) Aztec code.
	* (Type {@link Integer}, or {@link String} representation of the integer value).
	*/
	int aztecLayers() const {
		return _aztecLayers;
	}
	void setAztecLayers(int nbLayers) {
		_aztecLayers = nbLayers;
	}

private:
	int _ecLevel = -1;
	std::string _charset;
	int _margin = -1;
	int _aztecLayers = 0;
	bool _pdf417Compact = false;
	int _pdf417Compaction = 0;
	int _pdf417Dimens[4] = {};
};

} // ZXing
