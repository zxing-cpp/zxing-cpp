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

#include "DMReader.h"
#include "DMDecoder.h"
#include "DMDetector.h"
#include "Result.h"
#include "BitMatrix.h"
#include "BinaryBitmap.h"
#include "DecodeHints.h"
#include "DecoderResult.h"
#include "DetectorResult.h"

#include <utility>
#include <vector>

namespace ZXing {
namespace DataMatrix {

Reader::Reader(const DecodeHints& hints)
	: _tryRotate(hints.tryRotate()), _tryHarder(hints.tryHarder()), _isPure(hints.isPure())
{
}

/**
* Locates and decodes a Data Matrix code in an image.
*
* @return a string representing the content encoded by the Data Matrix code
* @throws NotFoundException if a Data Matrix code cannot be found
* @throws FormatException if a Data Matrix code cannot be decoded
* @throws ChecksumException if error correction fails
*/
Result
Reader::decode(const BinaryBitmap& image) const
{
	auto binImg = image.getBlackMatrix();
	if (binImg == nullptr) {
		return Result(DecodeStatus::NotFound);
	}

	auto detectorResult = Detector::Detect(*binImg, _tryHarder, _tryRotate, _isPure);
	if (!detectorResult.isValid())
		return Result(DecodeStatus::NotFound);

	return Result(Decoder::Decode(detectorResult.bits()), std::move(detectorResult).position(),
				  BarcodeFormat::DATA_MATRIX);
}

} // DataMatrix
} // ZXing
