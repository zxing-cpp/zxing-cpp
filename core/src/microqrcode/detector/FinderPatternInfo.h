#ifndef __FINDER_PATTERN_INFO_H__
#define __FINDER_PATTERN_INFO_H__

/*
 * Copyright 2007 ZXing authors All rights reserved.
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

#include "FinderPattern.h"

#include <vector>

namespace ZXing {

namespace MicroQRCode {

/**
 * <p>Encapsulates information about finder patterns in an image, including the location of
 * the finder pattern, and its estimated module size.</p>
 * For the Micro QR Code we just assume where these patterns should be located.
 */
class FinderPatternInfo
{
public:
	FinderPatternInfo(const std::vector<FinderPattern>& patternCenters);

	FinderPattern getActualTopLeft() const;
	FinderPattern getFakeTopRight() const;
	FinderPattern getFakeBottomLeft() const;
private:
	FinderPattern actualTopLeft_;
	FinderPattern fakeTopRight_;
	FinderPattern fakeBottomLeft_;
};

}

} // ZXing

#endif // __FINDER_PATTERN_INFO_H__
