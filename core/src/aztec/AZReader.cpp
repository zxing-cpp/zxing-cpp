/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
* Copyright 2022 Axel Waggershauser
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

	DetectorResult detectorResult = Detect(*binImg, _hints.isPure(), _hints.tryHarder());
	if (!detectorResult.isValid())
		return {};

	auto decodeResult = Decode(detectorResult)
							.setReaderInit(detectorResult.readerInit())
							.setIsMirrored(detectorResult.isMirrored())
							.setVersionNumber(detectorResult.nbLayers());

	return Result(std::move(decodeResult), std::move(detectorResult).position(), BarcodeFormat::Aztec);
}

Results Reader::decode(const BinaryBitmap& image, int maxSymbols) const
{
	auto binImg = image.getBitMatrix();
	if (binImg == nullptr)
		return {};

	auto detRess = Detect(*binImg, _hints.isPure(), _hints.tryHarder(), maxSymbols);

	Results results;
	for (auto&& detRes : detRess) {
		auto decRes =
			Decode(detRes).setReaderInit(detRes.readerInit()).setIsMirrored(detRes.isMirrored()).setVersionNumber(detRes.nbLayers());
		if (decRes.isValid(_hints.returnErrors())) {
			results.emplace_back(std::move(decRes), std::move(detRes).position(), BarcodeFormat::Aztec);
			if (maxSymbols > 0 && Size(results) >= maxSymbols)
				break;
		}
	}

	return results;
}

} // namespace ZXing::Aztec
