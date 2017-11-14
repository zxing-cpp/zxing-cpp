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
#include <string>
#include <memory>

namespace Gdiplus {
	class Bitmap;
}

namespace ZXing {

class MultiFormatWriter;

class BarcodeGenerator
{
public:
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
	 "PDF_417",
	 "QR_CODE",
	 "UPC_A",
	 "UPC_E",
	*/
	explicit BarcodeGenerator(const std::string& format);

	/**
	* Used for AZTEC, PDF417, and QR_CODE only.
	*/
	void setEncoding(const std::string& encoding);

	/**
	* Used for all except AZTEC, DATA_MATRIX.
	*/
	void setMargin(int margin);


	std::shared_ptr<Gdiplus::Bitmap> generate(const std::wstring& contents, int width, int height) const;

private:
	std::shared_ptr<MultiFormatWriter> _writer;
};

} // ZXing
