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
#include "ZXContainerAlgorithms.h"

#include <array>
#include <cmath>
#include <limits>

namespace ZXing {

template <typename T>
class Quadrilateral : public std::array<T, 4>
{
	using Base = std::array<T, 4>;
	using Base::at;
public:
	using Point = T;

	Quadrilateral() = default;
	Quadrilateral(T tl, T tr, T br, T bl) : Base{tl, tr, br, bl} {}
	template <typename U>
	Quadrilateral(PointT<U> tl, PointT<U> tr, PointT<U> br, PointT<U> bl)
		: Quadrilateral(Point(tl), Point(tr), Point(br), Point(bl))
	{}

	inline constexpr Point topLeft() const noexcept { return at(0); }
	inline constexpr Point topRight() const noexcept { return at(1); }
	inline constexpr Point bottomRight() const noexcept { return at(2); }
	inline constexpr Point bottomLeft() const noexcept { return at(3); }

	inline double orientation() const
	{
		auto centerLine = (topRight() + bottomRight()) - (topLeft() + bottomLeft());
		if (centerLine == Point{})
			return 0.;
		auto centerLineF = normalized(centerLine);
		return std::atan2(centerLineF.y, centerLineF.x);
	}
};

using QuadrilateralF = Quadrilateral<PointF>;
using QuadrilateralI = Quadrilateral<PointI>;

template <typename PointT = PointF>
Quadrilateral<PointT> Rectangle(int width, int height, typename PointT::value_t margin = 0)
{
	return {
		PointT{margin, margin}, {width - margin, margin}, {width - margin, height - margin}, {margin, height - margin}};
}

template <typename PointT = PointI>
Quadrilateral<PointT> Line(int y, int xStart, int xStop)
{
	return {PointT{xStart, y}, {xStop, y}, {xStop, y}, {xStart, y}};
}

template <typename PointT>
bool IsConvex(const Quadrilateral<PointT>& poly)
{
	const int N = Size(poly);
	bool sign = false;

	double m = std::numeric_limits<double>::max(), M = 0.0;

	for(int i = 0; i < N; i++)
	{
		auto d1 = poly[(i + 2) % N] - poly[(i + 1) % N];
		auto d2 = poly[i] - poly[(i + 1) % N];
		auto cp = cross(d1, d2);

		m = std::min(std::fabs(m), cp);
		M = std::max(std::fabs(M), cp);

		if (i == 0)
			sign = cp > 0;
		else if (sign != (cp > 0))
			return false;
	}

	// It turns out beeing convex is not enough to prevent a "numercial instability"
	// that can cause the corners beeing projected inside the image boundaries but
	// some points near the corners beeing projected outside. This has been observed
	// where one corner is almost in line with two others. The M/m ratio is below 2
	// for the complete existing sample set. For very "skewed" QRCodes a value of
	// around 3 is realistic. A value of 14 has been observed to trigger the
	// instability.
	return M / m < 4.0;
}


} // ZXing

