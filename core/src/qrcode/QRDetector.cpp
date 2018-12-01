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

#include "qrcode/QRDetector.h"
#include "qrcode/QRFinderPatternFinder.h"
#include "qrcode/QRFinderPatternInfo.h"
#include "qrcode/QRAlignmentPattern.h"
#include "qrcode/QRAlignmentPatternFinder.h"
#include "qrcode/QRVersion.h"
#include "BitMatrix.h"
#include "DecodeHints.h"
#include "DetectorResult.h"
#include "PerspectiveTransform.h"
#include "GridSampler.h"
#include "ZXNumeric.h"
#include "DecodeStatus.h"

#include <cstdlib>

namespace ZXing {
namespace QRCode {

/**
* <p>This method traces a line from a point in the image, in the direction towards another point.
* It begins in a black region, and keeps going until it finds white, then black, then white again.
* It reports the distance from the start to this point.</p>
*
* <p>This is used when figuring out how wide a finder pattern is, when the finder pattern
* may be skewed or rotated.</p>
*/
static float SizeOfBlackWhiteBlackRun(const BitMatrix& image, int fromX, int fromY, int toX, int toY) {
	// Mild variant of Bresenham's algorithm;
	// see http://en.wikipedia.org/wiki/Bresenham's_line_algorithm
	bool steep = std::abs(toY - fromY) > std::abs(toX - fromX);
	if (steep) {
		std::swap(fromX, fromY);
		std::swap(toX, toY);
	}

	int dx = std::abs(toX - fromX);
	int dy = std::abs(toY - fromY);
	int error = -dx / 2;
	int xstep = fromX < toX ? 1 : -1;
	int ystep = fromY < toY ? 1 : -1;

	// In black pixels, looking for white, first or second time.
	int state = 0;
	// Loop up until x == toX, but not beyond
	int xLimit = toX + xstep;
	for (int x = fromX, y = fromY; x != xLimit; x += xstep) {
		int realX = steep ? y : x;
		int realY = steep ? x : y;

		// Does current pixel mean we have moved white to black or vice versa?
		// Scanning black in state 0,2 and white in state 1, so if we find the wrong
		// color, advance to next state or end if we are in state 2 already
		if ((state == 1) == image.get(realX, realY)) {
			if (state == 2) {
				return ResultPoint::Distance(x, y, fromX, fromY);
			}
			state++;
		}

		error += dy;
		if (error > 0) {
			if (y == toY) {
				break;
			}
			y += ystep;
			error -= dx;
		}
	}
	// Found black-white-black; give the benefit of the doubt that the next pixel outside the image
	// is "white" so this last point at (toX+xStep,toY) is the right ending. This is really a
	// small approximation; (toX+xStep,toY+yStep) might be really correct. Ignore this.
	if (state == 2) {
		return ResultPoint::Distance(toX + xstep, toY, fromX, fromY);
	}
	// else we didn't find even black-white-black; no estimate is really possible
	return std::numeric_limits<float>::quiet_NaN();
}

/**
* See {@link #sizeOfBlackWhiteBlackRun(int, int, int, int)}; computes the total width of
* a finder pattern by looking for a black-white-black run from the center in the direction
* of another point (another finder pattern center), and in the opposite direction too.
*/
static float SizeOfBlackWhiteBlackRunBothWays(const BitMatrix& image, int fromX, int fromY, int toX, int toY) {

	float result = SizeOfBlackWhiteBlackRun(image, fromX, fromY, toX, toY);

	// Now count other way -- don't run off image though of course
	float scale = 1.0f;
	int otherToX = fromX - (toX - fromX);
	if (otherToX < 0) {
		scale = (float)fromX / (float)(fromX - otherToX);
		otherToX = 0;
	}
	else if (otherToX >= image.width()) {
		scale = (float)(image.width() - 1 - fromX) / (float)(otherToX - fromX);
		otherToX = image.width() - 1;
	}
	int otherToY = (int)(fromY - (toY - fromY) * scale);

	scale = 1.0f;
	if (otherToY < 0) {
		scale = (float)fromY / (float)(fromY - otherToY);
		otherToY = 0;
	}
	else if (otherToY >= image.height()) {
		scale = (float)(image.height() - 1 - fromY) / (float)(otherToY - fromY);
		otherToY = image.height() - 1;
	}
	otherToX = (int)(fromX + (otherToX - fromX) * scale);

	result += SizeOfBlackWhiteBlackRun(image, fromX, fromY, otherToX, otherToY);

	// Middle pixel is double-counted this way; subtract 1
	return result - 1.0f;
}

/**
* <p>Estimates module size based on two finder patterns -- it uses
* {@link #sizeOfBlackWhiteBlackRunBothWays(int, int, int, int)} to figure the
* width of each, measuring along the axis between their centers.</p>
*/
static float CalculateModuleSizeOneWay(const BitMatrix& image, const ResultPoint& pattern, const ResultPoint& otherPattern)
{
	float moduleSizeEst1 = SizeOfBlackWhiteBlackRunBothWays(image,
		static_cast<int>(pattern.x()),
		static_cast<int>(pattern.y()),
		static_cast<int>(otherPattern.x()),
		static_cast<int>(otherPattern.y()));

	float moduleSizeEst2 = SizeOfBlackWhiteBlackRunBothWays(image,
		static_cast<int>(otherPattern.x()),
		static_cast<int>(otherPattern.y()),
		static_cast<int>(pattern.x()),
		static_cast<int>(pattern.y()));

	if (std::isnan(moduleSizeEst1)) {
		return moduleSizeEst2 / 7.0f;
	}
	if (std::isnan(moduleSizeEst2)) {
		return moduleSizeEst1 / 7.0f;
	}
	// Average them, and divide by 7 since we've counted the width of 3 black modules,
	// and 1 white and 1 black module on either side. Ergo, divide sum by 14.
	return (moduleSizeEst1 + moduleSizeEst2) / 14.0f;
}

/**
* <p>Computes an average estimated module size based on estimated derived from the positions
* of the three finder patterns.</p>
*
* @param topLeft detected top-left finder pattern center
* @param topRight detected top-right finder pattern center
* @param bottomLeft detected bottom-left finder pattern center
* @return estimated module size
*/
static float CalculateModuleSize(const BitMatrix& image, const ResultPoint& topLeft, const ResultPoint& topRight, const ResultPoint& bottomLeft)
{
	// Take the average
	return (CalculateModuleSizeOneWay(image, topLeft, topRight) + CalculateModuleSizeOneWay(image, topLeft, bottomLeft)) / 2.0f;
}


/**
* <p>Attempts to locate an alignment pattern in a limited region of the image, which is
* guessed to contain it. This method uses {@link AlignmentPattern}.</p>
*
* @param overallEstModuleSize estimated module size so far
* @param estAlignmentX x coordinate of center of area probably containing alignment pattern
* @param estAlignmentY y coordinate of above
* @param allowanceFactor number of pixels in all directions to search from the center
* @return {@link AlignmentPattern} if found, or null otherwise
* @throws NotFoundException if an unexpected error occurs during detection
*/
AlignmentPattern FindAlignmentInRegion(const BitMatrix& image, float overallEstModuleSize, int estAlignmentX, int estAlignmentY, float allowanceFactor)
{
	// Look for an alignment pattern (3 modules in size) around where it
	// should be
	int allowance = (int)(allowanceFactor * overallEstModuleSize);
	int alignmentAreaLeftX = std::max(0, estAlignmentX - allowance);
	int alignmentAreaRightX = std::min(image.width() - 1, estAlignmentX + allowance);
	if (alignmentAreaRightX - alignmentAreaLeftX < overallEstModuleSize * 3) {
		return {};
	}

	int alignmentAreaTopY = std::max(0, estAlignmentY - allowance);
	int alignmentAreaBottomY = std::min(image.height() - 1, estAlignmentY + allowance);
	if (alignmentAreaBottomY - alignmentAreaTopY < overallEstModuleSize * 3) {
		return {};
	}

	return AlignmentPatternFinder::Find(image, alignmentAreaLeftX, alignmentAreaTopY, alignmentAreaRightX - alignmentAreaLeftX, alignmentAreaBottomY - alignmentAreaTopY, overallEstModuleSize);
}

static PerspectiveTransform CreateTransform(const ResultPoint& topLeft, const ResultPoint& topRight, const ResultPoint& bottomLeft, const AlignmentPattern& alignmentPattern, int dimension)
{
	float dimMinusThree = (float)dimension - 3.5f;
	float bottomRightX;
	float bottomRightY;
	float sourceBottomRightX;
	float sourceBottomRightY;
	if (alignmentPattern.isValid()) {
		bottomRightX = alignmentPattern.x();
		bottomRightY = alignmentPattern.y();
		sourceBottomRightX = dimMinusThree - 3.0f;
		sourceBottomRightY = sourceBottomRightX;
	}
	else {
		// Don't have an alignment pattern, just make up the bottom-right point
		bottomRightX = (topRight.x() - topLeft.x()) + bottomLeft.x();
		bottomRightY = (topRight.y() - topLeft.y()) + bottomLeft.y();
		sourceBottomRightX = dimMinusThree;
		sourceBottomRightY = dimMinusThree;
	}

	return PerspectiveTransform::QuadrilateralToQuadrilateral(
		3.5f,
		3.5f,
		dimMinusThree,
		3.5f,
		sourceBottomRightX,
		sourceBottomRightY,
		3.5f,
		dimMinusThree,
		topLeft.x(),
		topLeft.y(),
		topRight.x(),
		topRight.y(),
		bottomRightX,
		bottomRightY,
		bottomLeft.x(),
		bottomLeft.y());
}

/**
* <p>Computes the dimension (number of modules on a size) of the QR Code based on the position
* of the finder patterns and estimated module size.</p>
* Return -1 in case of error.
*/
static int ComputeDimension(const ResultPoint& topLeft, const ResultPoint& topRight, const ResultPoint& bottomLeft, float moduleSize)
{
	int tltrCentersDimension = RoundToNearest(ResultPoint::Distance(topLeft, topRight) / moduleSize);
	int tlblCentersDimension = RoundToNearest(ResultPoint::Distance(topLeft, bottomLeft) / moduleSize);
	int dimension = ((tltrCentersDimension + tlblCentersDimension) / 2) + 7;
	switch (dimension & 0x03) { // mod 4
	case 0:
		return ++dimension;
	case 1:
		return dimension;
	case 2:
		return --dimension;
	}
	return -1; // to signal error;
}

static DetectorResult
ProcessFinderPatternInfo(const BitMatrix& image, const FinderPatternInfo& info)
{
	float moduleSize = CalculateModuleSize(image, info.topLeft, info.topRight, info.bottomLeft);
	if (moduleSize < 1.0f) {
		return {};
	}
	int dimension = ComputeDimension(info.topLeft, info.topRight, info.bottomLeft, moduleSize);
	if (dimension < 0)
		return {};

	const Version* provisionalVersion = Version::ProvisionalVersionForDimension(dimension);
	if (provisionalVersion == nullptr)
		return {};

	int modulesBetweenFPCenters = provisionalVersion->dimensionForVersion() - 7;

	AlignmentPattern alignmentPattern;

	// Anything above version 1 has an alignment pattern
	if (!provisionalVersion->alignmentPatternCenters().empty()) {

		// Guess where a "bottom right" finder pattern would have been
		float bottomRightX = info.topRight.x() - info.topLeft.x() + info.bottomLeft.x();
		float bottomRightY = info.topRight.y() - info.topLeft.y() + info.bottomLeft.y();

		// Estimate that alignment pattern is closer by 3 modules
		// from "bottom right" to known top left location
		float correctionToTopLeft = 1.0f - 3.0f / (float)modulesBetweenFPCenters;
		int estAlignmentX = static_cast<int>(info.topLeft.x() + correctionToTopLeft * (bottomRightX - info.topLeft.x()));
		int estAlignmentY = static_cast<int>(info.topLeft.y() + correctionToTopLeft * (bottomRightY - info.topLeft.y()));

		// Kind of arbitrary -- expand search radius before giving up
		for (int i = 4; i <= 16; i <<= 1) {
			alignmentPattern = FindAlignmentInRegion(image, moduleSize, estAlignmentX, estAlignmentY, static_cast<float>(i));
			if (alignmentPattern.isValid())
				break;
		}
		// If we didn't find alignment pattern... well try anyway without it
	}

	PerspectiveTransform transform = CreateTransform(info.topLeft, info.topRight, info.bottomLeft, alignmentPattern, dimension);

	auto bits = GridSampler::Instance()->sampleGrid(image, dimension, dimension, transform);
	if (bits.empty())
		return {};

	if (alignmentPattern.isValid())
		return {std::move(bits), {info.bottomLeft, info.topLeft, info.topRight, alignmentPattern}};
	else
		return {std::move(bits), {info.bottomLeft, info.topLeft, info.topRight}};
}

DetectorResult Detector::Detect(const BitMatrix& image, bool tryHarder)
{
	FinderPatternInfo info = FinderPatternFinder::Find(image, tryHarder);

	if (!info.isValid())
		return {};
	
	return ProcessFinderPatternInfo(image, info);
}

} // QRCode
} // ZXing
