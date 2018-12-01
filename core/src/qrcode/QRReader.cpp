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

#include "qrcode/QRReader.h"
#include "qrcode/QRDecoder.h"
#include "qrcode/QRDetector.h"
#include "qrcode/QRDecoderMetadata.h"
#include "Result.h"
#include "DecoderResult.h"
#include "DetectorResult.h"
#include "ResultPoint.h"
#include "DecodeHints.h"
#include "BinaryBitmap.h"
#include "BitMatrix.h"
#include "ZXNumeric.h"
#include "ZXConfig.h"

namespace ZXing {
namespace QRCode {

static float
GetModuleSize(int x, int y, const BitMatrix& image)
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
		return 0.0f;
	}
	return static_cast<float>(x - orgX) / 7.0f;
}

/**
* This method detects a code in a "pure" image -- that is, pure monochrome image
* which contains only an unrotated, unskewed, image of a code, with some white border
* around it. This is a specialized method that works exceptionally fast in this special
* case.
*
* @see com.google.zxing.datamatrix.DataMatrixReader#extractPureBits(BitMatrix)
*/
static BitMatrix
ExtractPureBits(const BitMatrix& image)
{
	int left, top, right, bottom;
	if (!image.getTopLeftOnBit(left, top) || !image.getBottomRightOnBit(right, bottom)) {
		return {};
	}

	float moduleSize = GetModuleSize(left, top, image);
	if (moduleSize <= 0.0f) {
		return {};
	}

	// Sanity check!
	if (left >= right || top >= bottom) {
		return {};
	}

	if (bottom - top != right - left) {
		// Special case, where bottom-right module wasn't black so we found something else in the last row
		// Assume it's a square, so use height as the width
		right = left + (bottom - top);
		if (right >= image.width()) {
			// Abort if that would not make sense -- off image
			return {};
		}
	}

	int matrixWidth = RoundToNearest((right - left + 1) / moduleSize);
	int matrixHeight = RoundToNearest((bottom - top + 1) / moduleSize);
	if (matrixWidth <= 0 || matrixHeight <= 0) {
		return {};
	}
	if (matrixHeight != matrixWidth) {
		// Only possibly decode square regions
		return {};
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
			return {};
		}
		left -= nudgedTooFarRight;
	}
	// See logic above
	int nudgedTooFarDown = top + (int)((matrixHeight - 1) * moduleSize) - bottom;
	if (nudgedTooFarDown > 0) {
		if (nudgedTooFarDown > nudge) {
			// Neither way fits; abort
			return {};
		}
		top -= nudgedTooFarDown;
	}

	// Now just read off the bits (this is a crop + subsample)
	return Deflate(image, matrixWidth, matrixHeight, top, left, static_cast<int>(moduleSize));
}

Reader::Reader(const DecodeHints& hints) :
	_tryHarder(hints.shouldTryHarder()),
	_charset(hints.characterSet())
{
}

Result
Reader::decode(const BinaryBitmap& image) const
{
	auto binImg = image.getBlackMatrix();
	if (binImg == nullptr) {
		return Result(DecodeStatus::NotFound);
	}

	DecoderResult decoderResult;
	std::vector<ResultPoint> points;
	if (image.isPureBarcode()) {
		BitMatrix bits = ExtractPureBits(*binImg);
		if (bits.empty())
			return Result(DecodeStatus::NotFound);

		decoderResult = Decoder::Decode(bits, _charset);
	}
	else {
		DetectorResult detectorResult = Detector::Detect(*binImg, _tryHarder);
		if (!detectorResult.isValid())
			return Result(DecodeStatus::NotFound);

		decoderResult = Decoder::Decode(detectorResult.bits(), _charset);
		points = detectorResult.points();
	}

	if (!decoderResult.isValid()) {
		return Result(decoderResult.errorCode());
	}

	// If the code was mirrored: swap the bottom-left and the top-right points.
#if !defined(ZX_HAVE_CONFIG)
	#error "You need to include ZXConfig.h"
#elif !defined(ZX_NO_RTTI)
	if (auto extra = std::dynamic_pointer_cast<DecoderMetadata>(decoderResult.extra())) {
		extra->applyMirroredCorrection(points.begin(), points.end());
	}
#else
	if (auto extra = decoderResult.extra()) {
		static_cast<DecoderMetadata*>(extra.get())->applyMirroredCorrection(points.begin(), points.end());
	}
#endif

	return Result(std::move(decoderResult), std::move(points), BarcodeFormat::QR_CODE);
}

} // QRCode
} // ZXing
