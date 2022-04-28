#ifndef __CORNER_FINDER_H__
#define __CORNER_FINDER_H__

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

#include "BitMatrix.h"
#include "MQRFinderPattern.h"

#include <vector>

namespace ZXing {

namespace MicroQRCode {

class CornerFinder
{
public:
	CornerFinder(const BitMatrix& image, const FinderPattern& center);
	std::vector<ResultPoint> find() const;

private:
	ResultPoint calculateDirection() const;
	int numberOfWhiteInKernel(int x, int y) const;
	bool isQuietZoneDirection(int stepX, int stepY) const;
	ResultPoint getMidpointOfCode(const std::vector<ResultPoint>& centerRect, const ResultPoint& direction) const;
	std::vector<ResultPoint> getLineToBottomRightCorner(const std::vector<ResultPoint>& centerEnclosingRect,
														const ResultPoint& direction) const;
	std::vector<ResultPoint> defineCornersMorePrecisely(const std::vector<ResultPoint>& centerEnclosingRect,
														const std::vector<ResultPoint>& codeEnclosingRect,
														const ResultPoint& direction) const;
	ResultPoint calculateLineIntersection(const ResultPoint& diagonalStart, const ResultPoint& diagonalEnd,
										  const ResultPoint& start, const ResultPoint& end) const;
	std::vector<ResultPoint> sortRectCorners(const std::vector<ResultPoint>& codeEnclosingRect,
											 const ResultPoint& direction) const;
	void swapPoints(std::vector<ResultPoint>& codeEnclosingRect, int source, int destination) const;

private:
	BitMatrix image_;
	const FinderPattern center_;
	int moduleSize_;
};

} // namespace MicroQRCode

} // namespace ZXing

#endif // __CORNER_FINDER_H__
