/*
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

#include "ResultPoint.h"
#include <cmath>

namespace ZXing {

namespace {

/**
* Returns the z component of the cross product between vectors BC and BA.
*/
static float CrossProductZ(ResultPoint a, ResultPoint b, ResultPoint c)
{
	return (c.x() - b.x())*(a.y() - b.y()) - (c.y() - b.y())*(a.x() - b.x());
}

} // anonymous

  /**
  * @param pattern1 first pattern
  * @param pattern2 second pattern
  * @return distance between two points
  */
float 
ResultPoint::Distance(const ResultPoint& a, const ResultPoint& b)
{
	auto dx = a.x() - b.x();
	auto dy = a.y() - b.y();
	return std::sqrt(dx*dx + dy*dy);
}

float ResultPoint::Distance(float aX, float aY, float bX, float bY)
{
	float dx = aX - bX;
	float dy = aY - bY;
	return std::sqrt(dx * dx + dy * dy);
}

float ResultPoint::Distance(int aX, int aY, int bX, int bY)
{
	int dx = aX - bX;
	int dy = aY - bY;
	return static_cast<float>(std::sqrt(dx * dx + dy * dy));
}

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
