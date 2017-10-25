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

#include "aztec/AZReader.h"
#include "aztec/AZDetector.h"
#include "aztec/AZDetectorResult.h"
#include "aztec/AZDecoder.h"
#include "Result.h"
#include "BitMatrix.h"
#include "BinaryBitmap.h"
#include "DecoderResult.h"
#include "DecodeHints.h"

namespace ZXing {
namespace Aztec {

Result
Reader::decode(const BinaryBitmap& image) const
{
	auto binImg = image.getBlackMatrix();
	if (binImg == nullptr) {
		return Result(DecodeStatus::NotFound);
	}

	DetectorResult detectResult = Detector::Detect(*binImg, false);
	DecoderResult decodeResult = DecodeStatus::NotFound;
	std::vector<ResultPoint> points;
	if (detectResult.isValid()) {
		points = detectResult.points();
		decodeResult = Decoder::Decode(detectResult);
	}
	if (!decodeResult.isValid()) {
		detectResult = Detector::Detect(*binImg, true);
		if (detectResult.isValid()) {
			points = detectResult.points();
			decodeResult = Decoder::Decode(detectResult);
		}
	}

	if (!decodeResult.isValid()) {
		return Result(decodeResult.errorCode());
	}

	//auto rpcb = hints.resultPointCallback();
	//if (rpcb != nullptr) {
	//	for (auto& p : points) {
	//		rpcb(p.x(), p.y());
	//	}
	//}

	Result result(decodeResult.text(), decodeResult.rawBytes(), decodeResult.numBits(), points, BarcodeFormat::AZTEC);
	auto& byteSegments = decodeResult.byteSegments();
	if (!byteSegments.empty()) {
		result.metadata().put(ResultMetadata::BYTE_SEGMENTS, byteSegments);
	}
	auto ecLevel = decodeResult.ecLevel();
	if (!ecLevel.empty()) {
		result.metadata().put(ResultMetadata::ERROR_CORRECTION_LEVEL, ecLevel);
	}
	return result;
}

} // Aztec
} // ZXing
