// Copyright 2026 Axel Waggershauser
// SPDX-License-Identifier: Apache-2.0

#include "LocalGrid.h"

#include "Log.h"
#include "StdGenerator.h"
#include "ZXAlgorithms.h"
#include "ZXConfig.h"

#include <algorithm>
#include <optional>
#include <ranges>
#include <span>
#include <vector>

namespace ZXing {

// returns the average of the largest cluster of values where cluster is defined as values that are within threshold of each other
// and whether there were values outside of that cluster
static std::tuple<double, bool> clusterAvg(std::ranges::range auto& v, double threshold)
{
	std::ranges::sort(v);
	size_t bestStart = 0, bestLen = 0, start = 0;
	for (size_t end = 0; end < v.size(); ++end) {
		while (v[end] - v[start] >= threshold)
			++start;
		if (end - start + 1 > bestLen) {
			bestLen = end - start + 1;
			bestStart = start;
		}
	}
	double sum = 0;
	for (size_t i = bestStart; i < bestStart + bestLen; ++i)
		sum += v[i];
	sum /= bestLen;

	log_r("    clusterAvg: ", "%4.1f ", v, "");
	log_l(" with threshold: %4.1f -> len: %zu (of %zu)       -> delta: %4.1f", threshold, bestLen, v.size(), sum);

	return {sum, bestLen < v.size()};
};

// the number of pixels in a run of pixels with the same color, given the steps to the edges in both directions (0 if unknown)
static int runLength(int stepsPos, int stepsNeg)
{
	return stepsPos && stepsNeg ? stepsPos + stepsNeg - 1 : 0;
}

std::optional<double> LocalGrid::estimateModSize(PointF dir, double modSize, int limit, int last,
												 const std::span<const PointI> offsets) const
{
	// edge-to-edge distances are quantized to whole pixels, so they can't refine the module size of (near) 1px/module symbols
	if (last <= 0 || modSize < 1.5)
		return std::nullopt;

	ZX_THREAD_LOCAL std::vector<int> e2e;
	e2e.clear();

	// Go from center (start) outward in direction step (dir) and collect edge-to-edge distances.
	// Make at least 2*modSize steps, continue until next edge is found or limit steps are reached.
	// For each pair of adjacent runs, record the sum of their widths in e2e.
	// This is similar to BitMatrixCursor::readPattern but without a fixed pattern length.
	auto scanE2ERay = [this, modSize, limit, last](PointF start, PointF dir) {
		if (!img->isIn(start))
			return;

		const int minScan = std::ceil(last * modSize);
		log_t("\n    e2e ray from: (%4.0f, %4.0f), dir: (%5.2f, %5.2f), minScan: %d", start.x * 5, start.y * 5, dir.x, dir.y,
			  minScan);

		BitMatrixCursorF cursor(*img, start, dir);
		BitMatrixCursorF reverse(*img, start, -dir);
		int stepsPos = cursor.stepToEdge(1, limit, false, false);
		int stepsNeg = reverse.stepToEdge(1, limit, false, false);
		if (!stepsPos)
			return;

		int previousRun = runLength(stepsPos, stepsNeg);
		int scanned = stepsPos;
		while (scanned < limit) {
			int run = cursor.stepToEdge(1, limit - scanned, false, false);
			if (!run)
				break;

			scanned += run;
			if (previousRun) {
				log_t(" | %d + %d = %d", previousRun, run, previousRun + run);
				if (previousRun + run > limit)
					break;
				e2e.push_back(previousRun + run);
				if (scanned >= minScan)
					break;
			}
			previousRun = run;
		}
	};

	for (auto o : offsets) {
		if (length(o) > 1)
			continue;
		auto oDir = normalized(o.x * stepX + o.y * stepY);
		if (dot(oDir, dir) > 0.99)
			scanE2ERay(getPos(), dir);
		else if (dot(oDir, dir) < -0.99)
			scanE2ERay(getPos(), -dir);
		else {
			for (int i = 1; i <= last; ++i) {
				scanE2ERay(getPos(i * o), -dir);
				scanE2ERay(getPos(i * o), dir);
			}
		}
	}
	log_r("\n    e2e:", "%3d, ", e2e);

	auto fitModSize = [](const std::vector<int>& e2e, double estimate) {
		double bestMS = estimate;
		double bestErr = INFINITY;
		const double spread = Size(e2e) > 1 ? 0.75 : (estimate - 0.5) / estimate;
		const double minMS = estimate * spread, maxMS = estimate / spread;
		int bestN = 0;

		// Huber loss with delta 1
		auto huberLoss = [](double d) { return std::abs(d) < 1 ? 0.5 * d * d : std::abs(d) - 0.5; };

		for (double ms = minMS; ms <= maxMS; ms += estimate * 0.01) {
			double err = 0, weightSum = 0;
			int n = 0, sumMods = 0;

			for (auto steps : e2e) {
				if (steps < estimate)
					continue;
				const int mods = std::lround(steps / ms);
				const double d = steps - mods * ms;
				const double weight = estimate / steps;
				err += weight * huberLoss(d);
				weightSum += weight;
				++n;
				sumMods += mods;
			}

			err /= weightSum > 0 ? weightSum : 1;
			double avgMods = (n > 0 ? static_cast<double>(sumMods) / n : 0) * 0.2;
			double dist = huberLoss(ms - estimate) * .5;
			// log_l("%.2f %.2f %.2f %.2f", ms, err, avgMods, err + avgMods + dist);
			err += avgMods + dist;

			if (err < bestErr) {
				bestErr = err;
				bestMS = ms;
				bestN = n;
			}
		}

		if (bestMS == minMS || bestMS == maxMS) {
			log_l("    best mod size is at the boundary: %.1f -> returning estimate", bestMS);
			bestMS = estimate;
		}

		log_l("    min err: %5.2f, best ms: %5.2f, n: %2d", bestErr, bestMS, bestN);
		return std::tuple{bestMS, bestN};
	};

	auto [bestMS, n] = fitModSize(e2e, modSize);
	if (n < Size(e2e) / 2 || std::abs(bestMS - modSize) <= modSize * 0.02)
		return std::nullopt;

	log_l("      adjusted mod size from %.1f to %.1f from %d blocks", modSize, bestMS, n);
	return bestMS;
}

// distance of n to the nearest position that is half a period s away from a multiple of s
static double nearestHalfResidual(double n, double s)
{
	return n - (std::round((n - s / 2) / s) * s + s / 2);
}

std::optional<double> LocalGrid::estimateOriginShift(PointF dir, double modSize, int limit, int first, int last,
													 const std::span<const PointI> offsets, bool trimToGrid) const
{
	ZX_THREAD_LOCAL std::vector<double> deltas;
	deltas.clear();
	deltas.reserve(offsets.size() * last + 1);
	log_t("    +dist -dist (steps mods C) > dist (lms) delta: ");
	for (int i = first; i <= last; ++i)
		for (auto offset : offsets) {
			auto p = center + i * offset;
			auto start = getPos(i * offset);
			auto startC = centered(start);
			if (!img->isIn(startC) || (trimToGrid && (p.x < 0 || p.y < 0 || p.x >= dim.x || p.y >= dim.y)))
				continue;
			int stepsPos = BitMatrixCursorF(*img, startC, dir).stepToEdge(1, limit, false, false);
			int stepsNeg = BitMatrixCursorF(*img, startC, -dir).stepToEdge(1, limit, false, false);
			if (!stepsPos && !stepsNeg)
				continue; // no edge found, no information
			// +0.5 because the center of the pixel is at .5, .5
			auto distPos = stepsPos ? stepsPos - dot(start - startC, dir) - 0.5 : 0;
			auto distNeg = stepsNeg ? stepsNeg - dot(start - startC, -dir) - 0.5 : 0;

			// signed distance to the nearest edge (preferably in positive direction)
			double dist = stepsPos ? distPos : stepsNeg ? -distNeg : 0.0;

			// if both edges are known and the run is short enough, use it to refine the local module size
			double localModSize = modSize;
			if (int run = runLength(stepsPos, stepsNeg); run && run / modSize < 6)
				localModSize = run / std::max(1.0, std::round(run / modSize));

			deltas.push_back(dist < 0 ? -nearestHalfResidual(-dist, localModSize) : nearestHalfResidual(dist, localModSize));
			log_t("\n      +%4.1f -%4.1f (%3d %4.1f %c) > %4.1f (%4.1f) %.1f", distPos, distNeg, runLength(stepsPos, stepsNeg),
				  runLength(stepsPos, stepsNeg) / modSize, img->get(startC) ? 'B' : 'W', dist, localModSize, deltas.back());

			if (i == 0)
				break; // only do the center point once
		}
	log_l();

	if (deltas.empty())
		return std::nullopt;

	auto [meanDelta, hasOutliers] = clusterAvg(deltas, std::max(1.0, modSize / 2));
	if (hasOutliers) {
		// wrap the outliers by one module and try again
		for (auto& di : deltas) {
			if (di < meanDelta - modSize / 2)
				di += modSize;
			else if (di > meanDelta + modSize / 2)
				di -= modSize;
		}
		std::tie(meanDelta, hasOutliers) = clusterAvg(deltas, std::max(1.0, modSize / 3));
	}
	return meanDelta;
}

bool LocalGrid::adjustOriginAndStep(Axis axis, int first, int last, const std::span<const PointI> offsets, bool trimToGrid)
{
	if (offsets.empty() || first > last)
		return false;

	auto& step = axis == Axis::X ? stepX : stepY;
	auto dir = bresenhamDirection(step);
	auto modSize = length(step) / length(dir); // mod size in terms of steps in the given direction
	int limit = int(8 * modSize);
	log_t(" %s: origin (%.1f %.1f) step (%.1f %.1f) (%.1f): ", axis == Axis::X ? "X" : "Y", origin.x * 5, origin.y * 5, step.x,
		  step.y, modSize);

	if (auto ms = estimateModSize(dir, modSize, limit, last, offsets)) {
		step = *ms / modSize * step;
		modSize = *ms;
	}

	auto shift = estimateOriginShift(dir, modSize, limit, first, last, offsets, trimToGrid);
	if (!shift)
		return false;
	log(origin, origin + *shift * dir, LOG_G, 2);
	origin += *shift * dir;
	log(origin, LOG_B);
	return true;
}

constexpr PointI X{1, 0}, Y{0, 1};

LocalGrid& LocalGrid::at(PointI p, PointI offset)
{
	center = p;
	origin = mod2Pix(centered(p));
	stepX = mod2Pix(centered(p) + PointF{1, 0}) - origin;
	stepY = mod2Pix(centered(p) + PointF{0, 1}) - origin;

	log_l("\nLocalGrid @ (%d %d), initial origin: (%.1f %.1f), offset: (%d %d), stepX: (%.1f %.1f), stepY: (%.1f %.1f)", center.x,
		  center.y, origin.x * 5, origin.y * 5, offset.x, offset.y, stepX.x, stepX.y, stepY.x, stepY.y);
	log(origin, LOG_B);

	// auto offsets = std::array{-X, -Y, X, Y}; // works better for DataMatrix (especially near the symbol edges)
	// auto offsets = std::array{-X - Y, X - Y, X + Y, -X + Y};
	auto offsets = std::array{-X, -X - Y, -Y, X - Y, X, X + Y, Y, -X + Y};

	// evaluate the image at origin + offset (allows to effectively work near the border of the symbol)
	moveBy(offset);
	adjustOriginAndStep(Axis::X, 0, 0, offsets);
	adjustOriginAndStep(Axis::Y, 0, 0, offsets);
	for (int i = 0; i < 2; ++i) {
		adjustOriginAndStep(Axis::X, 0, 2, offsets);
		adjustOriginAndStep(Axis::Y, 0, 2, offsets);
	}
	moveBy(-offset);
	log(origin, LOG_B, 2);
	log_l("LocalGrid @ (%d %d), resulting origin: (%.1f %.1f), offset: (%d %d), stepX: (%.1f %.1f), stepY: (%.1f %.1f)", center.x,
		  center.y, origin.x * 5, origin.y * 5, offset.x, offset.y, stepX.x, stepX.y, stepY.x, stepY.y);
	return *this;
}

bool LocalGrid::tuneFromTiming(PointI start, std::string_view dirs, int radius)
{
	// adjust origin and step with radius and only in the direction of the timing pattern
	auto allX = std::array{-X, X};
	auto allY = std::array{-Y, Y};
	auto dirX = std::span<const PointI>(allX).subspan(Contains(dirs, 'l') ? 0 : 1, Contains(dirs, 'l') + Contains(dirs, 'r'));
	auto dirY = std::span<const PointI>(allY).subspan(Contains(dirs, 'u') ? 0 : 1, Contains(dirs, 'u') + Contains(dirs, 'd'));

	log_l("tuneFromTiming: start=%dx%d dirs=%s radius=%d", start.x, start.y, std::string(dirs).c_str(), radius);
	moveBy(start);
	bool resX = adjustOriginAndStep(Axis::X, 0, radius, dirX, true);
	bool resY = adjustOriginAndStep(Axis::Y, 0, radius, dirY, true);
	moveBy(-start);
	return resX || resY;
}

bool LocalGrid::isTimingPatternCross(PointI p, bool isBlack, int radius, int errorThreshold)
{
	// TODO: look into replacing this with something along the lines of CheckSymmetricAztecCenterPattern
	auto wrapOffset = [&](int center, int offset, int dim) {
		int pos = center + offset;
		return pos < 0 ? radius - pos : (pos >= dim ? -(radius + (pos - dim) + 1) : offset);
	};

	int errors = 0;
	for (int r = 0; r <= radius; ++r) {
		auto check = [&](int x, int y) {
			x = wrapOffset(center.x, x, dim.x);
			y = wrapOffset(center.y, y, dim.y);
			auto d = PointI(x, y);
			errors += !findValue(p, 1, d, Value((x + y) % 2 == (isBlack ? 0 : 1)));
		};
		check(-r, 0);
		check(r, 0);
		check(0, -r);
		check(0, r);
		if (errors > errorThreshold)
			return false;
	}
	return errors <= errorThreshold;
};

std::optional<PointF> LocalGrid::findTimingPatternCross(bool isBlack, int radius)
{
	for (auto p : Spiral(3)) {
		// check if there is a timing pattern cross candidate centered at p with half the radius
		if (isTimingPatternCross(p, isBlack, radius / 2)) {
			auto backup = *this;
			origin = getPos(p);
			// adjust origin and step with full radius and only in the direction of the timing pattern
			tuneFromTiming({}, "lrud", radius);

			// check again, now with the full radius, to make sure we are correctly aligned to the timing pattern
			if (isTimingPatternCross(PointI{0, 0}, isBlack, radius))
				return origin;

			*this = backup;
		}
	}
	return {};
}

static PointI gridDir(char c)
{
	switch (c) {
	case 'l': return -X;
	case 'r': return X;
	case 'u': return -Y;
	case 'd': return Y;
	}
	assert(false && "Invalid direction character");
	return {};
}

std::optional<PointF> LocalGrid::find(std::span<const LinePattern> patterns, int radius)
{
	auto isPatternAt = [&](PointI p) {
		for (auto lp : patterns) {
			int last = lp.last ? lp.last : radius;
			// if (Contains(lp.dirs, 'c'))
			// 	for (auto q : Spiral(last)) {
			// 		int r = maxAbsComponent(q);
			// 		if (!findValue(p + lp.start, 1, q, Value(lp.pattern & (1 << (r % 2)))))
			// 			return false;
			// 	} else
			for (int i = lp.first; i <= last; ++i) {
				for (auto d : lp.dirs)
					if (!findValue(p + lp.start, i, gridDir(d), Value(lp.pattern & (1 << (i % 2)))))
						return false;
			}
		}
		return true;
	};

	for (auto p : Spiral(3)) {
		if (isPatternAt(p)) {
			auto backup = *this;
			origin = getPos(p);
			auto originAtPattern = origin;
			log(origin, LOG_G);
			log_l("\nfound pattern at (%.1f, %.1f)", origin.x * 5, origin.y * 5);
			for (auto lp : patterns | std::views::filter(&LinePattern::isTiming)) {
				if (tuneFromTiming(lp.start, lp.dirs, lp.last ? lp.last : radius))
					log_l("tuned from %s -> (%.1f, %.1f)", std::string(lp.dirs).c_str(), origin.x * 5, origin.y * 5);
			}
			if (originAtPattern != origin && !isPatternAt({0, 0})) {
				*this = backup;
				log_l("pattern lost after tuneFromTiming, reverting origin to (%.1f, %.1f)", origin.x * 5, origin.y * 5);
				return {};
			}
			log(origin, LOG_I, 2);
			log_l();
			return origin;
		}
	}
	log_l();
	return {};
}

std::optional<PointF> LocalGrid::findCorner(int radius, PointI outward)
{
	assert(std::abs(outward.x) == 1 || std::abs(outward.y) == 1);
	PointI dx = PointI{-outward.x, 0}, dy = PointI{0, -outward.y};
	for (auto p : Spiral(3)) {
		int quietZone = 0, symbolX = 0, symbolY = 0;
		auto q = p + outward;
		for (int r = 0; r <= radius; ++r) {
			quietZone += get(q + r * dx).isBlack() + get(q + r * dy).isBlack();
			symbolX += findValue(p, r, dx, Value(true));
			symbolY += findValue(p, r, dy, Value(true));
		}
		if (quietZone == 0 && symbolX > 0 && symbolY > 0) {
			origin = getPos(p);
			log(origin, LOG_I, 2);
			return origin;
		}
	}
	return {};
}

} // namespace ZXing
