/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
* Copyright 2022 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "AZDetector.h"

#include "AZDetectorResult.h"
#include "BitArray.h"
#include "BitHacks.h"
#include "BitMatrix.h"
#include "ConcentricFinder.h"
#include "GenericGF.h"
#include "GridSampler.h"
#include "LogMatrix.h"
#include "Pattern.h"
#include "ReedSolomonDecoder.h"
#include "ZXAlgorithms.h"

#include <algorithm>
#include <cmath>
#include <optional>
#include <vector>

namespace ZXing::Aztec {

static bool IsAztectCenterPattern(const PatternView& view)
{
	// find min/max of all subsequent black/white pairs and check that they 'close together'
	auto m = view[0] + view[1];
	auto M = m;
	for (int i = 1; i < Size(view) - 1; ++i) {
		int v = view[i] + view[i + 1];
		if (v < m)
			m = v;
		else if (v > M)
			M = v;
		if (M > m * 4 / 3)
			return false;
	}
	return view[-1] >= view[Size(view) / 2] - 2 && view[Size(view)] >= view[Size(view) / 2] - 2;
};

// specialized version of FindLeftGuard to find the '1,1,1,1,1,1,1' pattern of a compact Aztec center pattern
static PatternView FindAztecCenterPattern(const PatternView& view)
{
	constexpr int minSize = 8; // Aztec runes
	auto window = view.subView(0, 7);
	for (auto end = view.end() - minSize; window.data() < end; window.skipPair())
		if (IsAztectCenterPattern(window))
			return window;

	return {};
};

static int CheckDirection(BitMatrixCursorF& cur, PointF dir, int range, bool updatePosition)
{
	range = range * 2 / 7; // TODO: tune
	auto pOri = cur.p;
	auto cuo = cur;
	cur.setDirection(dir);
	cuo.setDirection(-dir);

	int centerUp = cur.stepToEdge(1, range);
	if (!centerUp)
		return 0;
	int centerDown = cuo.stepToEdge(1, range);
	if (!centerDown)
		return 0;
	int center = centerUp + centerDown - 1; // -1 because the starting pixel is counted twice
	if (center > range || center < range / 6)
		return 0;

	if (updatePosition)
		pOri = (cur.p + cuo.p) / 2;

	int spread = center;
	int m = 0;
	int M = 0;
	for (auto c : {&cur, &cuo}) {
		int lastS = center;
		for (int i = 0; i < 3; ++i) {
			int s = c->stepToEdge(1, M);
			int v = s + lastS;
			if (m == 0)
				m = M = v;
			else if (v < m)
				m = v;
			else if (v > M)
				M = v;
			if (M > m * 4 / 3)
				return 0;
			spread += s;
			lastS = s;
		}
	}

	cur.p = pOri;

	return spread;
}

static std::optional<ConcentricPattern> LocateAztecCenter(const BitMatrix& image, PointF center, int spreadH)
{
	auto cur = BitMatrixCursorF(image, center, {});
	int minSpread = spreadH, maxSpread = spreadH;
	for (auto d : {PointF{0, 1}, {1, 0}, {1, 1}, {1, -1}}) {
		int spread = CheckDirection(cur, d, spreadH, d.x == 0);
		if (!spread)
			return {};
		minSpread = std::min(spread, minSpread);
		maxSpread = std::max(spread, maxSpread);
	}

	return ConcentricPattern{cur.p, (maxSpread + minSpread) / 2};
}

static std::vector<ConcentricPattern> FindPureFinderPattern(const BitMatrix& image)
{
	int left, top, width, height;
	if (!image.findBoundingBox(left, top, width, height, 11)) // 11 is the size of an Aztec Rune, see ISO/IEC 24778:2008(E) Annex A
		return {};

	PointF p(left + width / 2, top + height / 2);
	constexpr auto PATTERN = FixedPattern<7, 7>{1, 1, 1, 1, 1, 1, 1};
	if (auto pattern = LocateConcentricPattern(image, PATTERN, p, width / 3))
		return {*pattern};
	else
		return {};
}

static std::vector<ConcentricPattern> FindFinderPatterns(const BitMatrix& image, bool tryHarder)
{
	std::vector<ConcentricPattern> res;

	[[maybe_unused]] int N = 0;

#if 0 // reference algorithm for finding aztec center candidates
	constexpr auto PATTERN = FixedPattern<7, 7>{1, 1, 1, 1, 1, 1, 1};
	auto l0 = image.row(0);
	std::vector<uint8_t> line(l0.begin(), l0.end());
	const int width = image.width();

	int skip = tryHarder ? 1 : std::clamp(image.height() / 100, 1, 3);
	int margin = skip;
	for (int y = margin; y < image.height() - margin; y += skip) {
		auto lc = image.row(y).begin() + 1;
		auto lp = image.row(y - skip).begin() + 1;
		line.front() = image.get(0, y);
		// update line and swipe right
		for (int x = 1; x < image.width(); ++x) {
			line[x] += *lc++ != *lp++;
			while (line[x] > line[x - 1] + 1)
				line[x] -= 2;
		}
		// swipe left
		line.back() = image.get(width - 1, y);
		for (int x = width - 2; x > 0; --x)
			while (line[x] > line[x + 1] + 1)
				line[x] -= 2;

		int first = 0, last = 0;
		for (int x = 1; x < width - 5; ++x) {
			if (line[x] > line[x - 1] && line[x] >= 5 && line[x] % 2 == 1)
				first = x;
			else
				continue;
			while (line[x] == line[x + 1])
				++x;
			last = x;
			if (line[last + 1] < line[last]) {
				auto p = centered(PointI((first + last) / 2, y));
				// make sure p is not 'inside' an already found pattern area
				if (FindIf(res, [p](const auto& old) { return distance(p, old) < old.size / 2; }) == res.end()) {
					++N;
					auto pattern = LocateConcentricPattern(image, PATTERN, p, image.width() / 3);
					if (pattern){
						log(*pattern, 2);
						res.push_back(*pattern);
					}
				}
			}
		}
	}
#else // own algorithm based on PatternRow processing (between 0% and 100% faster than reference algo depending on input)
	int skip = tryHarder ? 1 : std::clamp(image.height() / 2 / 100, 1, 5);
	int margin = tryHarder ? 5 : image.height() / 4;

	for (int y = margin; y < image.height() - margin; y += skip)
	{
		PatternRow row;
		GetPatternRow(image, y, row, false);
		PatternView next = row;
		next.shift(1); // the center pattern we are looking for starts with white and is 7 wide (compact code)

#if 1
		while (next = FindAztecCenterPattern(next), next.isValid()) {
#else
		constexpr auto PATTERN = FixedPattern<7, 7>{1, 1, 1, 1, 1, 1, 1};
		while (next = FindLeftGuard(next, 0, PATTERN, 0.5), next.isValid()) {
#endif
			PointF p(next.pixelsInFront() + next[0] + next[1] + next[2] + next[3] / 2.0, y + 0.5);

			// make sure p is not 'inside' an already found pattern area
			bool found = false;
			for (auto old = res.rbegin(); old != res.rend(); ++old) {
				// search from back to front, stop once we are out of range due to the y-coordinate
				if (p.y - old->y > old->size / 2)
					break;
				if (distance(p, *old) < old->size / 2) {
					found = true;
					break;
				}
			}

			if (!found) {
				++N;
				log(p, 1);

				auto pattern = LocateAztecCenter(image, p, Reduce(next));
				if (pattern) {
					log(*pattern, 3);
					assert(image.get(*pattern));
					res.push_back(*pattern);
				}
			}

			next.skipPair();
			next.extend();
		}
	}
#endif

#ifdef PRINT_DEBUG
	printf("\n# checked centeres: %d, # found centers: %d\n", N, Size(res));
#endif
	return res;
}

static int FindRotation(uint32_t bits, bool mirror)
{
	const uint32_t mask = mirror ? 0b111'000'001'110 : 0b111'011'100'000;
	for (int i = 0; i < 4; ++i) {
		if (BitHacks::CountBitsSet(mask ^ bits) <= 2) // at most 2 bits may be wrong (24778:2008(E) 14.3.3 sais 3 but that is wrong)
			return i;
		bits = ((bits << 3) & 0xfff) | ((bits >> 9) & 0b111); // left shift/rotate, see RotatedCorners(Quadrilateral)
	}
	return -1;
}

// read 4*3=12 bits from the 4 corners of the finder pattern at radius
static uint32_t SampleOrientationBits(const BitMatrix& image, const PerspectiveTransform& mod2Pix, int radius)
{
	uint32_t bits = 0;
	for (auto d : {PointI{-1, -1}, {1, -1}, {1, 1}, {-1, 1}}) {
		auto corner = radius * d;
		auto cornerL = corner + PointI{0, -d.y};
		auto cornerR = corner + PointI{-d.x, 0};
		if (d.x != d.y)
			std::swap(cornerL, cornerR);
		for (auto ps : {cornerL, corner, cornerR}) {
			auto p = mod2Pix(PointF(ps));
			if (!image.isIn(p))
				return 0;
			log(p);
			AppendBit(bits, image.get(p));
		}
	}
	return bits;
}

static int ModeMessage(const BitMatrix& image, const PerspectiveTransform& mod2Pix, int radius)
{
	const bool compact = radius == 5;

	// read the bits between the corner bits along the 4 edges
	uint64_t bits = 0;
	for (auto d : {PointI{-1, -1}, {1, -1}, {1, 1}, {-1, 1}}) {
		auto corner = radius * d;
		auto next = (d.x == d.y) ? PointI{-d.x, 0} : PointI{0, -d.y};
		for (int i = 2; i <= 2 * radius - 2; ++i) {
			if (!compact && i == 7)
				continue; // skip the timing pattern
			auto p = mod2Pix(PointF(corner + i * next));
			log(p);
			if (!image.isIn(p))
				return -1;
			AppendBit(bits, image.get(p));
		}
	}

	// error correct bits
	int numCodewords = compact ? 7 : 10;
	int numDataCodewords = compact ? 2 : 4;
	int numECCodewords = numCodewords - numDataCodewords;

	std::vector<int> words(numCodewords);
	for (int i = numCodewords - 1; i >= 0; --i) {
		words[i] = narrow_cast<int>(bits & 0xF);
		bits >>= 4;
	}
	if (!ReedSolomonDecode(GenericGF::AztecParam(), words, numECCodewords))
		return -1;

	int res = 0;
	for (int i = 0; i < numDataCodewords; i++)
		res = (res << 4) + words[i];

	return res;
}

static void ExtractParameters(int modeMessage, bool compact, int& nbLayers, int& nbDataBlocks, bool& readerInit)
{
	readerInit = false;
	if (compact) {
		// 8 bits:  2 bits layers and 6 bits data blocks
		nbLayers = (modeMessage >> 6) + 1;
		if (nbLayers == 1 && (modeMessage & 0x20)) { // ISO/IEC 24778:2008 Section 9 MSB artificially set
			readerInit = true;
			modeMessage &= ~0x20;
		}
		nbDataBlocks = (modeMessage & 0x3F) + 1;
	} else {
		// 16 bits:  5 bits layers and 11 bits data blocks
		nbLayers = (modeMessage >> 11) + 1;
		if (nbLayers <= 22 && (modeMessage & 0x400)) { // ISO/IEC 24778:2008 Section 9 MSB artificially set
			readerInit = true;
			modeMessage &= ~0x400;
		}
		nbDataBlocks = (modeMessage & 0x7FF) + 1;
	}
}

DetectorResult Detect(const BitMatrix& image, bool isPure, bool tryHarder)
{
	return FirstOrDefault(Detect(image, isPure, tryHarder, 1));
}

DetectorResults Detect(const BitMatrix& image, bool isPure, bool tryHarder, int maxSymbols)
{
#ifdef PRINT_DEBUG
	LogMatrixWriter lmw(log, image, 5, "az-log.pnm");
#endif

	DetectorResults res;
	auto fps = isPure ? FindPureFinderPattern(image) : FindFinderPatterns(image, tryHarder);
	for (auto fp : fps) {
		auto fpQuad = FindConcentricPatternCorners(image, fp, fp.size, 3);
		if (!fpQuad)
			continue;

		auto srcQuad = CenteredSquare(7);
		auto mod2Pix = PerspectiveTransform(srcQuad, *fpQuad);
		if (!mod2Pix.isValid())
			continue;

		int radius; // 5 or 7 (compact vs. full)
		int mirror; // 0 or 1
		int rotate; // [0..3]
		int modeMessage = -1;
		[&]() {
			// 24778:2008(E) 14.3.3 reads:
			// In the outer layer of the Core Symbol, the 12 orientation bits at the corners are bitwise compared against the specified
			// pattern in each of four possible orientations and their four mirror inverse orientations as well. If in any of the 8
			// cases checked as many as 9 of the 12 bits correctly match, that is deemed to be the correct orientation, otherwise
			// decoding fails.
			// Unfortunately, this seems to be wrong: there are 12-bit patterns in those 8 cases that differ only in 4 bits like
			// 011'100'000'111 (rot90 && !mirror) and 111'000'001'110 (rot0 && mirror), meaning if two of those are wrong, both cases
			// have a hamming distance of 2, meaning only 1 bit errors can be relyable recovered from. The following code therefore
			// incorporates the complete set of mode message bits to help determine the orientation of the symbol. This is still not
			// sufficient for the ErrorInModeMessageZero test case in AZDecoderTest.cpp but good enough for the author.
			for (radius = 5; radius <= 7; radius += 2) {
				uint32_t bits = SampleOrientationBits(image, mod2Pix, radius);
				if (bits == 0)
					continue;
				for (mirror = 0; mirror <= 1; ++mirror) {
					rotate = FindRotation(bits, mirror);
					if (rotate == -1)
						continue;
					modeMessage = ModeMessage(image, PerspectiveTransform(srcQuad, RotatedCorners(*fpQuad, rotate, mirror)), radius);
					if (modeMessage != -1)
						return;
				}
			}
		}();

		if (modeMessage == -1)
			continue;

#if 1
		// improve prescision of sample grid by extrapolating from outer square of white pixels (5 edges away from center)
		if (radius == 7) {
			if (auto fpQuad5 = FindConcentricPatternCorners(image, fp, fp.size * 5 / 3, 5)) {
				if (auto mod2Pix = PerspectiveTransform(CenteredSquare(11), *fpQuad5); mod2Pix.isValid()) {
					int rotate5 = FindRotation(SampleOrientationBits(image, mod2Pix, radius), mirror);
					if (rotate5 != -1) {
						srcQuad = CenteredSquare(11);
						fpQuad = fpQuad5;
						rotate = rotate5;
					}
				}
			}
		}
#endif
		*fpQuad = RotatedCorners(*fpQuad, rotate, mirror);

		int nbLayers = 0;
		int nbDataBlocks = 0;
		bool readerInit = false;
		ExtractParameters(modeMessage, radius == 5, nbLayers, nbDataBlocks, readerInit);

		int dim = radius == 5 ? 4 * nbLayers + 11 : 4 * nbLayers + 2 * ((2 * nbLayers + 6) / 15) + 15;
		double low = dim / 2.0 + srcQuad[0].x;
		double high = dim / 2.0 + srcQuad[2].x;

		auto bits = SampleGrid(image, dim, dim, PerspectiveTransform{{PointF{low, low}, {high, low}, {high, high}, {low, high}}, *fpQuad});
		if (!bits.isValid())
			continue;

		res.emplace_back(std::move(bits), radius == 5, nbDataBlocks, nbLayers, readerInit, mirror != 0);

		if (Size(res) == maxSymbols)
			break;
	}

	return res;
}

} // namespace ZXing::Aztec
