/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
* Copyright 2022 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "QRReader.h"

#include "BinaryBitmap.h"
#include "ConcentricFinder.h"
#include "DecodeHints.h"
#include "DecoderResult.h"
#include "DetectorResult.h"
#include "LogMatrix.h"
#include "QRDecoder.h"
#include "QRDetector.h"
#include "Result.h"

#include <utility>

namespace ZXing::QRCode {

Reader::Reader(const DecodeHints& hints)
	: _tryHarder(hints.tryHarder()),
	  _isPure(hints.isPure()),
	  _testQR(hints.hasFormat(BarcodeFormat::QRCode)),
	  _testMQR(hints.hasFormat(BarcodeFormat::MicroQRCode))
{}

Result Reader::decode(const BinaryBitmap& image) const
{
#if 1
	if (!_isPure) {
		auto res = decode(image, 1);
		return res.empty() ? Result(DecodeStatus::NotFound) : res.front();
	}
#endif

	auto binImg = image.getBitMatrix();
	if (binImg == nullptr) {
		return Result(DecodeStatus::NotFound);
	}

	DetectorResult detectorResult;
	if (_testQR)
		detectorResult = DetectPureQR(*binImg);
	if (_testMQR && !detectorResult.isValid())
		detectorResult = DetectPureMQR(*binImg);

	if (!detectorResult.isValid())
		return Result(DecodeStatus::NotFound);

	auto decoderResult = Decode(detectorResult.bits());
	auto position = detectorResult.position();

	return Result(std::move(decoderResult), std::move(position),
				  detectorResult.bits().width() < 21 ? BarcodeFormat::MicroQRCode : BarcodeFormat::QRCode);
}

Results Reader::decode(const BinaryBitmap& image, int maxSymbols) const
{
	auto binImg = image.getBitMatrix();
	if (binImg == nullptr)
		return {};

#ifdef PRINT_DEBUG
	LogMatrixWriter lmw(log, *binImg, 5, "qr-log.pnm");
#endif

	auto allFPs = FindFinderPatterns(*binImg, _tryHarder);
	auto allFPSets = GenerateFinderPatternSets(std::move(allFPs));

	std::vector<ConcentricPattern> usedFPs;
	Results results;

	if (_testQR) {
		for (auto& fpSet : allFPSets) {
			if (Contains(usedFPs, fpSet.bl) || Contains(usedFPs, fpSet.tl) || Contains(usedFPs, fpSet.tr))
				continue;

			auto detectorResult = SampleQR(*binImg, fpSet);
			if (detectorResult.isValid()) {
				auto decoderResult = Decode(detectorResult.bits());
				auto position = detectorResult.position();
				if (decoderResult.isValid()) {
					usedFPs.push_back(fpSet.bl);
					usedFPs.push_back(fpSet.tl);
					usedFPs.push_back(fpSet.tr);
					results.emplace_back(std::move(decoderResult), std::move(position), BarcodeFormat::QRCode);
					if (maxSymbols && Size(results) == maxSymbols)
						break;
				}
			}
		}
	}

	if (_testMQR && !(maxSymbols && Size(results) == maxSymbols)) {
		for (auto fp : allFPs) {
			if (Contains(usedFPs, fp))
				continue;

			auto detectorResult = SampleMQR(*binImg, fp);
			if (detectorResult.isValid()) {
				auto decoderResult = Decode(detectorResult.bits());
				auto position = detectorResult.position();
				if (decoderResult.isValid()) {
					results.emplace_back(std::move(decoderResult), std::move(position), BarcodeFormat::MicroQRCode);
					if (maxSymbols && Size(results) == maxSymbols)
						break;
				}

			}
		}
	}

	return results;
}

} // namespace ZXing::QRCode
