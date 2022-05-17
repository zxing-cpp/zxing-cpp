#pragma once
/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
* Copyright 2021 Axel Waggershauser
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
FinderPatternSets GenerateFinderPatternSets(FinderPatterns&& patterns);

DetectorResult SampleQR(const BitMatrix& image, const FinderPatternSet& fp);
DetectorResult SampleMQR(const BitMatrix& image, const ConcentricPattern& fp);

DetectorResult DetectPureQR(const BitMatrix& image);
DetectorResult DetectPureMQR(const BitMatrix& image);

} // QRCode
} // ZXing
