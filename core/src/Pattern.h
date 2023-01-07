/*
* Copyright 2020 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Range.h"
#include "ZXAlgorithms.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
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

	// A PatternRow always starts with the width of whitespace in front of the first black bar.
	// The first element of the PatternView is the first bar.
	PatternView(const PatternRow& bars)
		: _data(bars.data() + 1), _size(Size(bars) - 1), _base(bars.data()), _end(bars.data() + bars.size())
	{}

	PatternView(Iterator data, int size, Iterator base, Iterator end) : _data(data), _size(size), _base(base), _end(end) {}

	template <size_t N>
	PatternView(const std::array<value_type, N>& row) : _data(row.data()), _size(N)
	{}

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

	// index is the number of bars and spaces from the first bar to the current position
	int index() const { return narrow_cast<int>(_data - _base) - 1; }
	int pixelsInFront() const { return std::accumulate(_base, _data, 0); }
	int pixelsTillEnd() const { return std::accumulate(_base, _data + _size, 0) - 1; }
	bool isAtFirstBar() const { return _data == _base + 1; }
	bool isAtLastBar() const { return _data + _size == _end - 1; }
	bool isValid(int n) const { return _data && _data >= _base && _data + n <= _end; }
	bool isValid() const { return isValid(size()); }

	template<bool acceptIfAtFirstBar = false>
	bool hasQuietZoneBefore(float scale) const
	{
		return (acceptIfAtFirstBar && isAtFirstBar()) || _data[-1] >= sum() * scale;
	}

	template<bool acceptIfAtLastBar = true>
	bool hasQuietZoneAfter(float scale) const
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

	bool shift(int n)
	{
		return _data && ((_data += n) + _size <= _end);
	}

	bool skipPair()
	{
		return shift(2);
	}

	bool skipSymbol()
	{
		return shift(_size);
	}

	bool skipSingle(int maxWidth)
	{
		return shift(1) && _data[-1] <= maxWidth;
	}

	void extend()
	{
		_size = std::max(0, narrow_cast<int>(_end - _data));
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
	constexpr const value_type* data() const noexcept { return _data; }
	constexpr int size() const noexcept { return N; }
};

template <int N, int SUM>
using FixedSparcePattern = FixedPattern<N, SUM, true>;

template <bool RELAXED_THRESHOLD = false, int N, int SUM>
float IsPattern(const PatternView& view, const FixedPattern<N, SUM, false>& pattern, int spaceInPixel = 0,
				float minQuietZone = 0, float moduleSizeRef = 0)
{
	int width = view.sum(N);
	if (SUM > N && width < SUM)
		return 0;

	const float moduleSize = (float)width / SUM;

	if (minQuietZone && spaceInPixel < minQuietZone * moduleSize - 1)
		return 0;

	if (!moduleSizeRef)
		moduleSizeRef = moduleSize;

	// the offset of 0.5 is to make the code less sensitive to quantization errors for small (near 1) module sizes.
	// TODO: review once we have upsampling in the binarizer in place.
	const float threshold = moduleSizeRef * (0.5f + RELAXED_THRESHOLD * 0.25f) + 0.5f;

	for (int x = 0; x < N; ++x)
		if (std::abs(view[x] - pattern[x] * moduleSizeRef) > threshold)
			return 0;

	return moduleSize;
}

template <bool RELAXED_THRESHOLD = false, int N, int SUM>
float IsPattern(const PatternView& view, const FixedPattern<N, SUM, true>& pattern, int spaceInPixel = 0,
				float minQuietZone = 0, float moduleSizeRef = 0)
{
	// pattern contains the indices with the bars/spaces that need to be equally wide
	int width = 0;
	for (int x = 0; x < SUM; ++x)
		width += view[pattern[x]];

	const float moduleSize = (float)width / SUM;

	if (minQuietZone && spaceInPixel < minQuietZone * moduleSize - 1)
		return 0;

	if (!moduleSizeRef)
		moduleSizeRef = moduleSize;

	// the offset of 0.5 is to make the code less sensitive to quantization errors for small (near 1) module sizes.
	// TODO: review once we have upsampling in the binarizer in place.
	const float threshold = moduleSizeRef * (0.5f + RELAXED_THRESHOLD * 0.25f) + 0.5f;

	for (int x = 0; x < SUM; ++x)
		if (std::abs(view[pattern[x]] - moduleSizeRef) > threshold)
			return 0;

	return moduleSize;
}

template <int N, int SUM, bool IS_SPARCE>
bool IsRightGuard(const PatternView& view, const FixedPattern<N, SUM, IS_SPARCE>& pattern, float minQuietZone,
				  float moduleSizeRef = 0.f)
{
	int spaceInPixel = view.isAtLastBar() ? std::numeric_limits<int>::max() : *view.end();
	return IsPattern(view, pattern, spaceInPixel, minQuietZone, moduleSizeRef) != 0;
}

template<int LEN, typename Pred>
PatternView FindLeftGuard(const PatternView& view, int minSize, Pred isGuard)
{
	if (view.size() < minSize)
		return {};

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
						  float minQuietZone)
{
	return FindLeftGuard<LEN>(view, std::max(minSize, LEN),
							  [&pattern, minQuietZone](const PatternView& window, int spaceInPixel) {
								  return IsPattern(window, pattern, spaceInPixel, minQuietZone);
							  });
}

template <int LEN, int SUM>
std::array<int, LEN - 2> NormalizedE2EPattern(const PatternView& view)
{
	float moduleSize = static_cast<float>(view.sum(LEN)) / SUM;
	std::array<int, LEN - 2> e2e;

	for (int i = 0; i < LEN - 2; i++) {
		float v = (view[i] + view[i + 1]) / moduleSize;
		e2e[i] = int(v + .5f);
	}

	return e2e;
}

template <int LEN, int SUM>
std::array<int, LEN> NormalizedPattern(const PatternView& view)
{
	float moduleSize = static_cast<float>(view.sum(LEN)) / SUM;
	int err = SUM;
	std::array<int, LEN> is;
	std::array<float, LEN> rs;
	for (int i = 0; i < LEN; i++) {
		float v = view[i] / moduleSize;
		is[i] = int(v + .5f);
		rs[i] = v - is[i];
		err -= is[i];
	}

	if (std::abs(err) > 1)
		return {};

	if (err) {
		auto mi = err > 0 ? std::max_element(std::begin(rs), std::end(rs)) - std::begin(rs)
						  : std::min_element(std::begin(rs), std::end(rs)) - std::begin(rs);
		is[mi] += err;
		rs[mi] -= err;
	}

	return is;
}

template<typename I>
void GetPatternRow(Range<I> b_row, PatternRow& p_row)
{
	// TODO: if reactivating the bit-packed array (!ZX_FAST_BIT_STORAGE) should be of interest then the following code could be
	// considerably speed up by using a specialized variant along the lines of the old BitArray::getNextSetTo() function that
	// was removed between 1.4 and 2.0.

#if 0
	p_row.reserve(64);
	p_row.clear();

	auto lastPos = b_row.begin();
	if (*lastPos)
		p_row.push_back(0); // first value is number of white pixels, here 0

	for (auto p = b_row.begin() + 1; p < b_row.end(); ++p)
		if (bool(*p) != bool(*lastPos))
			p_row.push_back(p - std::exchange(lastPos, p));

	p_row.push_back(b_row.end() - lastPos);

	if (*lastPos)
		p_row.push_back(0); // last value is number of white pixels, here 0
#else
	p_row.resize(b_row.size() + 2);
	std::fill(p_row.begin(), p_row.end(), 0);

	auto bitPos = b_row.begin();
	auto intPos = p_row.data();

	if (*bitPos)
		intPos++; // first value is number of white pixels, here 0

	while (++bitPos < b_row.end()) {
		++(*intPos);
		intPos += bitPos[0] != bitPos[-1];
	}
	++(*intPos);

	if (bitPos[-1])
		intPos++;

	p_row.resize(intPos - p_row.data() + 1);
#endif
}

} // ZXing
