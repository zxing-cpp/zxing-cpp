/*
* Copyright 2020 Axel Waggershauser
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

#include "ConcentricFinder.h"

#include "LogMatrix.h"

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
	auto res = CenterOfRings(image, PointI(center), range, finderPatternSize / 2);
	if (!res || !image.get(*res))
		res = CenterOfDoubleCross(image, PointI(center), range, finderPatternSize / 2 + 1);
	if (!res || !image.get(*res))
		res = center;
	return res;
}

} // ZXing
