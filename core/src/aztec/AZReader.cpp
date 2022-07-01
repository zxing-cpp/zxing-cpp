/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "AZReader.h"

#include "AZDecoder.h"
#include "AZDetector.h"
#include "AZDetectorResult.h"
#include "BinaryBitmap.h"
#include "DecodeHints.h"
#include "DecoderResult.h"
#include "Result.h"

#include <memory>
#include <utility>

namespace ZXing::Aztec {

Result
Reader::decode(const BinaryBitmap& image) const
{
	auto binImg = image.getBitMatrix();
	if (binImg == nullptr)
		return {};

	DetectorResult detectResult = Detect(*binImg, false, _hints.isPure());
	DecoderResult decodeResult;
	if (detectResult.isValid())
		decodeResult = Decode(detectResult);

	//TODO: don't start detection all over again, just to swap 2 corner points
	if (!decodeResult.isValid()) {
		detectResult = Detect(*binImg, true, _hints.isPure());
		if (detectResult.isValid())
			decodeResult = Decode(detectResult);
	}

	return Result(std::move(decodeResult), std::move(detectResult).position(), BarcodeFormat::Aztec);
}

} // namespace ZXing::Aztec
