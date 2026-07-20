/*
* Copyright 2020 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "ConcentricFinder.h"

#include "LogMatrix.h"
#include "RegressionLine.h"
#include "ZXAlgorithms.h"

namespace ZXing {

static std::optional<ConcentricPattern> AverageEdgePixels(BitMatrixCursorI cur, int range, int numOfEdges)
{
	PointF sum = {};
	PointF::value_t totalSteps = 0;
	for (int i = 0; i < numOfEdges; ++i) {
		int steps = cur.stepToEdge(1, range - totalSteps);
		if (!steps)
			return {};
		totalSteps += steps;
		sum += centered(cur.p) + centered(cur.p + cur.back());
		log(cur.p + cur.back(), 2);
	}
	return ConcentricPattern{sum / (2 * numOfEdges), totalSteps};
}

static std::optional<ConcentricPattern> CenterOfDoubleCross(const BitMatrix& image, PointI center, int width, int numOfEdges)
{
	PointF sumP = {};
	PointF::value_t sumS = 0;
	for (auto d : {PointI{0, 1}, {1, 0}, {1, 1}, {1, -1}}) {
		auto avr1 = AverageEdgePixels({image, center,  d}, width * 3 / 5, numOfEdges);
		auto avr2 = AverageEdgePixels({image, center, -d}, width * 3 / 5, numOfEdges);
		if (!avr1 || !avr2)
			return {};
		auto [m, M] = std::minmax(avr1->size, avr2->size);
		if (M > 1.2 * m + 1) // only accept if the two legs are very similar in size
			return {};
		sumP += *avr1 + *avr2;
		sumS += avr1->size + avr2->size;
	}
	return ConcentricPattern{sumP / 8, 2 * sumS / 8};
}

static std::vector<PointF> CollectRingPoints(const BitMatrix& image, PointF center, int width, int edgeIndex, bool backup);

std::optional<ConcentricPattern> CenterOfRing(const BitMatrix& image, PointI center, int width, int nth, bool requireCircle)
{
	auto meanRToSquareWidth = [](double r) { return 1.74 * r; }; // a square with mean radius r has a width of approx. 1.74*r

#if 0
	if (requireCircle) {
		// alternative implementation with the aim of discarding closed loops that are not all circle like (M > 5*m)
		auto points = CollectRingPoints(image, PointF(center), width, std::abs(nth), nth < 0);
		if (points.empty())
			return {};
		auto res = Reduce(points, PointF{}, std::plus{}) / Size(points);

		double m = width, M = 0, sumR = 0;
		for (auto p : points) {
			auto r = distance(p, res);
			UpdateMinMax(m, M, r);
			sumR += r;
		}

		if (M > 5 * m)
			return {};

		return ConcentricPattern{res, meanRToSquareWidth(sumR / Size(points))};
	}
#endif
	const int maxN = 4 * (width + 1) * 3 / 2; // upper limit for circumference of the ring
	const int maxR = requireCircle ? width + 1 : 2 * width;
	bool inner = nth < 0;
	nth = std::abs(nth);
	log(center, 3);
	BitMatrixCursorI cur(image, center, {1, 0});
	if (!cur.stepToEdge(nth, maxR, inner))
		return {};
	cur.turnRight(); // move clock wise and keep edge on the right/left depending on backup
	const auto edgeDir = inner ? Direction::LEFT : Direction::RIGHT;

	uint32_t neighbourMask = 0;
	auto start = cur.p;
	PointF sumP = {};
	double sumR = 0;
	int nP = 0;
#ifdef PRINT_DEBUG
	double sumR2 = 0, mR = maxR, MR = 0;
#endif
	do {
		log(cur.p, 4);
		sumP += centered(cur.p);
		++nP;

		// find out if we come full circle around the center. 8 bits have to be set in the end.
		neighbourMask |= (1 << (4 + dot(bresenhamDirection(cur.p - center), PointI(1, 3))));

		if (!cur.stepAlongEdge(edgeDir))
			return {};

		double r = distance(cur.p, center) + (inner ? 0.5 : -0.5); // add/subtract 0.5 to go from pixel center to pixel edge
		sumR += r;
#ifdef PRINT_DEBUG
		sumR2 += r * r;
		UpdateMinMax(mR, MR, r);
#endif
		if (r > maxR || center == cur.p || nP > maxN)
			return {};
	} while (cur.p != start);

	if (requireCircle && neighbourMask != 0b111101111)
		return {};

	auto meanP = sumP / nP;
	auto meanR = sumR / nP;
	double C = nP / meanR; // C is the number of edge pixels per unit length, for a perfect circle C = 2*pi ~ 6.28, for a square C = 8
	double centerMove = distance(meanP, PointF(center)) / width; // normalized distance between estimated and calculated center
#ifdef PRINT_DEBUG
	auto variation = std::sqrt(sumR2 / nP - meanR * meanR) / meanR;
	printf("CenterOfRing: center=(%4d,%4d), width=%3d, nth=%d, reqCircle=%d, n=%3d, meanR=%5.2f, var=%5.2f, C=%5.2f, mR=%4.1f, MR=%4.1f, "
		   "neighbourMask=%x -> res=(%5.1f,%5.1f), delta=%3.1f\n",
		   center.x*5, center.y*5, width, nth, requireCircle, nP, meanR, variation, C, mR, MR, neighbourMask, meanP.x*5, meanP.y*5, centerMove);
#endif
	// C > 12 means that the edge is very irregular, centerMove > 0.5 means the center moved more than half the estimated width of the ring
	if (requireCircle && (C > 12 || centerMove > 0.5))
		return {};

	return ConcentricPattern{meanP, meanRToSquareWidth(meanR)};
}

std::optional<ConcentricPattern> CenterOfRings(const BitMatrix& image, PointF center, int range, int numOfRings)
{
	int n = 1;
	PointF::value_t size = 0;
	PointF sum = center;
	for (int i = 2; i < numOfRings + 1; ++i) {
		auto c = CenterOfRing(image, PointI(center), range, i);
		if (!c) {
			if (n == 1)
				return {};
			else
				break;
		} else if (distance(*c, center) > range / numOfRings / 2) {
			return {};
		}

		sum += *c;
		size = c->size;
		n++;
	}
	return ConcentricPattern{sum / n, n == numOfRings ? size : 0};
}

static std::vector<PointF> CollectRingPoints(const BitMatrix& image, PointF center, int width, int edgeIndex, bool backup)
{
	PointI centerI(center);
	const int maxN = 4 * (width + 1) * 3 / 2; // upper limit for circumference of the ring
	const int maxR = width * 2; // TODO fix discrepancy between this and FindRind()
	BitMatrixCursorI cur(image, centerI, {1, 0});
	if (!cur.stepToEdge(edgeIndex, maxR, backup))
		return {};
	cur.turnRight(); // move clock wise and keep edge on the right/left depending on backup
	const auto edgeDir = backup ? Direction::LEFT : Direction::RIGHT;

	uint32_t neighbourMask = 0;
	auto start = cur.p;
	std::vector<PointF> points;
	points.reserve(maxN / 2);

	do {
		log(cur.p, 4);
		points.push_back(centered(cur.p));

		// find out if we come full circle around the center. 8 bits have to be set in the end.
		neighbourMask |= (1 << (4 + dot(bresenhamDirection(cur.p - centerI), PointI(1, 3))));

		if (!cur.stepAlongEdge(edgeDir))
			return {};

		if (distance(cur.p, centerI) > maxR || centerI == cur.p || Size(points) > maxN)
			return {};

	} while (cur.p != start);

	if (neighbourMask != 0b111101111)
		return {};

	return points;
}

static std::optional<QuadrilateralF> FitQuadrilateralToPoints(PointF center, std::vector<PointF>& points)
{
	auto dist2Center = [c = center](auto a, auto b) { return distance(a, c) < distance(b, c); };
	auto [minDistElem, maxDistElem] = std::minmax_element(points.begin(), points.end(), dist2Center);

	// check if points are on a circle: for a square the min/max ratio is 0.7, for a circle it is 1
	if (distance(center, *minDistElem) / distance(center, *maxDistElem) > 0.85)
		return {};

	// rotate points such that the first one is the furthest away from the center (hence, a corner)
	std::rotate(points.begin(), maxDistElem, points.end());

	std::array<const PointF*, 4> corners;
	corners[0] = &points[0];
	// find the opposite corner by looking for the farthest point near the opposite point
	corners[2] = std::max_element(&points[Size(points) * 3 / 8], &points[Size(points) * 5 / 8], dist2Center);

	// find the two in between corners by looking for the points farthest from the long diagonal
	auto dist2Diagonal = [l = RegressionLine(*corners[0], *corners[2])](auto a, auto b) { return l.distance(a) < l.distance(b); };
	corners[1] = std::max_element(&points[Size(points) * 1 / 8], &points[Size(points) * 3 / 8], dist2Diagonal);
	corners[3] = std::max_element(&points[Size(points) * 5 / 8], &points[Size(points) * 7 / 8], dist2Diagonal);

	std::array lines{RegressionLine{corners[0] + 1, corners[1]}, RegressionLine{corners[1] + 1, corners[2]},
					 RegressionLine{corners[2] + 1, corners[3]}, RegressionLine{corners[3] + 1, &points.back() + 1}};

	if (std::any_of(lines.begin(), lines.end(), [](const auto& line) { return !line.isValid(); }))
		return {};

	std::array<const PointF*, 4> beg = {corners[0] + 1, corners[1] + 1, corners[2] + 1, corners[3] + 1};
	std::array<const PointF*, 4> end = {corners[1], corners[2], corners[3], &points.back() + 1};

	// check if all points belonging to each line segment are sufficiently close to that line
	for (int i = 0; i < 4; ++i)
		for (const PointF* p = beg[i]; p != end[i]; ++p) {
			auto len = std::distance(beg[i], end[i]);
			if (len > 3 && lines[i].distance(*p) > std::max(1., std::min(8., len / 8.))) {
#ifdef PRINT_DEBUG
				printf("FitQuadrilateral failed: center=(%3.f,%3.f), i=%d, dist=%.2f > %.2f @ (%3.f,%3.f)\n", center.x, center.y, i,
					   lines[i].distance(*p), std::distance(beg[i], end[i]) / 8., p->x, p->y);
#endif
				return {};
			}
		}

	QuadrilateralF res;
	for (int i = 0; i < 4; ++i)
		res[i] = intersect(lines[i], lines[(i + 1) % 4]);

	return res;
}

static bool QuadrilateralIsPlausibleSquare(const QuadrilateralF q, int lineIndex)
{
	double m, M;
	m = M = distance(q[0], q[3]);
	for (int i = 1; i < 4; ++i)
		UpdateMinMax(m, M, distance(q[i - 1], q[i]));

	return m >= lineIndex * 2 && m > M / 3;
}

std::optional<QuadrilateralF> FitSquareToPoints(const BitMatrix& image, PointF center, int width, int lineIndex, bool backup)
{
	auto points = CollectRingPoints(image, center, width, lineIndex, backup);
	if (points.empty())
		return {};

	auto res = FitQuadrilateralToPoints(center, points);
	if (!res || !QuadrilateralIsPlausibleSquare(*res, lineIndex - backup))
		return {};

	return res;
}

std::optional<QuadrilateralF> FindConcentricPatternCorners(const BitMatrix& image, PointF center, int width, int ringIndex)
{
	auto innerCorners = FitSquareToPoints(image, center, width, ringIndex, false);
	if (!innerCorners)
		return {};

	auto outerCorners = FitSquareToPoints(image, center, width, ringIndex + 1, true);
	if (!outerCorners)
		return {};

	auto res = Blend(*innerCorners, *outerCorners);

	for (auto p : *innerCorners)
		log(p, 3);

	for (auto p : *outerCorners)
		log(p, 3);

	for (auto p : res)
		log(p, 3);

	return res;
}

std::optional<ConcentricPattern> FinetuneConcentricPatternCenter(const BitMatrix& image, PointF center, int width, int finderPatternSize)
{
	// make sure we have at least one path of white around the center
	if (auto res1 = CenterOfRing(image, PointI(center), width * 2 / 3, 1); res1 && image.get(*res1)) {
		// and then either at least one more ring around that
		if (auto res2 = CenterOfRings(image, *res1, width, finderPatternSize / 2); res2 && image.get(*res2)) {
			res2->size = res2->size * 7 / 5; // CenterOfRings only estimates the radius of the white ring
			return res2;
		}
		// or the center can be approximated by a square
		// TODO: With the following CenterOfDoubleCross() check, this currently saves no additional test cases
		// if (FitSquareToPoints(image, *res1, width, 1, false))
		// 	return res1;
		// TODO: this is currently only keeping #258 alive, evaluate if still worth it
		if (auto res2 = CenterOfDoubleCross(image, PointI(*res1), width, finderPatternSize / 2 + 1); res2 && image.get(*res2))
			return res2;
	}
	return {};
}

} // ZXing
