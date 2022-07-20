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

Result Reader::decode(const BinaryBitmap& image) const
{
#ifdef __cpp_impl_coroutine
	return FirstOrDefault(decode(image, 1));
#else
	auto binImg = image.getBitMatrix();
	if (binImg == nullptr)
		return {};

	auto detectorResult = Detect(*binImg, _hints.tryHarder(), _hints.tryRotate(), _hints.isPure());
	if (!detectorResult.isValid())
		return {};

	return Result(Decode(detectorResult.bits()), std::move(detectorResult).position(), BarcodeFormat::DataMatrix);
#endif
}

#ifdef __cpp_impl_coroutine
Results Reader::decode(const BinaryBitmap& image, int maxSymbols) const
{
	auto binImg = image.getBitMatrix();
	if (binImg == nullptr)
		return {};

	Results results;
	for (auto&& detRes : Detect(*binImg, _hints.tryHarder(), _hints.tryRotate(), _hints.isPure())) {
		auto decRes = Decode(detRes.bits());
		if (decRes.isValid(_hints.returnErrors())) {
			results.emplace_back(std::move(decRes), std::move(detRes).position(), BarcodeFormat::DataMatrix);
			if (maxSymbols > 0 && Size(results) >= maxSymbols)
				break;
		}
	}

	return results;
}
#endif
} // namespace ZXing::DataMatrix
