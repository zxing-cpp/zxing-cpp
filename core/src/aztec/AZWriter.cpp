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

#include "aztec/AZWriter.h"
#include "aztec/AZEncoder.h"
#include "CharacterSet.h"
#include "TextEncoder.h"

#include <algorithm>

namespace ZXing {
namespace Aztec {

static BitMatrix RenderResult(const EncodeResult& code, int width, int height)
{
	const BitMatrix &input = code.matrix;
	int inputWidth = input.width();
	int inputHeight = input.height();
	int outputWidth = std::max(width, inputWidth);
	int outputHeight = std::max(height, inputHeight);

	int multiple = std::min(outputWidth / inputWidth, outputHeight / inputHeight);
	int leftPadding = (outputWidth - (inputWidth * multiple)) / 2;
	int topPadding = (outputHeight - (inputHeight * multiple)) / 2;

	BitMatrix result(outputWidth, outputHeight);

	for (int inputY = 0, outputY = topPadding; inputY < inputHeight; inputY++, outputY += multiple) {
		// Write the contents of this row of the barcode
		for (int inputX = 0, outputX = leftPadding; inputX < inputWidth; inputX++, outputX += multiple) {
			if (input.get(inputX, inputY)) {
				result.setRegion(outputX, outputY, multiple, multiple);
			}
		}
	}
	return result;
}

Writer::Writer() :
	_encoding(CharacterSet::ISO8859_1),
	_eccPercent(Encoder::DEFAULT_EC_PERCENT),
	_layers(Encoder::DEFAULT_AZTEC_LAYERS)
{
}

BitMatrix
Writer::encode(const std::wstring& contents, int width, int height) const
{
	std::string bytes;
	TextEncoder::GetBytes(contents, _encoding, bytes);
	EncodeResult aztec = Encoder::Encode(bytes, _eccPercent, _layers);
	return RenderResult(aztec, width, height);
}

} // Aztec
} // ZXing
