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

#include "qrcode/QRReader.h"
#include "Result.h"
#include "DecoderResult.h"
#include "ResultPoint.h"
#include "DecodeHints.h"
#include "BinaryBitmap.h"
#include "BitMatrix.h"
#include "ZXNumeric.h"

#include <ciso646>

namespace ZXing {
namespace QRCode {

namespace {

bool GetModuleSize(int x, int y, BitMatrix image, float& outSize)
{
	int orgX = x;
	int height = image.height();
	int width = image.width();
	bool inBlack = true;
	int transitions = 0;
	while (x < width && y < height) {
		if (inBlack != image.get(x, y)) {
			if (++transitions == 5) {
				break;
			}
			inBlack = !inBlack;
		}
		x++;
		y++;
	}
	if (x == width || y == height) {
		return false;
	}
	outSize = (x - orgX) / 7.0f;
	return true;
}


/**
* This method detects a code in a "pure" image -- that is, pure monochrome image
* which contains only an unrotated, unskewed, image of a code, with some white border
* around it. This is a specialized method that works exceptionally fast in this special
* case.
*
* @see com.google.zxing.datamatrix.DataMatrixReader#extractPureBits(BitMatrix)
*/
bool extractPureBits(const BitMatrix& image, BitMatrix& outBits)
{
	int left, top, right, bottom;
	if (not image.getTopLeftOnBit(left, top) || not image.getBottomRightOnBit(right, bottom))
	{
		return false;
	}

	float moduleSize;
	if (!GetModuleSize(left, top, image, moduleSize))
		return false;

	// Sanity check!
	if (left >= right || top >= bottom) {
		return false;
	}

	if (bottom - top != right - left) {
		// Special case, where bottom-right module wasn't black so we found something else in the last row
		// Assume it's a square, so use height as the width
		right = left + (bottom - top);
	}

	int matrixWidth = RoundToNearest((right - left + 1) / moduleSize);
	int matrixHeight = RoundToNearest((bottom - top + 1) / moduleSize);
	if (matrixWidth <= 0 || matrixHeight <= 0) {
		return false;
	}
	if (matrixHeight != matrixWidth) {
		// Only possibly decode square regions
		return false;
	}

	// Push in the "border" by half the module width so that we start
	// sampling in the middle of the module. Just in case the image is a
	// little off, this will help recover.
	int nudge = (int)(moduleSize / 2.0f);
	top += nudge;
	left += nudge;

	// But careful that this does not sample off the edge
	// "right" is the farthest-right valid pixel location -- right+1 is not necessarily
	// This is positive by how much the inner x loop below would be too large
	int nudgedTooFarRight = left + (int)((matrixWidth - 1) * moduleSize) - right;
	if (nudgedTooFarRight > 0) {
		if (nudgedTooFarRight > nudge) {
			// Neither way fits; abort
			return false;
		}
		left -= nudgedTooFarRight;
	}
	// See logic above
	int nudgedTooFarDown = top + (int)((matrixHeight - 1) * moduleSize) - bottom;
	if (nudgedTooFarDown > 0) {
		if (nudgedTooFarDown > nudge) {
			// Neither way fits; abort
			return false;
		}
		top -= nudgedTooFarDown;
	}

	// Now just read off the bits
	outBits = BitMatrix(matrixWidth, matrixHeight);
	for (int y = 0; y < matrixHeight; y++) {
		int iOffset = top + (int)(y * moduleSize);
		for (int x = 0; x < matrixWidth; x++) {
			if (image.get(left + (int)(x * moduleSize), iOffset)) {
				outBits.set(x, y);
			}
		}
	}
	return true;
}

} // anonymous

Result
Reader::decode(const BinaryBitmap& image, const DecodeHints* hints) const
{
	DecoderResult decoderResult;
	std::vector<ResultPoint> points;
	if (hints != nullptr && hints->contains(DecodeHint::PURE_BARCODE))
	{
		BitMatrix binImg;
		BitMatrix bits;
		if (not image.getBlackMatrix(binImg) || not extractPureBits(binImg, bits))
		{

		}
		decoderResult = decoder.decode(bits, hints);
		points = NO_POINTS;
	}
	else {
		DetectorResult detectorResult = new Detector(image.getBlackMatrix()).detect(hints);
		decoderResult = decoder.decode(detectorResult.getBits(), hints);
		points = detectorResult.getPoints();
	}

	// If the code was mirrored: swap the bottom-left and the top-right points.
	if (decoderResult.getOther() instanceof QRCodeDecoderMetaData) {
		((QRCodeDecoderMetaData)decoderResult.getOther()).applyMirroredCorrection(points);
	}

	Result result = new Result(decoderResult.getText(), decoderResult.getRawBytes(), points, BarcodeFormat.QR_CODE);
	List<byte[]> byteSegments = decoderResult.getByteSegments();
	if (byteSegments != null) {
		result.putMetadata(ResultMetadataType.BYTE_SEGMENTS, byteSegments);
	}
	String ecLevel = decoderResult.getECLevel();
	if (ecLevel != null) {
		result.putMetadata(ResultMetadataType.ERROR_CORRECTION_LEVEL, ecLevel);
	}
	if (decoderResult.hasStructuredAppend()) {
		result.putMetadata(ResultMetadataType.STRUCTURED_APPEND_SEQUENCE,
			decoderResult.getStructuredAppendSequenceNumber());
		result.putMetadata(ResultMetadataType.STRUCTURED_APPEND_PARITY,
			decoderResult.getStructuredAppendParity());
	}
	return result;
}

} // QRCode
} // ZXing
