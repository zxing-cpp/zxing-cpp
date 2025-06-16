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
#include "ReaderOptions.h"
#include "DecoderResult.h"
#include "Barcode.h"

#include <utility>

namespace ZXing::Aztec {

Barcode Reader::decode(const BinaryBitmap& image) const
{
	return FirstOrDefault(decode(image, 1));
}

Barcodes Reader::decode(const BinaryBitmap& image, int maxSymbols) const
{
	auto binImg = image.getBitMatrix();
	if (binImg == nullptr)
		return {};
	
	auto detRess = Detect(*binImg, _opts.isPure(), _opts.tryHarder(), maxSymbols);

	Barcodes res;
	for (auto&& detRes : detRess) {
		auto decRes =
			Decode(detRes).setReaderInit(detRes.readerInit()).setIsMirrored(detRes.isMirrored()).setVersionNumber(detRes.nbLayers());
		if (decRes.isValid(_opts.returnErrors())) {
			res.emplace_back(std::move(decRes), std::move(detRes), BarcodeFormat::Aztec);
			if (maxSymbols > 0 && Size(res) >= maxSymbols)
				break;
		}
	}

	return res;
}

} // namespace ZXing::Aztec
