/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "DMReader.h"

#include "BinaryBitmap.h"
#include "DMDecoder.h"
#include "DMDetector.h"
#include "ReaderOptions.h"
#include "DecoderResult.h"
#include "DetectorResult.h"
#include "BarcodeData.h"

#include <utility>

namespace ZXing::DataMatrix {

BarcodesData Reader::read(const BinaryBitmap& image, int maxSymbols) const
{
#ifdef __cpp_impl_coroutine
	auto binImg = image.getBitMatrix();
	if (binImg == nullptr)
		return {};

	BarcodesData res;
	for (auto&& detRes : Detect(*binImg, _opts.tryHarder(), _opts.tryRotate(), _opts.isPure())) {
		auto decRes = Decode(detRes.bits());
		if (decRes.isValid(_opts.returnErrors())) {
			res.emplace_back(MatrixBarcode(std::move(decRes), std::move(detRes), BarcodeFormat::DataMatrix));
			if (maxSymbols > 0 && Size(res) >= maxSymbols)
				break;
		}
	}

	return res;
#else
	auto binImg = image.getBitMatrix();
	if (binImg == nullptr)
		return {};

	auto detectorResult = Detect(*binImg, _opts.tryHarder(), _opts.tryRotate(), _opts.isPure());
	if (!detectorResult.isValid())
		return {};

	return ToVector(MatrixBarcode(Decode(detectorResult.bits()), std::move(detectorResult), BarcodeFormat::DataMatrix));
#endif
}

} // namespace ZXing::DataMatrix
