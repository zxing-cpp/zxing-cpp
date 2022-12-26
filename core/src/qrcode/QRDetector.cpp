/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
* Copyright 2020 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "QRDetector.h"

#include "BitArray.h"
#include "BitMatrix.h"
#include "BitMatrixCursor.h"
#include "ConcentricFinder.h"
#include "GridSampler.h"
#include "LogMatrix.h"
#include "Pattern.h"
#include "QRFormatInformation.h"
#include "QRVersion.h"
#include "Quadrilateral.h"
#include "RegressionLine.h"

#include "BitMatrixIO.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iterator>
#include <map>
#include <utility>
#include <vector>

namespace ZXing::QRCode {

constexpr auto PATTERN = FixedPattern<5, 7>{1, 1, 3, 1, 1};

std::vector<ConcentricPattern> FindFinderPatterns(const BitMatrix& image, bool tryHarder)
{
	constexpr int MIN_SKIP         = 3;           // 1 pixel/module times 3 modules/center
	constexpr int MAX_MODULES_FAST = 20 * 4 + 17; // support up to version 20 for mobile clients

	// Let's assume that the maximum version QR Code we support takes up 1/4 the height of the
	// image, and then account for the center being 3 modules in size. This gives the smallest
	// number of pixels the center could be, so skip this often. When trying harder, look for all
	// QR versions regardless of how dense they are.
	int height = image.height();
	int skip = (3 * height) / (4 * MAX_MODULES_FAST);
	if (skip < MIN_SKIP || tryHarder)
		skip = MIN_SKIP;

	std::vector<ConcentricPattern> res;

	for (int y = skip - 1; y < height; y += skip) {
		PatternRow row;
		GetPatternRow(image, y, row, false);
		PatternView next = row;

		while (next = FindLeftGuard(next, 0, PATTERN, 0.5), next.isValid()) {
			PointF p(next.pixelsInFront() + next[0] + next[1] + next[2] / 2.0, y + 0.5);

			// make sure p is not 'inside' an already found pattern area
			if (FindIf(res, [p](const auto& old) { return distance(p, old) < old.size / 2; }) == res.end()) {
				auto pattern = LocateConcentricPattern(image, PATTERN, p,
													   Reduce(next) * 3 / 2); // 1.5 for very skewed samples
				if (pattern) {
					log(*pattern, 3);
					assert(image.get(pattern->x, pattern->y));
					res.push_back(*pattern);
				}
			}

			next.skipPair();
			next.skipPair();
			next.extend();
		}
	}

	return res;
}

/**
 * @brief GenerateFinderPatternSets
 * @param patterns list of ConcentricPattern objects, i.e. found finder pattern squares
 * @return list of plausible finder pattern sets, sorted by decreasing plausibility
 */
FinderPatternSets GenerateFinderPatternSets(FinderPatterns& patterns)
{
	std::sort(patterns.begin(), patterns.end(), [](const auto& a, const auto& b) { return a.size < b.size; });

	auto sets            = std::multimap<double, FinderPatternSet>();
	auto squaredDistance = [](const auto* a, const auto* b) {
		// The scaling of the distance by the b/a size ratio is a very coarse compensation for the shortening effect of
		// the camera projection on slanted symbols. The fact that the size of the finder pattern is proportional to the
		// distance from the camera is used here. This approximation only works if a < b < 2*a (see below).
		// Test image: fix-finderpattern-order.jpg
		return dot((*a - *b), (*a - *b)) * std::pow(double(b->size) / a->size, 2);
	};
	const double cosUpper = std::cos(45. / 180 * 3.1415); // TODO: use c++20 std::numbers::pi_v
	const double cosLower = std::cos(135. / 180 * 3.1415);

	int nbPatterns = Size(patterns);
	for (int i = 0; i < nbPatterns - 2; i++) {
		for (int j = i + 1; j < nbPatterns - 1; j++) {
			for (int k = j + 1; k < nbPatterns - 0; k++) {
				const auto* a = &patterns[i];
				const auto* b = &patterns[j];
				const auto* c = &patterns[k];
				// if the pattern sizes are too different to be part of the same symbol, skip this
				// and the rest of the innermost loop (sorted list)
				if (c->size > a->size * 2)
					break;

				// Orders the three points in an order [A,B,C] such that AB is less than AC
				// and BC is less than AC, and the angle between BC and BA is less than 180 degrees.

				auto distAB2 = squaredDistance(a, b);
				auto distBC2 = squaredDistance(b, c);
				auto distAC2 = squaredDistance(a, c);

				if (distBC2 >= distAB2 && distBC2 >= distAC2) {
					std::swap(a, b);
					std::swap(distBC2, distAC2);
				} else if (distAB2 >= distAC2 && distAB2 >= distBC2) {
					std::swap(b, c);
					std::swap(distAB2, distAC2);
				}

				auto distAB = std::sqrt(distAB2);
				auto distBC = std::sqrt(distBC2);

				// Estimate the module count and ignore this set if it can not result in a valid decoding
				if (auto moduleCount = (distAB + distBC) / (2 * (a->size + b->size + c->size) / (3 * 7.f)) + 7;
					moduleCount < 21 * 0.9 || moduleCount > 177 * 1.05)
					continue;

				// Make sure the angle between AB and BC does not deviate from 90° by more than 45°
				auto cosAB_BC = (distAB2 + distBC2 - distAC2) / (2 * distAB * distBC);
				if (std::isnan(cosAB_BC) || cosAB_BC > cosUpper || cosAB_BC < cosLower)
					continue;

				// a^2 + b^2 = c^2 (Pythagorean theorem), and a = b (isosceles triangle).
				// Since any right triangle satisfies the formula c^2 - b^2 - a^2 = 0,
				// we need to check both two equal sides separately.
				// The value of |c^2 - 2 * b^2| + |c^2 - 2 * a^2| increases as dissimilarity
				// from isosceles right triangle.
				double d = (std::abs(distAC2 - 2 * distAB2) + std::abs(distAC2 - 2 * distBC2));

				// Use cross product to figure out whether A and C are correct or flipped.
				// This asks whether BC x BA has a positive z component, which is the arrangement
				// we want for A, B, C. If it's negative then swap A and C.
				if (cross(*c - *b, *a - *b) < 0)
					std::swap(a, c);

				// arbitrarily limit the number of potential sets
				// (this has performance implications while limiting the maximal number of detected symbols)
				const auto setSizeLimit = 256;
				if (sets.size() < setSizeLimit || sets.crbegin()->first > d) {
					sets.emplace(d, FinderPatternSet{*a, *b, *c});
					if (sets.size() > setSizeLimit)
						sets.erase(std::prev(sets.end()));
				}
			}
		}
	}

	// convert from multimap to vector
	FinderPatternSets res;
	res.reserve(sets.size());
	for (auto& [d, s] : sets)
		res.push_back(s);
	return res;
}

static double EstimateModuleSize(const BitMatrix& image, PointF a, PointF b)
{
	BitMatrixCursorF cur(image, a, b - a);
	assert(cur.isBlack());

	if (!cur.stepToEdge(3, static_cast<int>(distance(a, b) / 3), true))
		return -1;

	assert(cur.isBlack());
	cur.turnBack();


	auto pattern = cur.readPattern<std::array<int, 5>>();

	return (2 * Reduce(pattern) - pattern[0] - pattern[4]) / 12.0 * length(cur.d);
}

struct DimensionEstimate
{
	int dim = 0;
	double ms = 0;
	int err = 0;
};

static DimensionEstimate EstimateDimension(const BitMatrix& image, PointF a, PointF b)
{
	auto ms_a = EstimateModuleSize(image, a, b);
	auto ms_b = EstimateModuleSize(image, b, a);

	if (ms_a < 0 || ms_b < 0)
		return {};

	auto moduleSize = (ms_a + ms_b) / 2;

	int dimension = narrow_cast<int>(std::lround(distance(a, b) / moduleSize) + 7);
	int error     = 1 - (dimension % 4);

	return {dimension + error, moduleSize, std::abs(error)};
}

static RegressionLine TraceLine(const BitMatrix& image, PointF p, PointF d, int edge)
{
	BitMatrixCursorF cur(image, p, d - p);
	RegressionLine line;
	line.setDirectionInward(cur.back());

	// collect points inside the black line -> backup on 3rd edge
	cur.stepToEdge(edge, 0, edge == 3);
	if (edge == 3)
		cur.turnBack();

	auto curI = BitMatrixCursorI(image, PointI(cur.p), PointI(mainDirection(cur.d)));
	// make sure curI positioned such that the white->black edge is directly behind
	// Test image: fix-traceline.jpg
	while (!curI.edgeAtBack()) {
		if (curI.edgeAtLeft())
			curI.turnRight();
		else if (curI.edgeAtRight())
			curI.turnLeft();
		else
			curI.step(-1);
	}

	for (auto dir : {Direction::LEFT, Direction::RIGHT}) {
		auto c = BitMatrixCursorI(image, curI.p, curI.direction(dir));
		auto stepCount = static_cast<int>(maxAbsComponent(cur.p - p));
		do {
			line.add(centered(c.p));
		} while (--stepCount > 0 && c.stepAlongEdge(dir, true));
	}

	line.evaluate(1.0, true);

	for (auto p : line.points())
		log(p, 2);

	return line;
}

// estimate how tilted the symbol is (return value between 1 and 2, see also above)
static double EstimateTilt(const FinderPatternSet& fp)
{
	int min = std::min({fp.bl.size, fp.tl.size, fp.tr.size});
	int max = std::max({fp.bl.size, fp.tl.size, fp.tr.size});
	return double(max) / min;
}

DetectorResult SampleQR(const BitMatrix& image, const FinderPatternSet& fp)
{
	auto top  = EstimateDimension(image, fp.tl, fp.tr);
	auto left = EstimateDimension(image, fp.tl, fp.bl);

	if (!top.dim || !left.dim)
		return {};

	auto best = top.err < left.err ? top : left;
	int dimension = best.dim;
	int moduleSize = static_cast<int>(best.ms + 1);

	auto quad = Rectangle(dimension, dimension, 3.5);

	auto sample = [&](PointF br, PointF quad2) {
		log(br, 3);
		quad[2] = quad2;
		return SampleGrid(image, dimension, dimension, {quad, {fp.tl, fp.tr, br, fp.bl}});
	};

	// Everything except version 1 (21 modules) has an alignment pattern. Estimate the center of that by intersecting
	// line extensions of the 1 module wide square around the finder patterns. This could also help with detecting
	// slanted symbols of version 1.

	// generate 4 lines: outer and inner edge of the 1 module wide black line between the two outer and the inner
	// (tl) finder pattern
	auto bl2 = TraceLine(image, fp.bl, fp.tl, 2);
	auto bl3 = TraceLine(image, fp.bl, fp.tl, 3);
	auto tr2 = TraceLine(image, fp.tr, fp.tl, 2);
	auto tr3 = TraceLine(image, fp.tr, fp.tl, 3);

	if (bl2.isValid() && tr2.isValid() && bl3.isValid() && tr3.isValid()) {
		// intersect both outer and inner line pairs and take the center point between the two intersection points
		auto brInter = (intersect(bl2, tr2) + intersect(bl3, tr3)) / 2;
		log(brInter, 3);

		// check that the estimated alignment pattern position is inside of the image
		if (image.isIn(PointI(brInter), 3 * moduleSize)) {
			if (dimension > 21) {
				// just in case we landed outside of the central black module of the alignment pattern, use the center
				// of the next best circle (either outer or inner edge of the white part of the alignment pattern)
				auto brCoR = CenterOfRing(image, PointI(brInter), moduleSize * 4, 1, false).value_or(brInter);
				// if we did not land on a black pixel or the concentric pattern finder fails,
				// leave the intersection of the lines as the best guess
				if (image.get(brCoR)) {
					if (auto brCP = LocateConcentricPattern<true>(image, FixedPattern<3, 3>{1, 1, 1}, brCoR, moduleSize * 3))
						return sample(*brCP, quad[2] - PointF(3, 3));
				}
			}

			// if the symbol is tilted or the resolution of the RegressionLines is sufficient, use their intersection
			// as the best estimate (see discussion in #199 and test image estimate-tilt.jpg )
			if (EstimateTilt(fp) > 1.1 || (bl2.isHighRes() && bl3.isHighRes() && tr2.isHighRes() && tr3.isHighRes()))
				return sample(brInter, quad[2] - PointF(3, 3));
		}
	}

	// otherwise the simple estimation used by upstream is used as a best guess fallback
	return sample(fp.tr - fp.tl + fp.bl, quad[2]);
}

/**
* This method detects a code in a "pure" image -- that is, pure monochrome image
* which contains only an unrotated, unskewed, image of a code, with some white border
* around it. This is a specialized method that works exceptionally fast in this special
* case.
*/
DetectorResult DetectPureQR(const BitMatrix& image)
{
	using Pattern = std::array<PatternView::value_type, PATTERN.size()>;

#ifdef PRINT_DEBUG
	SaveAsPBM(image, "weg.pbm");
#endif

	constexpr int MIN_MODULES = Version::DimensionOfVersion(1, false);
	constexpr int MAX_MODULES = Version::DimensionOfVersion(40, false);

	int left, top, width, height;
	if (!image.findBoundingBox(left, top, width, height, MIN_MODULES) || std::abs(width - height) > 1)
		return {};
	int right  = left + width - 1;
	int bottom = top + height - 1;

	PointI tl{left, top}, tr{right, top}, bl{left, bottom};
	Pattern diagonal;
	// allow corners be moved one pixel inside to accommodate for possible aliasing artifacts
	for (auto [p, d] : {std::pair(tl, PointI{1, 1}), {tr, {-1, 1}}, {bl, {1, -1}}}) {
		diagonal = BitMatrixCursorI(image, p, d).readPatternFromBlack<Pattern>(1, width / 3);
		if (!IsPattern(diagonal, PATTERN))
			return {};
	}

	auto fpWidth = Reduce(diagonal);
	auto dimension = EstimateDimension(image, tl + fpWidth / 2 * PointF(1, 1), tr + fpWidth / 2 * PointF(-1, 1)).dim;

	float moduleSize = float(width) / dimension;
	if (dimension < MIN_MODULES || dimension > MAX_MODULES ||
		!image.isIn(PointF{left + moduleSize / 2 + (dimension - 1) * moduleSize,
						   top + moduleSize / 2 + (dimension - 1) * moduleSize}))
		return {};

#ifdef PRINT_DEBUG
	LogMatrix log;
	LogMatrixWriter lmw(log, image, 5, "grid2.pnm");
	for (int y = 0; y < dimension; y++)
		for (int x = 0; x < dimension; x++)
			log(PointF(left + (x + .5f) * moduleSize, top + (y + .5f) * moduleSize));
#endif

	// Now just read off the bits (this is a crop + subsample)
	return {Deflate(image, dimension, dimension, top + moduleSize / 2, left + moduleSize / 2, moduleSize),
			{{left, top}, {right, top}, {right, bottom}, {left, bottom}}};
}

DetectorResult DetectPureMQR(const BitMatrix& image)
{
	using Pattern = std::array<PatternView::value_type, PATTERN.size()>;

	constexpr int MIN_MODULES = Version::DimensionOfVersion(1, true);
	constexpr int MAX_MODULES = Version::DimensionOfVersion(4, true);

	int left, top, width, height;
	if (!image.findBoundingBox(left, top, width, height, MIN_MODULES) || std::abs(width - height) > 1)
		return {};
	int right  = left + width - 1;
	int bottom = top + height - 1;

	// allow corners be moved one pixel inside to accommodate for possible aliasing artifacts
	auto diagonal = BitMatrixCursorI(image, {left, top}, {1, 1}).readPatternFromBlack<Pattern>(1);
	if (!IsPattern(diagonal, PATTERN))
		return {};

	auto fpWidth = Reduce(diagonal);
	float moduleSize = float(fpWidth) / 7;
	int dimension = narrow_cast<int>(std::lround(width / moduleSize));

	if (dimension < MIN_MODULES || dimension > MAX_MODULES ||
		!image.isIn(PointF{left + moduleSize / 2 + (dimension - 1) * moduleSize,
						   top + moduleSize / 2 + (dimension - 1) * moduleSize}))
		return {};

#ifdef PRINT_DEBUG
	LogMatrix log;
	LogMatrixWriter lmw(log, image, 5, "grid2.pnm");
	for (int y = 0; y < dimension; y++)
		for (int x = 0; x < dimension; x++)
			log(PointF(left + (x + .5f) * moduleSize, top + (y + .5f) * moduleSize));
#endif

	// Now just read off the bits (this is a crop + subsample)
	return {Deflate(image, dimension, dimension, top + moduleSize / 2, left + moduleSize / 2, moduleSize),
			{{left, top}, {right, top}, {right, bottom}, {left, bottom}}};
}

DetectorResult SampleMQR(const BitMatrix& image, const ConcentricPattern& fp)
{
	auto fpQuad = FindConcentricPatternCorners(image, fp, fp.size, 2);
	if (!fpQuad)
		return {};

	auto srcQuad = Rectangle(7, 7, 0.5);

#if defined(_MSVC_LANG) // TODO: see MSVC issue https://developercommunity.visualstudio.com/t/constexpr-object-is-unable-to-be-used-as/10035065
	static
#else
	constexpr
#endif
		const PointI FORMAT_INFO_COORDS[] = {{0, 8}, {1, 8}, {2, 8}, {3, 8}, {4, 8}, {5, 8}, {6, 8}, {7, 8}, {8, 8},
											 {8, 7}, {8, 6}, {8, 5}, {8, 4}, {8, 3}, {8, 2}, {8, 1}, {8, 0}};

	FormatInformation bestFI;
	PerspectiveTransform bestPT;

	for (int i = 0; i < 4; ++i) {
		auto mod2Pix = PerspectiveTransform(srcQuad, RotatedCorners(*fpQuad, i));

		auto check = [&](int i, bool checkOne) {
			auto p = mod2Pix(centered(FORMAT_INFO_COORDS[i]));
			return image.isIn(p) && (!checkOne || image.get(p));
		};

		// check that we see both innermost timing pattern modules
		if (!check(0, true) || !check(8, false) || !check(16, true))
			continue;

		int formatInfoBits = 0;
		for (int i = 1; i <= 15; ++i)
			AppendBit(formatInfoBits, image.get(mod2Pix(centered(FORMAT_INFO_COORDS[i]))));

		auto fi = FormatInformation::DecodeMQR(formatInfoBits);
		if (fi.hammingDistance < bestFI.hammingDistance) {
			bestFI = fi;
			bestPT = mod2Pix;
		}
	}

	if (!bestFI.isValid())
		return {};

	const int dim = Version::DimensionOfVersion(bestFI.microVersion, true);

	// check that we are in fact not looking at a corner of a non-micro QRCode symbol
	// we accept at most 1/3rd black pixels in the quite zone (in a QRCode symbol we expect about 1/2).
	int blackPixels = 0;
	for (int i = 0; i < dim; ++i) {
		auto px = bestPT(centered(PointI{i, dim}));
		auto py = bestPT(centered(PointI{dim, i}));
		blackPixels += (image.isIn(px) && image.get(px)) + (image.isIn(py) && image.get(py));
	}
	if (blackPixels > 2 * dim / 3)
		return {};

	return SampleGrid(image, dim, dim, bestPT);
}

} // namespace ZXing::QRCode
