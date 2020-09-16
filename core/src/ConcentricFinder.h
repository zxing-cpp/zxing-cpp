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

#include "BitMatrixCursor.h"
#include "Pattern.h"
#include "ZXContainerAlgorithms.h"

#include <optional>

namespace ZXing {

template <typename T, size_t N>
static float CenterFromEnd(const std::array<T, N>& pattern, float end)
{
	if (N == 5) {
		float a = pattern[4] + pattern[3] + pattern[2] / 2.f;
		float b = pattern[4] + (pattern[3] + pattern[2] + pattern[1]) / 2.f;
		float c = (pattern[4] + pattern[3] + pattern[2] + pattern[1] + pattern[0]) / 2.f;
		return end - (2 * a + b + c) / 4;
	} else if (N == 3) {
		float a = pattern[2] + pattern[1] / 2.f;
		float b = (pattern[2] + pattern[1] + pattern[0]) / 2.f;
		return end - (2 * a + b) / 3;
	}
}

template<typename Pattern>
std::optional<Pattern> ReadSymmetricPattern(BitMatrixCursorF& cur, int range)
{
	if (!cur.stepToEdge(std::tuple_size<Pattern>::value / 2 + 1, range))
		return std::nullopt;

	cur.turnBack();
	cur.step();

	auto pattern = cur.readPattern<Pattern>(range);
	if (pattern.back() == 0)
		return std::nullopt;
	return pattern;
}

template<bool RELAXED_THRESHOLD = false, typename FinderPattern>
int CheckDirection(BitMatrixCursorF& cur, PointF dir, FinderPattern finderPattern, int range, bool updatePosition)
{
	using Pattern = std::array<PatternView::value_type, finderPattern.size()>;

	auto pOri = cur.p;
	cur.setDirection(dir);
	auto pattern = ReadSymmetricPattern<Pattern>(cur, range);
	if (!pattern || !IsPattern<RELAXED_THRESHOLD>(*pattern, finderPattern))
		return 0;

	if (updatePosition)
		cur.step(CenterFromEnd(*pattern, 0.5) - 1);
	else
		cur.p = pOri;

	return Reduce(*pattern);
}

std::optional<PointF> CenterOfRing(const BitMatrix& image, PointI center, int range, int nth, bool requireCircle = true);

std::optional<PointF> FinetuneConcentricPatternCenter(const BitMatrix& image, PointF center, int range, int finderPatternSize);

struct ConcentricPattern : public PointF
{
	int size = 0;
};

template <bool RELAXED_THRESHOLD = false, typename FINDER_PATTERN>
std::optional<ConcentricPattern> LocateConcentricPattern(const BitMatrix& image, FINDER_PATTERN finderPattern, PointF center, int range)
{
	auto cur = BitMatrixCursorF(image, center, {});
	int minSpread = image.width(), maxSpread = 0;
	for (auto d : {PointF{0, 1}, {1, 0}, {1, 1}, {1, -1}}) {
		int spread =
			CheckDirection<RELAXED_THRESHOLD>(cur, d, finderPattern, range, length(d) < 1.1 && !RELAXED_THRESHOLD);
		if (!spread)
			return {};
		minSpread = std::min(spread, minSpread);
		maxSpread = std::max(spread, maxSpread);
	}

	if (maxSpread > 5 * minSpread)
		return {};

	auto newCenter = FinetuneConcentricPatternCenter(image, cur.p, range, finderPattern.size());
	if (!newCenter)
		return {};

	return ConcentricPattern{*newCenter, (maxSpread + minSpread) / 2};
}

} // ZXing

