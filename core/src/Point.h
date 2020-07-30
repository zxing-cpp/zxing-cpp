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

#include <cassert>
#include <cmath>

namespace ZXing {

template <typename T>
struct PointT
{
	using value_t = T;
	T x = 0, y = 0;

	PointT() = default;
	PointT(T x, T y) : x(x), y(y) {}

	template <typename U>
	explicit PointT(const PointT<U>& p) : x(static_cast<T>(p.x)), y(static_cast<T>(p.y))
	{}
};

template <typename T>
bool operator==(const PointT<T>& a, const PointT<T>& b)
{
	return a.x == b.x && a.y == b.y;
}

template <typename T>
bool operator!=(const PointT<T>& a, const PointT<T>& b)
{
	return !(a == b);
}

template <typename T, typename U>
auto operator+(const PointT<T>& a, const PointT<U>& b) -> PointT<decltype(a.x + b.x)>
{
	return {a.x + b.x, a.y + b.y};
}

template <typename T, typename U>
auto operator-(const PointT<T>& a, const PointT<U>& b) -> PointT<decltype(a.x - b.x)>
{
	return {a.x - b.x, a.y - b.y};
}

template <typename T, typename U>
auto operator*(const PointT<T>& a, const PointT<U>& b) -> PointT<decltype(a.x * b.x)>
{
	return {a.x * b.x, a.y * b.y};
}

template <typename T, typename U>
PointT<T> operator*(U s, const PointT<T>& a)
{
	return {s * a.x, s * a.y};
}

template <typename T, typename U>
PointT<T> operator/(const PointT<T>& a, U d)
{
	return {a.x / d, a.y / d};
}

template <typename T>
double length(PointT<T> d)
{
	return std::sqrt(dot(d, d));
}

template <typename T>
double distance(PointT<T> a, PointT<T> b)
{
	return length(a - b);
}

template <typename T, typename U>
double dot(const PointT<T>& a, const PointT<U>& b)
{
	return double(a.x) * b.x + a.y * b.y;
}

template <typename T>
double cross(PointT<T> a, PointT<T> b)
{
	return a.x * b.y - b.x * a.y;
}

using PointI = PointT<int>;
using PointF = PointT<double>;

/// Calculate a floating point pixel coordiante representing the 'center' of the pixel.
/// This is sort of the inverse operation of the PointI(PointF) conversion constructor.
/// See also the documentation of the GridSampler API.
inline PointF centered(PointI p)
{
	return p + PointF(0.5, 0.5);
}

inline PointF centered(PointF p)
{
	return {std::floor(p.x) + 0.5, std::floor(p.y) + 0.5};
}

template <typename T>
PointF normalized(PointT<T> a)
{
	return PointF(a) / length(a);
}

inline PointF bresenhamDirection(PointF d)
{
	return d / std::fmax(std::fabs(d.x), std::fabs(d.y));
}

inline PointF mainDirection(PointF d)
{
	assert(std::fabs(d.x) != std::fabs(d.y));
	return std::fabs(d.x) > std::fabs(d.y) ? PointF(d.x, 0) : PointF(0, d.y);
}

inline PointF movedTowardsBy(PointF a, PointF b, double d)
{
	return a + d * normalized(b - a);
}

} // ZXing

