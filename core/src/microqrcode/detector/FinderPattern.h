#ifndef __FINDER_PATTERN_H__
#define __FINDER_PATTERN_H__

/*
 * Copyright 2007 ZXing authors All rights reserved.
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

namespace MicroQRCode {

/**
 * <p>Encapsulates a finder pattern, which is the square pattern found in
 * the corner of Micro QR Codes. It also encapsulates a count of similar finder patterns,
 * as a convenience to the finder's bookkeeping.</p>
 *
 * @author Sean Owen
 */
class FinderPattern : public ResultPoint
{
private:
	float estimatedModuleSize_;
	int count_;
			
	FinderPattern(float posX, float posY, float estimatedModuleSize, int count);

public:
	FinderPattern(float posX, float posY, float estimatedModuleSize);
	int getCount() const;
	float getEstimatedModuleSize() const;
	bool aboutEquals(float moduleSize, float i, float j) const;
	FinderPattern combineEstimate(float i, float j, float newModuleSize) const;
};

}

} // ZXing

#endif // __FINDER_PATTERN_H__
