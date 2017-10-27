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

#include "maxicode/MCReader.h"
#include "maxicode/MCDecoder.h"
#include "maxicode/MCBitMatrixParser.h"
#include "DecodeHints.h"
#include "Result.h"
#include "DecoderResult.h"
#include "BinaryBitmap.h"
#include "BitMatrix.h"

namespace ZXing {
namespace MaxiCode {


/**
* This method detects a code in a "pure" image -- that is, pure monochrome image
* which contains only an unrotated, unskewed, image of a code, with some white border
* around it. This is a specialized method that works exceptionally fast in this special
* case.
*
* @see com.google.zxing.datamatrix.DataMatrixReader#extractPureBits(BitMatrix)
* @see com.google.zxing.qrcode.QRCodeReader#extractPureBits(BitMatrix)
*/
static BitMatrix ExtractPureBits(const BitMatrix& image)
{
	int left, top, width, height;
	if (!image.getEnclosingRectangle(left, top, width, height)) {
		return {};
	}

	// Now just read off the bits
	BitMatrix result(BitMatrixParser::MATRIX_WIDTH, BitMatrixParser::MATRIX_HEIGHT);
	for (int y = 0; y < BitMatrixParser::MATRIX_HEIGHT; y++) {
		int iy = top + (y * height + height / 2) / BitMatrixParser::MATRIX_HEIGHT;
		for (int x = 0; x < BitMatrixParser::MATRIX_WIDTH; x++) {
			int ix = left + (x * width + width / 2 + (y & 0x01) *  width / 2) / BitMatrixParser::MATRIX_WIDTH;
			if (image.get(ix, iy)) {
				result.set(x, y);
			}
		}
	}
	return result;
}

Result
Reader::decode(const BinaryBitmap& image) const
{
	if (!image.isPureBarcode()) {
		return Result(DecodeStatus::NotFound);
	}

	auto binImg = image.getBlackMatrix();
	if (binImg == nullptr) {
		return Result(DecodeStatus::NotFound);
	}

	BitMatrix bits = ExtractPureBits(*binImg);
	if (bits.empty()) {
		return Result(DecodeStatus::NotFound);
	}

	return Result(Decoder::Decode(bits), {}, BarcodeFormat::MAXICODE);
}

} // MaxiCode
} // ZXing
