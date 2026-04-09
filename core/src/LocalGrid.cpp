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
	printf(" -> len: %zu, avg: %5.2f | ", bestLen, sum);
#endif
	return sum;
};

void LocalGrid::adjustOrigin(PointF dir, int radius, const std::span<const PointF> offsets, double clusterThreshold)
{
	auto modSize = length(dir);
	printf("\nMS: %.2f: ", modSize);
	int limit = static_cast<int>(std::round(5 * modSize));
	dir = bresenhamDirection(dir);

	auto nearestHalfResidual = [](double n, double s) { return n - (std::round((n - s / 2) / s) * s + s / 2); };

	thread_local std::vector<double> d;
	d.clear();
	d.reserve(offsets.size() * radius + 1);
	for (int r = 0; r <= radius; ++r)
		for (auto offset : offsets) {
			auto start = origin + r * offset;
			auto startC = centered(start);
			int stepsPos = BitMatrixCursorF(*img, startC, dir).stepToEdge(1, limit);
			int stepsNeg = BitMatrixCursorF(*img, startC, -dir).stepToEdge(1, limit);
			auto distPos = stepsPos - dot(start - startC, dir) - 0.5; // +0.5 because the center of the pixel is at .5, .5
			auto distNeg = stepsNeg - dot(start - startC, -dir) - 0.5;

			auto localModSize = modSize;
			if (stepsPos != 0 && stepsNeg != 0) {
				int blockSize = stepsPos + stepsNeg - 1;
				localModSize = blockSize / std::max(1.0, std::round(blockSize / modSize));
				// if (r == 0 && (std::min(distNeg, distPos) < modSize / 4)) {
				// 	origin += (distNeg < distPos ? 1 : -1) * modSize / 4 * dir;
				// 	continue;
				// }
			}

			printf("+%.2f -%.2f (%.1f) -> %.2f | ", distPos, distNeg, localModSize, nearestHalfResidual(distPos, localModSize));
			if (stepsPos)
				d.push_back(nearestHalfResidual(distPos, localModSize));
			else if (stepsNeg)
				d.push_back(-nearestHalfResidual(distNeg, localModSize));

			if (r == 0)
				break; // only do the center point once
		}

	printf("\n");
	origin += clusterAvg(d, clusterThreshold ? clusterThreshold : modSize * 0.5) * dir;
}

LocalGrid::LocalGrid(const BitMatrix& image, const PerspectiveTransform& mod2Pix, PointI p, PointI dim)
	: img(&image), dim(dim), center(p)
{
	origin = mod2Pix(centered(p));
	stepX = mod2Pix(centered(p) + PointF{1, 0}) - origin;
	stepY = mod2Pix(centered(p) + PointF{0, 1}) - origin;

#ifdef PRINT_DEBUG
	printf("initial origin: (%.2f, %.2f), stepX: (%.2f, %.2f), stepY: (%.2f, %.2f)\n", origin.x, origin.y, stepX.x, stepX.y, stepY.x,
		   stepY.y);
#endif
	log(origin, 3);

	// auto offsets = std::array{-stepX, -stepY, stepX, stepY};
	auto offsets = std::array{-stepX - stepY, stepX - stepY, stepX + stepY, -stepX + stepY};
	// auto offsets = std::array{-stepX, -stepY, stepX, stepY, -stepX - stepY, stepX - stepY, stepX + stepY, -stepX + stepY};
	for (int i = 0; i < 2; ++i) {
		adjustOrigin(stepX, 2, offsets);
		adjustOrigin(stepY, 2, offsets);
#ifdef PRINT_DEBUG
		printf("\n");
#endif
	}
}

bool LocalGrid::isTimingPatternCross(PointI p, int radius, int errorThreshold)
{
	auto wrapOffset = [&](int center, int offset, int dim) {
		int pos = center + offset;
		return pos < 0 ? radius - pos : (pos >= dim ? -(radius + (pos - dim) + 1) : offset);
	};

	int errors = 0;
	for (int r = 0; r <= radius; ++r) {
		auto check = [&](int x, int y) {
			x = wrapOffset(center.x, x, dim.x);
			y = wrapOffset(center.y, y, dim.y);
			errors += get(p.x + x, p.y + y) != LocalGrid::Value((x + y) % 2 == 0);
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
			// adjust origin with full radius and only in the direction of the timing pattern
			adjustOrigin(stepX, radius, std::array{stepX, -stepX}, INFINITY);
			adjustOrigin(stepY, radius, std::array{stepY, -stepY}, INFINITY);
#ifdef PRINT_DEBUG
			printf(" <- timing pattern\n");
#endif
			// check again, now with the full radius, to make sure we are correctly aligned to the timing pattern
			if (isTimingPatternCross(PointI{0, 0}, radius))
				return origin;

			origin = original;
		}
	}
	return {};
}

} // namespace ZXing::Aztec
