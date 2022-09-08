/*
* Copyright 2020 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "BitMatrixCursor.h"
#include "Pattern.h"
#include "Quadrilateral.h"
#include "ZXAlgorithms.h"

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
	} else { // aztec
		auto a = std::accumulate(&pattern[N/2 + 1], &pattern[N], pattern[N/2] / 2.f);
		return end - a;
	}
}

template<typename Pattern, typename Cursor>
std::optional<Pattern> ReadSymmetricPattern(Cursor& cur, int range)
{
	if (!cur.stepToEdge(std::tuple_size<Pattern>::value / 2 + 1, range))
		return std::nullopt;

	cur.turnBack();
	cur.step();

	auto pattern = cur.template readPattern<Pattern>(range);
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

std::optional<QuadrilateralF> FindConcentricPatternCorners(const BitMatrix& image, PointF center, int range, int ringIndex);

struct ConcentricPattern : public PointF
{
	int size = 0;
};

template <bool RELAXED_THRESHOLD = false, typename FINDER_PATTERN>
std::optional<ConcentricPattern> LocateConcentricPattern(const BitMatrix& image, FINDER_PATTERN finderPattern, PointF center, int range)
{
	auto cur = BitMatrixCursorF(image, center, {});
	int minSpread = image.width(), maxSpread = 0;
	for (auto d : {PointF{0, 1}, {1, 0}}) {
		int spread = CheckDirection<RELAXED_THRESHOLD>(cur, d, finderPattern, range, !RELAXED_THRESHOLD);
		if (!spread)
			return {};
		minSpread = std::min(spread, minSpread);
		maxSpread = std::max(spread, maxSpread);
	}

#if 1
	for (auto d : {PointF{1, 1}, {1, -1}}) {
		int spread = CheckDirection<true>(cur, d, finderPattern, range, false);
		if (!spread)
			return {};
		minSpread = std::min(spread, minSpread);
		maxSpread = std::max(spread, maxSpread);
	}
#endif

	if (maxSpread > 5 * minSpread)
		return {};

	auto newCenter = FinetuneConcentricPatternCenter(image, cur.p, range, finderPattern.size());
	if (!newCenter)
		return {};

	return ConcentricPattern{*newCenter, (maxSpread + minSpread) / 2};
}

} // ZXing

