#ifndef __FINDER_PATTERN_FINDER_H__
#define __FINDER_PATTERN_FINDER_H__

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

#include "BitMatrix.h"
#include "MQRFinderPattern.h"
#include "MQRFinderPatternInfo.h"

#include <vector>

namespace ZXing {

class DecodeHints;

namespace MicroQRCode {

class FinderPatternFinder
{
public:
	FinderPatternFinder(const BitMatrix& image);
	std::vector<ResultPoint> findCorners(DecodeHints const& hints);
	FinderPatternInfo findCenters(DecodeHints const& hints);

private:
	static int CENTER_QUORUM;
	static int MIN_SKIP;
	static int MAX_MODULES;
	BitMatrix image_;
	std::vector<FinderPattern> possibleCenters_;
	std::vector<int> crossCheckStateCount_;

private: // methods
	FinderPattern findBestPattern(DecodeHints const& hints);
	std::vector<ResultPoint> getCodeEnclosingRect(const FinderPattern& actualPattern);
	FinderPatternInfo generatePatternInfoForPattern(const FinderPattern& actualPattern);
	float centerFromEnd(const std::vector<int>& stateCount, int end) const;
	bool foundPatternCross(const std::vector<int>& stateCount) const;
	std::vector<int> getCrossCheckStateCount();
	bool crossCheckDiagonal(int startI, int centerJ, int maxCount, int originalStateCountTotal);
	float crossCheckVertical(int startI, int centerJ, int maxCount, int originalStateCountTotal);
	float crossCheckHorizontal(int startJ, int centerI, int maxCount, int originalStateCountTotal);

	bool handlePossibleCenter(const std::vector<int>& stateCount, int i, int j, bool pureBarcode);
	bool haveMultiplyConfirmedCenters() const;
	FinderPattern selectBestPattern();
};

} // namespace MicroQRCode

} // namespace ZXing

#endif // __FINDER_PATTERN_FINDER_H__
