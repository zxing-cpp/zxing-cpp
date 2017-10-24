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

#include "WhiteRectDetector.h"
#include "BitMatrix.h"
#include "DecodeStatus.h"
#include "ZXNumeric.h"
#include "ResultPoint.h"

namespace ZXing {

static const int INIT_SIZE = 10;
static const int CORR = 1;

bool WhiteRectDetector::Detect(const BitMatrix& image, ResultPoint& p0, ResultPoint& p1, ResultPoint& p2, ResultPoint& p3)
{
	return Detect(image, INIT_SIZE, image.width() / 2, image.height() / 2, p0, p1, p2, p3);
}

/**
* Determines whether a segment contains a black point
*
* @param a          min value of the scanned coordinate
* @param b          max value of the scanned coordinate
* @param fixed      value of fixed coordinate
* @param horizontal set to true if scan must be horizontal, false if vertical
* @return true if a black point has been found, else false.
*/
static bool ContainsBlackPoint(const BitMatrix& image, int a, int b, int fixed, bool horizontal) {

	if (horizontal) {
		for (int x = a; x <= b; x++) {
			if (image.get(x, fixed)) {
				return true;
			}
		}
	}
	else {
		for (int y = a; y <= b; y++) {
			if (image.get(fixed, y)) {
				return true;
			}
		}
	}

	return false;
}

static bool GetBlackPointOnSegment(const BitMatrix& image, int aX, int aY, int bX, int bY, ResultPoint& result) {
	int dist = RoundToNearest(ResultPoint::Distance(aX, aY, bX, bY));
	float xStep = static_cast<float>(bX - aX) / dist;
	float yStep = static_cast<float>(bY - aY) / dist;

	for (int i = 0; i < dist; i++) {
		int x = RoundToNearest(aX + i * xStep);
		int y = RoundToNearest(aY + i * yStep);
		if (image.get(x, y)) {
			result.set(static_cast<float>(x), static_cast<float>(y));
			return true;
		}
	}
	return false;
}

/**
* recenters the points of a constant distance towards the center
*
* @param y bottom most point
* @param z left most point
* @param x right most point
* @param t top most point
* @return {@link ResultPoint}[] describing the corners of the rectangular
*         region. The first and last points are opposed on the diagonal, as
*         are the second and third. The first point will be the topmost
*         point and the last, the bottommost. The second point will be
*         leftmost and the third, the rightmost
*/
static void CenterEdges(const ResultPoint& y, const ResultPoint& z, const ResultPoint& x, const ResultPoint& t, int width, ResultPoint& p0, ResultPoint& p1, ResultPoint& p2, ResultPoint& p3)
{
	//
	//       t            t
	//  z                      x
	//        x    OR    z
	//   y                    y
	//

	float yi = y.x();
	float yj = y.y();
	float zi = z.x();
	float zj = z.y();
	float xi = x.x();
	float xj = x.y();
	float ti = t.x();
	float tj = t.y();

	if (yi < width / 2.0f) {
		p0 = ResultPoint(ti - CORR, tj + CORR);
		p1 = ResultPoint(zi + CORR, zj + CORR);
		p2 = ResultPoint(xi - CORR, xj - CORR);
		p3 = ResultPoint(yi + CORR, yj - CORR);
	}
	else {
		p0 = ResultPoint(ti + CORR, tj + CORR);
		p1 = ResultPoint(zi + CORR, zj - CORR);
		p2 = ResultPoint(xi - CORR, xj + CORR);
		p3 = ResultPoint(yi - CORR, yj - CORR);
	}
}

/**
* <p>
* Detects a candidate barcode-like rectangular region within an image. It
* starts around the center of the image, increases the size of the candidate
* region until it finds a white rectangular region.
* </p>
*
* @return {@link ResultPoint}[] describing the corners of the rectangular
*         region. The first and last points are opposed on the diagonal, as
*         are the second and third. The first point will be the topmost
*         point and the last, the bottommost. The second point will be
*         leftmost and the third, the rightmost
* @throws NotFoundException if no Data Matrix Code can be found
*/
bool WhiteRectDetector::Detect(const BitMatrix& image, int initSize, int x, int y, ResultPoint& p0, ResultPoint& p1, ResultPoint& p2, ResultPoint& p3)
{
	int height = image.height();
	int width = image.width();
	int halfsize = initSize / 2;
	int left = x - halfsize;
	int right = x + halfsize;
	int up = y - halfsize;
	int down = y + halfsize;
	if (up < 0 || left < 0 || down >= height || right >= width) {
		return false;
	}

	bool sizeExceeded = false;
	bool aBlackPointFoundOnBorder = true;
	bool atLeastOneBlackPointFoundOnBorder = false;

	bool atLeastOneBlackPointFoundOnRight = false;
	bool atLeastOneBlackPointFoundOnBottom = false;
	bool atLeastOneBlackPointFoundOnLeft = false;
	bool atLeastOneBlackPointFoundOnTop = false;

	while (aBlackPointFoundOnBorder) {

		aBlackPointFoundOnBorder = false;

		// .....
		// .   |
		// .....
		bool rightBorderNotWhite = true;
		while ((rightBorderNotWhite || !atLeastOneBlackPointFoundOnRight) && right < width) {
			rightBorderNotWhite = ContainsBlackPoint(image, up, down, right, false);
			if (rightBorderNotWhite) {
				right++;
				aBlackPointFoundOnBorder = true;
				atLeastOneBlackPointFoundOnRight = true;
			}
			else if (!atLeastOneBlackPointFoundOnRight) {
				right++;
			}
		}

		if (right >= width) {
			sizeExceeded = true;
			break;
		}

		// .....
		// .   .
		// .___.
		bool bottomBorderNotWhite = true;
		while ((bottomBorderNotWhite || !atLeastOneBlackPointFoundOnBottom) && down < height) {
			bottomBorderNotWhite = ContainsBlackPoint(image, left, right, down, true);
			if (bottomBorderNotWhite) {
				down++;
				aBlackPointFoundOnBorder = true;
				atLeastOneBlackPointFoundOnBottom = true;
			}
			else if (!atLeastOneBlackPointFoundOnBottom) {
				down++;
			}
		}

		if (down >= height) {
			sizeExceeded = true;
			break;
		}

		// .....
		// |   .
		// .....
		bool leftBorderNotWhite = true;
		while ((leftBorderNotWhite || !atLeastOneBlackPointFoundOnLeft) && left >= 0) {
			leftBorderNotWhite = ContainsBlackPoint(image, up, down, left, false);
			if (leftBorderNotWhite) {
				left--;
				aBlackPointFoundOnBorder = true;
				atLeastOneBlackPointFoundOnLeft = true;
			}
			else if (!atLeastOneBlackPointFoundOnLeft) {
				left--;
			}
		}

		if (left < 0) {
			sizeExceeded = true;
			break;
		}

		// .___.
		// .   .
		// .....
		bool topBorderNotWhite = true;
		while ((topBorderNotWhite || !atLeastOneBlackPointFoundOnTop) && up >= 0) {
			topBorderNotWhite = ContainsBlackPoint(image, left, right, up, true);
			if (topBorderNotWhite) {
				up--;
				aBlackPointFoundOnBorder = true;
				atLeastOneBlackPointFoundOnTop = true;
			}
			else if (!atLeastOneBlackPointFoundOnTop) {
				up--;
			}
		}

		if (up < 0) {
			sizeExceeded = true;
			break;
		}

		if (aBlackPointFoundOnBorder) {
			atLeastOneBlackPointFoundOnBorder = true;
		}

	}

	if (!sizeExceeded && atLeastOneBlackPointFoundOnBorder) {

		int maxSize = right - left;

		ResultPoint z;
		bool found = false;
		for (int i = 1; !found && i < maxSize; i++) {
			found = GetBlackPointOnSegment(image, left, down - i, left + i, down, z);
		}

		if (!found) {
			return false;
		}

		ResultPoint t;
		found = false;
		//go down right
		for (int i = 1; !found && i < maxSize; i++) {
			found = GetBlackPointOnSegment(image, left, up + i, left + i, up, t);
		}

		if (!found) {
			return false;
		}

		ResultPoint x;
		found = false;
		//go down left
		for (int i = 1; !found && i < maxSize; i++) {
			found = GetBlackPointOnSegment(image, right, up + i, right - i, up, x);
		}

		if (!found) {
			return false;
		}

		ResultPoint y;
		found = false;
		//go up left
		for (int i = 1; !found && i < maxSize; i++) {
			found = GetBlackPointOnSegment(image, right, down - i, right - i, down, y);
		}

		if (!found) {
			return false;
		}

		CenterEdges(y, z, x, t, width, p0, p1, p2, p3);
		return true;
	}
	else {
		return false;
	}
}

} // ZXing
