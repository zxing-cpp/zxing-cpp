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

#include <array>

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

	inline constexpr Point topLeft() const noexcept { return at(0); }
	inline constexpr Point topRight() const noexcept { return at(1); }
	inline constexpr Point bottomRight() const noexcept { return at(2); }
	inline constexpr Point bottomLeft() const noexcept { return at(3); }
};

using QuadrilateralF = Quadrilateral<PointF>;
using QuadrilateralI = Quadrilateral<PointI>;

template <typename PointT = PointF>
Quadrilateral<PointT> Rectangle(int width, int height, typename PointT::value_t margin = 0)
{
	return {
		PointT{margin, margin}, {width - margin, margin}, {width - margin, height - margin}, {margin, height - margin}};
}

template <typename PointT>
bool IsConvex(const Quadrilateral<PointT>& poly)
{
	const int N = static_cast<int>(poly.size());
	bool sign = false;

	for(int i = 0; i < N; i++)
	{
		auto d1 = poly[(i + 2) % N] - poly[(i + 1) % N];
		auto d2 = poly[i] - poly[(i + 1) % N];
		auto cp = crossProduct(d1, d2);

		if (i == 0)
			sign = cp > 0;
		else if (sign != (cp > 0))
			return false;
	}

	return true;
}


} // ZXing

