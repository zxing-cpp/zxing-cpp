// Copyright 2026 Axel Waggershauser
// SPDX-License-Identifier: Apache-2.0

#include "LocalGrid.h"

#include "LogMatrix.h"
#include "StdGenerator.h"
#include "ZXAlgorithms.h"

#include <algorithm>
#include <optional>
#include <ranges>
#include <span>
#include <vector>

#ifndef PRINT_DEBUG
#define printf(...){}
#endif

namespace ZXing {

// generates points in a spiral pattern around the center
inline std::generator<PointI> spiral(int radius)
{
	co_yield{0, 0};
	for (int r = 1; r <= radius; ++r) {
		// clang-format off
		for (int k = 0; k < 2 * r; ++k) co_yield{           r, -(r - 1) + k}; // right -> down
		for (int k = 0; k < 2 * r; ++k)	co_yield{ (r - 1) - k,            r}; // bottom -> left
		for (int k = 0; k < 2 * r; ++k)	co_yield{          -r,  (r - 1) - k}; // left -> up
		for (int k = 0; k < 2 * r; ++k)	co_yield{-(r - 1) + k,           -r}; // top -> right
		// clang-format on
	}
}

// returns the average of the largest cluster of values where cluster is defined as values that are within threshold of each other
double clusterAvg(std::ranges::range auto& v, double threshold)
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

#ifdef PRINT_DEBUG
	printf("ds: ");
	for (auto d : v)
		printf("%5.2f ", d);
	printf(" -> len: %zu, avg: %5.2f\n", bestLen, sum);
#endif
	return sum;
};

void LocalGrid::adjustOriginAndStep(PointF& step, int radius, const std::span<const PointF> offsets, double clusterThreshold)
{
	auto modSize = length(step);
	int limit = int(6 * modSize);
	auto dir = bresenhamDirection(step);
	printf("step: (%.2f, %.2f) %.2f: ", step.x, step.y, modSize);

	auto nearestHalfResidual = [](double n, double s) { return n - (std::round((n - s / 2) / s) * s + s / 2); };

	struct DistMod {
		double dist, modSize;
	};
	thread_local std::vector<DistMod> distMod;
	distMod.clear();
	distMod.reserve(offsets.size() * radius + 1);
	for (int r = 0; r <= radius; ++r)
		for (auto offset : offsets) {
			auto start = origin + r * offset;
			auto startC = centered(start);
			int stepsPos = BitMatrixCursorF(*img, startC, dir).stepToEdge(1, limit);
			int stepsNeg = BitMatrixCursorF(*img, startC, -dir).stepToEdge(1, limit);
			auto distPos = stepsPos - dot(start - startC, dir) - 0.5; // +0.5 because the center of the pixel is at .5, .5
			auto distNeg = stepsNeg - dot(start - startC, -dir) - 0.5;

			if (stepsPos && stepsNeg) {
				int blockSize = stepsPos + stepsNeg - 1;
				auto localModSize = blockSize / std::max(1.0, std::round(blockSize / modSize));
				distMod.emplace_back(distPos, blockSize / modSize < 5 ? localModSize : 0.0);
				// if (r == 0 && (std::min(distNeg, distPos) < modSize / 4)) {
				// 	origin += (distNeg < distPos ? 1 : -1) * modSize / 4 * dir;
				// 	continue;
				// }
			}
			else if (stepsPos)
				distMod.emplace_back(distPos, 0.0);
			else if (stepsNeg)
				distMod.emplace_back(-distNeg, 0.0);

			printf("+%.2f -%.2f (%.1f) | ", distPos, distNeg, distMod.empty() ? 0.0 : distMod.back().modSize);

			if (r == 0)
				break; // only do the center point once
		}

	// calcuate the average local module size from the points where we found one...
	double localModSize = 0.0;
	int n = 0;
	for (const auto& t : distMod | std::views::filter([](const DistMod& t) { return t.modSize > 0; }))
		localModSize += t.modSize, ++n;
	if (n == 0)
		return;
	localModSize /= n;
	printf("\nlocal mod size: %.2f\n", localModSize);

	// ... and use it for the points where we didn't find one
	thread_local std::vector<double> d;
	d.clear();
	d.reserve(distMod.size());
	for (auto& t : distMod) {
		if (t.modSize == 0.0)
			t.modSize = localModSize;
		d.push_back(t.dist < 0 ? -nearestHalfResidual(-t.dist, t.modSize) : nearestHalfResidual(t.dist, t.modSize));
		printf("%.2f (%.1f) -> %.2f | ", t.dist, t.modSize, d.back());
	}
	printf("\n");

	origin += clusterAvg(d, clusterThreshold ? clusterThreshold : modSize * 0.5) * dir;

	// if we found local mod sizes for each point (hopefully at the timing pattern crosses), we update the step size
	if (n == Size(distMod) && std::abs(localModSize - modSize) > modSize * 0.1) {
		step = localModSize / modSize * step;
		printf("   adjusted mod size from %.2f to %.2f\n", modSize, localModSize);
	}
}

LocalGrid::LocalGrid(const BitMatrix& image, const PerspectiveTransform& mod2Pix, PointI p, PointI dim)
	: img(&image), dim(dim), center(p)
{
	origin = mod2Pix(centered(p));
	stepX = mod2Pix(centered(p) + PointF{1, 0}) - origin;
	stepY = mod2Pix(centered(p) + PointF{0, 1}) - origin;

	printf("initial origin: (%.2f, %.2f), stepX: (%.2f, %.2f), stepY: (%.2f, %.2f)\n", origin.x * 5, origin.y * 5, stepX.x, stepX.y,
		   stepY.x, stepY.y);
	log(origin, 3);

	// auto offsets = std::array{-stepX, -stepY, stepX, stepY};
	auto offsets = std::array{-stepX - stepY, stepX - stepY, stepX + stepY, -stepX + stepY};
	// auto offsets = std::array{-stepX, -stepY, stepX, stepY, -stepX - stepY, stepX - stepY, stepX + stepY, -stepX + stepY};
	for (int i = 0; i < 2; ++i) {
		adjustOriginAndStep(stepX, 2, offsets);
		adjustOriginAndStep(stepY, 2, offsets);
		printf("\n");
	}
}

bool LocalGrid::isTimingPatternCross(PointI p, int radius, int errorThreshold)
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
			auto t = LocalGrid::Value((x + y) % 2 == 0);
			double dx = x ? 0.25 : 0.0, dy = y ? 0.25 : 0.0;
			errors += (get(p.x + x, p.y + y) != t && get(p.x + x + dx, p.y + y + dy) != t && get(p.x + x - dx, p.y + y - dy) != t);
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

std::optional<PointF> LocalGrid::findTimingPatternCross(int radius)
{
	for (auto p : spiral(3)) {
		// check if there is a timing pattern cross candidate centered at p with half the radius
		if (isTimingPatternCross(p, radius / 2)) {
			auto original = origin;
			origin += p.x * stepX + p.y * stepY;
			// adjust origin and step with full radius and only in the direction of the timing pattern
			printf("timing pattern:\n");
			adjustOriginAndStep(stepX, radius, std::array{-stepX, stepX});
			adjustOriginAndStep(stepY, radius, std::array{-stepY, stepY});

			// check again, now with the full radius, to make sure we are correctly aligned to the timing pattern
			if (isTimingPatternCross(PointI{0, 0}, radius))
				return origin;

			origin = original;
		}
	}
	return {};
}

} // namespace ZXing::Aztec
