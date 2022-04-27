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

#include "FinderPatternInfo.h"

namespace ZXing {

namespace MicroQRCode {

FinderPatternInfo::FinderPatternInfo(const std::vector<FinderPattern>& patternCenters) :
    actualTopLeft_(patternCenters[0]), fakeTopRight_(patternCenters[1]), fakeBottomLeft_(patternCenters[2])
{
}

FinderPattern FinderPatternInfo::getActualTopLeft() const
{
  return actualTopLeft_;
}

FinderPattern FinderPatternInfo::getFakeTopRight() const
{
  return fakeTopRight_;
}

FinderPattern FinderPatternInfo::getFakeBottomLeft() const
{
	return fakeBottomLeft_;
}

}

}
