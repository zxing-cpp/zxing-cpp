/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
* Copyright 2021 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "ConcentricFinder.h"
#include "DetectorResult.h"

#include <vector>

namespace ZXing {

class DetectorResult;
class BitMatrix;

namespace QRCode {

struct FinderPatternSet
{
	ConcentricPattern bl, tl, tr;
};

using FinderPatterns = std::vector<ConcentricPattern>;
using FinderPatternSets = std::vector<FinderPatternSet>;

FinderPatterns FindFinderPatterns(const BitMatrix& image, bool tryHarder);
FinderPatternSets GenerateFinderPatternSets(FinderPatterns& patterns);

DetectorResult SampleQR(const BitMatrix& image, const FinderPatternSet& fp);
DetectorResult DetectPureQR(const BitMatrix& image);

#ifndef ZXING_EMBEDDED_QR_ONLY
DetectorResult SampleMQR(const BitMatrix& image, const ConcentricPattern& fp);
DetectorResult SampleRMQR(const BitMatrix& image, const ConcentricPattern& fp);
DetectorResult DetectPureMQR(const BitMatrix& image);
DetectorResult DetectPureRMQR(const BitMatrix& image);
#endif // ZXING_EMBEDDED_QR_ONLY

} // QRCode
} // ZXing
