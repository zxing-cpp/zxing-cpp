/*
* Copyright 2020 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Point.h"
#include "ZXAlgorithms.h"

#include <array>
#include <cmath>

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

	constexpr Point topLeft() const noexcept { return at(0); }
	constexpr Point topRight() const noexcept { return at(1); }
	constexpr Point bottomRight() const noexcept { return at(2); }
	constexpr Point bottomLeft() const noexcept { return at(3); }

	double orientation() const
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

	typename PointT::value_t m = INFINITY, M = 0;

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

	// It turns out being convex is not enough to prevent a "numerical instability"
	// that can cause the corners being projected inside the image boundaries but
	// some points near the corners being projected outside. This has been observed
	// where one corner is almost in line with two others. The M/m ratio is below 2
	// for the complete existing sample set. For very "skewed" QRCodes a value of
	// around 3 is realistic. A value of 14 has been observed to trigger the
	// instability.
	return M / m < 4.0;
}

template <typename PointT>
Quadrilateral<PointT> Scale(const Quadrilateral<PointT>& q, int factor)
{
	return {factor * q[0], factor * q[1], factor * q[2], factor * q[3]};
}

template <typename PointT>
PointT Center(const Quadrilateral<PointT>& q)
{
	return Reduce(q) / Size(q);
}

template <typename PointT>
Quadrilateral<PointT> RotatedCorners(const Quadrilateral<PointT>& q, int n = 1)
{
	Quadrilateral<PointT> res;
	std::rotate_copy(q.begin(), q.begin() + ((n + 4) % 4), q.end(), res.begin());
	return res;
}

template <typename PointT>
bool IsInside(const PointT& p, const Quadrilateral<PointT>& q)
{
	// Test if p is on the same side (right or left) of all polygon segments
	int pos = 0, neg = 0;
	for (int i = 0; i < Size(q); ++i)
		(cross(p - q[i], q[(i + 1) % Size(q)] - q[i]) < 0 ? neg : pos)++;
	return pos == 0 || neg == 0;
}

template <typename PointT>
bool HaveIntersectingBoundingBoxes(const Quadrilateral<PointT>& a, const Quadrilateral<PointT>& b)
{
	// TODO: this is only a quick and dirty approximation that works for the trivial standard cases
	bool x = b.topRight().x < a.topLeft().x || b.topLeft().x > a.topRight().x;
	bool y = b.bottomLeft().y < a.topLeft().y || b.topLeft().y > a.bottomLeft().y;
	return !(x || y);
}

} // ZXing

