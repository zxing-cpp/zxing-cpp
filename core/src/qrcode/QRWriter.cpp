/*
* Copyright 2016 Huy Cuong Nguyen
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

#include "qrcode/QRWriter.h"
#include "qrcode/QRErrorCorrectionLevel.h"
#include "qrcode/QREncoder.h"
#include "qrcode/QREncodeResult.h"
#include "BitMatrix.h"

namespace ZXing {
namespace QRCode {

static const int QUIET_ZONE_SIZE = 4;

static void RenderResult(const EncodeResult& code, int width, int height, int quietZone, BitMatrix& output)
{
	const ByteMatrix& input = code.matrix;
	int inputWidth = input.width();
	int inputHeight = input.height();
	int qrWidth = inputWidth + (quietZone * 2);
	int qrHeight = inputHeight + (quietZone * 2);
	int outputWidth = std::max(width, qrWidth);
	int outputHeight = std::max(height, qrHeight);

	int multiple = std::min(outputWidth / qrWidth, outputHeight / qrHeight);
	// Padding includes both the quiet zone and the extra white pixels to accommodate the requested
	// dimensions. For example, if input is 25x25 the QR will be 33x33 including the quiet zone.
	// If the requested size is 200x160, the multiple will be 4, for a QR of 132x132. These will
	// handle all the padding from 100x100 (the actual QR) up to 200x160.
	int leftPadding = (outputWidth - (inputWidth * multiple)) / 2;
	int topPadding = (outputHeight - (inputHeight * multiple)) / 2;

	output.init(outputWidth, outputHeight);

	for (int inputY = 0, outputY = topPadding; inputY < inputHeight; inputY++, outputY += multiple) {
		// Write the contents of this row of the barcode
		for (int inputX = 0, outputX = leftPadding; inputX < inputWidth; inputX++, outputX += multiple) {
			if (input.get(inputX, inputY) == 1) {
				output.setRegion(outputX, outputY, multiple, multiple);
			}
		}
	}
}

Writer::Writer() :
	_margin(QUIET_ZONE_SIZE),
	_ecLevel(ErrorCorrectionLevel::Low),
	_encoding(Encoder::DEFAULT_BYTE_MODE_ENCODING)
{
}

Writer&
Writer::setMargin(int margin)
{
	_margin = margin;
	return *this;
}

Writer&
Writer::setErrorCorrectionLevel(ErrorCorrectionLevel ecLevel)
{
	_ecLevel = ecLevel;
	return *this;
}

Writer& Writer::setEncoding(CharacterSet encoding)
{
	_encoding = encoding;
	return *this;
}

void
Writer::encode(const std::wstring& contents, int width, int height, BitMatrix& output) const
{
	if (contents.empty()) {
		throw std::invalid_argument("Found empty contents");
	}

	if (width < 0 || height < 0) {
		throw std::invalid_argument("Requested dimensions are invalid");
	}

	EncodeResult code;
	Encoder::Encode(contents, _ecLevel, _encoding, code);
	RenderResult(code, width, height, _margin, output);
}

} // QRCode
} // ZXing
