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

#include "datamatrix/DMDetector.h"
#include "BitMatrix.h"
#include "DetectorResult.h"
#include "WhiteRectDetector.h"
#include "GridSampler.h"
#include "ErrorStatus.h"

#include <array>
#include <algorithm>
#include <map>

namespace ZXing {
namespace DataMatrix {

/**
* Simply encapsulates two points and a number of transitions between them.
*/
struct ResultPointsAndTransitions
{
	const ResultPoint* from;
	const ResultPoint* to;
	int transitions;
};

/**
* Counts the number of black/white transitions between two points, using something like Bresenham's algorithm.
*/
static ResultPointsAndTransitions TransitionsBetween(const BitMatrix& image, const ResultPoint& from, const ResultPoint& to)
{
	// See QR Code Detector, sizeOfBlackWhiteBlackRun()
	int fromX = static_cast<int>(from.x());
	int fromY = static_cast<int>(from.y());
	int toX = static_cast<int>(to.x());
	int toY = static_cast<int>(to.y());
	bool steep = std::abs(toY - fromY) > std::abs(toX - fromX);
	if (steep) {
		int temp = fromX;
		fromX = fromY;
		fromY = temp;
		temp = toX;
		toX = toY;
		toY = temp;
	}

	int dx = std::abs(toX - fromX);
	int dy = std::abs(toY - fromY);
	int error = -dx / 2;
	int ystep = fromY < toY ? 1 : -1;
	int xstep = fromX < toX ? 1 : -1;
	int transitions = 0;
	bool inBlack = image.get(steep ? fromY : fromX, steep ? fromX : fromY);
	for (int x = fromX, y = fromY; x != toX; x += xstep) {
		bool isBlack = image.get(steep ? y : x, steep ? x : y);
		if (isBlack != inBlack) {
			transitions++;
			inBlack = isBlack;
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
	return ResultPointsAndTransitions{ &from, &to, transitions };
}

inline static bool IsValidPoint(const ResultPoint& p, int imgWidth, int imgHeight)
{
	return p.x() >= 0 && p.x() < imgWidth && p.y() > 0 && p.y() < imgHeight;
}

/**
* Calculates the position of the white top right module using the output of the rectangle detector
* for a rectangular matrix
*/
static bool CorrectTopRightRectangular(const BitMatrix& image, const ResultPoint& bottomLeft, const ResultPoint& bottomRight, const ResultPoint& topLeft, const ResultPoint& topRight, int dimensionTop, int dimensionRight, ResultPoint& result)
{
	float corr = std::round(ResultPoint::Distance(bottomLeft, bottomRight)) / static_cast<float>(dimensionTop);
	float norm = std::round(ResultPoint::Distance(topLeft, topRight));
	float cos = (topRight.x() - topLeft.x()) / norm;
	float sin = (topRight.y() - topLeft.y()) / norm;

	ResultPoint c1(topRight.x() + corr*cos, topRight.y() + corr*sin);

	corr = std::round(ResultPoint::Distance(bottomLeft, topLeft)) / (float)dimensionRight;
	norm = std::round(ResultPoint::Distance(bottomRight, topRight));
	cos = (topRight.x() - bottomRight.x()) / norm;
	sin = (topRight.y() - bottomRight.y()) / norm;

	ResultPoint c2(topRight.x() + corr*cos, topRight.y() + corr*sin);

	if (!IsValidPoint(c1, image.width(), image.height())) {
		if (IsValidPoint(c2, image.width(), image.height())) {
			result = c2;
			return true;
		}
		return false;
	}
	if (!IsValidPoint(c2, image.width(), image.height())) {
		result = c1;
		return true;
	}

	int l1 = std::abs(dimensionTop - TransitionsBetween(image, topLeft, c1).transitions) + std::abs(dimensionRight - TransitionsBetween(image, bottomRight, c1).transitions);
	int l2 = std::abs(dimensionTop - TransitionsBetween(image, topLeft, c2).transitions) + std::abs(dimensionRight - TransitionsBetween(image, bottomRight, c2).transitions);

	result = l1 <= l2 ? c1 : c2;
	return true;
}

/**
* Calculates the position of the white top right module using the output of the rectangle detector
* for a square matrix
*/
static bool CorrectTopRight(const BitMatrix& image, const ResultPoint& bottomLeft, const ResultPoint& bottomRight, const ResultPoint& topLeft, const ResultPoint& topRight, int dimension, ResultPoint& result)
{
	float corr = std::round(ResultPoint::Distance(bottomLeft, bottomRight)) / (float)dimension;
	float norm = std::round(ResultPoint::Distance(topLeft, topRight));
	float cos = (topRight.x() - topLeft.x()) / norm;
	float sin = (topRight.y() - topLeft.y()) / norm;

	ResultPoint c1(topRight.x() + corr * cos, topRight.y() + corr * sin);

	corr = std::round(ResultPoint::Distance(bottomLeft, topLeft)) / (float)dimension;
	norm = std::round(ResultPoint::Distance(bottomRight, topRight));
	cos = (topRight.x() - bottomRight.x()) / norm;
	sin = (topRight.y() - bottomRight.y()) / norm;

	ResultPoint c2(topRight.x() + corr * cos, topRight.y() + corr * sin);

	if (!IsValidPoint(c1, image.width(), image.height())) {
		if (IsValidPoint(c2, image.width(), image.height())) {
			result = c2;
			return true;
		}
		return false;
	}
	if (!IsValidPoint(c2, image.width(), image.height())) {
		result = c1;
		return true;
	}

	int l1 = std::abs(TransitionsBetween(image, topLeft, c1).transitions - TransitionsBetween(image, bottomRight, c1).transitions);
	int l2 = std::abs(TransitionsBetween(image, topLeft, c2).transitions - TransitionsBetween(image, bottomRight, c2).transitions);
	result = l1 <= l2 ? c1 : c2;
	return true;
}

static ErrorStatus
SampleGrid(const BitMatrix& image, const ResultPoint& topLeft, const ResultPoint& bottomLeft, const ResultPoint& bottomRight, const ResultPoint& topRight, int dimensionX, int dimensionY, BitMatrix& result)
{
	return GridSampler::Instance()->sampleGrid(
		image,
		dimensionX,
		dimensionY,
		0.5f,
		0.5f,
		dimensionX - 0.5f,
		0.5f,
		dimensionX - 0.5f,
		dimensionY - 0.5f,
		0.5f,
		dimensionY - 0.5f,
		topLeft.x(),
		topLeft.y(),
		topRight.x(),
		topRight.y(),
		bottomRight.x(),
		bottomRight.y(),
		bottomLeft.x(),
		bottomLeft.y(),
		result);
}

/**
* Returns the z component of the cross product between vectors BC and BA.
*/
static float CrossProductZ(const ResultPoint& a, const ResultPoint& b, const ResultPoint& c)
{
	return (c.x() - b.x())*(a.y() - b.y()) - (c.y() - b.y())*(a.x() - b.x());
}

/**
* Orders an array of three ResultPoints in an order [A,B,C] such that AB is less than AC
* and BC is less than AC, and the angle between BC and BA is less than 180 degrees.
*
* @param patterns array of three {@code ResultPoint} to order
*/
void OrderByBestPatterns(const ResultPoint*& p0, const ResultPoint*& p1, const ResultPoint*& p2)
{
	// Find distances between pattern centers
	float zeroOneDistance = ResultPoint::Distance(*p0, *p1);
	float oneTwoDistance = ResultPoint::Distance(*p1, *p2);
	float zeroTwoDistance = ResultPoint::Distance(*p0, *p2);

	const ResultPoint* pointA;
	const ResultPoint* pointB;
	const ResultPoint* pointC;
	// Assume one closest to other two is B; A and C will just be guesses at first
	if (oneTwoDistance >= zeroOneDistance && oneTwoDistance >= zeroTwoDistance) {
		pointB = p0;
		pointA = p1;
		pointC = p2;
	}
	else if (zeroTwoDistance >= oneTwoDistance && zeroTwoDistance >= zeroOneDistance) {
		pointB = p1;
		pointA = p0;
		pointC = p2;
	}
	else {
		pointB = p2;
		pointA = p0;
		pointC = p1;
	}

	// Use cross product to figure out whether A and C are correct or flipped.
	// This asks whether BC x BA has a positive z component, which is the arrangement
	// we want for A, B, C. If it's negative, then we've got it flipped around and
	// should swap A and C.
	if (CrossProductZ(*pointA, *pointB, *pointC) < 0.0f) {
		std::swap(pointA, pointC);
	}

	p0 = pointA;
	p1 = pointB;
	p2 = pointC;
}

ErrorStatus
Detector::Detect(const BitMatrix& image, DetectorResult& result)
{
	ResultPoint pointA, pointB, pointC, pointD;
	ErrorStatus status = WhiteRectDetector::Detect(image, pointA, pointB, pointC, pointD);
	if (StatusIsError(status)) {
		return status;
	}

	// Point A and D are across the diagonal from one another,
	// as are B and C. Figure out which are the solid black lines
	// by counting transitions
	std::array<ResultPointsAndTransitions, 4> transitions = {
		TransitionsBetween(image, pointA, pointB),
		TransitionsBetween(image, pointA, pointC),
		TransitionsBetween(image, pointB, pointD),
		TransitionsBetween(image, pointC, pointD),
	};
	std::sort(transitions.begin(), transitions.end(), [](const auto& a, const auto& b) { return a.transitions < b.transitions; });

	// Sort by number of transitions. First two will be the two solid sides; last two
	// will be the two alternating black/white sides
	const auto& lSideOne = transitions[0];
	const auto& lSideTwo = transitions[1];

	// Figure out which point is their intersection by tallying up the number of times we see the
	// endpoints in the four endpoints. One will show up twice.
	std::map<const ResultPoint*, int> pointCount;
	pointCount[lSideOne.from] += 1;
	pointCount[lSideOne.to] += 1;
	pointCount[lSideTwo.from] += 1;
	pointCount[lSideTwo.to] += 1;

	const ResultPoint* bottomRight = nullptr;
	const ResultPoint* bottomLeft = nullptr;
	const ResultPoint* topLeft = nullptr;
	for (const auto& entry : pointCount) {
		if (entry.second == 2) {
			bottomLeft = entry.first; // this is definitely the bottom left, then -- end of two L sides
		}
		else {
			// Otherwise it's either top left or bottom right -- just assign the two arbitrarily now
			if (bottomRight == nullptr) {
				bottomRight = entry.first;
			}
			else {
				topLeft = entry.first;
			}
		}
	}

	if (bottomRight == nullptr || bottomLeft == nullptr || topLeft == nullptr) {
		return ErrorStatus::NotFound;
	}

	// Bottom left is correct but top left and bottom right might be switched
	// Use the dot product trick to sort them out
	OrderByBestPatterns(bottomRight, bottomLeft, topLeft);

	// Which point didn't we find in relation to the "L" sides? that's the top right corner
	const ResultPoint* topRight;
	if (pointCount.find(&pointA) == pointCount.end()) {
		topRight = &pointA;
	}
	else if (pointCount.find(&pointB) == pointCount.end()) {
		topRight = &pointB;
	}
	else if (pointCount.find(&pointC) == pointCount.end()) {
		topRight = &pointC;
	}
	else {
		topRight = &pointD;
	}

	// Next determine the dimension by tracing along the top or right side and counting black/white
	// transitions. Since we start inside a black module, we should see a number of transitions
	// equal to 1 less than the code dimension. Well, actually 2 less, because we are going to
	// end on a black module:

	// The top right point is actually the corner of a module, which is one of the two black modules
	// adjacent to the white module at the top right. Tracing to that corner from either the top left
	// or bottom right should work here.

	int dimensionTop = TransitionsBetween(image, *topLeft, *topRight).transitions;
	int dimensionRight = TransitionsBetween(image, *bottomRight, *topRight).transitions;

	if ((dimensionTop & 0x01) == 1) {
		// it can't be odd, so, round... up?
		dimensionTop++;
	}
	dimensionTop += 2;

	if ((dimensionRight & 0x01) == 1) {
		// it can't be odd, so, round... up?
		dimensionRight++;
	}
	dimensionRight += 2;

	auto bits = std::make_shared<BitMatrix>();
	ResultPoint correctedTopRight;

	// Rectanguar symbols are 6x16, 6x28, 10x24, 10x32, 14x32, or 14x44. If one dimension is more
	// than twice the other, it's certainly rectangular, but to cut a bit more slack we accept it as
	// rectangular if the bigger side is at least 7/4 times the other:
	if (4 * dimensionTop >= 7 * dimensionRight || 4 * dimensionRight >= 7 * dimensionTop) {
		// The matrix is rectangular

		if (!CorrectTopRightRectangular(image, *bottomLeft, *bottomRight, *topLeft, *topRight, dimensionTop, dimensionRight, correctedTopRight)) {
			correctedTopRight = *topRight;
		}

		dimensionTop = TransitionsBetween(image, *topLeft, correctedTopRight).transitions;
		dimensionRight = TransitionsBetween(image, *bottomRight, correctedTopRight).transitions;

		if ((dimensionTop & 0x01) == 1) {
			// it can't be odd, so, round... up?
			dimensionTop++;
		}

		if ((dimensionRight & 0x01) == 1) {
			// it can't be odd, so, round... up?
			dimensionRight++;
		}

		status = SampleGrid(image, *topLeft, *bottomLeft, *bottomRight, correctedTopRight, dimensionTop, dimensionRight, *bits);
		if (StatusIsError(status)) {
			return status;
		}

	}
	else {
		// The matrix is square

		int dimension = std::min(dimensionRight, dimensionTop);
		// correct top right point to match the white module
		if (!CorrectTopRight(image, *bottomLeft, *bottomRight, *topLeft, *topRight, dimension, correctedTopRight)) {
			correctedTopRight = *topRight;
		}

		// Redetermine the dimension using the corrected top right point
		int dimensionCorrected = std::max(TransitionsBetween(image, *topLeft, correctedTopRight).transitions, TransitionsBetween(image, *bottomRight, correctedTopRight).transitions);
		dimensionCorrected++;
		if ((dimensionCorrected & 0x01) == 1) {
			dimensionCorrected++;
		}

		status = SampleGrid(image, *topLeft, *bottomLeft, *bottomRight, correctedTopRight, dimensionCorrected, dimensionCorrected, *bits);
		if (StatusIsError(status)) {
			return status;
		}
	}
	result.setBits(bits);
	result.setPoints({ *topLeft, *bottomLeft, *bottomRight, correctedTopRight });
	return ErrorStatus::NoError;
}


} // DataMatrix
} // ZXing
