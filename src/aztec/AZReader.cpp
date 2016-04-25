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
Reader::decode(const BinaryBitmap& image, const DecodeHints* hints)
{
	BitMatrix binImg;
	ErrorStatus status = image.getBlackMatrix(binImg);
	if (StatusIsError(status)) {
		return Result(status);
	}

	DetectorResult detectResult = Detector::Detect(binImg, false);
	status = detectResult.status();
	DecoderResult decodeResult(status);
	std::vector<ResultPoint> points;
	if (StatusIsOK(status)) {
		points = detectResult.points();
		decodeResult = Decoder::Decode(detectResult);
		status = decodeResult.status();
	}
	if (StatusIsError(status)) {
		detectResult = Detector::Detect(binImg, true);
		if (detectResult.isValid()) {
			points = detectResult.points();
			decodeResult = Decoder::Decode(detectResult);
			if (!decodeResult.isValid()) {
				return Result(status);
			}
		}
		else {
			return Result(status);
		}
	}

	if (hints != nullptr) {
		auto rpcb = hints->getPointCallback(DecodeHint::NEED_RESULT_POINT_CALLBACK);
		if (rpcb != nullptr) {
			for (auto& p : points) {
				rpcb(p.x(), p.y());
			}
		}
	}

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
