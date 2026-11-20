// SPDX-License-Identifier: Apache-2.0

#include "WXDetector.h"

#include "BitMatrix.h"
#include "ImageView.h"
#include "WXConstants.h"

#include <algorithm>
#include <array>
#include <climits>
#include <cmath>
#include <numbers>
#include <optional>
#include <utility>

namespace ZXing::WXCode {
namespace {

constexpr double FINDER_DISTANCE = 31.819807;
constexpr size_t MAX_FINDERS = 128;
constexpr int MAX_SAMPLES = 128;
constexpr int SAMPLES_PER_AXIS = 5;
constexpr int SAMPLES_PER_CELL = SAMPLES_PER_AXIS * SAMPLES_PER_AXIS;
constexpr double SYMBOL_HALF_EXTENT = 27;
constexpr double INNER_RADIUS = 10;

struct Raster
{
	const ImageView& image;
	const BitMatrix* matrix;
	int threshold;
};
struct Finder
{
	PointF center;
	double diameter;
	double area;
	double centerOffset;
};
struct Mapping
{
	PointF center;
	PointF xAxis;
	PointF yAxis;
	double perspectiveX;
	double perspectiveY;
};

static bool IsDark(const Raster& raster, int x, int y)
{
	return x >= 0 && y >= 0 && x < raster.image.width() && y < raster.image.height()
		   && (raster.matrix ? raster.matrix->get(x, y) : *raster.image.data(x, y) <= raster.threshold);
}

// Include the midpoint of an equally optimal intensity gap.
static int OtsuThreshold(const ImageView& image)
{
	std::array<size_t, 256> bins{};
	size_t lower = 0;
	double total = 0;
	double partial = 0;
	double best = -1;
	int first = 0;
	int last = 0;
	size_t count = static_cast<size_t>(image.width()) * image.height();
	for (int y = 0; y < image.height(); ++y)
		for (int x = 0; x < image.width(); ++x)
			++bins[*image.data(x, y)];
	for (int v = 0; v < 256; ++v)
		total += v * static_cast<double>(bins[v]);
	for (int v = 0; v < 255; ++v) {
		lower += bins[v];
		partial += v * static_cast<double>(bins[v]);
		if (!lower)
			continue;
		if (lower == count)
			break;
		double delta = partial / lower - (total - partial) / (count - lower);
		double score = delta * delta * lower * (count - lower);
		if (score > best) {
			best = score;
			first = last = v;
		} else if (score == best)
			last = v;
	}
	return (first + last) / 2;
}

/* Verify the dark center, surrounding light annulus, and outer dark ring
   along eight rays. Chords of the isolated center give subpixel centers. */
static bool CheckBullseye(const Raster& raster, int left, int right, int y, Finder& found)
{
	int diameter = right - left + 1;
	int x = (left + right) / 2;
	int top = y;
	int bottom = y;
	while (top > 0 && y - top < diameter && IsDark(raster, x, top - 1))
		--top;
	while (bottom + 1 < raster.image.height() && bottom - y < diameter && IsDark(raster, x, bottom + 1))
		++bottom;
	// Rasterized rotations can move the vertical midpoint off the scanline.
	double centerOffset = std::abs((top + bottom) * 0.5 - y);
	if (centerOffset > 1 || bottom - top + 1 < diameter * 0.55 || bottom - top + 1 > diameter * 1.8)
		return false;
	double cx = (left + right) * 0.5;
	double cy = (top + bottom) * 0.5;
	/* Diameter of the outer ring is about five central-dot diameters.
	   Estimate using measured ring boundaries, independent of sample size. */
	double sumRadius = 0;
	double sumSquared = 0;
	for (int dir = 0; dir < 8; ++dir) {
		static constexpr std::array<double, 8> dx = {1, .70710678, 0, -.70710678, -1, -.70710678, 0, .70710678};
		static constexpr std::array<double, 8> dy = {0, .70710678, 1, .70710678, 0, -.70710678, -1, -.70710678};
		std::array<int, 3> ends{};
		int r = 1;
		for (int run = 0; run < 3; ++run) {
			int start = r;
			while (r <= diameter * 5) {
				int px = static_cast<int>(std::lround(cx + r * dx[dir]));
				int py = static_cast<int>(std::lround(cy + r * dy[dir]));
				if (px < 0 || py < 0 || px >= raster.image.width() || py >= raster.image.height())
					return false;
				if (IsDark(raster, px, py) != (run != 1))
					break;
				++r;
			}
			if (r == start || r > diameter * 5)
				return false;
			ends[run] = r;
		}
		double radius = ends[2] - 0.5;
		if (ends[0] < radius * .10 || ends[0] > radius * .36 || ends[1] < radius * .54 || ends[1] > radius * .86)
			return false;
		sumRadius += radius;
		sumSquared += radius * radius;
	}
	found.center = PointF{cx, cy};
	found.diameter = sumRadius / 4;
	found.area = sumSquared;
	found.centerOffset = centerOffset;
	return true;
}

// Center horizontal chord: ring, gap, dot, gap, ring.
static bool MatchesFinderRuns(const std::array<int, 5>& runs)
{
	int diameter = runs[2];
	return diameter >= 2 && runs[0] >= .35 * diameter && runs[0] <= 1.8 * diameter && runs[4] >= .35 * diameter
		   && runs[4] <= 1.8 * diameter && runs[1] >= .60 * diameter && runs[1] <= 2.3 * diameter && runs[3] >= .60 * diameter
		   && runs[3] <= 2.3 * diameter;
}

static bool AddFinder(std::vector<Finder>& finders, const Finder& finder)
{
	// Adjacent scanlines can observe the same ring. Keep its most centered
	// observation before counting candidates or triples.
	auto existing = std::find_if(finders.begin(), finders.end(), [&](const Finder& other) {
		double size = std::min(finder.diameter, other.diameter);
		return std::abs(finder.diameter - other.diameter) <= .25 * size
			   && std::hypot(finder.center.x - other.center.x, finder.center.y - other.center.y) <= std::max(1.5, .1 * size);
	});
	if (existing != finders.end()) {
		if (finder.centerOffset < existing->centerOffset)
			*existing = finder;
		return true;
	}
	if (finders.size() == MAX_FINDERS)
		return false;
	finders.push_back(finder);
	return true;
}

static bool FindCenters(const Raster& raster, std::vector<Finder>& finders)
{
	for (int y = 1; y + 1 < raster.image.height(); ++y) {
		std::array<int, 5> runs{};
		int runCount = 0;
		int begin = 0;
		while (begin < raster.image.width()) {
			int end = begin + 1;
			bool ink = IsDark(raster, begin, y);
			while (end < raster.image.width() && IsDark(raster, end, y) == ink)
				++end;
			std::move(runs.begin() + 1, runs.end(), runs.begin());
			runs[4] = end - begin;
			if (runCount < 5)
				++runCount;
			int centerRight = begin - runs[3] - 1;
			begin = end;
			if (runCount != 5 || !ink || !MatchesFinderRuns(runs))
				continue;
			Finder finder;
			if (CheckBullseye(raster, centerRight - runs[2] + 1, centerRight, y, finder) && !AddFinder(finders, finder))
				return false;
		}
	}
	return true;
}

/* Fit the two perspective terms from the local area changes of the
   three equal-size rings. For a homography with denominator w, the local
   area scale is proportional to 1/w^3. Small differences are quantization
   noise, where the three-center affine fit is more accurate. */
static Mapping FitMapping(const Finder& tl, const Finder& tr, const Finder& bl)
{
	double a = std::cbrt(tl.area / tr.area);
	double b = std::cbrt(tl.area / bl.area);
	if (std::fabs(a - 1) < .018)
		a = 1;
	if (std::fabs(b - 1) < .018)
		b = 1;
	double w = (a + b) * .5;
	double unit = w * FINDER_DISTANCE;
	return Mapping{{(a * tr.center.x + b * bl.center.x) / (2 * w), (a * tr.center.y + b * bl.center.y) / (2 * w)},
				   {(a * tr.center.x - tl.center.x) / unit, (a * tr.center.y - tl.center.y) / unit},
				   {(b * bl.center.x - tl.center.x) / unit, (b * bl.center.y - tl.center.y) / unit},
				   (a - 1) / unit,
				   (b - 1) / unit};
}

static PointF MapPoint(const Mapping& m, double x, double y)
{
	double w = 1 + m.perspectiveX * x + m.perspectiveY * y;
	return PointF{(m.center.x + x * m.xAxis.x + y * m.yAxis.x) / w, (m.center.y + x * m.xAxis.y + y * m.yAxis.y) / w};
}

static std::optional<Detection> SampleGrid(const Raster& raster, const Mapping& mapping)
{
	Detection result;
	auto& bits = result.bits;
	auto& corners = result.position;
	/* The homography denominator is linear. Its minimum on the canonical
	   square is at one of the four corners. */
	if (1 - SYMBOL_HALF_EXTENT * (std::fabs(mapping.perspectiveX) + std::fabs(mapping.perspectiveY)) <= 0)
		return {};
	for (int i = 0; i < 4; ++i) {
		PointF point = MapPoint(mapping, i == 1 || i == 2 ? SYMBOL_HALF_EXTENT : -SYMBOL_HALF_EXTENT,
								i >= 2 ? SYMBOL_HALF_EXTENT : -SYMBOL_HALF_EXTENT);
		if (!(point.x >= INT_MIN && point.x <= INT_MAX && point.y >= INT_MIN && point.y <= INT_MAX))
			return {};
		corners[i] = {static_cast<int>(std::lround(point.x)), static_cast<int>(std::lround(point.y))};
	}
	for (int angle = 0; angle < ANGULAR_ROWS; ++angle) {
		std::array<int, RADIAL_CELLS> counts{};
		for (int angularSample = 0; angularSample < SAMPLES_PER_AXIS; ++angularSample) {
			double theta =
				(angle + (angularSample - SAMPLES_PER_AXIS / 2) / static_cast<double>(SAMPLES_PER_AXIS)) * std::numbers::pi / 36;
			double directionX = -std::cos(theta);
			double directionY = -std::sin(theta);
			for (int radiusIndex = 0; radiusIndex < RADIAL_CELLS; ++radiusIndex) {
				for (int radialSample = 0; radialSample < SAMPLES_PER_AXIS; ++radialSample) {
					double radius = INNER_RADIUS + radiusIndex + (radialSample + .5) / SAMPLES_PER_AXIS;
					PointF point = MapPoint(mapping, directionX * radius, directionY * radius);
					if (!(point.x > -.5 && point.x < raster.image.width() - .5 && point.y > -.5
						  && point.y < raster.image.height() - .5))
						return {};
					int x = static_cast<int>(std::lround(point.x));
					int y = static_cast<int>(std::lround(point.y));
					counts[radiusIndex] += IsDark(raster, x, y);
				}
			}
		}
		for (int radiusIndex = 0; radiusIndex < RADIAL_CELLS; ++radiusIndex) {
			double threshold = .53301 + ((4 - radiusIndex) / 11.0) * .3;
			bits[angle][radiusIndex] = counts[radiusIndex] > SAMPLES_PER_CELL * threshold;
		}
	}
	return result;
}

static bool HasValidAxes(const Finder& origin, const Finder& right, const Finder& down)
{
	PointF xAxis = {(right.center.x - origin.center.x) / FINDER_DISTANCE, (right.center.y - origin.center.y) / FINDER_DISTANCE};
	PointF yAxis = {(down.center.x - origin.center.x) / FINDER_DISTANCE, (down.center.y - origin.center.y) / FINDER_DISTANCE};
	double xScale = std::hypot(xAxis.x, xAxis.y);
	double yScale = std::hypot(yAxis.x, yAxis.y);
	if (xScale == 0 || yScale == 0 || xAxis.x * yAxis.y - xAxis.y * yAxis.x <= 0
		|| std::fabs(xAxis.x * yAxis.x + xAxis.y * yAxis.y) > .48 * xScale * yScale || xScale < .55 * yScale
		|| yScale < .55 * xScale)
		return false;
	double expectedDiameter = 5 * std::sqrt(xScale * yScale);
	if (origin.diameter < .70 * expectedDiameter || origin.diameter > 1.45 * expectedDiameter
		|| right.diameter < .70 * expectedDiameter || right.diameter > 1.45 * expectedDiameter
		|| down.diameter < .70 * expectedDiameter || down.diameter > 1.45 * expectedDiameter)
		return false;
	return true;
}

static bool TryAxes(const Raster& raster, const Finder& origin, const Finder& right, const Finder& down, int& samples,
					std::vector<Detection>& results)
{
	if (!HasValidAxes(origin, right, down))
		return true;
	if (auto result = SampleGrid(raster, FitMapping(origin, right, down)))
		results.push_back(std::move(*result));
	return ++samples < MAX_SAMPLES;
}

} // namespace

std::vector<Detection> Detect(const ImageView& image, const BitMatrix* matrix)
{
	if (!image.data() || !IsSupportedImageSize(image.width(), image.height()))
		return {};

	Raster raster{image, matrix, matrix ? 0 : OtsuThreshold(image)};
	std::vector<Finder> finders;
	if (!FindCenters(raster, finders))
		return {};

	int samples = 0;
	std::vector<Detection> results;
	for (size_t i = 0; i < finders.size(); ++i) {
		for (size_t j = 0; j < finders.size(); ++j) {
			if (i == j)
				continue;
			for (size_t k = 0; k < finders.size(); ++k) {
				if (k == i || k == j)
					continue;
				if (!TryAxes(raster, finders[i], finders[j], finders[k], samples, results))
					return results;
			}
		}
	}
	return results;
}

} // namespace ZXing::WXCode
