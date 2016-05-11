/*
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

#include "datamatrix/DMReader.h"
#include "datamatrix/DMDecoder.h"
#include "datamatrix/DMDetector.h"
#include "Result.h"
#include "DecodeHints.h"
#include "BitMatrix.h"
#include "BinaryBitmap.h"
#include "DecoderResult.h"
#include "DetectorResult.h"

namespace ZXing {
namespace DataMatrix {

static int
GetModuleSize(int x, int y, const BitMatrix& image)
{
	int oldX = x;
	int width = image.width();
	while (x < width && image.get(x, y)) {
		x++;
	}
	if (x == width) {
		return 0;
	}
	return x - oldX;
}

/**
* This method detects a code in a "pure" image -- that is, pure monochrome image
* which contains only an unrotated, unskewed, image of a code, with some white border
* around it. This is a specialized method that works exceptionally fast in this special
* case.
*
* @see com.google.zxing.qrcode.QRCodeReader#extractPureBits(BitMatrix)
*/
static ErrorStatus
ExtractPureBits(const BitMatrix& image, BitMatrix& outBits)
{
	int left, top, right, bottom;
	if (!image.getTopLeftOnBit(left, top) || !image.getBottomRightOnBit(right, bottom)) {
		return ErrorStatus::NotFound;
	}

	int moduleSize = GetModuleSize(left, top, image);
	int matrixWidth = (right - left + 1) / moduleSize;
	int matrixHeight = (bottom - top + 1) / moduleSize;
	if (matrixWidth <= 0 || matrixHeight <= 0) {
		return ErrorStatus::NotFound;
	}

	// Push in the "border" by half the module width so that we start
	// sampling in the middle of the module. Just in case the image is a
	// little off, this will help recover.
	int nudge = moduleSize / 2;
	top += nudge;
	left += nudge;

	// Now just read off the bits
	outBits.init(matrixWidth, matrixHeight);
	for (int y = 0; y < matrixHeight; y++) {
		int iOffset = top + y * moduleSize;
		for (int x = 0; x < matrixWidth; x++) {
			if (image.get(left + x * moduleSize, iOffset)) {
				outBits.set(x, y);
			}
		}
	}
	return ErrorStatus::NoError;
}

/**
* Locates and decodes a Data Matrix code in an image.
*
* @return a string representing the content encoded by the Data Matrix code
* @throws NotFoundException if a Data Matrix code cannot be found
* @throws FormatException if a Data Matrix code cannot be decoded
* @throws ChecksumException if error correction fails
*/
Result
Reader::decode(const BinaryBitmap& image) const
{
	auto binImg = image.getBlackMatrix();
	if (binImg == nullptr) {
		return Result(ErrorStatus::NotFound);
	}

	DecoderResult decoderResult;
	std::vector<ResultPoint> points;
	ErrorStatus status;
	if (image.isPureBarcode()) {
		BitMatrix bits;
		status = ExtractPureBits(*binImg, bits);
		if (StatusIsOK(status)) {
			status = Decoder::Decode(bits, decoderResult);
		}
	}
	else {
		DetectorResult detectorResult;
		status = Detector::Detect(*binImg, detectorResult);
		if (StatusIsOK(status)) {
			status = Decoder::Decode(*detectorResult.bits(), decoderResult);
			points = detectorResult.points();
		}
	}

	if (StatusIsError(status)) {
		return Result(status);
	}

	Result result(decoderResult.text(), decoderResult.rawBytes(), points, BarcodeFormat::DATA_MATRIX);
	auto& byteSegments = decoderResult.byteSegments();
	if (!byteSegments.empty()) {
		result.metadata().put(ResultMetadata::BYTE_SEGMENTS, byteSegments);
	}
	auto ecLevel = decoderResult.ecLevel();
	if (!ecLevel.empty()) {
		result.metadata().put(ResultMetadata::ERROR_CORRECTION_LEVEL, ecLevel);
	}
	return result;
}

} // DataMatrix
} // ZXing
