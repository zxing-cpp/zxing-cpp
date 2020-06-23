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

#include "QRReader.h"
#include "QRDecoder.h"
#include "QRDetector.h"
#include "QRDecoderMetadata.h"
#include "Result.h"
#include "DecoderResult.h"
#include "DetectorResult.h"
#include "ResultPoint.h"
#include "DecodeHints.h"
#include "BinaryBitmap.h"
#include "BitMatrix.h"
#include "ZXNumeric.h"

#include <utility>

namespace ZXing {
namespace QRCode {

Reader::Reader(const DecodeHints& hints)
	: _tryHarder(hints.tryHarder()), _isPure(hints.isPure()), _charset(hints.characterSet())
{
}

Result
Reader::decode(const BinaryBitmap& image) const
{
	auto binImg = image.getBlackMatrix();
	if (binImg == nullptr) {
		return Result(DecodeStatus::NotFound);
	}

	auto detectorResult = Detector::Detect(*binImg, _tryHarder, _isPure);
	if (!detectorResult.isValid())
		return Result(DecodeStatus::NotFound);

	auto decoderResult = Decoder::Decode(detectorResult.bits(), _charset);
	auto position = detectorResult.position();

	// If the code was mirrored: swap the bottom-left and the top-right position.
	// No need to 'fix' top-left and alignment pattern.
	if (decoderResult.extra() && static_cast<DecoderMetadata*>(decoderResult.extra().get())->isMirrored()) {
		std::swap(position[1], position[3]);
	}

	return Result(std::move(decoderResult), std::move(position), BarcodeFormat::QR_CODE);
}

} // QRCode
} // ZXing
