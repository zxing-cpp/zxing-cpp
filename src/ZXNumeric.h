#pragma once

#include <cmath>
#include <limits>
#include <cstdint>
#include <type_traits>

namespace ZXing {

static const float kPi = NB_REAL(3.14159265358979323846);
static const float kPi2 = NB_REAL(1.57079632679489661923);
static const float kPi4 = NB_REAL(0.785398163397448309616);
static const float kEpsilon = std::numeric_limits<float>::epsilon() * 10;
static const float kDegPerRad = NbReal(180.)/ kPi;
static const float kRadPerDeg = kPi / NbReal(180.);
static const float kDeg2Rad = kRadPerDeg;
static const float kRad2Deg = kDegPerRad;
static const float kInfinity = std::numeric_limits<float>::infinity();
static const float kTinyDistance = NB_REAL(1e-4);

/// Test the 'closeness' of two numbers
template <typename T>
typename std::enable_if<std::is_floating_point<T>::value, bool>::type
FuzzyEqual(T a, T b, T tolerance = std::numeric_limits<T>::epsilon() * 10)
{
	NbReal fa = std::fabs(a);
	NbReal fb = std::fabs(b);
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

/// <summary>
/// Round to neares integer.
/// Note that if 'x' is equally close to its two nearest integers,
/// this function returns the greater of the two independently x is positive
/// or negative. In other words, 3.5 will return 4 and -3.5 will return -3.
/// This behavior is intended. If it's not what you want, consider using
/// other way.
/// </summary>
template <typename T>
typename std::enable_if<std::is_floating_point<T>::value, int>::type
RoundToNearest(T x)
{
	return static_cast<int>(std::floor(x + T(0.5)));
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
