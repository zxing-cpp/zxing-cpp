#pragma once
/*
* Copyright 2016 Nu-book Inc.
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
#include <memory>

namespace Gdiplus {
	class Bitmap;
}

namespace ZXing {

class MultiFormatReader;

class BarcodeReader
{
public:
	struct ScanResult {
		std::string format;
		std::string text;	// in UTF-8
	};

	enum Rotation {
		Rotation0 = 1,
		RotationCW90 = 2,
		Rotation180 = 4,
		RotationCCW90 = 8,
	};

	/**
	 Supported formats are:
	 "AZTEC",
	 "CODABAR",
	 "CODE_39",
	 "CODE_93",
	 "CODE_128",
	 "DATA_MATRIX",
	 "EAN_8",
	 "EAN_13",
	 "ITF",
	 "MAXICODE",
	 "PDF_417",
	 "QR_CODE",
	 "RSS_14",
	 "RSS_EXPANDED",
	 "UPC_A",
	 "UPC_E",
	 "UPC_EAN_EXTENSION",
	*/
	explicit BarcodeReader(bool tryHarder = false, bool tryRotate = true, const std::string& format = std::string());

	ScanResult scan(Gdiplus::Bitmap& bitmap, int rotations = Rotation0);

private:
	std::shared_ptr<MultiFormatReader> _reader;
};

} // ZXing
