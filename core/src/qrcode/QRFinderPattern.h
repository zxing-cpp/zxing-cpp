#pragma once
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

#include "ResultPoint.h"

namespace ZXing {
namespace QRCode {

/**
* <p>Encapsulates a finder pattern, which are the three square patterns found in
* the corners of QR Codes. It also encapsulates a count of similar finder patterns,
* as a convenience to the finder's bookkeeping.</p>
*
* @author Sean Owen
*/
class FinderPattern : public ResultPoint
{
	float _estimatedModuleSize = 0;
	int _count = 0;

public:
	FinderPattern() = default;
	FinderPattern(float posX, float posY, float estimatedModuleSize, int count = 1);

	float estimatedModuleSize() const {
		return _estimatedModuleSize;
	}

	int count() const {
		return _count;
	}

	bool isValid() const {
		return _count > 0;
	}

	/**
	* <p>Determines if this finder pattern "about equals" a finder pattern at the stated
	* position and size -- meaning, it is at nearly the same center with nearly the same size.</p>
	*/
	bool aboutEquals(float moduleSize, float i, float j) const;

	/**
	* Combines this object's current estimate of a finder pattern position and module size
	* with a new estimate. It returns a new {@code FinderPattern} containing a weighted average
	* based on count.
	*/
	FinderPattern combineEstimate(float i, float j, float newModuleSize) const;
};

} // QRCode
} // ZXing
