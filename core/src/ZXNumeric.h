#pragma once
/*
* Copyright 2016 Huy Cuong Nguyen
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
#include <limits>
#include <cstdint>
#include <type_traits>

namespace ZXing {

static const float kPi = 3.14159265358979323846f;
static const float kPi2 = 1.57079632679489661923f;
static const float kPi4 = 0.785398163397448309616f;
static const float kEpsilon = std::numeric_limits<float>::epsilon() * 10;
static const float kDegPerRad = 180.0f/ kPi;
static const float kRadPerDeg = kPi / 180.0f;
static const float kDeg2Rad = kRadPerDeg;
static const float kRad2Deg = kDegPerRad;
static const float kInfinity = std::numeric_limits<float>::infinity();
static const float kTinyDistance = 1e-4f;

/// Test the 'closeness' of two numbers
template <typename T>
typename std::enable_if<std::is_floating_point<T>::value, bool>::type
FuzzyEqual(T a, T b, T tolerance = std::numeric_limits<T>::epsilon() * 10)
{
	T fa = std::fabs(a);
	T fb = std::fabs(b);
	return std::fabs(a - b) <= tolerance * (fa > fb ? fa : fb);
}

/// Test the equality of two numbers with a fixed tolerance
template <typename T>
typename std::enable_if<std::is_floating_point<T>::value, bool>::type
IsEqual(T a, T b, T tolerance = std::numeric_limits<T>::epsilon() * 10)
{
	return std::fabs(a - b) <= tolerance;
}

template <typename T>
typename std::enable_if<std::is_integral<T>::value, bool>::type
IsEqual(T a, T b)
{
	return a == b;
}

/// Test if a number is == 0
template <typename T>
typename std::enable_if<std::is_floating_point<T>::value, bool>::type
IsZero(T a, T tolerance = std::numeric_limits<T>::epsilon() * 10)
{
	return std::fabs(a) <= tolerance;
}

template <typename T>
typename std::enable_if<std::is_integral<T>::value, bool>::type
IsZero(T a)
{
	return a == 0;
}

template <typename T>
inline T Clamp(T n, T lower, T upper)
{
	return n <= lower ? lower : n >= upper ? upper : n;
}

template <typename T>
typename std::enable_if<std::is_floating_point<T>::value, int>::type
RoundToNearest(T x)
{
	return static_cast<int>(std::round(x));
}

/// <summary>
/// Return -1 if x is negative, 1 if it's positve or 0 otherwise.
/// </summary>
template <typename T>
typename std::enable_if<std::is_floating_point<T>::value, T>::type
SignOf(T x)
{
	return T((x > 0) - (x < 0));
}

template <typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type
SignOf(T x)
{
	return (x >> (sizeof(T)*8 - 1)) | (T(-x) >> (sizeof(T)*8 - 1));
}

template <typename T>
typename std::enable_if<std::is_integral<T>::value, bool>::type
SameSign(T a, T b)
{
	return (a^b) >= 0;
}

template <typename T>
typename std::enable_if<std::is_floating_point<T>::value, bool>::type
SameSign(T a, T b)
{
	return a*b >= 0;
}

template <typename T>
typename std::enable_if<std::is_floating_point<T>::value, T>::type
HalfOf(T x)
{
	return T(0.5) * x;
}

} // namespace ZXing
