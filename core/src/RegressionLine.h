#pragma once
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

#include "Point.h"

#include <algorithm>
#include <numeric>
#include <cmath>
#include <vector>

namespace ZXing {

class RegressionLine
{
protected:
	std::vector<PointF> _points;
	PointF _directionInward;
	PointF::value_t a = NAN, b = NAN, c = NAN;

	friend PointF intersect(const RegressionLine& l1, const RegressionLine& l2);

	bool evaluate(const std::vector<PointF>& ps)
	{
		auto mean = std::accumulate(ps.begin(), ps.end(), PointF()) / ps.size();
		PointF::value_t sumXX = 0, sumYY = 0, sumXY = 0;
		for (auto& p : ps) {
			auto d = p - mean;
			sumXX += d.x * d.x;
			sumYY += d.y * d.y;
			sumXY += d.x * d.y;
		}
		if (sumYY >= sumXX) {
			auto l = std::sqrt(sumYY * sumYY + sumXY * sumXY);
			a = +sumYY / l;
			b = -sumXY / l;
		} else {
			auto l = std::sqrt(sumXX * sumXX + sumXY * sumXY);
			a = +sumXY / l;
			b = -sumXX / l;
		}
		if (dot(_directionInward, normal()) < 0) {
			a = -a;
			b = -b;
		}
		c = dot(normal(), mean); // (a*mean.x + b*mean.y);
		return dot(_directionInward, normal()) > 0.5f; // angle between original and new direction is at most 60 degree
	}

public:
	RegressionLine() { _points.reserve(16); } // arbitrary but plausible start size (tiny performance improvement)

	const auto& points() const { return _points; }
	int length() const { return _points.size() >= 2 ? int(distance(_points.front(), _points.back())) : 0; }
	bool isValid() const { return !std::isnan(a); }
	PointF normal() const { return isValid() ? PointF(a, b) : _directionInward; }
	auto signedDistance(PointF p) const { return dot(normal(), p) - c; }
	PointF project(PointF p) const { return p - signedDistance(p) * normal(); }

	void reset()
	{
		_points.clear();
		_directionInward = {};
		a = b = c = NAN;
	}

	void add(PointF p) {
		assert(_directionInward != PointF());
		_points.push_back(p);
		if (_points.size() == 1)
			c = dot(normal(), p);
	}

	void pop_back() { _points.pop_back(); }

	void setDirectionInward(PointF d) { _directionInward = normalized(d); }

	bool evaluate(double maxSignedDist = -1)
	{
		bool ret = evaluate(_points);
		if (maxSignedDist > 0) {
			auto points = _points;
			while (true) {
				auto old_points_size = points.size();
				points.erase(
					std::remove_if(points.begin(), points.end(),
								   [this, maxSignedDist](auto p) { return this->signedDistance(p) > maxSignedDist; }),
					points.end());
				if (old_points_size == points.size())
					break;
#ifdef PRINT_DEBUG
				printf("removed %zu points\n", old_points_size - points.size());
#endif
				ret = evaluate(points);
			}
#ifdef PRINT_DEBUG
			_points = points;
#endif
		}
		return ret;
	}
};

inline PointF intersect(const RegressionLine& l1, const RegressionLine& l2)
{
	assert(l1.isValid() && l2.isValid());
	auto d = l1.a * l2.b - l1.b * l2.a;
	auto x = (l1.c * l2.b - l1.b * l2.c) / d;
	auto y = (l1.a * l2.c - l1.c * l2.a) / d;
	return {x, y};
}

} // ZXing

