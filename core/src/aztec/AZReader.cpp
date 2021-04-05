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
#include <vector>

namespace ZXing::Aztec {

Reader::Reader(const DecodeHints& hints)
	: _isPure(hints.isPure()), _characterSet(hints.characterSet())
{
}

Result
Reader::decode(const BinaryBitmap& image) const
{
	auto binImg = image.getBlackMatrix();
	if (binImg == nullptr) {
		return Result(DecodeStatus::NotFound);
	}

	DetectorResult detectResult = Detector::Detect(*binImg, false, _isPure);
	DecoderResult decodeResult = DecodeStatus::NotFound;
	if (detectResult.isValid()) {
		decodeResult = Decoder::Decode(detectResult, _characterSet);
	}

	//TODO: don't start detection all over again, just to swap 2 corner points
	if (!decodeResult.isValid()) {
		detectResult = Detector::Detect(*binImg, true, _isPure);
		if (detectResult.isValid()) {
			decodeResult = Decoder::Decode(detectResult, _characterSet);
		}
	}

	return Result(std::move(decodeResult), std::move(detectResult).position(), BarcodeFormat::Aztec);
}

} // namespace ZXing::Aztec
