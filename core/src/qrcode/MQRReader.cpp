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

#include "MQRReader.h"

#include "BinaryBitmap.h"
#include "DecodeHints.h"
#include "DecoderResult.h"
#include "DetectorResult.h"
#include "LogMatrix.h"
#include "MQRDetector.h"
#include "QRDecoder.h"
#include "Result.h"

#include <utility>

namespace ZXing::MicroQRCode {

Reader::Reader(const DecodeHints& hints)
	: _tryHarder(hints.tryHarder()), _isPure(hints.isPure()), _charset(hints.characterSet())
{}

Result Reader::decode(const BinaryBitmap& image) const
{
	auto binImg = image.getBitMatrix();
	if (binImg == nullptr) {
		return Result(DecodeStatus::NotFound);
	}

	auto detectorResult = Detect(*binImg, _tryHarder, _isPure);
	if (!detectorResult.isValid())
		return Result(DecodeStatus::NotFound);

	auto decoderResult = QRCode::Decode(detectorResult.bits(), _charset, true);
	auto position = detectorResult.position();

	return Result(std::move(decoderResult), std::move(position), BarcodeFormat::MicroQRCode);
}

} // namespace ZXing::MicroQRCode
