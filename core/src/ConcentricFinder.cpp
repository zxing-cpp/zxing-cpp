/*
* Copyright 2020 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "ConcentricFinder.h"

#include "LogMatrix.h"
#include "RegressionLine.h"

namespace ZXing {

std::optional<PointF> AverageEdgePixels(BitMatrixCursorI cur, int range, int numOfEdges)
{
	PointF sum = {};
	for (int i = 0; i < numOfEdges; ++i) {
		if (!cur.isIn())
			return {};
		cur.stepToEdge(1, range);
		sum += centered(cur.p) + centered(cur.p + cur.back());
		log(cur.p + cur.back(), 2);
	}
	return sum / (2 * numOfEdges);
}

std::optional<PointF> CenterOfDoubleCross(const BitMatrix& image, PointI center, int range, int numOfEdges)
{
	PointF sum = {};
	for (auto d : {PointI{0, 1}, {1, 0}, {1, 1}, {1, -1}}) {
		auto avr1 = AverageEdgePixels({image, center,  d}, range, numOfEdges);
		auto avr2 = AverageEdgePixels({image, center, -d}, range, numOfEdges);
		if (!avr1 || !avr2)
			return {};
		sum += *avr1 + *avr2;
	}
	return sum / 8;
}

std::optional<PointF> CenterOfRing(const BitMatrix& image, PointI center, int range, int nth, bool requireCircle)
{
	BitMatrixCursorI cur(image, center, {0, 1});
	cur.stepToEdge(nth, range);
	cur.turnRight(); // move clock wise and keep edge on the right

	uint32_t neighbourMask = 0;
	auto start = cur.p;
	PointF sum = {};
	int n = 0;
	do {
		log(cur.p, 4);
		sum += centered(cur.p);
		++n;

		// find out if we come full circle around the center. 8 bits have to be set in the end.
		neighbourMask |= (1 << (4 + dot(bresenhamDirection(cur.p - center), PointI(1, 3))));

		if (!cur.stepAlongEdge(Direction::RIGHT))
			return {};

		// use L-inf norm, simply because it is a lot faster than L2-norm and sufficiently accurate
		if (maxAbsComponent(cur.p - center) > range || center == cur.p || n > 4 * 2 * range)
			return {};
	} while (cur.p != start);

	if (requireCircle && neighbourMask != 0b111101111)
		return {};

	return sum / n;
}

std::optional<PointF> CenterOfRings(const BitMatrix& image, PointI center, int range, int numOfRings)
{
	PointF sum = {};
	int n = 0;
	for (int i = 0; i < numOfRings; ++i) {
		auto c = CenterOfRing(image, center, range, i + 1);
		if (!c)
			return {};
		// TODO: decide whether this wheighting depending on distance to the center is worth it
		int weight = numOfRings - i;
		sum += weight * *c;
		n += weight;
	}
	return sum / n;
}

std::optional<PointF> FinetuneConcentricPatternCenter(const BitMatrix& image, PointF center, int range, int finderPatternSize)
{
	// make sure we have at least one path of white around the center
	if (!CenterOfRing(image, PointI(center), range, 1))
		return {};

	auto res = CenterOfRings(image, PointI(center), range, finderPatternSize / 2);
	if (!res || !image.get(*res))
		res = CenterOfDoubleCross(image, PointI(center), range, finderPatternSize / 2 + 1);
	if (!res || !image.get(*res))
		res = center;
	if (!res || !image.get(*res))
		return {};
	return res;
}

static std::vector<PointF> CollectRingPoints(const BitMatrix& image, PointF center, int range, int edgeIndex, bool backup)
{
	PointI centerI(center);
	BitMatrixCursorI cur(image, centerI, {0, 1});
	cur.stepToEdge(edgeIndex, range, backup);
	cur.turnRight(); // move clock wise and keep edge on the right/left depending on backup
	const auto edgeDir = backup ? Direction::LEFT : Direction::RIGHT;

	uint32_t neighbourMask = 0;
	auto start = cur.p;
	std::vector<PointF> points;
	points.reserve(4 * range);

	do {
		log(cur.p, 4);
		points.push_back(centered(cur.p));

		// find out if we come full circle around the center. 8 bits have to be set in the end.
		neighbourMask |= (1 << (4 + dot(bresenhamDirection(cur.p - centerI), PointI(1, 3))));

		if (!cur.stepAlongEdge(edgeDir))
			return {};

		// use L-inf norm, simply because it is a lot faster than L2-norm and sufficiently accurate
		if (maxAbsComponent(cur.p - center) > range || centerI == cur.p || Size(points) > 4 * 2 * range)
			return {};

	} while (cur.p != start);

	if (neighbourMask != 0b111101111)
		return {};

	return points;
}

static std::optional<QuadrilateralF> FitQadrilateralToPoints(PointF center, std::vector<PointF>& points)
{
	auto dist2Center = [c = center](auto a, auto b) { return distance(a, c) < distance(b, c); };
	// rotate points such that the first one is the furthest away from the center (hence, a corner)
	std::rotate(points.begin(), std::max_element(points.begin(), points.end(), dist2Center), points.end());

	std::array<const PointF*, 4> corners;
	corners[0] = &points[0];
	// find the oposite corner by looking for the farthest point near the oposite point
	corners[2] = std::max_element(&points[Size(points) * 3 / 8], &points[Size(points) * 5 / 8], dist2Center);

	// find the two in between corners by looking for the points farthest from the long diagonal
	auto dist2Diagonal = [l = RegressionLine(*corners[0], *corners[2])](auto a, auto b) { return l.distance(a) < l.distance(b); };
	corners[1] = std::max_element(&points[Size(points) * 1 / 8], &points[Size(points) * 3 / 8], dist2Diagonal);
	corners[3] = std::max_element(&points[Size(points) * 5 / 8], &points[Size(points) * 7 / 8], dist2Diagonal);

	std::array lines{RegressionLine{corners[0] + 1, corners[1]}, RegressionLine{corners[1] + 1, corners[2]},
					 RegressionLine{corners[2] + 1, corners[3]}, RegressionLine{corners[3] + 1, &points.back() + 1}};

	if (std::any_of(lines.begin(), lines.end(), [](auto line) { return !line.isValid(); }))
		return {};

	QuadrilateralF res;
	for (int i = 0; i < 4; ++i)
		res[i] = intersect(lines[i], lines[(i + 1) % 4]);

	return res;
}

static bool QuadrilateralIsPlausibleSquare(const QuadrilateralF q, int lineIndex)
{
	double m, M;
	m = M = distance(q[0], q[3]);
	for (int i = 1; i < 4; ++i) {
		double d = distance(q[i - 1], q[i]);
		m = std::min(m, d);
		M = std::max(M, d);
	}

	return m >= lineIndex * 2 && m > M / 3;
}

std::optional<QuadrilateralF> FindConcentricPatternCorners(const BitMatrix& image, PointF center, int range, int lineIndex)
{
	auto innerPoints = CollectRingPoints(image, center, range, lineIndex, false);
	auto outerPoints = CollectRingPoints(image, center, range, lineIndex + 1, true);

	if (innerPoints.empty() || outerPoints.empty())
		return {};

	auto oInnerCorners = FitQadrilateralToPoints(center, innerPoints);
	if (!oInnerCorners || !QuadrilateralIsPlausibleSquare(*oInnerCorners, lineIndex))
		return {};

	auto oOuterCorners = FitQadrilateralToPoints(center, outerPoints);
	if (!oOuterCorners || !QuadrilateralIsPlausibleSquare(*oOuterCorners, lineIndex))
		return {};

	auto& innerCorners = *oInnerCorners;
	auto& outerCorners = *oOuterCorners;

	auto dist2First = [c = innerCorners[0]](auto a, auto b) { return distance(a, c) < distance(b, c); };
	// rotate points such that the the two topLeft points are closest to each other
	std::rotate(outerCorners.begin(), std::min_element(outerCorners.begin(), outerCorners.end(), dist2First), outerCorners.end());

	QuadrilateralF res;
	for (int i=0; i<4; ++i)
		res[i] = (innerCorners[i] + outerCorners[i]) / 2;

	for (auto p : innerCorners)
		log(p, 3);

	for (auto p : outerCorners)
		log(p, 3);

	for (auto p : res)
		log(p, 3);

	return res;
}

} // ZXing
