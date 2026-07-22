/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
* Copyright 2020 Axel Waggershauser
* Copyright 2023 gitlost
*/
// SPDX-License-Identifier: Apache-2.0

#include "QRDetector.h"

#include "BinaryBitmap.h"
#include "DecoderResult.h"
#include "QRDecoder.h"
#include "BitArray.h"
#include "BitMatrix.h"
#include "BitMatrixCursor.h"
#include "ConcentricFinder.h"
#include "GridSampler.h"
#include "LogMatrix.h"
#include "Matrix.h"
#include "Pattern.h"
#include "QRFormatInformation.h"
#include "QRVersion.h"
#include "Quadrilateral.h"
#include "RegressionLine.h"

#include <algorithm>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <iterator>
#include <map>
#include <numbers>
#include <utility>
#include <vector>

#ifdef PRINT_DEBUG
#include "BitMatrixIO.h"
#else
#define printf(...){}
#endif

namespace ZXing::QRCode {

constexpr auto PATTERN = FixedPattern<5, 7>{1, 1, 3, 1, 1};
constexpr bool E2E = true;

PatternView FindPattern(const PatternView& view)
{
	return FindLeftGuard<PATTERN.size()>(view, PATTERN.size(), [](const PatternView& view, int spaceInPixel) {
		// perform a fast plausibility test for 1:1:3:1:1 pattern
		if (view[2] < 3 || view[2] < 2 * std::max(view[0], view[4]) || view[2] < std::max(view[1], view[3]))
			return 0.;
		return IsPattern<E2E>(view, PATTERN, spaceInPixel, 0.1); // the requires 4, here we accept almost 0
	});
}

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
	[[maybe_unused]] int N = 0;
	PatternRow row;

	for (int y = skip - 1; y < height; y += skip) {
		GetPatternRow(image, y, row, false);
		PatternView next = row;

		while (next = FindPattern(next), next.isValid()) {
			PointF p(next.pixelsInFront() + next[0] + next[1] + next[2] / 2.0, y + 0.5);

			// make sure p is not 'inside' an already found pattern area
			if (FindIf(res, [p](const auto& old) { return distance(p, old) < old.size / 2; }) == res.end()) {
				log(p);
				N++;
				auto width = 2 * next.sum(); // the factor 2 allows for a maximum aspect ratio of 4:1 due to perspective distortion
				auto pattern = LocateConcentricPattern<E2E>(image, PATTERN, p, width);
				if (pattern) {
					log(*pattern, 3);
					log(*pattern + PointF(.2, 0), 3);
					log(*pattern - PointF(.2, 0), 3);
					log(*pattern + PointF(0, .2), 3);
					log(*pattern - PointF(0, .2), 3);
					assert(image.get(pattern->x, pattern->y));
					res.push_back(*pattern);
				}
			}

			next.skipPair();
			next.skipPair();
			next.extend();
		}
	}

	printf("FPs: FindPattern: %d, LocateConcentric: %d\n", N, Size(res));

	return res;
}

/**
 * @brief GenerateFinderPatternSets
 * @param patterns list of ConcentricPattern objects, i.e. found finder pattern squares
 * @return list of plausible finder pattern sets, sorted by decreasing plausibility
 */
FinderPatternSets GenerateFinderPatternSets(FinderPatterns& patterns)
{
	std::sort(patterns.begin(), patterns.end(), [](const auto& a, const auto& b) { return a.size > b.size; });

	struct {
		int rejSize = 0;
		int nearFPs = 0;
		int candidates = 0;
		int rejLegRatio = 0;
		int rejModCount = 0;
		int rejAngle = 0;
		int accepted = 0;
	} stats;

	auto sets            = std::multimap<double, FinderPatternSet>();
	auto squaredDistance = [](const auto* a, const auto* b) {
		// The scaling of the distance based on the b/a size ratio is a very coarse compensation for the shortening effect of
		// the camera projection on slanted symbols. The fact that the size of the finder pattern is proportional to the
		// distance from the camera is used here. This approximation only works if a < b < 2*a (see below).
		// Test image: fix-finderpattern-order.jpg
		// Originally, I scaled the squaredDistance with the (b/a)^2 ratio but that could skew the cosine calculation
		// below too much, resulting in the acceptance of degenerate triangles (a, b and c on a line).
		return dot((*a - *b), (*a - *b)) * double(b->size) / a->size;
	};
	const double cosUpper = std::cos(60. / 180 * std::numbers::pi);
	const double cosLower = std::cos(120. / 180 * std::numbers::pi);

#if 1
	if (Size(patterns) < 3)
		return {};

	// Bin finder patterns into spatial bins to reduce the number of candidates to compare with geometry heuristics below.
	// For each finder pattern, we only compare it to patterns in bins that are not further away than the largest symbol (177 modules)
	// can occupy.  We search from inside out and stop after we found a limited number of candidates which reduces the complexity from
	// O(n^3) to O(n). E.g. a sample with 140 small QRCodes has 420 finder patterns, which results in 12 million candidates to process
	// while with the binning, we only compare 20k candidates -> total runtime goes from 190ms to 9ms.
	auto [mX, MX] = std::ranges::minmax_element(patterns, {}, &PointF::x);
	auto [mY, MY] = std::ranges::minmax_element(patterns, {}, &PointF::y);
	int medianSize = patterns[Size(patterns) / 2].size;
	int binSize = std::max(32, medianSize * 3); // 3 for minimum symbol size of 21 modules
	Matrix<std::vector<int>> bins(std::ceil((MX->x - mX->x + 1) / binSize), std::ceil((MY->y - mY->y + 1) / binSize));

	printf("medianSize=%d binSize=%d bins=(%dx%d) ", medianSize, binSize, bins.width(), bins.height());

	auto bin = [&](PointF p) {
		return PointI(std::clamp(int((p.x - mX->x) / binSize), 0, bins.width() - 1),
					  std::clamp(int((p.y - mY->y) / binSize), 0, bins.height() - 1));
	};

	for (int idx = 0; idx < Size(patterns); ++idx)
		bins(bin(patterns[idx])).push_back(idx);

	constexpr double maxModuleCount = 177 * 1.5;
	// manually tuned to work with e.g. https://github.com/eventualbuddha/zedbar/blob/f0d9d9fa6158c108a21f7cde42c0339fb32dff69/examples/qr-code-140-grid02.jpg
	constexpr size_t maxCandidates = 15;
	auto candidates = std::vector<int>();
	candidates.reserve(maxCandidates * 2);

	int nbPatterns = Size(patterns);
	bool useFilters = nbPatterns > 5; // for a small number of patterns, we apply no/less filters (like size ratio, leg ratio, angle)
	for (int i = 0; i < nbPatterns - 2; i++) {
		const auto* c0 = &patterns[i];
		double maxDistToC = c0->size / 7.0 * maxModuleCount;
		auto cBin = bin(*c0);
		int binRadius = std::ceil(maxDistToC / binSize);
		candidates.clear();

		for (auto d : Spiral(binRadius)) {
			auto b = cBin + d;
			if (b.x < 0 || b.x >= bins.width() || b.y < 0 || b.y >= bins.height())
				continue;

			for (int idx : bins(b)) {
				if (idx <= i)
					continue;

				const auto* p = &patterns[idx];
				if (useFilters && c0->size > p->size * 2 + 2) {
					stats.rejSize++;
					continue;
				}

				candidates.push_back(idx);
				stats.nearFPs++;
			}

			if (candidates.size() >= maxCandidates)
				break;
		}

		for (int u = 0; u < Size(candidates) - 1; ++u) {
			for (int v = u + 1; v < Size(candidates); ++v) {
				stats.candidates++;
				int j = candidates[u];
				int k = candidates[v];

				// patterns is sorted descending by size (the larger the pattern, the less likely is it noise),
				// but the geometry/size heuristics below assume a <= b <= c in size. Keep that convention by remapping indices.
				const auto* a = &patterns[std::max(j, k)];
				const auto* b = &patterns[std::min(j, k)];
				const auto* c = c0;

#else

	int nbPatterns = Size(patterns);
	for (int i = 0; i < nbPatterns - 2; i++) {
		for (int j = i + 1; j < nbPatterns - 1; j++) {
			for (int k = j + 1; k < nbPatterns - 0; k++) {
				stats.candidates++;
				// patterns is sorted descending by size (the larger the pattern, the less likely is it noise),
				// but the geometry/size heuristics below assume a <= b <= c in size. Keep that convention by remapping i/j/k.
				const auto* a = &patterns[k];
				const auto* b = &patterns[j];
				const auto* c = &patterns[i];

				// if the pattern sizes are too different to be part of the same symbol, skip this
				// and the rest of the innermost loop (sorted list)
				if (c->size > a->size * 2)
					break;
#endif

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

				// Make sure distAB and distBC don't differ more than reasonable:
				// equivalent to distAB > 2 * distBC || distBC > 2 * distAB but avoids sqrt.
				// TODO: make sure the constant 2 is not too conservative for reasonably tilted symbols
				if (useFilters && (distAB2 > 4 * distBC2 || distBC2 > 4 * distAB2)) {
					stats.rejLegRatio++;
					continue;
				}

				auto distAB = std::sqrt(distAB2);
				auto distBC = std::sqrt(distBC2);

				// Estimate the module count and ignore this set if it can not result in a valid decoding
				if (auto moduleCount = (distAB + distBC) / (2 * (a->size + b->size + c->size) / (3 * 7.f)) + 7;
					moduleCount < 21 * 0.9 || moduleCount > 177 * 1.5) { // moduleCount may be overestimated, see above
					stats.rejModCount++;
					continue;
				}

				// Make sure the angle between AB and BC does not deviate from 90° too much
				auto cosAB_BC = (distAB2 + distBC2 - distAC2) / (2 * distAB * distBC);
				if (useFilters && (std::isnan(cosAB_BC) || cosAB_BC > cosUpper || cosAB_BC < cosLower)) {
					stats.rejAngle++;
					continue;
				}

				// a^2 + b^2 = c^2 (Pythagorean theorem), and a = b (isosceles triangle).
				// Since any right triangle satisfies the formula c^2 - b^2 - a^2 = 0,
				// we need to check both two equal sides separately.
				// The value of |c^2 - 2 * b^2| + |c^2 - 2 * a^2| increases as dissimilarity
				// from isosceles right triangle.
				// double score = (std::abs(distAC2 - 2 * distAB2) + std::abs(distAC2 - 2 * distBC2));

				// Calculate a score that is used to determine wich sets are most likely to be actual finder pattern sets,
				// the smaller the better. Prefer finder patterns that are close to each with similar distances to each other.
				// Note: experiments incorporating cosAB_BC or the difference of the finder pattern sizes did not yield better results.
				auto score = distAB + distBC + std::abs(distAB - distBC);

				// arbitrarily limit the number of potential sets
				// (this has performance implications while limiting the maximal number of detected symbols)
				const size_t setSizeLimit = 256;
				if (sets.size() < setSizeLimit || sets.crbegin()->first > score) {
					// Use cross product to figure out whether A and C are correct or flipped.
					// This asks whether BC x BA has a positive z component, which is the arrangement
					// we want for A, B, C. If it's negative then swap A and C.
					if (cross(*c - *b, *a - *b) < 0)
						std::swap(a, c);

					sets.emplace(score, FinderPatternSet{*a, *b, *c});
					if (sets.size() > setSizeLimit)
						sets.erase(std::prev(sets.end()));
					stats.accepted++;
				}
			}
		}
	}

	printf("rejectSize=%d nearFPs=%d candidates=%d rejectLeg=%d rejectMod=%d rejectAng=%d accepted=%d\n", stats.rejSize,
		   stats.nearFPs, stats.candidates, stats.rejLegRatio, stats.rejModCount, stats.rejAngle, stats.accepted);

	// convert from multimap to vector
	FinderPatternSets res;
	res.reserve(sets.size());
	for (auto& [d, s] : sets)
		res.push_back(s);

	printf("FPSets: %d\n", Size(res));

	return res;
}

static double EstimateModuleSize(const BitMatrix& image, ConcentricPattern a, ConcentricPattern b)
{
	BitMatrixCursorF cur(image, a, b - a);
	assert(cur.isBlack());

	auto pattern = ReadSymmetricPattern<5>(cur, a.size * 2);
	if (!pattern || !IsPattern<true>(*pattern, PATTERN))
		return -1;

	return (2 * Reduce(*pattern) - (*pattern)[0] - (*pattern)[4]) / 12.0 * length(cur.d);
}

struct DimensionEstimate
{
	int dim = 0;
	double ms = 0;
	int err = 4;
};

static DimensionEstimate EstimateDimension(const BitMatrix& image, ConcentricPattern a, ConcentricPattern b)
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
	while (curI.isIn() && !curI.edgeAtBack()) {
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

static PerspectiveTransform Mod2Pix(int dimension, PointF brOffset, QuadrilateralF pix)
{
	auto quad = Rectangle(dimension, dimension, 3.5);
	quad[2] = quad[2] - brOffset;
	return {quad, pix};
}

static std::optional<PointF> LocateAlignmentPattern(const BitMatrix& image, int moduleSize, PointF estimate)
{
	log(estimate, 4);

	for (auto d : {PointF{0, 0}, {0, -1}, {0, 1}, {-1, 0}, {1, 0}, {-1, -1}, {1, -1}, {1, 1}, {-1, 1},
#if 1
				   }) {
#else
				   {0, -2}, {0, 2}, {-2, 0}, {2, 0}, {-1, -2}, {1, -2}, {-1, 2}, {1, 2}, {-2, -1}, {-2, 1}, {2, -1}, {2, 1}}) {
#endif
		auto p = estimate + moduleSize * 2.25 * d;
		if (!image.isIn(p))
			continue;

		auto cor = CenterOfRing(image, PointI(p), moduleSize * 3, 1, false);

		// if we did not land on a black pixel the concentric pattern finder will fail
		if (!cor || !image.get(*cor))
			continue;

		if (auto cor1 = CenterOfRing(image, PointI(*cor), moduleSize * 2, 1))
			if (auto cor2 = CenterOfRing(image, PointI(*cor), moduleSize * 3, 2))
				if (distance(*cor1, *cor2) < moduleSize / 2 && cor2->size > cor1->size) {
					auto res = (*cor1 + *cor2) / 2;
					log(res, 3);
					return res;
				}
	}

	return {};
}

static const Version* ReadVersion(const BitMatrix& image, int dimension, const PerspectiveTransform& mod2Pix)
{
	int bits[2] = {};

	for (bool mirror : {false, true}) {
		// Read top-right/bottom-left version info: 3 wide by 6 tall (depending on mirrored)
		int versionBits = 0;
		for (int y = 5; y >= 0; --y)
			for (int x = dimension - 9; x >= dimension - 11; --x) {
				auto mod = mirror ? PointI{y, x} : PointI{x, y};
				auto pix = mod2Pix(centered(mod));
				if (!image.isIn(pix))
					versionBits = -1;
				else
					AppendBit(versionBits, image.get(pix));
				log(pix, 3);
			}
		bits[static_cast<int>(mirror)] = versionBits;
	}

	return Version::DecodeVersionInformation(bits[0], bits[1]);
}

DetectorResults SampleQR(const BitMatrix& image, const FinderPatternSet& fp)
{
	auto top  = EstimateDimension(image, fp.tl, fp.tr);
	auto left = EstimateDimension(image, fp.tl, fp.bl);

	if (!top.dim && !left.dim)
		co_return;

	auto best = top.err == left.err ? (top.dim > left.dim ? top : left) : (top.err < left.err ? top : left);
	int dimension = best.dim;
	int moduleSize = static_cast<int>(top.dim == left.dim ? std::midpoint(top.ms, left.ms) : best.ms) + 1;

	auto br = PointF{-1, -1};
	auto brOffset = PointF{3, 3};
	bool brFound = false;

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

		if (dimension > 21)
			if (auto brCP = LocateAlignmentPattern(image, moduleSize, brInter))
				br = *brCP;

		brFound = image.isIn(br);
		if (!brFound)
			br = brInter;
	}

	// otherwise or if finder patterns are not square, the simple estimation used by upstream is used as a best guess fallback
	if (!image.isIn(br) || !FitSquareToPoints(image, fp.bl, fp.bl.size, 2, false)) {
		br = fp.tr - fp.tl + fp.bl;
		brOffset = PointF(0, 0);
	}

	log(br, 3);
	auto mod2Pix = Mod2Pix(dimension, brOffset, {fp.tl, fp.tr, br, fp.bl});

	if( dimension >= Version::SymbolSize(7, Type::Model2).x) {
		auto version = ReadVersion(image, dimension, mod2Pix);

		// if the version bits are garbage -> discard the detection
		if (!version || std::min(std::abs(version->dimension() - top.dim), std::abs(version->dimension() - left.dim)) > 8)
			co_return;
		if (version->dimension() != dimension) {
			printf("update dimension: %d -> %d\n", dimension, version->dimension());
			dimension = version->dimension();
			mod2Pix = Mod2Pix(dimension, brOffset, {fp.tl, fp.tr, br, fp.bl});
		}

#if 1 // finding and evaluating the alignment patterns to enable a tiled sampling of the symbol

		auto& apM = version->alignmentPatternCenters(); // alignment pattern positions in modules
		auto apP = Matrix<std::optional<PointF>>(Size(apM), Size(apM)); // found/guessed alignment pattern positions in pixels
		const int N = Size(apM) - 1;

		// project the alignment pattern at module coordinates x/y to pixel coordinate based on current mod2Pix
		auto projectM2P = [&mod2Pix, &apM](int x, int y) { return mod2Pix(centered(PointI(apM[x], apM[y]))); };

		auto findInnerCornerOfConcentricPattern = [&image, &apP, &projectM2P](int x, int y, const ConcentricPattern& fp) {
			auto pc = *apP.set(x, y, projectM2P(x, y));
			if (auto fpQuad = FindConcentricPatternCorners(image, fp, fp.size, 2))
				for (auto c : *fpQuad)
					if (distance(c, pc) < fp.size / 2)
						apP.set(x, y, c);
		};

		findInnerCornerOfConcentricPattern(0, 0, fp.tl);
		findInnerCornerOfConcentricPattern(0, N, fp.bl);
		findInnerCornerOfConcentricPattern(N, 0, fp.tr);

		auto bestGuessAPP = [&](int x, int y){
			if (auto p = apP(x, y))
				return *p;
			return projectM2P(x, y);
		};

		for (int y = 0; y <= N; ++y)
			for (int x = 0; x <= N; ++x) {
				if (apP(x, y))
					continue;

				PointF guessed =
					x * y == 0 ? bestGuessAPP(x, y) : bestGuessAPP(x - 1, y) + bestGuessAPP(x, y - 1) - bestGuessAPP(x - 1, y - 1);
				if (auto found = LocateAlignmentPattern(image, moduleSize, guessed))
					apP.set(x, y, found);
			}

		// go over the whole set of alignment patters again and try to fill any remaining gap by using available neighbors as guides
		for (int y = 0; y <= N; ++y)
			for (int x = 0; x <= N; ++x) {
				if (apP(x, y))
					continue;

				// find the two closest valid alignment pattern pixel positions both horizontally and vertically
				std::vector<PointF> hori, verti;
				for (int i = 2; i < 2 * N + 2 && Size(hori) < 2; ++i) {
					int xi = x + i / 2 * (i%2 ? 1 : -1);
					if (0 <= xi && xi <= N && apP(xi, y))
						hori.push_back(*apP(xi, y));
				}
				for (int i = 2; i < 2 * N + 2 && Size(verti) < 2; ++i) {
					int yi = y + i / 2 * (i%2 ? 1 : -1);
					if (0 <= yi && yi <= N && apP(x, yi))
						verti.push_back(*apP(x, yi));
				}

				// if we found 2 each, intersect the two lines that are formed by connecting the point pairs
				if (Size(hori) == 2 && Size(verti) == 2) {
					auto guessed = intersect(RegressionLine(hori[0], hori[1]), RegressionLine(verti[0], verti[1]));
					auto found = LocateAlignmentPattern(image, moduleSize, guessed);
					// search again near that intersection and if the search fails, use the intersection
					if (!found) printf("location guessed at %dx%d\n", x, y);
					apP.set(x, y, found ? *found : guessed);
				}
			}

		if (auto c = apP.get(N, N))
			mod2Pix = Mod2Pix(dimension, PointF(3, 3), {fp.tl, fp.tr, *c, fp.bl});

		co_yield SampleGrid(image, dimension, dimension, mod2Pix, std::move(apP), apM, apM);
#endif
	}
	else
		co_yield SampleGrid(image, dimension, dimension, mod2Pix);

	// if we have not found the br alignment pattern, we check
	// a) if we have a version 1 symbol and tried and failed with the intersection of the trace lines (#1086), or
	// b) if the symbol is almost level and the resolution of the RegressionLines is not sufficient (#199 and estimate-tilt.jpg)
	// we then try the fallback method of sampling the symbol with the br corner extrapolated from the other three corners.
	if (!brFound
		&& ((dimension == 21 && brOffset != PointF(0, 0))
			|| (EstimateTilt(fp) < 1.1 && !(bl2.isHighRes() && bl3.isHighRes() && tr2.isHighRes() && tr3.isHighRes()))))
		{
			mod2Pix = Mod2Pix(dimension, PointF(0, 0), {fp.tl, fp.tr, fp.tr - fp.tl + fp.bl, fp.bl});
			co_yield SampleGrid(image, dimension, dimension, mod2Pix);
		}
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

	constexpr int MIN_MODULES = Version::SymbolSize(1, Type::Model2).x;

	int left, top, width, height;
	if (!image.findBoundingBox(left, top, width, height, MIN_MODULES) || std::abs(width - height) > 1)
		return {};
	auto pos = Rectangle<PointI>(left, top, width, height);

	const PointI &tl = pos.topLeft(), &tr = pos.topRight(), &bl = pos.bottomLeft();
	Pattern diagonal;
	// allow corners be moved one pixel inside to accommodate for possible aliasing artifacts
	for (auto [p, d] : {std::pair(tl, PointI{1, 1}), {tr, {-1, 1}}, {bl, {1, -1}}}) {
		diagonal = BitMatrixCursorI(image, p, d).readPatternFromBlack<Pattern>(1, width / 3 + 1);
		if (!IsPattern(diagonal, PATTERN))
			return {};
	}

	PointF::value_t fpWidth = Reduce(diagonal);
	auto dimension =
		EstimateDimension(image, {tl + fpWidth / 2 * PointF(1, 1), fpWidth}, {tr + fpWidth / 2 * PointF(-1, 1), fpWidth}).dim;

	float moduleSize = float(width) / dimension;
	if (!Version::IsValidSize({dimension, dimension}, Type::Model2) ||
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
	return {Deflate(image, dimension, dimension, top + moduleSize / 2, left + moduleSize / 2, moduleSize), std::move(pos)};
}

DetectorResult DetectPureMQR(const BitMatrix& image)
{
	using Pattern = std::array<PatternView::value_type, PATTERN.size()>;

	constexpr int MIN_MODULES = Version::SymbolSize(1, Type::Micro).x;

	int left, top, width, height;
	if (!image.findBoundingBox(left, top, width, height, MIN_MODULES) || std::abs(width - height) > 1)
		return {};

	// allow corners be moved one pixel inside to accommodate for possible aliasing artifacts
	auto diagonal = BitMatrixCursorI(image, {left, top}, {1, 1}).readPatternFromBlack<Pattern>(1);
	if (!IsPattern(diagonal, PATTERN))
		return {};

	auto fpWidth = Reduce(diagonal);
	float moduleSize = float(fpWidth) / 7;
	int dimension = narrow_cast<int>(std::lround(width / moduleSize));

	if (!Version::IsValidSize({dimension, dimension}, Type::Micro) ||
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
			Rectangle<PointI>(left, top, width, height)};
}

DetectorResult DetectPureRMQR(const BitMatrix& image)
{
	constexpr auto SUBPATTERN = FixedPattern<4, 4>{1, 1, 1, 1};
	constexpr auto TIMINGPATTERN = FixedPattern<10, 10>{1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

	using Pattern = std::array<PatternView::value_type, PATTERN.size()>;
	using SubPattern = std::array<PatternView::value_type, SUBPATTERN.size()>;
	using TimingPattern = std::array<PatternView::value_type, TIMINGPATTERN.size()>;

#ifdef PRINT_DEBUG
	SaveAsPBM(image, "weg.pbm");
#endif

	constexpr int MIN_MODULES = Version::SymbolSize(1, Type::rMQR).y;

	int left, top, width, height;
	if (!image.findBoundingBox(left, top, width, height, MIN_MODULES) || height >= width)
		return {};
	auto pos = Rectangle<PointI>(left, top, width, height);

	const PointI &tl = pos.topLeft(), &tr = pos.topRight(), &bl = pos.bottomLeft(), &br = pos.bottomRight();

	// allow corners be moved one pixel inside to accommodate for possible aliasing artifacts
	auto diagonal = BitMatrixCursorI(image, tl, {1, 1}).readPatternFromBlack<Pattern>(1);
	if (!IsPattern(diagonal, PATTERN))
		return {};

	// Finder sub pattern
	auto subdiagonal = BitMatrixCursorI(image, br, {-1, -1}).readPatternFromBlack<SubPattern>(1);
	if (!IsPattern(subdiagonal, SUBPATTERN))
		return {};

	float moduleSize = Reduce(diagonal) + Reduce(subdiagonal);

	// Horizontal timing patterns
	for (auto [p, d] : {std::pair(tr, PointI{-1, 0}), {bl, {1, 0}}, {tl, {1, 0}}, {br, {-1, 0}}}) {
		auto cur = BitMatrixCursorI(image, p, d);
		// skip corner / finder / sub pattern edge
		cur.stepToEdge(2 + cur.isWhite());
		auto timing = cur.readPattern<TimingPattern>();
		if (!IsPattern(timing, TIMINGPATTERN))
			return {};
		moduleSize += Reduce(timing);
	}

	moduleSize /= 7 + 4 + 4 * 10; // fp + sub + 4 x timing
	int dimW = narrow_cast<int>(std::lround(width / moduleSize));
	int dimH = narrow_cast<int>(std::lround(height / moduleSize));

	if (!Version::IsValidSize(PointI{dimW, dimH}, Type::rMQR))
		return {};

#ifdef PRINT_DEBUG
	LogMatrix log;
	LogMatrixWriter lmw(log, image, 5, "grid2.pnm");
	for (int y = 0; y < dimH; y++)
		for (int x = 0; x < dimW; x++)
			log(pos.topLeft() + moduleSize * PointF(x + .5f, y + .5f));
#endif

	// Now just read off the bits (this is a crop + subsample)
	return {Deflate(image, dimW, dimH, top + moduleSize / 2, left + moduleSize / 2, moduleSize), std::move(pos)};
}

DetectorResult SampleMQR(const BitMatrix& image, const ConcentricPattern& fp)
{
	auto fpQuad = FindConcentricPatternCorners(image, fp, fp.size, 2);
	if (!fpQuad)
		return {};

	auto srcQuad = Rectangle(7, 7, 0.5);

#if defined(_MSVC_LANG) && !(_MSC_VER >= 1940) // VS2022 17.10 and later work
	// see MSVC issue https://developercommunity.visualstudio.com/t/constexpr-object-is-unable-to-be-used-as/10035065
	static
#else
	constexpr
#endif
		const PointI FORMAT_INFO_COORDS[] = {{0, 8}, {1, 8}, {2, 8}, {3, 8}, {4, 8}, {5, 8}, {6, 8}, {7, 8}, {8, 8},
											 {8, 7}, {8, 6}, {8, 5}, {8, 4}, {8, 3}, {8, 2}, {8, 1}, {8, 0}};

	FormatInformation bestFI;
	PerspectiveTransform bestPT;
	BitMatrixCursorF cur(image, {}, {});

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
			AppendBit(formatInfoBits, cur.blackAt(mod2Pix(centered(FORMAT_INFO_COORDS[i]))));

		auto fi = FormatInformation::DecodeMQR(formatInfoBits);
		if (fi.hammingDistance < bestFI.hammingDistance) {
			bestFI = fi;
			bestPT = mod2Pix;
		}
	}

	if (!bestFI.isValid())
		return {};

	const int dim = Version::SymbolSize(bestFI.microVersion, Type::Micro).x;

	// check that we are in fact not looking at a corner of a non-micro QRCode symbol
	// we accept at most 1/3rd black pixels in the quite zone (in a QRCode symbol we expect about 1/2).
	int blackPixels = 0;
	for (int i = 0; i < dim; ++i) {
		auto px = bestPT(centered(PointI{i, dim}));
		auto py = bestPT(centered(PointI{dim, i}));
		blackPixels += cur.blackAt(px) && cur.blackAt(py);
	}
	if (blackPixels > 2 * dim / 3)
		return {};

	return SampleGrid(image, dim, dim, bestPT);
}

// Follow an rMQR edge timing pattern, starting at pixel `start` stepping `stepX` per
// module, for `nMods` modules. Advances transition-by-transition; a gap longer than one
// module (a solid function pattern interrupting the timing) is counted by rounding the
// travelled distance to whole modules. The direction and module size are continuously
// re-estimated from the accumulated displacement so the trace follows steep rotation /
// perspective. Returns the pixel position `nMods` modules along.
static std::optional<PointF> TraceTimingLine(const BitMatrix& image, PointF start, PointF stepX, int nMods)
{
	double modSize = length(stepX);
	if (modSize < 1 || nMods < 1)
		return {};
	BitMatrixCursorF cur(image, start, bresenhamDirection(stepX));
	if (!cur.isIn())
		return {};
	double travelled = 0;
	int guard = nMods * 4 + 20;
	PointF anchor = cur.p;
	double anchorTravelled = 0;
	while (travelled < nMods - 0.25 && guard-- > 0) {
		PointF prev = cur.p;
		if (!cur.stepToEdge(1, static_cast<int>(modSize * 5) + 8))
			return {};
		travelled += std::max(1.0, std::round(distance(cur.p, prev) / modSize));
		// Re-estimate the line direction and module size from the accumulated displacement
		// so the trace follows tilt / perspective instead of walking off the timing row.
		if (travelled - anchorTravelled >= 3) {
			PointF disp = cur.p - anchor;
			double n = travelled - anchorTravelled;
			if (length(disp) > modSize) {
				cur.setDirection(disp);
				modSize = length(disp) / n;
			}
			anchor = cur.p;
			anchorTravelled = travelled;
		}
	}
	return travelled >= nMods - 0.5 ? std::optional<PointF>(cur.p) : std::nullopt;
}


// Robust straight-line fit  p(t) = A*t + B  over (t, point) samples. A line models a
// rMQR edge under rotation/skew exactly and mild perspective closely, so residuals expose
// the discrete jumps produced by a timing mis-count: while the worst residual exceeds `tol`
// (and more than 3 samples remain) the worst sample is dropped and the line is refit.
struct EdgeFit { PointF A{}, B{}; bool ok = false; int inliers = 0; };
static EdgeFit fitEdge(std::vector<std::pair<double, PointF>> s, double tol)
{
	while (Size(s) >= 2) {
		double st = 0, stt = 0;
		PointF sp{}, stp{};
		for (const auto& [t, p] : s) { st += t; stt += t * t; sp = sp + p; stp = stp + t * p; }
		const double n = Size(s);
		const double denom = n * stt - st * st;
		if (std::abs(denom) < 1e-6)
			return {};
		const PointF A = (1.0 / denom) * (n * stp - st * sp);
		const PointF B = (1.0 / n) * (sp - st * A);
		double worst = -1;
		size_t wi = 0;
		for (size_t i = 0; i < s.size(); ++i)
			if (double r = length(s[i].second - (s[i].first * A + B)); r > worst) { worst = r; wi = i; }
		if (worst <= tol || Size(s) <= 3)
			return {A, B, true, Size(s)};
		s.erase(s.begin() + wi);
	}
	return {};
}


// Modules of a short rMQR symbol whose color is known a priori - the finder pattern, both
// full-width edge timing patterns (dark on even columns, interrupted only by the always-
// dark alignment columns), the corner pattern and the finder sub pattern. Used both to
// calibrate luminance thresholds and to score how well a sampled grid matches reality.
struct KnownModule
{
	int x, y;
	bool dark;
};
static std::vector<KnownModule> RMQRKnownModules(PointI dim, const Version* version)
{
	std::vector<KnownModule> res;
	res.reserve(2 * dim.x + 24);
	// finder pattern: dark core, ring corners and midpoints; light separator ring
	for (auto [x, y] : {std::pair{3, 3}, {2, 3}, {4, 3}, {3, 2}, {3, 4}, {0, 0}, {6, 0}, {0, 6}, {6, 6}, {3, 0}, {0, 3}, {6, 3}, {3, 6}})
		res.push_back({x, y, true});
	for (auto [x, y] : {std::pair{1, 1}, {5, 1}, {1, 5}, {5, 5}, {3, 1}, {1, 3}, {5, 3}, {3, 5}})
		res.push_back({x, y, false});
	// both edge timing patterns
	for (int c = 7; c < dim.x; ++c) {
		const bool align = version && Contains(version->alignmentPatternCenters(), c);
		res.push_back({c, 0, align || c == dim.x - 1 || c % 2 == 0});
		if (c < dim.x - 5)
			res.push_back({c, dim.y - 1, align || c % 2 == 0});
		else
			res.push_back({c, dim.y - 1, true}); // sub pattern bottom edge is solid dark
	}
	// sub pattern: dark center, light ring
	res.push_back({dim.x - 3, dim.y - 3, true});
	res.push_back({dim.x - 2, dim.y - 3, false});
	res.push_back({dim.x - 3, dim.y - 2, false});
	return res;
}

// Sample a fitted short-rMQR module grid directly from the luminance image instead of the
// globally binarized one. The global binarizer thresholds with a fixed block grid whose
// alignment relative to a small, soft symbol is luck - the same symbol reads differently
// depending on where it sits in the frame (this is exactly why panning a camera across a
// wall "finds" codes a still misses). Here the symbol calibrates its own thresholds from
// its known modules, interpolated per column so illumination gradients are followed, and
// each module is read as the mean of a few sub-module taps against its local threshold.
static DetectorResult SampleRMQRLuma(const BinaryBitmap& lum, bool inverted, PointI dim, const ROIs& rois,
									 const std::vector<KnownModule>& known)
{
	// pixel position of a module-space coordinate via its covering ROI
	auto pix = [&](double x, double y) -> std::optional<PointF> {
		for (auto&& [x0, x1, y0, y1, mod2Pix] : rois)
			if (x0 <= x && x <= x1 && y0 <= y && y <= y1)
				return mod2Pix(PointF(x + 0.5, y + 0.5));
		return {};
	};
	auto lumaAt = [&](PointF p) { return lum.luma(narrow_cast<int>(p.x + 0.5), narrow_cast<int>(p.y + 0.5)); };

	struct Calib { int col; int v; bool dark; };
	std::vector<Calib> calib;
	calib.reserve(known.size());
	for (const auto& k : known)
		if (auto p = pix(k.x, k.y))
			if (int v = lumaAt(*p); v >= 0)
				calib.push_back({k.x, v, k.dark});

	// per-column threshold: midpoint between the means of nearby dark and light samples,
	// widening the window (up to global) until both sides have evidence
	auto threshold = [&](int col) -> std::optional<int> {
		for (int win = 8; win <= 2 * dim.x; win = std::min(win * 2, 2 * dim.x) + (win == 2 * dim.x)) {
			int sd = 0, nd = 0, sl = 0, nl = 0;
			for (const auto& c : calib)
				if (std::abs(c.col - col) <= win) {
					if (c.dark)
						sd += c.v, ++nd;
					else
						sl += c.v, ++nl;
				}
			// require both evidence and usable contrast in this window; otherwise widen -
			// a defocused region has low local contrast but the symbol as a whole thresholds
			if (nd >= 2 && nl >= 2 && std::abs(sd / nd - sl / nl) >= (win >= 2 * dim.x ? 4 : 8))
				return (sd / nd + sl / nl) / 2;
		}
		return {};
	};
	std::vector<int> thr(dim.x);
	for (int c = 0; c < dim.x; ++c) {
		auto t = threshold(c);
		if (!t)
			return {}; // no usable contrast - the caller keeps the binary sampling
		thr[c] = *t;
	}

	// read each module as the mean of five sub-module taps against its column threshold
	BitMatrix res(dim.x, dim.y);
	for (int y = 0; y < dim.y; ++y)
		for (int x = 0; x < dim.x; ++x) {
			int sum = 0, n = 0;
			for (auto [dx, dy] : {std::pair{0., 0.}, {.3, 0.}, {-.3, 0.}, {0., .3}, {0., -.3}})
				if (auto p = pix(x + dx, y + dy))
					if (int v = lumaAt(*p); v >= 0)
						sum += v, ++n;
			if (!n)
				return {};
			if (inverted ? sum / n > thr[x] : sum / n < thr[x])
				res.set(x, y);
		}

	auto projectCorner = [&](PointI p) {
		for (auto&& [x0, x1, y0, y1, mod2Pix] : rois)
			if (x0 <= p.x && p.x <= x1 && y0 <= p.y && p.y <= y1)
				return PointI(mod2Pix(PointF(p)) + PointF(0.5, 0.5));
		return PointI();
	};

	return {std::move(res),
			{projectCorner({0, 0}), projectCorner({dim.x, 0}), projectCorner({dim.x, dim.y}), projectCorner({0, dim.y})}};
}

DetectorResult SampleRMQR(const BitMatrix& image, const ConcentricPattern& fp, const BinaryBitmap* lum, bool tryHarder)
{
	// The finder pattern quad: ideally the blend of its 5x5 and 7x7 boundary squares, but on
	// a small / blurry symbol often only one of the two ring lines is traceable. Use whatever
	// is measurable - each line maps to a known square in module coordinates (the srcQuad
	// margin), and the format information's error tolerance arbitrates correctness downstream.
	auto innerQuad = FitSquareToPoints(image, fp, fp.size, 2, false);
	auto outerQuad = FitSquareToPoints(image, fp, fp.size, 3, true);
	std::optional<QuadrilateralF> fpQuad;
	double srcMargin = 0.5;
	if (innerQuad && outerQuad)
		fpQuad = Blend(*innerQuad, *outerQuad);
	else if (outerQuad)
		fpQuad = *outerQuad, srcMargin = 0.0;
	else if (innerQuad)
		fpQuad = *innerQuad, srcMargin = 1.0;
	else if (auto coreQuad = FitSquareToPoints(image, fp, fp.size, 1, false))
		fpQuad = *coreQuad, srcMargin = 2.0; // the 3x3 core boundary - sharpest feature under blur
	else {
		return {};
	}

	auto srcQuad = Rectangle(7, 7, srcMargin);

	static const PointI FORMAT_INFO_EDGE_COORDS[] = {{8, 0}, {9, 0}, {10, 0}, {11, 0}};
	static const PointI FORMAT_INFO_COORDS[] = {
		{11, 3}, {11, 2}, {11, 1},
		{10, 5}, {10, 4}, {10, 3}, {10, 2}, {10, 1},
		{ 9, 5}, { 9, 4}, { 9, 3}, { 9, 2}, { 9, 1},
		{ 8, 5}, { 8, 4}, { 8, 3}, { 8, 2}, { 8, 1},
	};

	// Luminance threshold calibrated on the finder pattern's own known modules, letting
	// the format information be read straight from grayscale - the global binarization of
	// a small blurry symbol is exactly what corrupts these reads.
	int fiThr = -1;
	if (lum && lum->luma(0, 0) >= 0) {
		const auto m2p = PerspectiveTransform(srcQuad, RotatedCorners(*fpQuad, 0));
		int sd = 0, nd = 0, sl = 0, nl = 0;
		auto sample = [&](int x, int y, bool dark) {
			const PointF p = m2p(centered(PointI(x, y)));
			if (int v = lum->luma(narrow_cast<int>(p.x + 0.5), narrow_cast<int>(p.y + 0.5)); v >= 0) {
				if (dark)
					sd += v, ++nd;
				else
					sl += v, ++nl;
			}
		};
		for (auto [x, y] : {std::pair{3, 3}, {2, 3}, {4, 3}, {3, 2}, {3, 4}, {0, 0}, {6, 0}, {0, 6}, {6, 6}, {3, 0}, {0, 3}, {6, 3}, {3, 6}})
			sample(x, y, true);
		for (auto [x, y] : {std::pair{1, 1}, {5, 1}, {1, 5}, {5, 5}, {3, 1}, {1, 3}, {5, 3}, {3, 5}})
			sample(x, y, false);
		if (nd >= 3 && nl >= 3 && std::abs(sd / nd - sl / nl) >= 8)
			fiThr = (sd / nd + sl / nl) / 2;
	}

	// Collect format information candidates instead of a single best guess: on a blurry
	// symbol the finder-local transform can corrupt the FI reads enough that a *wrong*
	// version wins the pure hamming race, and a read one bit past the BCH validity limit
	// can still be the truth. Keep the best read per distinct version (up to a loose
	// hamming bound) and let the sampled image content arbitrate below.
	struct FICand { FormatInformation fi; PerspectiveTransform pt; };
	std::vector<FICand> cands;
	BitMatrixCursorF cur(image, {}, {});

	for (int i = 0; i < 4; ++i) {
		auto mod2Pix = PerspectiveTransform(srcQuad, RotatedCorners(*fpQuad, i));

		// On a small / blurry symbol the finder-derived transform can be off by a fraction
		// of a module, which corrupts these single-pixel reads. Sample the format
		// information under a handful of sub-module offsets and keep whichever read the
		// BCH code scores best - one more instance of measure-everything-and-score.
		// a quad from a single ring line is less precise - search a denser, wider offset grid
		const double jr = srcMargin == 0.5 ? 0.2 : 0.4;
		for (double jx = -jr; jx <= jr + 0.01; jx += 0.2)
		for (double jy = -jr; jy <= jr + 0.01; jy += 0.2) {
			const PointF off{jx, jy};
			auto at = [&](PointI p) { return mod2Pix(centered(p) + off); };
			auto check = [&](int j, bool on) { return cur.testAt(at(FORMAT_INFO_EDGE_COORDS[j])) == BitMatrixCursorF::Value(on); };

			// Check that we see top edge timing pattern modules. Require most but not all:
			// a single blurred module must not veto the rotation - the format information's
			// hamming distance is the real arbiter between rotations.
			if (check(0, true) + check(1, false) + check(2, true) + check(3, false) < 3)
				continue;

			uint32_t formatInfoBits = 0;
			for (auto c : FORMAT_INFO_COORDS)
				AppendBit(formatInfoBits, cur.blackAt(at(c)));

			auto keep = [&](const FormatInformation& fi) {
				if (fi.hammingDistance > 5)
					return;
				auto same = std::find_if(cands.begin(), cands.end(), [&](auto& c) { return c.fi.microVersion == fi.microVersion; });
				if (same == cands.end())
					cands.push_back({fi, mod2Pix});
				else if (fi.hammingDistance < same->fi.hammingDistance)
					*same = {fi, mod2Pix};
			};
			keep(FormatInformation::DecodeRMQR(formatInfoBits, 0 /*formatInfoBits2*/));

			// the same read from calibrated luminance - global binarization out of the loop
			if (fiThr >= 0) {
				auto darkAt = [&](PointI c) {
					const PointF p = at(c);
					const int v = lum->luma(narrow_cast<int>(p.x + 0.5), narrow_cast<int>(p.y + 0.5));
					return v >= 0 && (lum->inverted() ? v > fiThr : v < fiThr);
				};
				if (darkAt(FORMAT_INFO_EDGE_COORDS[0]) + !darkAt(FORMAT_INFO_EDGE_COORDS[1])
						+ darkAt(FORMAT_INFO_EDGE_COORDS[2]) + !darkAt(FORMAT_INFO_EDGE_COORDS[3]) >= 3) {
					uint32_t bitsL = 0;
					for (auto c : FORMAT_INFO_COORDS)
						AppendBit(bitsL, darkAt(c));
					keep(FormatInformation::DecodeRMQR(bitsL, 0));
				}
			}
		}
	}

	if (cands.empty()) {
		return {};
	}
	std::sort(cands.begin(), cands.end(), [](auto& a, auto& b) { return a.fi.hammingDistance < b.fi.hammingDistance; });

	// TODO: this is a WIP
	auto intersectQuads = [](QuadrilateralF& a, QuadrilateralF& b) {
		auto tl = Center(a);
		auto br = Center(b);
		// rotate points such that topLeft of a is furthest away from b and topLeft of b is closest to a
		auto dist2B = [c = br](auto a, auto b) { return distance(a, c) < distance(b, c); };
		auto offsetA = narrow_cast<int>(std::max_element(a.begin(), a.end(), dist2B) - a.begin());
		auto dist2A = [c = tl](auto a, auto b) { return distance(a, c) < distance(b, c); };
		auto offsetB = narrow_cast<int>(std::min_element(b.begin(), b.end(), dist2A) - b.begin());

		a = RotatedCorners(a, offsetA);
		b = RotatedCorners(b, offsetB);

		auto tr = (intersect(RegressionLine(a[0], a[1]), RegressionLine(b[1], b[2]))
				   + intersect(RegressionLine(a[3], a[2]), RegressionLine(b[0], b[3])))
				  / 2;
		auto bl = (intersect(RegressionLine(a[0], a[3]), RegressionLine(b[2], b[3]))
				   + intersect(RegressionLine(a[1], a[2]), RegressionLine(b[0], b[1])))
				  / 2;

		log(tr, 2);
		log(bl, 2);

		return QuadrilateralF{tl, tr, br, bl};
	};

	// Attempt one candidate: build the module grid for its version and measure how well
	// the resulting geometry explains the image (the joint timing-content score of the
	// fused edges, normalized to its maximum). The confidence lets a near-valid FI read
	// prove itself against the image and arbitrates between competing versions.
	auto attempt = [&](const FormatInformation& fi, PerspectiveTransform bestPT) -> std::pair<DetectorResult, double> {
	const PointI dim = Version::SymbolSize(fi.microVersion, Type::rMQR);

	// Tall rMQR symbols: anchor the far side with the finder sub pattern. The corner
	// intersection assumes the blended finder quad; with a single-ring quad (blurry
	// finder) skip the refinement and sample with the transform we have.
	if (dim.y > 9) {
		if (auto found = LocateAlignmentPattern(image, fp.size / 7, bestPT(dim - PointF(3, 3))); found && srcMargin == 0.5)
			if (auto spQuad = FindConcentricPatternCorners(image, *found, fp.size / 2, 1)) {
				auto dest = intersectQuads(*fpQuad, *spQuad);
				dest[0] = fp;
				dest[2] = *found;
				bestPT = PerspectiveTransform({{3.5, 3.5}, {dim.x - 2.5, 3.5}, {dim.x - 2.5, dim.y - 2.5}, {3.5, dim.y - 2.5}}, dest);
			}
		return {SampleGrid(image, dim.x, dim.y, bestPT), 0.5};
	}

	// Short rMQR symbols (7/9 modules tall): one scored fusion of every available function
	// pattern. Gather anchor points from the finder, both edge timing patterns and the finder
	// sub pattern; robustly line-fit each edge (rejecting the discrete jumps of a timing
	// mis-count while tolerating rotation/skew); rebuild a wholly-lost edge from the other plus
	// the finder's vertical; then sample tiles between the fitted anchors. Features that aren't
	// reliable are simply dropped by the fit - one clean path, no separate fallbacks.
	{
		const PointF vDown = bestPT(centered(PointI(0, dim.y - 1))) - bestPT(centered(PointI(0, 0)));
		const double modSize = length(vDown) / std::max(1, dim.y - 1);
		const PointF stepTop = bestPT(centered(PointI(8, 0))) - bestPT(centered(PointI(7, 0)));
		const PointF stepBot = bestPT(centered(PointI(8, dim.y - 1))) - bestPT(centered(PointI(7, dim.y - 1)));

		std::vector<std::pair<double, PointF>> topS, botS;
		for (int c : {0, 6}) { // two reliable finder anchors per edge
			topS.push_back({double(c), bestPT(centered(PointI(c, 0)))});
			botS.push_back({double(c), bestPT(centered(PointI(c, dim.y - 1)))});
		}
		// Reject a traced anchor that drifted *perpendicular* to the edge (off this symbol's
		// row, e.g. onto a neighbouring symbol on a steep angle). Drift *along* the edge is
		// the legitimate correction the trace exists for, so only the perpendicular
		// component is checked against the finder's prediction.
		auto perpDev = [](PointF v, PointF dir) {
			PointF u = (1.0 / length(dir)) * dir;
			return length(v - dot(v, u) * u);
		};
		for (int c : {dim.x / 3, dim.x * 2 / 3, dim.x - 1}) { // both edge timing patterns
			if (auto t = TraceTimingLine(image, bestPT(centered(PointI(7, 0))), stepTop, c - 7))
				if (perpDev(*t - bestPT(centered(PointI(c, 0))), stepTop) < 2.5 * modSize) topS.push_back({double(c), *t});
			if (auto b = TraceTimingLine(image, bestPT(centered(PointI(7, dim.y - 1))), stepBot, c - 7))
				if (perpDev(*b - bestPT(centered(PointI(c, dim.y - 1))), stepBot) < 2.5 * modSize) botS.push_back({double(c), *b});
		}
		// finder sub pattern: an independent far anchor, projected onto the bottom edge row
		if (auto sp = LocateAlignmentPattern(image, fp.size / 7, bestPT(dim - PointF(3, 3))))
			if (auto spQuad = FindConcentricPatternCorners(image, *sp, fp.size / 2, 1))
				botS.push_back({double(dim.x) - 1.5, spQuad->bottomRight() + (1.0 / std::max(1, dim.y - 1)) * vDown});

		EdgeFit ft = fitEdge(topS, 0.7 * modSize), fb = fitEdge(botS, 0.7 * modSize);
		// Score a candidate edge line against the row's known content: the edge timing
		// pattern alternates dark/light (dark on even columns) along the entire edge, broken
		// only by the dark alignment columns and the corner / sub pattern at the right end,
		// which are skipped. The number of module centers reading back the expected color
		// measures how well the line explains the image.
		const Version* version = Version::rMQR(fi.microVersion);
		auto timingMax = [&](int row) {
			int n = 0;
			const int last = row == 0 ? dim.x - 1 : dim.x - 5;
			for (int c = 8; c < last; ++c)
				n += !(version && Contains(version->alignmentPatternCenters(), c));
			return n;
		};
		auto timingScore = [&](const EdgeFit& e, int row) {
			if (!e.ok)
				return -1;
			int score = 0;
			const int last = row == 0 ? dim.x - 1 : dim.x - 5;
			for (int c = 8; c < last; ++c) {
				if (version && Contains(version->alignmentPatternCenters(), c))
					continue;
				PointF p = double(c) * e.A + e.B + PointF(0.5, 0.5);
				if (!image.isIn(PointI(p)))
					return -1;
				score += image.get(PointI(p)) == (c % 2 == 0);
			}
			return score;
		};
		// Jointly refine the two edges against the image content. The near ends rest on
		// reliable finder anchors; the far corner is where extrapolation errors land - a
		// module mis-count corrupts the fitted length, and when few trace anchors survive
		// (small, blurry modules) a tiny angular error displaces it by whole modules. Search
		// a window around the predicted far corner and score each candidate line by how well
		// BOTH rows then explain their known timing content, the bottom edge running one
		// symbol height (the finder's vertical) below the top. Scoring the rows together
		// pins the shared geometry with twice the evidence and rejects decoys such as a
		// neighbouring symbol's timing row, which cannot satisfy both rows at once. The far
		// corner is probed at the scale suggested by the trace fit and at the finder's own
		// module size; ties prefer the smallest correction.
		// Continuous, grayscale version of the timing-content score: how well separated
		// are the luminances of the modules a candidate line predicts to be light vs dark?
		// Unlike counting thresholded pixels this needs no threshold at all, degrades
		// gracefully under defocus (smaller but still-ranking separation instead of noise),
		// and has a genuine optimum at the true grid instead of a plateau.
		auto lumaSep = [&](const EdgeFit& e, int row) -> int {
			if (!e.ok || !lum)
				return -1;
			int sd = 0, nd = 0, sl = 0, nl = 0;
			const int last = row == 0 ? dim.x - 1 : dim.x - 5;
			for (int c = 8; c < last; ++c) {
				if (version && Contains(version->alignmentPatternCenters(), c))
					continue;
				PointF p = double(c) * e.A + e.B;
				const int v = lum->luma(narrow_cast<int>(p.x + 0.5), narrow_cast<int>(p.y + 0.5));
				if (v < 0)
					return -1;
				if (c % 2 == 0)
					sd += v, ++nd;
				else
					sl += v, ++nl;
			}
			if (!nd || !nl)
				return -1;
			const int sep = sl / nl - sd / nd;
			return lum->inverted() ? -sep : sep;
		};
		const bool useLuma = lum && lum->luma(0, 0) >= 0;
		auto rowScore = [&](const EdgeFit& e, int row) { return useLuma ? lumaSep(e, row) : timingScore(e, row); };
		const auto known = RMQRKnownModules(dim, version);
		// The refinement objective: luminance separation over ALL modules of known color -
		// finder, both timing rows including the always-dark alignment columns, corner and
		// sub pattern. The extra anchors are essential: alternation alone is invariant to
		// even-module shifts along the row, so an impostor alignment can outscore the true
		// one; the finder columns, alignment columns and sub pattern break that symmetry.
		auto knownSep = [&](const EdgeFit& t) -> int {
			int sd = 0, nd = 0, sl = 0, nl = 0;
			for (const auto& k : known) {
				const PointF p = double(k.x) * t.A + t.B + (double(k.y) / (dim.y - 1)) * vDown;
				const int v = lum ? lum->luma(narrow_cast<int>(p.x + 0.5), narrow_cast<int>(p.y + 0.5)) : -1;
				if (v < 0)
					return INT_MIN / 2;
				if (k.dark)
					sd += v, ++nd;
				else
					sl += v, ++nl;
			}
			if (!nd || !nl)
				return INT_MIN / 2;
			const int sep = sl / nl - sd / nd;
			return lum->inverted() ? -sep : sep;
		};
		auto jointScore = [&](const EdgeFit& t) {
			if (useLuma)
				return knownSep(t);
			EdgeFit b = t;
			b.B = b.B + vDown;
			return timingScore(t, 0) + timingScore(b, dim.y - 1);
		};
		// Edge hypotheses whose content score ties (or nearly ties) the winner: the score
		// plateaus once every probed timing module reads back correctly, so hypotheses on
		// the plateau are indistinguishable to it - but not to the error correction, which
		// sees every module. Kept for a decode-verified rescue below when the primary
		// configuration fails to decode.
		std::vector<std::pair<int, EdgeFit>> plateau;
		if ((ft.ok || fb.ok) && modSize > 1) {
			// the configuration used if no refinement is adopted: the fitted top edge
			// (or the fitted bottom shifted up when only that one exists)
			EdgeFit base = ft;
			if (!base.ok) { base = fb; base.B = base.B - vDown; }
			const int baseScore = jointScore(base);
			int bestScore = -1;
			double bestD = 0;
			EdgeFit best;
			plateau.clear();
			// seed the search from each fitted edge (expressed as a top line) - whichever
			// edge was measured well provides the geometry, the joint score arbitrates
			for (auto [seed, ok] : {std::pair{ft, ft.ok}, std::pair{[&] { EdgeFit b = fb; b.B = b.B - vDown; return b; }(), fb.ok}}) {
				if (!ok || length(seed.A) < 0.1)
					continue;
				const PointF dir = (1 / length(seed.A)) * seed.A;
				const PointF perp = PointF(-dir.y, dir.x);
				const double xf = dim.x - 1;
				if (int sc = jointScore(seed); sc > bestScore) { bestScore = sc; bestD = 0; best = seed; }
				for (PointF A0 : {seed.A, modSize * dir}) {
					const PointF far0 = xf * A0 + seed.B;
					const double aMax = tryHarder ? 5.0 : 2.5;
					for (double a = -aMax; a <= aMax; a += 0.25)
						for (double q = -2.5; q <= 2.5; q += 0.25) {
							EdgeFit c = seed;
							c.A = (1 / xf) * (far0 + (a * modSize) * dir + (q * modSize) * perp - seed.B);
							const int sc = jointScore(c);
							const double d = std::abs(a) + std::abs(q);
							if (sc > bestScore || (sc == bestScore && d < bestD)) {
								bestScore = sc;
								bestD = d;
								best = c;
							}
							if (sc >= baseScore - (useLuma ? 4 : 1))
								plateau.push_back({sc, c});
						}
				}
			}
			// A fitted line is anchored in measured positions; a scored line is only as
			// good as the score's resolution, which plateaus once every probed module reads
			// the expected color. Only adopt the refined line when it beats the fit by a
			// clear margin, i.e. when the fit is demonstrably wrong - never for plateau
			// noise that would nudge a good fit off its measurement.
			// Polish by shrinking-step coordinate descent over BOTH endpoints - the grid
			// above only moves the far end, but a mis-fit intercept costs modules across
			// the whole symbol. The objective is non-convex (neighbouring symbols, even-
			// module impostors), so descend from several independent starts: the grid
			// winner, the raw fit, and pure finder geometry.
			auto descend = [&](EdgeFit e) -> std::pair<int, EdgeFit> {
				if (!e.ok || length(e.A) < 0.1)
					return {INT_MIN / 2, e};
				const double xf = dim.x - 1;
				const PointF dir0 = (1 / length(e.A)) * e.A;
				const PointF perp0 = PointF(-dir0.y, dir0.x);
				PointF nearEnd = e.B, farEnd = xf * e.A + e.B;
				int sc0 = jointScore(e);
				for (double step : {1.0, 0.4, 0.15, 0.06}) {
					for (int iter = 0; iter < 8; ++iter) {
						bool improved = false;
						for (auto [dn, df] : {std::pair{step, 0.0}, {-step, 0.0}, {0.0, step}, {0.0, -step}, {step, step}, {-step, -step}}) {
							for (PointF d : {dir0, perp0}) {
								const PointF n2 = nearEnd + (dn * modSize) * d, f2 = farEnd + (df * modSize) * d;
								EdgeFit c;
								c.A = (1 / xf) * (f2 - n2);
								c.B = n2;
								c.ok = true;
								if (const int sc = jointScore(c); sc > sc0) {
									sc0 = sc;
									nearEnd = n2;
									farEnd = f2;
									improved = true;
								}
							}
						}
						if (!improved)
							break;
					}
				}
				EdgeFit r;
				r.A = (1 / xf) * (farEnd - nearEnd);
				r.B = nearEnd;
				r.ok = true;
				return {sc0, r};
			};
			EdgeFit finderSeed;
			finderSeed.A = length(stepTop) > 0.1 ? (modSize / length(stepTop)) * stepTop : PointF{};
			finderSeed.B = bestPT(centered(PointI(0, 0)));
			finderSeed.ok = length(stepTop) > 0.1;
			// Coarse 4D sweep over both endpoints around pure finder geometry - a bad fit
			// can be wrong in intercept AND scale at once, which neither the far-end grid
			// nor a greedy descent from it can recover. The sweep's best cell becomes one
			// more descent start.
			EdgeFit coarse;
			if (finderSeed.ok && tryHarder) {
				const double xf = dim.x - 1;
				const PointF dir0 = (1 / length(finderSeed.A)) * finderSeed.A;
				const PointF perp0 = PointF(-dir0.y, dir0.x);
				const PointF n0 = finderSeed.B, f0 = n0 + xf * finderSeed.A;
				int cBest = INT_MIN / 2;
				for (double np : {-2., -1., 0., 1., 2.})
					for (double nq : {-1., 0., 1.})
						for (double fpp = -5.; fpp <= 5.; fpp += 1.)
							for (double fq = -4.; fq <= 4.; fq += 1.) {
								const PointF n2 = n0 + (np * modSize) * dir0 + (nq * modSize) * perp0;
								const PointF f2 = f0 + (fpp * modSize) * dir0 + (fq * modSize) * perp0;
								EdgeFit c;
								c.A = (1 / xf) * (f2 - n2);
								c.B = n2;
								c.ok = true;
								const int sc = jointScore(c);
								if (sc > cBest) {
									cBest = sc;
									coarse = c;
								}
								// sweep cells are hypotheses too: expose them to the
								// decode-verified rescue below
								if (sc > baseScore - 4)
									plateau.push_back({sc, c});
							}
			}
			for (const EdgeFit& start : {best, base, finderSeed, coarse}) {
				auto [sc, line] = descend(start);
				if (sc > bestScore) {
					bestScore = sc;
					best = line;
				}
				if (sc > baseScore - 4)
					plateau.push_back({sc, line});
			}
			if (bestScore >= baseScore + (useLuma ? std::max(4, baseScore / 8) : 3)) {
				ft = best;
				ft.ok = true;
			} else {
				ft = base;
			}
			if (ft.ok) {
				// The bottom edge: either the independently fitted one (which can track real
				// perspective) or the top edge shifted rigidly one symbol height down (which
				// is what the joint score just validated). Keep the fitted one only if it
				// agrees with the refined top about the symbol height AND its row content
				// scores no worse - otherwise sampling would silently deviate from the
				// configuration the scoring approved.
				const double xf = dim.x - 1;
				EdgeFit rigid = ft;
				rigid.B = rigid.B + vDown;
				if (!fb.ok || length(((xf * fb.A + fb.B) - (xf * ft.A + ft.B)) - vDown) > modSize
					|| rowScore(fb, dim.y - 1) < rowScore(rigid, dim.y - 1))
					fb = rigid;
			}
		}
		if (ft.ok && fb.ok) {
			auto makeROIs = [&](const EdgeFit& t, const EdgeFit& b) {
				const std::vector<int> cols = {0, dim.x / 3, dim.x * 2 / 3, dim.x - 1};
				auto top = [&](int c) { return double(c) * t.A + t.B; };
				auto bot = [&](int c) { return double(c) * b.A + b.B; };
				ROIs rois;
				for (size_t i = 0; i + 1 < cols.size(); ++i) {
					const double x0 = cols[i] + 0.5, x1 = cols[i + 1] + 0.5, yT = 0.5, yB = dim.y - 0.5;
					rois.push_back({i == 0 ? 0 : cols[i], i + 2 == cols.size() ? dim.x : cols[i + 1], 0, dim.y,
									PerspectiveTransform({{x0, yT}, {x1, yT}, {x1, yB}, {x0, yB}},
														 {top(cols[i]), top(cols[i + 1]), bot(cols[i + 1]), bot(cols[i])})});
				}
				return rois;
			};
			ROIs rois = makeROIs(ft, fb);
			// Sample the grid from BOTH the globally binarized image and (when available)
			// the luminance image with per-symbol calibrated thresholds, and keep whichever
			// matrix agrees better with the symbol's known modules. Ties keep the binary
			// sampling. This makes the choice of binarization a scored measurement instead
			// of a global gamble on threshold-block alignment.
			auto knownScore = [&](const DetectorResult& g) {
				if (!g.isValid())
					return -1;
				int s = 0;
				for (const auto& k : known)
					s += g.bits().get(k.x, k.y) == k.dark;
				return s;
			};
			auto rB = SampleGrid(image, dim.x, dim.y, rois);
			auto rL = lum && lum->luma(0, 0) >= 0 ? SampleRMQRLuma(*lum, lum->inverted(), dim, rois, known)
												  : DetectorResult();
			// the error correction is the one scorer that sees every module: if exactly one
			// of the two samplings decodes, that one is right, whatever the proxy says
			const bool okB = rB.isValid() && Decode(rB.bits()).isValid();
			const bool okL = rL.isValid() && Decode(rL.bits()).isValid();
			auto r = okB		  ? std::move(rB)
					 : okL		  ? std::move(rL)
					 : knownScore(rL) > knownScore(rB) ? std::move(rL)
												  : std::move(rB);
			// Plateau rescue: the primary configuration did not decode, but hypotheses the
			// content score could not separate from it might. Trial-decode them, best score
			// first - only symbols that failed anyway pay for this.
			if (!okB && !okL && !plateau.empty()) {
				std::sort(plateau.begin(), plateau.end(), [](auto& a, auto& b) { return a.first > b.first; });
				const double xf = dim.x - 1;
				std::vector<PointF> tried{xf * ft.A + ft.B};
				for (auto& [sc, line] : plateau) {
					const PointF farEnd = xf * line.A + line.B;
					if (FindIf(tried, [&](PointF p) { return distance(p, farEnd) < 0.4 * modSize; }) != tried.end())
						continue;
					tried.push_back(farEnd);
					if (Size(tried) > (tryHarder ? 33 : 8))
						break;
					EdgeFit b2 = line;
					b2.B = b2.B + vDown;
					ROIs rois2 = makeROIs(line, b2);
					auto g = SampleGrid(image, dim.x, dim.y, rois2);
					if (g.isValid() && Decode(g.bits()).isValid())
						return {std::move(g), 1.0};
					if (lum && lum->luma(0, 0) >= 0) {
						auto gLum = SampleRMQRLuma(*lum, lum->inverted(), dim, rois2, known);
						if (gLum.isValid() && Decode(gLum.bits()).isValid())
							return {std::move(gLum), 1.0};
					}
				}
			}
			const double conf =
				double(timingScore(ft, 0) + timingScore(fb, dim.y - 1)) / std::max(1, timingMax(0) + timingMax(dim.y - 1));
			if (r.isValid())
				return {std::move(r), conf};
		}
	}

	return {SampleGrid(image, dim.x, dim.y, bestPT), 0.05};
	};

	// Try the candidates, best hamming first. A candidate with valid format information is
	// trusted unless a competing valid one explains the image better; a near-valid read
	// (hamming just past the BCH limit) is accepted only if the image confirms its
	// geometry emphatically. The decoder's checksum remains the final arbiter either way.
	DetectorResult bestGrid;
	double bestRank = 0;
	for (auto& [fi, pt] : cands) {
		auto [grid, conf] = attempt(fi, pt);
		if (!grid.isValid())
			continue;
		// the error correction sees every module - a candidate whose grid decodes is
		// confirmed outright, whatever the content-score proxies say
		if (Decode(grid.bits()).isValid())
			return std::move(grid);
		if (!fi.isValid() && conf < 0.75)
			continue;
		const double rank = (fi.isValid() ? 1.0 : 0.0) + conf;
		if (rank > bestRank) {
			bestRank = rank;
			bestGrid = std::move(grid);
		}
	}
	return bestGrid;
}


} // namespace ZXing::QRCode
