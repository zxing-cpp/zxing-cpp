/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
* Copyright 2017 Axel Waggershauser
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

//#ifndef NDEBUG
//#define PRINT_DEBUG
//#endif

#include "datamatrix/DMDetector.h"
#include "BitMatrix.h"
#include "DecodeStatus.h"
#include "DetectorResult.h"
#include "GridSampler.h"
#include "WhiteRectDetector.h"

#ifdef PRINT_DEBUG
#include "ByteMatrix.h"
#endif

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <map>
#include <numeric>

namespace ZXing {
namespace DataMatrix {

/**
* The following code is the 'old' code by Sean Owen based on the Java upstream project.
* It looks for a white rectangle, then cuts the corners until it hits a black pixel, which
* results in 4 corner points. Then it determines the dimension by counting transitions
* between the upper and right corners and samples the grid.
* This code has several limitations compared to the new code below but has one advantage:
* it works on high resolution scans with noisy/rippled black/white-edges and potentially
* on partly occluded locator patterns (the surrounding border of modules/pixels). It is
* therefore kept as a fall-back.
*/

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
static ResultPointsAndTransitions TransitionsBetween(const BitMatrix& image, const ResultPoint& from,
													 const ResultPoint& to)
{
	// See QR Code Detector, sizeOfBlackWhiteBlackRun()
	int fromX = static_cast<int>(from.x());
	int fromY = static_cast<int>(from.y());
	int toX = static_cast<int>(to.x());
	int toY = static_cast<int>(to.y());
	bool steep = std::abs(toY - fromY) > std::abs(toX - fromX);
	if (steep) {
		std::swap(fromX, fromY);
		std::swap(toX, toY);
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

inline static float RoundToNearest(float x)
{
#if defined(__ANDROID__) && defined(__GNUC__)
	return static_cast<float>(round(x));
#else
	return std::round(x);
#endif
}

/**
* Calculates the position of the white top right module using the output of the rectangle detector
* for a rectangular matrix
*/
static bool CorrectTopRightRectangular(const BitMatrix& image, const ResultPoint& bottomLeft,
									   const ResultPoint& bottomRight, const ResultPoint& topLeft,
									   const ResultPoint& topRight, int dimensionTop, int dimensionRight,
									   ResultPoint& result)
{
	float corr = RoundToNearest(ResultPoint::Distance(bottomLeft, bottomRight)) / static_cast<float>(dimensionTop);
	float norm = RoundToNearest(ResultPoint::Distance(topLeft, topRight));
	float cos = (topRight.x() - topLeft.x()) / norm;
	float sin = (topRight.y() - topLeft.y()) / norm;

	ResultPoint c1(topRight.x() + corr*cos, topRight.y() + corr*sin);

	corr = RoundToNearest(ResultPoint::Distance(bottomLeft, topLeft)) / (float)dimensionRight;
	norm = RoundToNearest(ResultPoint::Distance(bottomRight, topRight));
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

	int l1 = std::abs(dimensionTop - TransitionsBetween(image, topLeft, c1).transitions) +
			 std::abs(dimensionRight - TransitionsBetween(image, bottomRight, c1).transitions);
	int l2 = std::abs(dimensionTop - TransitionsBetween(image, topLeft, c2).transitions) +
			 std::abs(dimensionRight - TransitionsBetween(image, bottomRight, c2).transitions);

	result = l1 <= l2 ? c1 : c2;
	return true;
}

/**
* Calculates the position of the white top right module using the output of the rectangle detector
* for a square matrix
*/
static ResultPoint CorrectTopRight(const BitMatrix& image, const ResultPoint& bottomLeft, const ResultPoint& bottomRight,
                                   const ResultPoint& topLeft, const ResultPoint& topRight, int dimension)
{
	float corr = RoundToNearest(ResultPoint::Distance(bottomLeft, bottomRight)) / (float)dimension;
	float norm = RoundToNearest(ResultPoint::Distance(topLeft, topRight));
	float cos = (topRight.x() - topLeft.x()) / norm;
	float sin = (topRight.y() - topLeft.y()) / norm;

	ResultPoint c1(topRight.x() + corr * cos, topRight.y() + corr * sin);

	corr = RoundToNearest(ResultPoint::Distance(bottomLeft, topLeft)) / (float)dimension;
	norm = RoundToNearest(ResultPoint::Distance(bottomRight, topRight));
	cos = (topRight.x() - bottomRight.x()) / norm;
	sin = (topRight.y() - bottomRight.y()) / norm;

	ResultPoint c2(topRight.x() + corr * cos, topRight.y() + corr * sin);

	if (!IsValidPoint(c1, image.width(), image.height())) {
		if (!IsValidPoint(c2, image.width(), image.height()))
			return topRight;
		return c2;
	}
	if (!IsValidPoint(c2, image.width(), image.height()))
		return c1;

	int l1 = std::abs(TransitionsBetween(image, topLeft, c1).transitions -
					  TransitionsBetween(image, bottomRight, c1).transitions);
	int l2 = std::abs(TransitionsBetween(image, topLeft, c2).transitions -
					  TransitionsBetween(image, bottomRight, c2).transitions);
	return l1 <= l2 ? c1 : c2;
}

static BitMatrix SampleGrid(const BitMatrix& image, const ResultPoint& topLeft, const ResultPoint& bottomLeft,
							const ResultPoint& bottomRight, const ResultPoint& topRight, int dimensionX, int dimensionY)
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
		bottomLeft.y());
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
static void OrderByBestPatterns(const ResultPoint*& p0, const ResultPoint*& p1, const ResultPoint*& p2)
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

static DetectorResult DetectOld(const BitMatrix& image)
{
	ResultPoint pointA, pointB, pointC, pointD;
	if (!WhiteRectDetector::Detect(image, pointA, pointB, pointC, pointD)) {
		return {};
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
	std::sort(transitions.begin(), transitions.end(),
			  [](const ResultPointsAndTransitions& a, const ResultPointsAndTransitions& b) {
				  return a.transitions < b.transitions;
			  });

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
		return {};
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

	if (dimensionTop < 10 || dimensionTop > 144 || dimensionRight < 8 || dimensionRight > 144 )
		return {};

	ResultPoint correctedTopRight;

	// Rectanguar symbols are 6x16, 6x28, 10x24, 10x32, 14x32, or 14x44. If one dimension is more
	// than twice the other, it's certainly rectangular, but to cut a bit more slack we accept it as
	// rectangular if the bigger side is at least 7/4 times the other:
	if (4 * dimensionTop >= 7 * dimensionRight || 4 * dimensionRight >= 7 * dimensionTop) {
		// The matrix is rectangular

		if (!CorrectTopRightRectangular(image, *bottomLeft, *bottomRight, *topLeft, *topRight, dimensionTop,
										dimensionRight, correctedTopRight)) {
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
	}
	else {
		// The matrix is square

		int dimension = std::min(dimensionRight, dimensionTop);
		// correct top right point to match the white module
		correctedTopRight = CorrectTopRight(image, *bottomLeft, *bottomRight, *topLeft, *topRight, dimension);

		// Redetermine the dimension using the corrected top right point
		int dimensionCorrected = std::max(TransitionsBetween(image, *topLeft, correctedTopRight).transitions,
		                                  TransitionsBetween(image, *bottomRight, correctedTopRight).transitions);
		dimensionCorrected++;
		if ((dimensionCorrected & 0x01) == 1) {
			dimensionCorrected++;
		}

		dimensionTop = dimensionRight = dimension;
	}

	auto bits = SampleGrid(image, *topLeft, *bottomLeft, *bottomRight, correctedTopRight, dimensionTop, dimensionRight);
	if (bits.empty())
		return {};

	return {std::move(bits), {*topLeft, *bottomLeft, *bottomRight, correctedTopRight}};
}

/**
* The following code is the 'new' one implemented by Axel Waggershauser and is working completely different.
* It is performing something like a (back) trace search along edges through the bit matrix, first looking for
* the 'L'-pattern, then tracing the black/white borders at the top/right. Advantages over the old code are:
*  * works with lower resolution scans (around 2 pixel per module), due to sub-pixel precision grid placement
*  * works with real-world codes that have just one module wide quite-zone (which is perfectly in spec)
*/

template <typename T> struct PointT
{
	T x = 0, y = 0;
	PointT() = default;
	PointT(T x, T y) : x(x), y(y) {}
	template <typename U>
	explicit PointT(const PointT<U>& p) : x((T)p.x), y((T)p.y) {}
	explicit PointT(const ResultPoint& p) : x(p.x()), y(p.y()) {}
	operator ResultPoint() const { return {static_cast<float>(x), static_cast<float>(y)}; }
};

template <typename T> bool operator == (const PointT<T>& a, const PointT<T>& b)
{
	return a.x == b.x && a.y == b.y;
}

template <typename T> bool operator != (const PointT<T>& a, const PointT<T>& b)
{
	return !(a == b);
}

template <typename T, typename U> auto operator + (const PointT<T>& a, const PointT<U>& b) -> PointT<decltype(a.x + b.x)>
{
	return {a.x + b.x, a.y + b.y};
}

template <typename T, typename U> auto operator - (const PointT<T>& a, const PointT<U>& b) -> PointT<decltype(a.x - b.x)>
{
	return {a.x - b.x, a.y - b.y};
}

template <typename T, typename U> PointT<T> operator * (U s, const PointT<T>& a)
{
	return {s * a.x, s * a.y};
}

template <typename T, typename U> PointT<T> operator / (const PointT<T>& a, U d)
{
	return {a.x / d, a.y / d};
}

template <typename T, typename U> double operator * (const PointT<T>& a, const PointT<U>& b)
{
	return double(a.x) * b.x + a.y * b.y;
}

template <typename T> double distance(PointT<T> a, PointT<T> b)
{
	auto d = a - b;
	return std::sqrt(d * d);
}

using PointI = PointT<int>;
using PointF = PointT<double>;

template <typename T> PointF normalized(PointT<T> a)
{
	return PointF(a) / distance(a, {});
}

PointI round(PointF p)
{
    return PointI(::lround(p.x), ::lround(p.y));
}

class RegressionLine
{
	std::vector<PointI> _points;
	PointF _directionInward;
	double a = NAN, b = NAN, c = NAN;

	friend PointF intersect(const RegressionLine& l1, const RegressionLine& l2);

	bool evaluate(const std::vector<PointI> ps)
	{
		auto mean = std::accumulate(ps.begin(), ps.end(), PointF()) / ps.size();
		double sumXX = 0, sumYY = 0, sumXY = 0;
		for (auto& p : ps) {
			sumXX += (p.x - mean.x) * (p.x - mean.x);
			sumYY += (p.y - mean.y) * (p.y - mean.y);
			sumXY += (p.x - mean.x) * (p.y - mean.y);
		}
		if (sumYY >= sumXX) {
			a = +sumYY / std::sqrt(sumYY * sumYY + sumXY * sumXY);
			b = -sumXY / std::sqrt(sumYY * sumYY + sumXY * sumXY);
		} else {
			a = +sumXY / std::sqrt(sumXX * sumXX + sumXY * sumXY);
			b = -sumXX / std::sqrt(sumXX * sumXX + sumXY * sumXY);
		}
		if (_directionInward * normal() < 0) {
			a = -a;
			b = -b;
		}
		c = normal() * mean; // (a*mean.x + b*mean.y);
		return _directionInward * normal() > 0.5; // angle between original and new direction is at most 60 degree
	}

	template <typename Container, typename Filter>
	static double average(const Container& c, Filter f)
	{
		double sum = 0;
		int num = 0;
		for (const auto& v : c)
			if (f(v)) {
				sum += v;
				++num;
			}
		return sum / num;
	}

public:
	const std::vector<PointI>& points() const { return _points; }
	int length() const { return _points.size() >= 2 ? int(distance(_points.front(), _points.back())) : 0; }
	bool isValid() const { return !std::isnan(a); }
	PointF normal() const { return isValid() ? PointF(a, b) : _directionInward; }
	double signedDistance(PointI p) const { return normal() * p - c; }
	PointF project(PointI p) const { return p - signedDistance(p) * normal(); }

	void reverse() { std::reverse(_points.begin(), _points.end()); }

	void add(PointI p) {
		assert(_directionInward != PointF());
		_points.push_back(p);
		if (_points.size() == 1)
			c = normal() * p;
	}

	void pop_back() { _points.pop_back(); }

	void setDirectionInward(PointF d) { _directionInward = normalized(d); }

	bool evaluate(double maxDist = -1)
	{
		auto ps = _points;
		bool ret = evaluate(ps);
		if (maxDist > 0) {
			size_t old_points_size;
			while (true) {
				old_points_size = _points.size();
				_points.erase(std::remove_if(_points.begin(), _points.end(),
											 [this, maxDist](PointI p) { return this->signedDistance(p) > maxDist; }),
							  _points.end());
				if (old_points_size == _points.size())
					break;
#ifdef PRINT_DEBUG
				printf("removed %zu points\n", old_points_size - _points.size());
#endif
				ret = evaluate(_points);
			}
		}
		return ret;
	}

	double modules(PointF beg, PointF end) const
	{
		assert(_points.size() > 3);
		std::vector<double> gapSizes;
		gapSizes.reserve(_points.size());

		// calculate the distance between the points projected onto the regression line
		for (size_t i = 1; i < _points.size(); ++i)
			gapSizes.push_back(distance(project(_points[i]), project(_points[i - 1])));

		// calculate the (average) distance of two adjacent pixels
		auto unitPixelDist = average(gapSizes, [](double dist){ return 0.75 < dist && dist < 1.5; });

		// calculate the width of 2 modules (first black pixel to first black pixel)
		double sum = distance(beg, project(_points.front())) - unitPixelDist;
		auto i = gapSizes.begin();
		for (auto dist : gapSizes) {
			sum += dist;
			if (dist > 1.9 * unitPixelDist) {
				// c++14: *i++ = std::exchange(sum, 0.0);
				*i++ = sum;
				sum = 0.0;
			}
		}
		*i++ = sum + distance(end, project(_points.back()));
		gapSizes.erase(i, gapSizes.end());
		auto lineLength = distance(beg, end) - unitPixelDist;
		auto meanGapSize = lineLength / gapSizes.size();
#ifdef PRINT_DEBUG
		printf("unit pixel dist: %f\n", unitPixelDist);
		printf("lineLength: %f, meanGapSize: %f, gaps: %lu\n", lineLength, meanGapSize, gapSizes.size());
#endif
		meanGapSize = average(gapSizes, [&](double dist){ return std::abs(dist - meanGapSize) < meanGapSize/2; });
#ifdef PRINT_DEBUG
		printf("lineLength: %f, meanGapSize: %f, gaps: %lu\n", lineLength, meanGapSize, gapSizes.size());
#endif
		return lineLength / meanGapSize;
	}
};

PointF intersect(const RegressionLine& l1, const RegressionLine& l2)
{
	assert(l1.isValid() && l2.isValid());
	double x, y, d;
	d = l1.a * l2.b - l1.b * l2.a;
	x = (l1.c * l2.b - l1.b * l2.c) / d;
	y = (l1.a * l2.c - l1.c * l2.a) / d;
	return {x, y};
}

class EdgeTracer
{
	const BitMatrix& image;
	PointI p; // current position
	PointF d; // current direction

	static PointF mainDirection(PointF d)
	{
		assert(std::abs(d.x) != std::abs(d.y));
		return std::abs(d.x) > std::abs(d.y) ? PointF(d.x, 0) : PointF(0, d.y);
	}

	enum class StepResult { FOUND, OPEN_END, CLOSED_END };

	bool isIn(PointI p) const
	{
		const int b = 0;
		return  b <= p.x && p.x < image.width()-b &&
		        b <= p.y && p.y < image.height()-b;
	}
	bool isIn(PointF p) const { return isIn(round(p)); }
	bool isIn() const { return isIn(p); }

	class Value
	{
		enum { INVALID, WHITE, BLACK };
		int v = INVALID;
	public:
		Value() = default;
		Value(bool isBlack) : v(isBlack ? BLACK : WHITE) {}
		bool isValid() const { return v != INVALID; }
		bool isWhite() const { return v == WHITE; }
		bool isBlack() const { return v == BLACK; }
	};

	Value getAt(PointF p) const
	{
		auto q = round(p);
		if (!isIn(q))
			return {};
		return {image.get(q.x, q.y)};
	}

	bool blackAt(PointF p) const { return getAt(p).isBlack(); }
	bool whiteAt(PointF p) const { return getAt(p).isWhite(); }
	bool isEdge(PointF pos, PointF dir) const { return whiteAt(pos) && blackAt(pos + dir); }

	StepResult traceStep(PointF dEdge, int maxStepSize, bool goodDirection)
	{
		dEdge = mainDirection(dEdge);
		for (int breadth = 1; breadth <= (goodDirection ? 1 : (maxStepSize == 1 ? 2 : 3)); ++breadth)
			for (int step = 1; step <= maxStepSize; ++step)
				for (int i = 0; i <= 2*(step/4+1) * breadth; ++i) {
					auto pEdge = p + step * d + (i&1 ? (i+1)/2 : -i/2) * dEdge;
					log(round(pEdge));

					if (!blackAt(pEdge + dEdge))
						continue;

					// found black pixel -> go 'outward' until we hit the b/w border
					for (int j = 0; j < std::max(maxStepSize, 3) && isIn(pEdge); ++j) {
						if (whiteAt(pEdge)) {
							// if we are not making any progress, we still have another endless loop bug
							assert(p != round(pEdge));
							p = round(pEdge);
							return StepResult::FOUND;
						}
						pEdge = pEdge - dEdge;
						if (blackAt(pEdge - d))
							pEdge = pEdge - d;
						log(round(pEdge));
					}
					// no valid b/w border found within reasonable range
					return StepResult::CLOSED_END;
				}
		return StepResult::OPEN_END;
	}

public:
#ifdef PRINT_DEBUG
	static ByteMatrix _log;
	void log(const PointI& p) const
	{
		if (_log.height() != image.height() || _log.width() != image.width())
			_log = ByteMatrix(image.width(), image.height());
		if (isIn(p))
			_log.set(p.x, p.y, 1);
	}
#else
	void log(const PointI&) const {}
#endif

	EdgeTracer(const BitMatrix& img, PointF p, PointF d) : image(img), p(p), d(d) {}
	EdgeTracer& operator=(const EdgeTracer& other)
	{
		assert(&image == &other.image);
		p = other.p;
		d = other.d;
		return *this;
	}
	EdgeTracer(const EdgeTracer&) = default;
	~EdgeTracer() = default;
	EdgeTracer(EdgeTracer&&) noexcept(true) = default;
	EdgeTracer& operator=(EdgeTracer&&) = default;

	bool step(int s = 1)
	{
		p = round(p + s * d);
		log(p);
		return isIn(p);
	}

	void setDirection(PointF dir) { d = dir / std::max(std::abs(dir.x), std::abs(dir.y)); }

	bool updateDirectionFromOrigin(PointF origin)
	{
		auto old_d = d;
		setDirection(p - origin);
		// if the new direction is pointing "backward", i.e. angle(new, old) > 90 deg -> break
		if (d * old_d < 0)
			return false;
		// make sure d stays in the same quadrant to prevent an infinite loop
		if (std::abs(d.x) == std::abs(d.y))
			d = mainDirection(old_d) + 0.99 * (d - mainDirection(old_d));
		else if (mainDirection(d) != mainDirection(old_d))
			d = mainDirection(old_d) + 0.99 * mainDirection(d);
		return true;
	}

	PointF front() const { return d; }
	PointF back() const { return {-d.x, -d.y}; }
	PointF right() const { return {-d.y, d.x}; }
	PointF left() const { return {d.y, -d.x}; }

	bool isEdgeBehind() const { return isEdge(PointF(p), back()); }

	bool traceLine(PointF dEdge, RegressionLine& line)
	{
		line.setDirectionInward(dEdge);
		do {
			log(p);
			line.add(p);
			if (line.points().size() % 30 == 10) {
				if (!line.evaluate())
					return false;
				if (!updateDirectionFromOrigin(p - line.project(p) + line.points().front()))
					return false;
			}
			auto stepResult = traceStep(dEdge, 1, line.isValid());
			if (stepResult != StepResult::FOUND)
				return stepResult == StepResult::OPEN_END && line.points().size() > 1;
		} while (true);
	}

	bool traceGaps(PointF dEdge, RegressionLine& line, int maxStepSize, const RegressionLine& finishLine)
	{
		line.setDirectionInward(dEdge);
		int gaps = 0;
		do {
			assert(line.points().empty() || p != line.points().back());
			log(p);

			// if we drifted too far outside of the code, break
			if (line.isValid() && line.signedDistance(p) < -5 && (!line.evaluate() || line.signedDistance(p) < -5))
				return false;

			// if we are drifting towards the inside of the code, pull the current position back out onto the line
			if (line.isValid() && line.signedDistance(p) > 3) {
				// The current direction d and the line we are tracing are supposed to be roughly parallel.
				// In case the 'go outward' step in traceStep lead us astray, we might end up with a line
				// that is almost perpendicular to d. Then the back-projection below can result in an
				// endless loop. Break if the angle between d and line is greater than 45 deg.
				if (std::abs(normalized(d) * line.normal()) > 0.7) // thresh is approx. sin(45 deg)
					return false;

				auto np = line.project(p);
				// make sure we are making progress even when back-projecting:
				// consider a 90deg corner, rotated 45deg. we step away perpendicular from the line and get
				// back projected where we left off the line.
				if (distance(np, line.project(line.points().back())) < 1)
					np = np + d;
				p = round(np);
			}
			else {
				auto stepLengthInMainDir = line.points().empty() ? 0.0 : mainDirection(d) * (p - line.points().back());
				line.add(p);

				if (stepLengthInMainDir > 1) {
					++gaps;
					if (gaps >= 2 || line.points().size() > 5) {
						if (!line.evaluate(1.5))
							return false;
						if (!updateDirectionFromOrigin(p - line.project(p) + line.points().front()))
							return false;
						// check if the first half of the top-line trace is complete.
						// the minimum code size is 10x10 -> every code has at least 4 gaps
						//TODO: maybe switch to termination condition based on bottom line length to get a better
						// finishLine for the right line trace
						if (!finishLine.isValid() && gaps == 4) {
							// undo the last insert, it will be inserted again after the restart
							line.pop_back();
							--gaps;
							return true;
						}
					}
				} else if (gaps == 0 && line.points().size() >= 2u * maxStepSize)
					return false; // no point in following a line that has no gaps
			}

			if (finishLine.isValid())
				maxStepSize = std::min(maxStepSize, static_cast<int>(finishLine.signedDistance(p)));

			auto stepResult = traceStep(dEdge, maxStepSize, line.isValid());

			if (stepResult != StepResult::FOUND)
				// we are successful iff we found an open end across a valid finishLine
				return stepResult == StepResult::OPEN_END && finishLine.isValid() &&
					   static_cast<int>(finishLine.signedDistance(p)) <= maxStepSize + 1;
		} while (true);
	}

	bool traceCorner(PointF dir, PointF& corner)
	{
		step();
		corner = PointF(p);
		std::swap(d, dir);
		traceStep(-1 * dir, 2, false);
#ifdef PRINT_DEBUG
		printf("turn: %d x %d -> %.2f, %.2f\n", p.x, p.y, d.x, d.y);
#endif
		return isIn(corner) && isIn(p);
	}
};

#ifdef PRINT_DEBUG

ByteMatrix EdgeTracer::_log;

static void log(const std::vector<PointI>& points, int color = 2)
{
	for (auto p : points)
		EdgeTracer::_log.set(p.x, p.y, color);
}

static void dumpDebugPPM(const BitMatrix& image, const char* fn )
{
	FILE *f = fopen(fn, "wb");

	// Write PPM header, P5 == grey, P6 == rgb
	fprintf(f, "P6\n%d %d\n255\n", image.width(), image.height());

	// Write pixels
	for (int y = 0; y < image.height(); ++y)
		for (int x = 0; x < image.width(); ++x) {
			unsigned char r, g, b;
			r = g = b = image.get(x, y) ? 0 : 255;
			switch (EdgeTracer::_log.get(x, y)) {
			case 1: r = g = b = r ? 230 : 50; break;
			case 2: r = b = 50, g = 220; break;
			case 3: g = r = 100, b = 250; break;
			}
			fwrite(&r, 1, 1, f);
			fwrite(&g, 1, 1, f);
			fwrite(&b, 1, 1, f);
		}
	fclose(f);
}

static void printBitMatrix(const BitMatrix& matrix)
{
	for (int y = 0; y < matrix.height(); ++y) {
		for (int x = 0; x < matrix.width(); ++x)
			printf("%c ", matrix.get(x, y) ? '+' : '.');
		printf("\n");
	}
}
#endif

static BitMatrix SampleGrid(const BitMatrix& image, PointF tl, PointF bl, PointF br, PointF tr, int dimensionX,
							int dimensionY)
{
	auto moveTowardsBy = [](PointF& a, const PointF& b, double d) {
		auto a2b = normalized(b - a);
		a = a + d * a2b;
	};

	// shrink shape by half a pixel to go from center of white pixel outside of code to the edge between white and black
	moveTowardsBy(tl, br, 0.5);
	moveTowardsBy(br, tl, 0.5);
	moveTowardsBy(bl, tr, 0.5);
	// move the tr point a little less because the jagged top and right line tend to be statistically slightly
	// inclined toward the center anyway.
	moveTowardsBy(tr, bl, 0.3);

	// work around a missing 'round' in GridSampler.
	// TODO: the correct location for the rounding is after the transformation in GridSampler
	// but that would currently break other 2D encoders
	for (auto* p : {&tl, &bl, &br, &tr})
		*p = *p + PointF(0.5, 0.5);

	auto border = 0.f;

	return GridSampler::Instance()->sampleGrid(
		image,
		dimensionX, dimensionY,
		border, border,
		dimensionX - border, border,
		dimensionX - border, dimensionY - border,
		border,	dimensionY - border,
		(float)tl.x, (float)tl.y,
		(float)tr.x, (float)tr.y,
		(float)br.x, (float)br.y,
		(float)bl.x, (float)bl.y);
}

static DetectorResult DetectNew(const BitMatrix& image, bool tryRotate)
{
	// walk to the left at first
	for (auto startDirection : {PointF(-1, 0), PointF(1, 0), PointF(0, -1), PointF(0, 1)}) {
		EdgeTracer startTracer(image, PointF(image.width()/2, image.height()/2), startDirection);
		while (startTracer.step()) {
			// go forward until we reach a white/black border
			if (!startTracer.isEdgeBehind())
				continue;

#ifdef PRINT_DEBUG
#define continue { \
			printf("broke at %d\n", __LINE__); \
			for (auto* l : {&lineL, &lineB, &lineT, &lineR}) log(l->points()); \
			continue; \
		}
#endif

			PointF tl, bl, br, tr;
			RegressionLine lineL, lineB, lineR, lineT;

			auto t = startTracer;

			// follow left leg upwards
			t.setDirection(t.right());
			if (!t.traceLine(t.right(), lineL))
				continue;

			if (!t.traceCorner(t.right(), tl))
				continue;
			lineL.reverse();
			auto tlTracer = t;

			// follow left leg downwards
			t = startTracer;
			t.setDirection(tlTracer.right());
			if (!t.traceLine(t.left(), lineL))
				continue;

			if (!lineL.isValid())
				t.updateDirectionFromOrigin(tl);
			auto up = t.back();
			if (!t.traceCorner(t.left(), bl))
				continue;

			// follow bottom leg right
			if (!t.traceLine(t.left(), lineB))
				continue;

			if (!lineB.isValid())
				t.updateDirectionFromOrigin(bl);
			auto right = t.front();
			if (!t.traceCorner(t.left(), br))
				continue;

			auto lenL = distance(tl, bl) - 1;
			auto lenB = distance(bl, br) - 1;
			if (lenL < 10 || lenB < 10 || lenB < lenL / 4 || lenB > lenL * 8)
				continue;

			auto maxStepSize = static_cast<int>(lenB / 5 + 1); // datamatrix dim is at least 10x10

			// at this point we found a plausible L-shape and are now looking for the b/w pattern at the top and right:
			// follow top row right 'half way' (4 gaps), see traceGaps break condition with 'invalid' line
			tlTracer.setDirection(right);
			if (!tlTracer.traceGaps(tlTracer.right(), lineT, maxStepSize, RegressionLine()))
				continue;

			maxStepSize = std::min(lineT.length() / 3, static_cast<int>(lenL / 5)) * 2;

			// follow up until we reach the top line
			t.setDirection(up);
			if (!t.traceGaps(t.left(), lineR, maxStepSize, lineT))
				continue;

			if (!t.traceCorner(t.left(), tr))
				continue;

			auto lenT = distance(tl, tr) - 1;
			auto lenR = distance(tr, br) - 1;

			if (std::abs(lenT - lenB) / lenB > 0.5 || std::abs(lenR - lenL) / lenL > 0.5 ||
			        lineT.points().size() < 5 || lineR.points().size() < 5)
				continue;

			// continue top row right until we cross the right line
			if (!tlTracer.traceGaps(tlTracer.right(), lineT, maxStepSize, lineR))
				continue;

#ifdef PRINT_DEBUG
			printf("L: %f, %f ^ %f, %f > %f, %f (%d : %d : %d : %d)\n", bl.x, bl.y,
			       tl.x - bl.x, tl.y - bl.y, br.x - bl.x, br.y - bl.y, (int)lenL, (int)lenB, (int)lenT, (int)lenR);
#endif

			for (RegressionLine* l : {&lineL, &lineB, &lineT, &lineR})
				l->evaluate(1.0);

			// find the bounding box corners of the code with sub-pixel precision by intersecting the 4 border lines
			bl = intersect(lineB, lineL);
			tl = intersect(lineT, lineL);
			tr = intersect(lineT, lineR);
			br = intersect(lineB, lineR);

			int dimT, dimR;
			double fracT, fracR;
			auto splitDouble = [](double d, int* i, double* f) {
				*i = std::isnormal(d) ? static_cast<int>(d + 0.5) : 0;
				*f = std::isnormal(d) ? std::abs(d - *i) : INFINITY;
			};
			splitDouble(lineT.modules(tl, tr), &dimT, &fracT);
			splitDouble(lineR.modules(br, tr), &dimR, &fracR);

#ifdef PRINT_DEBUG
			printf("L: %f, %f ^ %f, %f > %f, %f ^> %f, %f\n", bl.x, bl.y,
			       tl.x - bl.x, tl.y - bl.y, br.x - bl.x, br.y - bl.y, tr.x, tr.y);
			printf("dim: %d x %d\n", dimT, dimR);
#endif

			// if we have an invalid rectangular data matrix dimension, we try to parse it by assuming a square
			// we use the dimension that is closer to an integral value
			if (dimT < 2 * dimR || dimT > 4 * dimR)
				dimT = dimR = fracR < fracT ? dimR : dimT;

			// the dimension is 2x the number of black/white transitions
			dimT *= 2;
			dimR *= 2;

			if (dimT < 10 || dimT > 144 || dimR < 8 || dimR > 144 )
				continue;

			auto bits = SampleGrid(image, tl, bl, br, tr, dimT, dimR);

#ifdef PRINT_DEBUG
			printf("modules top: %d, right: %d\n", dimT, dimR);
			printBitMatrix(bits);

			for (RegressionLine* l : {&lineL, &lineB, &lineT, &lineR})
				log(l->points());

			dumpDebugPPM(image, "binary.pnm");
#endif

			if (bits.empty())
				continue;

			return {std::move(bits), {tl, bl, br, tr}};
		}
		// reached border of image -> try next scan direction
#ifndef PRINT_DEBUG
		if (!tryRotate)
#endif
			break; // only test left direction
	}

#ifdef PRINT_DEBUG
	dumpDebugPPM(image, "binary.pnm");
#endif

	return {};
}

DetectorResult Detector::Detect(const BitMatrix& image, bool tryHarder, bool tryRotate)
{
	auto result = DetectNew(image, tryRotate);
	if (!result.isValid() && tryHarder)
		result = DetectOld(image);
	return result;
}

} // DataMatrix
} // ZXing
