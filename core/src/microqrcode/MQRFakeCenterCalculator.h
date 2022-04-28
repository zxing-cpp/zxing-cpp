#ifndef __FAKE_CENTER_CALCULATOR_H__
#define __FAKE_CENTER_CALCULATOR_H__

/*
 * Copyright 2017 KURZ Digital Solutions GmbH & Co. KG
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

#include "MQRFinderPattern.h"
#include "ResultPoint.h"

#include <vector>

namespace ZXing {
namespace MicroQRCode {

class FakeCenterCalculator
{
public:
	FakeCenterCalculator(const FinderPattern& actualCenter, const std::vector<ResultPoint>& rect);
	FinderPattern getTopRightCenter();
	FinderPattern getBottomLeftCenter();

private:
	ResultPoint calculateCenter(const ResultPoint& deltas) const;
	ResultPoint calculateNormalizedDeltas(const ResultPoint& source, const ResultPoint& destination);

private:
	FinderPattern actualCenter_;
	float moduleSize_;
	std::vector<ResultPoint> rect_;
	int dimension_;
};

} // namespace MicroQRCode

} // namespace ZXing

#endif // __FAKE_CENTER_CALCULATOR_H__
