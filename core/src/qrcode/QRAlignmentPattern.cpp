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

#include "qrcode/QRAlignmentPattern.h"

#include <cmath>

namespace ZXing {
namespace QRCode {

AlignmentPattern::AlignmentPattern(float posX, float posY, float estimatedModuleSize) :
	ResultPoint(posX, posY),
	_estimatedModuleSize(estimatedModuleSize)
{
}

bool
AlignmentPattern::aboutEquals(float moduleSize, float i, float j) const
{
	if (std::abs(i - y()) <= moduleSize && std::abs(j - x()) <= moduleSize) {
		float moduleSizeDiff = std::abs(moduleSize - _estimatedModuleSize);
		return moduleSizeDiff <= 1.0f || moduleSizeDiff <= _estimatedModuleSize;
	}
	return false;
}

/**
* Combines this object's current estimate of a finder pattern position and module size
* with a new estimate. It returns a new {@code FinderPattern} containing an average of the two.
*/
AlignmentPattern
AlignmentPattern::combineEstimate(float i, float j, float newModuleSize) const {
	float combinedX = (x() + j) / 2.0f;
	float combinedY = (y() + i) / 2.0f;
	float combinedModuleSize = (_estimatedModuleSize + newModuleSize) / 2.0f;
	return {combinedX, combinedY, combinedModuleSize};
}

} // QRCode
} // ZXing
