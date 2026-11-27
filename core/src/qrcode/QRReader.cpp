/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
* Copyright 2022 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "QRReader.h"

#include "BarcodeData.h"
#include "BinaryBitmap.h"
#include "ConcentricFinder.h"
#include "DecoderResult.h"
#include "DetectorResult.h"
#include "Log.h"
#include "QRDecoder.h"
#include "QRDetector.h"
#include "ReaderOptions.h"

#include <utility>

namespace ZXing::QRCode {

static BarcodeData readPure(const BitMatrix* binImg, const ReaderOptions& _opts)
{
	DetectorResult detectorResult;
	if (_opts.hasFormat(BarcodeFormat::QRCodeModel1 | BarcodeFormat::QRCodeModel2))
		detectorResult = DetectPureQR(*binImg);
	if (_opts.hasFormat(BarcodeFormat::MicroQRCode) && !detectorResult.isValid())
		detectorResult = DetectPureMQR(*binImg);
	if (_opts.hasFormat(BarcodeFormat::RMQRCode) && !detectorResult.isValid())
		detectorResult = DetectPureRMQR(*binImg);

	if (!detectorResult.isValid())
		return {};

	auto decoderResult = Decode(detectorResult.bits());
	auto format = detectorResult.bits().width() != detectorResult.bits().height() ? BarcodeFormat::RMQRCode
				  : detectorResult.bits().width() < 21                            ? BarcodeFormat::MicroQRCode
																				  : BarcodeFormat::QRCode;

	return MatrixBarcode(std::move(decoderResult), std::move(detectorResult), format);
}

BarcodesData Reader::read(const BinaryBitmap& image, int maxSymbols) const
{
	auto binImg = image.getBitMatrix();
	if (binImg == nullptr)
		return {};

#ifdef PRINT_DEBUG
	LogMatrixWriter lmw(log, *binImg, 5, "qr-log.pnm");
#endif

	if (_opts.isPure())
		return ToVector(readPure(binImg, _opts));

	auto allFPs = FindFinderPatterns(*binImg, _opts.tryHarder());

	std::vector<ConcentricPattern> usedFPs;
	BarcodesData res;

	auto decodeResult = [&](DetectorResults&& detectorResults, BarcodeFormat format, auto&& accept) {
		bool foundValid = false;
		for (auto&& detectorResult : detectorResults) {
			auto decoderResult = Decode(detectorResult.bits());
			if (!accept(decoderResult))
				continue;
			if (decoderResult.isValid())
				foundValid = true;
			if (decoderResult.isValid(_opts.returnErrors())) {
				res.emplace_back(MatrixBarcode(std::move(decoderResult), std::move(detectorResult), format));
				if (res.back().isValid() || (maxSymbols && Size(res) == maxSymbols))
					break;
			}
		}
		return foundValid;
	};

	if (_opts.hasFormat(BarcodeFormat::QRCodeModel1 | BarcodeFormat::QRCodeModel2)) {
		auto allFPSets = GenerateFinderPatternSets(allFPs);
		for (const auto& fpSet : allFPSets) {
			if (Contains(usedFPs, fpSet.bl) || Contains(usedFPs, fpSet.tl) || Contains(usedFPs, fpSet.tr))
				continue;

			log(fpSet.bl, fpSet.tl);
			log(fpSet.tl, fpSet.tr);
			log(fpSet.tr, fpSet.bl);

			if (decodeResult(SampleQR(*binImg, fpSet), BarcodeFormat::QRCode, [&](const auto& decoderResult) {
					return (decoderResult.content().symbology.modifier != '0' || _opts.hasFormat(BarcodeFormat::QRCodeModel1))
						   && (decoderResult.content().symbology.modifier != '1' || _opts.hasFormat(BarcodeFormat::QRCodeModel2));
				})) {
				usedFPs.push_back(fpSet.bl);
				usedFPs.push_back(fpSet.tl);
				usedFPs.push_back(fpSet.tr);
			}

			if (maxSymbols && Size(res) == maxSymbols)
				break;
		}
	}

	if (_opts.hasFormat(BarcodeFormat::MicroQRCode) && !(maxSymbols && Size(res) == maxSymbols)) {
		for (const auto& fp : allFPs) {
			if (Contains(usedFPs, fp))
				continue;

			if (decodeResult(SampleMQR(*binImg, fp), BarcodeFormat::MicroQRCode, [](const auto&) { return true; }))
				usedFPs.push_back(fp);

			if (maxSymbols && Size(res) == maxSymbols)
				break;
		}
	}

	if (_opts.hasFormat(BarcodeFormat::RMQRCode) && !(maxSymbols && Size(res) == maxSymbols)) {
		for (const auto& fp : allFPs) {
			if (Contains(usedFPs, fp))
				continue;

			if (decodeResult(SampleRMQR(*binImg, fp), BarcodeFormat::RMQRCode, [](const auto&) { return true; }))
				usedFPs.push_back(fp);

			if (maxSymbols && Size(res) == maxSymbols)
				break;
		}
	}

	return res;
}

} // namespace ZXing::QRCode
