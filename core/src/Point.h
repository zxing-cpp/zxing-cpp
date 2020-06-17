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
PointT<T> operator*(U s, const PointT<T>& a)
{
	return {s * a.x, s * a.y};
}

template <typename T, typename U>
PointT<T> operator/(const PointT<T>& a, U d)
{
	return {a.x / d, a.y / d};
}

template <typename T, typename U>
double operator*(const PointT<T>& a, const PointT<U>& b)
{
	return double(a.x) * b.x + a.y * b.y;
}

template <typename T>
double distance(PointT<T> a, PointT<T> b)
{
	auto d = a - b;
	return std::sqrt(d * d);
}

template <typename T>
double crossProduct(PointT<T> a, PointT<T> b)
{
	return a.x * b.y - b.x * a.y;
}

using PointI = PointT<int>;
using PointF = PointT<double>;

template <typename T>
PointF normalized(PointT<T> a)
{
	return PointF(a) / distance(a, {});
}

inline PointI round(PointF p)
{
	return PointI(::lround(p.x), ::lround(p.y));
}

} // ZXing

