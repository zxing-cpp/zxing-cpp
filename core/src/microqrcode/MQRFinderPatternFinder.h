#pragma once

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

#include <optional>
#include <vector>

namespace ZXing {

class DecodeHints;

namespace MicroQRCode {

class FinderPatternFinder
{
public:
	FinderPatternFinder();
	std::vector<ResultPoint> findCorners(const BitMatrix& image, DecodeHints const& hints);
	std::optional<FinderPatternInfo> findCenters(const BitMatrix& image, DecodeHints const& hints);

private:
	static int CENTER_QUORUM;
	static int MIN_SKIP;
	static int MAX_MODULES;
	std::vector<FinderPattern> possibleCenters_;
	std::vector<int> crossCheckStateCount_;

private: // methods
	std::optional<FinderPattern> findBestPattern(const BitMatrix& image, DecodeHints const& hints);
	std::vector<ResultPoint> getCodeEnclosingRect(const BitMatrix& image, const FinderPattern& actualPattern);
	std::optional<FinderPatternInfo> generatePatternInfoForPattern(const BitMatrix& image,
																   const FinderPattern& actualPattern);
	float centerFromEnd(const std::vector<int>& stateCount, int end) const;
	bool foundPatternCross(const std::vector<int>& stateCount) const;
	std::vector<int> getCrossCheckStateCount();
	bool crossCheckDiagonal(const BitMatrix& image, int startI, int centerJ, int maxCount, int originalStateCountTotal);
	float crossCheckVertical(const BitMatrix& image, int startI, int centerJ, int maxCount,
							 int originalStateCountTotal);
	float crossCheckHorizontal(const BitMatrix& image, int startJ, int centerI, int maxCount,
							   int originalStateCountTotal);
	bool handlePossibleCenter(const BitMatrix& image, const std::vector<int>& stateCount, int i, int j,
							  bool pureBarcode);
	bool haveMultiplyConfirmedCenters() const;
	std::optional<FinderPattern> selectBestPattern();
};

} // namespace MicroQRCode

} // namespace ZXing
