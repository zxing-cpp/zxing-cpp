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
		return Result(ErrorStatus::NotFound);
	}

	DetectorResult detectResult;
	ErrorStatus status = Detector::Detect(*binImg, false, detectResult);
	DecoderResult decodeResult;
	std::vector<ResultPoint> points;
	if (StatusIsOK(status)) {
		points = detectResult.points();
		status = Decoder::Decode(detectResult, decodeResult);
	}
	if (StatusIsError(status)) {
		auto status2 = Detector::Detect(*binImg, true, detectResult);
		if (StatusIsOK(status2)) {
			points = detectResult.points();
			status2 = Decoder::Decode(detectResult, decodeResult);
			if (StatusIsError(status2)) {
				return Result(status);
			}
		}
		else {
			return Result(status);
		}
	}

	//auto rpcb = hints.resultPointCallback();
	//if (rpcb != nullptr) {
	//	for (auto& p : points) {
	//		rpcb(p.x(), p.y());
	//	}
	//}

	Result result(decodeResult.text(), decodeResult.rawBytes(), points, BarcodeFormat::AZTEC);
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
