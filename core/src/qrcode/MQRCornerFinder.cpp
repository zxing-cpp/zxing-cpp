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

#include "MQRCornerFinder.h"

#include "WhiteRectDetector.h"

namespace ZXing::MicroQRCode {

/**
 * Detects the corners of a Micro QR Code. It will start with getting the corners of the inner center
 * of the qr code eye. From there it calculates the midpoint of the qr code and searches for the code enclosing
 * rect with an increasing search area.
 *
 * @author Christian Braun
 */

static int NumberOfWhiteInKernel(const BitMatrix& image, int moduleSize, int x, int y)
{
	const auto safePixelGet = [&image](const int x, const int y) {
		if (x >= 0 && x < image.width() && y >= 0 && y < image.height())
			return image.get(x, y);
		return false;
	};

	// 9 point imagekernel
	std::vector<bool> moduleKernel{safePixelGet(x, y),
								   safePixelGet(x - moduleSize, y),
								   safePixelGet(x - moduleSize, y + moduleSize),
								   safePixelGet(x, y + moduleSize),
								   safePixelGet(x + moduleSize, y + moduleSize),
								   safePixelGet(x + moduleSize, y),
								   safePixelGet(x + moduleSize, y - moduleSize),
								   safePixelGet(x, y - moduleSize),
								   safePixelGet(x - moduleSize, y - moduleSize)};
	int whiteModules = 0;

	for (bool isBlackModule : moduleKernel) {
		if (!isBlackModule) {
			++whiteModules;
		}
	}

	return whiteModules;
}

static bool IsQuietZoneDirection(const BitMatrix& image, const FinderPattern& center, int stepX, int stepY)
{
	int numberOfSteps = 7;
	int centerX = std::lround(center.x());
	int centerY = std::lround(center.y());
	int moduleSize = std::lround(center.getEstimatedModuleSize());
	bool firstStep = false;

	// We assume that we hit a quiet zone when we get at least 5 white modules
	// directly followed by 9 white modules in our image kernel
	for (int i = 0; i <= numberOfSteps; ++i) {
		int x = centerX + i * stepX * moduleSize;
		int y = centerY + i * stepY * moduleSize;
		if (firstStep && NumberOfWhiteInKernel(image, moduleSize, x, y) >= 9) {
			return true;
		}

		firstStep = NumberOfWhiteInKernel(image, moduleSize, x, y) >= 5;
	}

	return false;
}

/**
 * Calculates the direction of a Micro Qr Code. For this purpose the method is using the center
 * of the code and tries to find out in which direction the quite zones are closest to the center
 * of the FinderPattern.
 *
 * @return Resultpoint with direction. The direction is given as a vector (1, 1) means the code
 * expands in positive x and positive y direction
 * @throws NotFoundException If there was a quitezone detected twice either in x or y direction
 */
static ResultPoint CalculateDirection(const BitMatrix& image, const FinderPattern& center)
{
	int x = 0;
	int y = 0;

	if (!IsQuietZoneDirection(image, center, 1, 0)) {
		x += 1;
	}
	if (!IsQuietZoneDirection(image, center, 0, 1)) {
		y += 1;
	}
	if (!IsQuietZoneDirection(image, center, -1, 0)) {
		x += -1;
	}
	if (!IsQuietZoneDirection(image, center, 0, -1)) {
		y += -1;
	}

	return ResultPoint{x, y};
}

static std::vector<ResultPoint> GetLineToBottomRightCorner(const std::vector<ResultPoint>& centerEnclosingRect,
														   const ResultPoint& direction)
{
	ResultPoint startCenter;
	ResultPoint endCenter;

	if (direction.x() == 1 && direction.y() == 1) {
		startCenter = centerEnclosingRect[0];
		endCenter = centerEnclosingRect[3];
	} else if (direction.x() == -1 && direction.y() == -1) {
		startCenter = centerEnclosingRect[3];
		endCenter = centerEnclosingRect[0];
	} else if (direction.x() == 1 && direction.y() == -1) {
		startCenter = centerEnclosingRect[1];
		endCenter = centerEnclosingRect[2];
	} else if (direction.x() == -1 && direction.y() == 1) {
		startCenter = centerEnclosingRect[2];
		endCenter = centerEnclosingRect[1];
	}

	return std::vector<ResultPoint>{startCenter, endCenter};
}

static ResultPoint GetMidpointOfCode(const FinderPattern& center, const std::vector<ResultPoint>& centerRect,
									 const ResultPoint& direction)
{
	const std::vector<ResultPoint> diagonal = GetLineToBottomRightCorner(centerRect, direction);
	const ResultPoint startCenter = diagonal[0];
	const ResultPoint endCenter = diagonal[1];
	// Where to set the midpoint on x axis
	int modulesAwayFromCenterX = 12;

	float delta = (endCenter.y() - startCenter.y()) / (endCenter.x() - startCenter.x());
	float t = startCenter.y() - delta * startCenter.x();

	float x = center.x() + direction.x() * modulesAwayFromCenterX * center.getEstimatedModuleSize();
	float middleBetweenCornersX = (x + startCenter.x()) / 2;
	float middleBetweenCornersY = delta * middleBetweenCornersX + t;

	return ResultPoint{middleBetweenCornersX, middleBetweenCornersY};
}

static ResultPoint CalculateLineIntersection(const ResultPoint& diagonalStart, const ResultPoint& diagonalEnd,
											 const ResultPoint& start, const ResultPoint& end)
{
	const float deltaDiagonal = (diagonalEnd.y() - diagonalStart.y()) / (diagonalEnd.x() - diagonalStart.x());
	const float delta = (end.y() - start.y()) / (end.x() - start.x());

	const float tDiagonal = diagonalStart.y() - deltaDiagonal * diagonalStart.x();
	const float t = start.y() - delta * start.x();

	float intersectionX;
	float intersectionY;

	// Java code had signed infinity check.  Not sure C++ does.
	if (std::numeric_limits<float>::infinity() == delta) {
		intersectionX = start.x();
		intersectionY = deltaDiagonal * intersectionX + tDiagonal;
	} else {
		intersectionX = (t - tDiagonal) / (deltaDiagonal - delta);
		intersectionY = deltaDiagonal * intersectionX + tDiagonal;
	}

	return ResultPoint{intersectionX, intersectionY};
}

static std::vector<ResultPoint> DefineCornersMorePrecisely(const std::vector<ResultPoint>& centerEnclosingRect,
														   const std::vector<ResultPoint>& codeEnclosingRect,
														   const ResultPoint& direction)
{
	ResultPoint start;
	ResultPoint end = codeEnclosingRect[3];

	if (ResultPoint::Distance(std::lround(codeEnclosingRect[2].x()), std::lround(codeEnclosingRect[2].y()),
							  std::lround(codeEnclosingRect[3].x()), std::lround(codeEnclosingRect[3].y())) >
		ResultPoint::Distance(std::lround(codeEnclosingRect[1].x()), std::lround(codeEnclosingRect[1].y()),
							  std::lround(codeEnclosingRect[3].x()), std::lround(codeEnclosingRect[3].y()))) {
		start = codeEnclosingRect[1];
	} else {
		start = codeEnclosingRect[2];
	}

	std::vector<ResultPoint> diagonalLine = GetLineToBottomRightCorner(centerEnclosingRect, direction);
	ResultPoint bottomRightCorner = CalculateLineIntersection(diagonalLine[0], diagonalLine[1], start, end);

	// TODO: What?
	//    codeEnclosingRect[3] = bottomRightCorner;
	return codeEnclosingRect;
}

static void SwapPoints(std::vector<ResultPoint>& codeEnclosingRect, int source, int destination)
{
	ResultPoint temp = codeEnclosingRect[source];
	codeEnclosingRect[source] = codeEnclosingRect[destination];
	codeEnclosingRect[destination] = temp;
}

static std::vector<ResultPoint> SortRectCorners(const std::vector<ResultPoint>& codeEnclosingRect,
												const ResultPoint& direction)
{
	auto sortedCorners = codeEnclosingRect;
	if (direction.x() == 1 && direction.y() == 1) {
		return sortedCorners;
	} else if (direction.x() == -1 && direction.y() == -1) {
		SwapPoints(sortedCorners, 0, 3);
		SwapPoints(sortedCorners, 1, 2);
	} else if (direction.x() == 1 && direction.y() == -1) {
		SwapPoints(sortedCorners, 0, 1);
		SwapPoints(sortedCorners, 2, 1);
		SwapPoints(sortedCorners, 3, 1);
	} else if (direction.x() == -1 && direction.y() == 1) {
		SwapPoints(sortedCorners, 2, 0);
		SwapPoints(sortedCorners, 2, 1);
		SwapPoints(sortedCorners, 3, 2);
	}

	return sortedCorners;
}

/**
 * @return the corners of the Micro QR Code. They will always be sorted like the Micro QR Code is in
 * normal position without any rotation. That means the corner closest to the center will always be at index 0
 * an the corner at the opposite site will always be at index 3 and so on.
 * If no corners are found an empty vector is returned.
 * @param image
 * @param center of the pattern.
 */
std::vector<ResultPoint> FindCorners(const BitMatrix& image, const FinderPattern& center)
{
	ResultPoint direction = CalculateDirection(image, center);
	if (direction.x() == 0 || direction.y() == 0)
		return {};

	ResultPoint centerEnclosingRectA, centerEnclosingRectB, centerEnclosingRectC, centerEnclosingRectD;
	if (!DetectWhiteRect(image, std::lround(center.getEstimatedModuleSize() * 4), std::lround(center.x()),
						 std::lround(center.y()), centerEnclosingRectA, centerEnclosingRectB, centerEnclosingRectC,
						 centerEnclosingRectD))
		return {};

	std::vector<ResultPoint> centerEnclosingRect = {centerEnclosingRectA, centerEnclosingRectB, centerEnclosingRectC,
													centerEnclosingRectD};

	ResultPoint midPoint = GetMidpointOfCode(center, centerEnclosingRect, direction);

	ResultPoint codeEnclosingRectA, codeEnclosingRectB, codeEnclosingRectC, codeEnclosingRectD;
	if (!DetectWhiteRect(image, std::lround(center.getEstimatedModuleSize() * 5), std::lround(midPoint.x()),
						 std::lround(midPoint.y()), codeEnclosingRectA, codeEnclosingRectB, codeEnclosingRectC,
						 codeEnclosingRectD))
		return {};

	std::vector<ResultPoint> codeEnclosingRect = {codeEnclosingRectA, codeEnclosingRectB, codeEnclosingRectC,
												  codeEnclosingRectD};

	codeEnclosingRect = SortRectCorners(codeEnclosingRect, direction);
	return DefineCornersMorePrecisely(centerEnclosingRect, codeEnclosingRect, direction);
}

} // namespace ZXing::MicroQRCode