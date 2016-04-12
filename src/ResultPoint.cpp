#include "ResultPoint.h"
#include <cmath>

namespace ZXing {

namespace {
/**
* @param pattern1 first pattern
* @param pattern2 second pattern
* @return distance between two points
*/
static float Distance(const ResultPoint& a, const ResultPoint& b)
{
	auto dx = a.x() - b.x();
	auto dy = a.y() - b.y();
	return std::sqrt(dx*dx + dy*dy);
}

/**
* Returns the z component of the cross product between vectors BC and BA.
*/
static float CrossProductZ(ResultPoint a, ResultPoint b, ResultPoint c)
{
	return (c.x() - b.x())*(a.y() - b.y()) - (c.y() - b.y())*(a.x() - b.x());
}

} // anonymous

void ResultPoint::OrderByBestPatterns(ResultPoint* patterns)
{
	// Find distances between pattern centers
	float zeroOneDistance = Distance(patterns[0], patterns[1]);
	float oneTwoDistance = Distance(patterns[1], patterns[2]);
	float zeroTwoDistance = Distance(patterns[0], patterns[2]);

	ResultPoint pointA;
	ResultPoint pointB;
	ResultPoint pointC;
	// Assume one closest to other two is B; A and C will just be guesses at first
	if (oneTwoDistance >= zeroOneDistance && oneTwoDistance >= zeroTwoDistance) {
		pointB = patterns[0];
		pointA = patterns[1];
		pointC = patterns[2];
	}
	else if (zeroTwoDistance >= oneTwoDistance && zeroTwoDistance >= zeroOneDistance) {
		pointB = patterns[1];
		pointA = patterns[0];
		pointC = patterns[2];
	}
	else {
		pointB = patterns[2];
		pointA = patterns[0];
		pointC = patterns[1];
	}

	// Use cross product to figure out whether A and C are correct or flipped.
	// This asks whether BC x BA has a positive z component, which is the arrangement
	// we want for A, B, C. If it's negative, then we've got it flipped around and
	// should swap A and C.
	if (CrossProductZ(pointA, pointB, pointC) < 0.0f) {
		ResultPoint temp = pointA;
		pointA = pointC;
		pointC = temp;
	}

	patterns[0] = pointA;
	patterns[1] = pointB;
	patterns[2] = pointC;
}

} // ZXing