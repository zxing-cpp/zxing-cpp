#pragma once
/*
* Copyright 2017 Huy Cuong Nguyen
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
#include "CharacterSet.h"

namespace ZXing {

class BitMatrix;

/**
* This class is here just for convenience as it offers single-point service
* to generate barcodes for all supported formats. As a result, this class
* offer very limited customization compared to what are available in each
* individual encoder.
*/
class MultiFormatWriter
{
public:
	explicit MultiFormatWriter(BarcodeFormat format) : _format(format) {}

	/**
	* Used for Aztec, PDF417, and QRCode only.
	*/
	MultiFormatWriter& setEncoding(CharacterSet encoding) {
		_encoding = encoding;
		return *this;
	}

	/**
	* Used for Aztec, PDF417, and QRCode only, [0-8].
	*/
	MultiFormatWriter& setEccLevel(int level) {
		_eccLevel = level;
		return *this;
	}

	/**
	* Used for all 1D formats, PDF417, and QRCode only.
	*/
	MultiFormatWriter& setMargin(int margin) {
		_margin = margin;
		return *this;
	}

	BitMatrix encode(const std::wstring& contents, int width, int height) const;

private:
	BarcodeFormat _format;
	CharacterSet _encoding = CharacterSet::Unknown;
	int _margin = -1;
	int _eccLevel = -1;
};

} // ZXing
