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

#include "ZXContainerAlgorithms.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <vector>

namespace ZXing {

using PatternRow = std::vector<uint16_t>;

class PatternView
{
	using Iterator = PatternRow::const_pointer;
	Iterator _data = nullptr;
	int _size = 0;
	Iterator _base = nullptr;
	Iterator _end = nullptr;

public:
	using value_type = PatternRow::value_type;

	PatternView() = default;
	PatternView(const PatternRow& bars)
		: _data(bars.data() + 1), _size(Size(bars)), _base(bars.data()), _end(bars.data() + bars.size())
	{}
	PatternView(Iterator data, int size, Iterator base, Iterator end) : _data(data), _size(size), _base(base), _end(end) {}

	Iterator data() const { return _data; }
	Iterator begin() const { return _data; }
	Iterator end() const { return _data + _size; }

	value_type operator[](int i) const
	{
//		assert(i < _count);
		return _data[i];
	}

	int sum(int n = 0) const { return std::accumulate(_data, _data + (n == 0 ? _size : n), 0); }
	int size() const { return _size; }

	int index() const { return static_cast<int>(_data - _base); }
	int pixelsInFront() const { return std::accumulate(_base, _data, 0); }
	int pixelsTillEnd() const { return std::accumulate(_base, _data + _size, 0) - 1; }
	bool isAtFirstBar() const { return _data == _base + 1; }
	bool isAtLastBar() const { return _data + _size == _end - 1; }
	bool isValid() const { return _data && _data + _size <= _end; }
	bool isValid(int n) const { return _data && _data + n <= _end; }

	template<bool acceptIfAtFirstBar = false>
	bool hasQuiteZoneBefore(float scale) const
	{
		return (acceptIfAtFirstBar && isAtFirstBar()) || _data[-1] >= sum() * scale;
	}

	template<bool acceptIfAtLastBar = true>
	bool hasQuiteZoneAfter(float scale) const
	{
		return (acceptIfAtLastBar && isAtLastBar()) || _data[_size] >= sum() * scale;
	}

	PatternView subView(int offset, int size = 0) const
	{
//		if(std::abs(size) > count())
//			printf("%d > %d\n", std::abs(size), _count);
//		assert(std::abs(size) <= count());
		if (size == 0)
			size = _size - offset;
		else if (size < 0)
			size = _size - offset + size;
		return {begin() + offset, std::max(size, 0), _base, _end};
	}

	bool skipPair()
	{
		_data += 2;
		return isValid();
	}

	bool skipSymbol()
	{
		_data += _size;
		return isValid();
	}

	bool skipSingle(int maxWidth)
	{
		_data += 1;
		return _data <= _end && _data[-1] <= maxWidth;
	}
};

/**
 * @brief The BarAndSpace struct is a simple 2 element data structure to hold information about bar(s) and space(s).
 *
 * The operator[](int) can be used in combination with a PatternView
 */
template <typename T>
struct BarAndSpace
{
	using value_type = T;
	T bar = {}, space = {};
	// even index -> bar, odd index -> space
	T& operator[](int i) { return reinterpret_cast<T*>(this)[i & 1]; }
	T operator[](int i) const { return reinterpret_cast<const T*>(this)[i & 1]; }
	bool isValid() const { return bar != T{} && space != T{}; }
};

using BarAndSpaceI = BarAndSpace<PatternView::value_type>;

/**
 * @brief FixedPattern describes a compile-time constant (start/stop) pattern.
 *
 * @param N  number of bars/spaces
 * @param SUM  sum over all N elements (size of pattern in modules)
 * @param IS_SPARCE  whether or not the pattern contains '0's denoting 'wide' bars/spaces
 */
template <int N, int SUM, bool IS_SPARCE = false>
struct FixedPattern
{
	using value_type = PatternRow::value_type;
	value_type _data[N];
	constexpr value_type operator[](int i) const noexcept { return _data[i]; }
	constexpr int size() const noexcept { return N; }
};

template <int N, int SUM>
using FixedSparcePattern = FixedPattern<N, SUM, true>;

template <int N, int SUM>
float IsPattern(const PatternView& view, const FixedPattern<N, SUM, false>& pattern, int spaceInPixel = 0,
				float minQuiteZone = 0, float moduleSizeRef = 0.f)
{
	int width = view.sum(N);
	if (SUM > N && width < SUM)
		return 0;

	const float moduleSize = (float)width / SUM;

	if (minQuiteZone && spaceInPixel < minQuiteZone * moduleSize)
		return 0;

	if (!moduleSizeRef)
		moduleSizeRef = moduleSize;

	for (int x = 0; x < N; ++x)
		// the offset of 0.5 is to make the code less sensitive to quantization errors for small (near 1) module sizes.
		// TODO: review once we have upsampling in the binarizer in place.
		if (std::abs(view[x] - pattern[x] * moduleSize) > moduleSizeRef * 0.5f + 0.5f)
			return 0;

	return moduleSize;
}

template <int N, int SUM>
float IsPattern(const PatternView& view, const FixedPattern<N, SUM, true>& pattern, int spaceInPixel = 0,
				float minQuiteZone = 0, float moduleSizeRef = 0.f)
{
	// note: fully optimized with at compile-time known constants in pattern, this code
	// should be as fast as IsPattern in case it is called with a pattern without '0's.
	// As of gcc-9, this is not the case.

	int width = 0;
	for (int x = 0; x < N; ++x)
		width += view[x] * (pattern[x] > 0);

	const float moduleSize = (float)width / SUM;

	if (minQuiteZone && spaceInPixel < minQuiteZone * moduleSize)
		return 0;

	if (!moduleSizeRef)
		moduleSizeRef = moduleSize;

	for (int x = 0; x < N; ++x)
		if (pattern[x]) {
			if (std::abs(view[x] - pattern[x] * moduleSize) > moduleSizeRef / 2)
				return 0;
		} else {
			if (view[x] < 1.5f * moduleSizeRef)
				return 0;
		}

	return moduleSize;
}

template <int N, int SUM, bool IS_SPARCE>
bool IsRightGuard(const PatternView& view, const FixedPattern<N, SUM, IS_SPARCE>& pattern, float minQuiteZone,
				  float moduleSizeRef = 0.f)
{
	int spaceInPixel = view.isAtLastBar() ? std::numeric_limits<int>::max() : *view.end();
	return IsPattern(view, pattern, spaceInPixel, minQuiteZone, moduleSizeRef) != 0;
}

template<int LEN, typename Pred>
PatternView FindLeftGuard(const PatternView& view, int minSize, Pred isGuard)
{
	auto window = view.subView(0, LEN);
	if (window.isAtFirstBar() && isGuard(window, std::numeric_limits<int>::max()))
		return window;
	for (auto end = view.end() - minSize; window.data() < end; window.skipPair())
		if (isGuard(window, window[-1]))
			return window;
	return {};
}

template <int LEN, int SUM, bool IS_SPARCE>
PatternView FindLeftGuard(const PatternView& view, int minSize, const FixedPattern<LEN, SUM, IS_SPARCE>& pattern,
						  float minQuiteZone)
{
	return FindLeftGuard<LEN>(view, minSize, [&pattern, minQuiteZone](auto window, int spaceInPixel) {
		return IsPattern(window, pattern, spaceInPixel, minQuiteZone);
	});
}

} // ZXing
