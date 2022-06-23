/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "DMReader.h"

#include "BinaryBitmap.h"
#include "DMDecoder.h"
#include "DMDetector.h"
#include "DecodeHints.h"
#include "DecoderResult.h"
#include "DetectorResult.h"
#include "Result.h"

#include <utility>

namespace ZXing::DataMatrix {

Reader::Reader(const DecodeHints& hints)
	: _tryRotate(hints.tryRotate()),
	  _tryHarder(hints.tryHarder()),
	  _isPure(hints.isPure())
{}

Result Reader::decode(const BinaryBitmap& image) const
{
	auto binImg = image.getBitMatrix();
	if (binImg == nullptr)
		return Result(DecodeStatus::NotFound);

	auto detectorResult = Detect(*binImg, _tryHarder, _tryRotate, _isPure);
	if (!detectorResult.isValid())
		return Result(DecodeStatus::NotFound);

	return Result(Decode(detectorResult.bits()), std::move(detectorResult).position(), BarcodeFormat::DataMatrix);
}

} // namespace ZXing::DataMatrix
