/*
* Copyright 2026 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "MicroPDFReader.h"

#include "BinaryBitmap.h"
#include "BitArray.h"
#include "BitMatrixCursor.h"
#include "ReaderOptions.h"
#include "DecoderResult.h"
#include "LogMatrix.h"
#include "PDF417.h"
#include "PDFCodewordDecoder.h"
#include "PDFScanningDecoder.h"
#include "Pattern.h"
#include "PerspectiveTransform.h"
#include "RegressionLine.h"

#include <list>
#include <limits>
#include <vector>

#define LRAP_WITH_CW 1
#define USE_E2E_PATTERNS

namespace ZXing::MicroPdf417 {

using namespace PDF417;

static constexpr FixedPattern<6, 10> LRRAPs[] = {
	{2, 2, 1, 3, 1, 1}, /* 1*/
	{3, 1, 1, 3, 1, 1}, /* 2*/
	{3, 1, 2, 2, 1, 1}, /* 3*/
	{2, 2, 2, 2, 1, 1}, /* 4*/
	{2, 1, 3, 2, 1, 1}, /* 5*/
	{2, 1, 4, 1, 1, 1}, /* 6*/
	{2, 2, 3, 1, 1, 1}, /* 7*/
	{3, 1, 3, 1, 1, 1}, /* 8*/
	{3, 2, 2, 1, 1, 1}, /* 9*/
	{4, 1, 2, 1, 1, 1}, /*10*/
	{4, 2, 1, 1, 1, 1}, /*11*/
	{3, 3, 1, 1, 1, 1}, /*12*/
	{2, 4, 1, 1, 1, 1}, /*13*/
	{2, 3, 2, 1, 1, 1}, /*14*/
	{2, 3, 1, 2, 1, 1}, /*15*/
	{3, 2, 1, 2, 1, 1}, /*16*/
	{4, 1, 1, 2, 1, 1}, /*17*/
	{4, 1, 1, 1, 2, 1}, /*18*/
	{4, 1, 1, 1, 1, 2}, /*19*/
	{3, 2, 1, 1, 1, 2}, /*20*/
	{3, 1, 2, 1, 1, 2}, /*21*/
	{3, 1, 1, 2, 1, 2}, /*22*/
	{3, 1, 1, 2, 2, 1}, /*23*/
	{3, 1, 1, 1, 3, 1}, /*24*/
	{3, 1, 1, 1, 2, 2}, /*25*/
	{3, 1, 1, 1, 1, 3}, /*26*/
	{2, 2, 1, 1, 1, 3}, /*27*/
	{2, 2, 1, 1, 2, 2}, /*28*/
	{2, 2, 1, 1, 3, 1}, /*29*/
	{2, 2, 1, 2, 2, 1}, /*30*/
	{2, 2, 2, 1, 2, 1}, /*31*/
	{3, 1, 2, 1, 2, 1}, /*32*/
	{3, 2, 1, 1, 2, 1}, /*33*/
	{2, 3, 1, 1, 2, 1}, /*34*/
	{2, 3, 1, 1, 1, 2}, /*35*/
	{2, 2, 2, 1, 1, 2}, /*36*/
	{2, 1, 3, 1, 1, 2}, /*37*/
	{2, 1, 2, 2, 1, 2}, /*38*/
	{2, 1, 2, 2, 2, 1}, /*39*/
	{2, 1, 2, 1, 3, 1}, /*40*/
	{2, 1, 2, 1, 2, 2}, /*41*/
	{2, 1, 2, 1, 1, 3}, /*42*/
	{2, 1, 1, 2, 1, 3}, /*43*/
	{2, 1, 1, 1, 2, 3}, /*44*/
	{2, 1, 1, 1, 3, 2}, /*45*/
	{2, 1, 1, 1, 4, 1}, /*46*/
	{2, 1, 1, 2, 3, 1}, /*47*/
	{2, 1, 1, 2, 2, 2}, /*48*/
	{2, 1, 1, 3, 1, 2}, /*49*/
	{2, 1, 1, 3, 2, 1}, /*50*/
	{2, 1, 1, 4, 1, 1}, /*51*/
	{2, 1, 2, 3, 1, 1}, /*52*/
};

static constexpr FixedPattern<6, 10> CRAPs[] = {
	{1, 1, 2, 2, 3, 1}, /*01*/
	{1, 2, 1, 2, 3, 1}, /*02*/
	{1, 2, 2, 1, 3, 1}, /*03*/
	{1, 3, 1, 1, 3, 1}, /*04*/
	{1, 3, 1, 2, 2, 1}, /*05*/
	{1, 3, 2, 1, 2, 1}, /*06*/
	{1, 4, 1, 1, 2, 1}, /*07*/
	{1, 4, 1, 2, 1, 1}, /*08*/
	{1, 4, 2, 1, 1, 1}, /*09*/
	{1, 3, 3, 1, 1, 1}, /*10*/
	{1, 3, 2, 2, 1, 1}, /*11*/
	{1, 3, 1, 3, 1, 1}, /*12*/
	{1, 2, 2, 3, 1, 1}, /*13*/
	{1, 2, 3, 2, 1, 1}, /*14*/
	{1, 2, 4, 1, 1, 1}, /*15*/
	{1, 1, 5, 1, 1, 1}, /*16*/
	{1, 1, 4, 2, 1, 1}, /*17*/
	{1, 1, 4, 1, 2, 1}, /*18*/
	{1, 2, 3, 1, 2, 1}, /*19*/
	{1, 2, 3, 1, 1, 2}, /*20*/
	{1, 2, 2, 2, 1, 2}, /*21*/
	{1, 2, 2, 2, 2, 1}, /*22*/
	{1, 2, 1, 3, 2, 1}, /*23*/
	{1, 2, 1, 4, 1, 1}, /*24*/
	{1, 1, 2, 4, 1, 1}, /*25*/
	{1, 1, 3, 3, 1, 1}, /*26*/
	{1, 1, 3, 2, 2, 1}, /*27*/
	{1, 1, 3, 2, 1, 2}, /*28*/
	{1, 1, 3, 1, 2, 2}, /*29*/
	{1, 2, 2, 1, 2, 2}, /*30*/
	{1, 3, 1, 1, 2, 2}, /*31*/
	{1, 3, 1, 1, 1, 3}, /*32*/
	{1, 2, 2, 1, 1, 3}, /*33*/
	{1, 1, 3, 1, 1, 3}, /*34*/
	{1, 1, 2, 2, 1, 3}, /*35*/
	{1, 1, 2, 2, 2, 2}, /*36*/
	{1, 1, 2, 3, 1, 2}, /*37*/
	{1, 1, 2, 3, 2, 1}, /*38*/
	{1, 1, 1, 4, 2, 1}, /*39*/
	{1, 1, 1, 3, 3, 1}, /*40*/
	{1, 1, 1, 3, 2, 2}, /*41*/
	{1, 1, 1, 2, 3, 2}, /*42*/
	{1, 1, 1, 2, 2, 3}, /*43*/
	{1, 1, 1, 1, 3, 3}, /*44*/
	{1, 1, 1, 1, 2, 4}, /*45*/
	{1, 1, 1, 2, 1, 4}, /*46*/
	{1, 1, 2, 1, 1, 4}, /*47*/
	{1, 2, 1, 1, 1, 4}, /*48*/
	{1, 2, 1, 1, 2, 3}, /*49*/
	{1, 2, 1, 1, 3, 2}, /*50*/
	{1, 1, 2, 1, 3, 2}, /*51*/
	{1, 1, 2, 1, 4, 1}, /*52*/
};

static constexpr auto ToInts(const FixedPattern<6, 10> in[52])
{
	std::array<int, 52> res{};
	for (int i = 0; i < 52; ++i)
#ifdef USE_E2E_PATTERNS
		res[i] = ToInt(NormalizedE2EPattern<6, 10, 5>(in[i]));
#else
		res[i] = ToInt(in[i]);
#endif
	return res;
}

static constexpr auto LRRAPInts = ToInts(LRRAPs);
static constexpr auto CRAPInts = ToInts(CRAPs);

enum class RAP {L, C, R};

static int RAPIndex(int v, RAP t)
{
	if (v == 0)
		return 0;
	return IndexOf(t == RAP::C ? CRAPInts : LRRAPInts, v) + 1;
}

static int RAPCluster(int idx)
{
	return ((idx - 1) % 3) * 3;
}

using PatternRAP = std::array<uint16_t, 6>;

static int ReadRAP(BitMatrixModuleCursorF& cur, RAP type)
{
	log(cur.p, 2);
	auto pattern = cur.readPatternFromBlack<PatternRAP>(cur.ms * 1, cur.ms * (10 + MS_THR), cur.ms * (10 - MS_THR));
#ifdef USE_E2E_PATTERNS
	int res = RAPIndex(ToInt(NormalizedE2EPattern<6, 10, 5>(pattern)), type);
#else
	int res = RAPIndex(ToInt(NormalizedPattern<6, 10>(pattern)), type);
#endif
	if (res) {
		cur.ms = Reduce(pattern) / 10.;
		if (type == RAP::R) {
			auto c = cur;
			auto ms_thr = cur.ms * 1.5 + 1;
			if (cur.stepToEdge(1, ms_thr) == 0 || ((c = cur).stepToEdge(1, ms_thr) != 0 && c.isIn()))
				res = 0;
		}
	}
	log(cur.p, res ? -1 : 1);
	return res;
}

static int IsLRAP(const PatternView& view)
{
	int l = view.sum(6);
#if LRAP_WITH_CW
	int r = view.subView(6).sum(8);

	// nominal l:r ratio is 10:17, accept ratios between 10:13 and 10:22 (approx. 45° tolerance)
	if (l < 10 || r < 17 || l * 20 < r * 10 || l * 14 > r * 10 || (!view.isAtFirstBar() && view[-1] < l / 10))
#else
	if (l < 10 || (!view.isAtFirstBar() && view[-1] < l / 10))
#endif
		return 0;

#if 1
	auto m = view[0];
	auto M = m;
	for (int i = 1; i < 6; ++i)
		UpdateMinMax(m, M, view[i]);

	// the first bar is nominally >= 2 * m, M is nominally <= 4 * m
	if (view[0] < m * 3 / 2 || M > m * 6 || view[5] > 5 * m)
		return 0;
#endif
#ifdef USE_E2E_PATTERNS
	auto v = RAPIndex(ToInt(NormalizedE2EPattern<6, 10, 5>(view)), RAP::L);
#else
	auto v = RAPIndex(ToInt(NormalizedPattern<6, 10>(view)), RAP::L);
#endif

	return v;
};

static std::tuple<PatternView, int> FindLRAP(const PatternView& view)
{
	constexpr int minSize = 6 + 8 * LRAP_WITH_CW; // 1 column
	auto window = view.subView(0, 6 + 8 * LRAP_WITH_CW);
#if 1
	for (auto end = view.end() - minSize; window.data() < end; window.skipPair())
#else
	if (window.data() < view.end() - minSize)
#endif
		if (int i = IsLRAP(window))
			return {window, i};

	return {};
};

struct LRAP : PointI
{
	int idx, width;
};

struct Segment
{
	int idx;
	std::vector<LRAP*> lraps;
};

struct RAPPair
{
	int first, second, offset, family;

	RAPPair(int f, int s): first(f), second(s), offset(0), family(0) {
		int diff = second - first;
		if (diff < -4)
			diff += 52;
		family = (diff + 4) / 8 * 8;
		offset = diff - family;
	}

	bool isValid() const { return first != 0 && second != 0 && family <= 24 && std::abs(offset) <= 3; }
};

using Cluster = std::vector<LRAP>;
using Clusters = std::list<Cluster>;

static Clusters FindCandidates(const BitMatrix& image, bool tryHarder, bool reversed)
{
	const int height = image.height();
	const int width = image.width();

	if (height < 4 || width < 27)
		return {};

	int minClusterSize = 4;                                             // shortest MicroPDF417 has 4 rows
	int margin = tryHarder ? std::min(8, height / 44) : height / 4;     // tallest MicroPDF417 has 44 rows
	int skip = tryHarder ? 8 : std::max((height - 2 * margin) / 32, 8); // shortest MicroPDF417 symbol is 8 pixels high

	PatternRow row;
	Clusters res;

	for (int y = margin; y < height - margin; y += skip) {
		GetPatternRow(image, reversed ? height - 1 - y : y, row, false);
		if (reversed)
			std::ranges::reverse(row);

		PatternView next = row;
		int idx;
#if 1
		while
#else
		if
#endif
		(std::tie(next, idx) = FindLRAP(next), next.isValid()) {
			LRAP p{{next.pixelsInFront(), y}, idx, next.sum()};
			log(centered(PointF(reversed ? width - 1 - p.x : p.x, reversed ? height - 1 - p.y : p.y)), 1);

			// Try to attach this LRAP to an existing cluster by proximity in x/y; prune stale short
			// clusters and create a new cluster if no suitable match was found.
			for (auto pCluster = res.begin(); pCluster != res.end();) {
				auto diff = p - pCluster->back();
				if (diff.y <= 3 * skip && std::abs(diff.x) <= std::max(diff.y, 2)) {
					pCluster->push_back(p);
					p = {};
					break;
				} else if (diff.y > 2 * skip && Size(*pCluster) < minClusterSize) {
					res.erase(pCluster++);
				} else {
					++pCluster;
				}
			}

			if (p != PointI{}) {
#ifdef ZXING_SUPPORT_MINIMAL_SIZE_MIRCOPDF417
				if (tryHarder) {
					int rowHeight = 2 * p.width / 27; // p is 27 modules wide, a row is nominally 2 modules high
					if (rowHeight < skip) {
						// TODO: this skip adjustment + 'restart' to detect very small symbols may break detection of multiple symbls
						// with different sizes horizontally next to each other
						y = std::max(0, y - skip); // the for loop will add `skip` back, so this effectively y - skip + rowHeight
						skip = rowHeight;
						printf("decreasing skip to %d at x=%3d, y=%3d, p.width=%d\n", skip, p.x, y, p.width);
						break;
					} else if (rowHeight > 2 * skip
							   && std::ranges::all_of(res, [&](const Cluster& c) { return p.y - c.back().y > 2 * skip; })) {
						skip = std::min(8, rowHeight); // reset skip to default when starting a new cluster
						printf("increasing skip to %d at x=%3d, y=%3d, p.width=%d\n", skip, p.x, y, p.width);
					}
				}
#endif
				res.emplace_back(Cluster{p});
			}

			next.skipPair();
			next.extend();
		}
	}

	res.remove_if([&](Cluster& lraps) { return Size(lraps) < minClusterSize; });
	res.remove_if([&](Cluster& lraps) {
		// `segs` groups consecutive LRAP points by idx so we can denoise, enforce monotonic order,
		// and keep one representative center point per streak for robust cluster cleanup.
		std::list<Segment> segs = {{lraps.front().idx, {}}};
		for (LRAP& p : lraps) {
			if (p.idx != segs.back().idx)
				segs.push_back({p.idx, {}});
			segs.back().lraps.push_back(&p);
		}
		// remove noise from the segment, i.e. streaks that are less than half as long as the longest streak
		int M = Size(std::ranges::max(segs, {}, [](const auto& seg) { return Size(seg.lraps); }).lraps);
		segs.remove_if([M](Segment& h) { return Size(h.lraps) < M / 2; });

		// remove duplicates (can appear after small ones are dropped)
		segs.unique([](auto& l, auto& r) { return l.idx == r.idx; });

		// remove non-monotonic and too far away elements from the front and the back
		int diff = 0;
		while (segs.size() > 1 && ((diff = std::next(segs.begin())->idx - segs.begin()->idx) < 0 || diff > 3))
			segs.pop_front();
		while (segs.size() > 1 && ((diff = std::next(segs.rbegin())->idx - segs.rbegin()->idx) > 0 || diff < -3))
			segs.pop_back();

#if LRAP_WITH_CW == 0 // support rotated symbols > 30°
		if (Size(segs) < 3)
			return true;
		// remove single elements that are not part of a monotonic sequence
		for (auto it = std::next(segs.begin()); it != std::prev(segs.end());) {
			if (it->idx > std::next(it)->idx && std::next(it)->idx > std::prev(it)->idx)
				it = segs.erase(it);
			else
				++it;
		}
#endif

		// remove complete segment if too small, too spread out or contains non-monotonic sequence
		if (Size(segs) < 3 || std::abs(segs.back().idx - segs.front().idx) > 2 * Size(segs)
			|| (LRAP_WITH_CW && !std::ranges::is_sorted(segs, {}, &Segment::idx)))
			return true;

		// replace current segment with one containing only the center points of each lrap streak
		auto center = [](auto& h) { return *h.lraps[Size(h.lraps) / 2]; };
		lraps.erase(std::ranges::transform(segs, lraps.begin(), center).out, lraps.end());

		return false;
	});

	if (reversed)
		for (auto& cluster : res)
			for (auto& lrap : cluster) {
				lrap.x = width - 1 - lrap.x;
				lrap.y = height - 1 - lrap.y;
			}

#ifdef PRINT_DEBUG
	printf("\n# found LRAPs: %d\n", Size(res));
	for (const auto& cluster : res) {
		for (auto lrap : cluster)
			printf("%d @ %dx%d (width: %d)\n", lrap.idx, lrap.x, lrap.y, lrap.width);
		printf("\n");
	}
#endif
	return res;
}

static int DetermineNumCols(BitMatrixModuleCursorF& start, const Cluster& lraps)
{
	printf("right: %s, ms: %.1f\n", ToString(start.d).c_str(), start.ms);
	std::array<int, 16> colHist = {}, offsets = {};
	for (int s = 0; s < 2 && std::ranges::max(colHist) < 3; ++s)
		for (auto& p : lraps) {
			auto cur = start;
			auto tmp = cur;
			cur.p = centered(p) + s * cur.ms * right(cur.d); // if we don't have enough LRAPs, scan again with 1 modSize offset

			auto pair = RAPPair(ReadRAP(cur, RAP::L), 0);
			if (!pair.first || !SkipCodeword(cur))
				continue;

			printf("\nLRAP: %2d @ %5.1fx%5.1f ", pair.first, cur.p.x, cur.p.y);

			auto checkRAP = [&](RAP rap, int colI) {
				--colI;
				pair = RAPPair(pair.first, ReadRAP((tmp = cur), rap));
				if (pair.isValid()) {
					colHist[colI * 4 + pair.family / 8] += 1;
					offsets[colI * 4 + pair.family / 8] += pair.offset;
				}
				printf("%2d/%2d: %2d %s ", pair.first, pair.second, pair.offset, pair.isValid() ? "<  " : "   ");
				return pair.isValid();
			};

			checkRAP(RAP::R, 1);
			if (checkRAP(RAP::C, 3)) {
				pair.first = pair.second;
				cur = tmp;
				if (SkipCodeword(cur) && SkipCodeword(cur) && checkRAP(RAP::R, 3))
					continue;
			}
			if (SkipCodeword(cur)) {
				checkRAP(RAP::R, 2);
				if (checkRAP(RAP::C, 4)) {
					pair.first = pair.second;
					cur = tmp;
					if (SkipCodeword(cur) && SkipCodeword(cur) && checkRAP(RAP::R, 4))
						continue;
				}
			}
		}

	printv("\ncolHist: ", "%2d ", "", colHist);
	printv("\noffsets: ", "%2d ", "\n", offsets);
	int nCol = std::ranges::max_element(colHist) - colHist.begin();

	if (colHist[nCol]) {
		auto offset = double(offsets[nCol]) / colHist[nCol];
		start.d = bresenhamDirection((10. + 17. * 2) * start.d - 2. * offset * start.right());
		printf("average offset: %.1f, new right: %s, ms: %.1f\n", offset, ToString(start.d).c_str(), start.ms);
	}

	return nCol / 4 + 1;
}

struct SymbolInfo
{
	int nCols = 0, nRows = 0, nECCs = 0, rotFam = 0, startRow = 0, rowB = 0, rowE = 0;
	int nCWs() const { return nCols * nRows; }
	int lastRow() const { return startRow + nRows - 1; }
	int width() const { return 21 + nCols * 17 + (nCols > 2) * 10; }
	int height() const { return nRows * 2; }
};

static constexpr std::array<SymbolInfo, 35> SYMBOLS = {{
	// the first element is a dummy
	{0, 0, 0, 0, 0, 0},
	// see tables 1, 10, 11, 12 in ISO 24728::2006
	{1, 11, 7, 8, 1, 1, 8},
	{1, 14, 7, 0, 8, 8, 18},
	{1, 17, 7, 0, 36, 39, 52},
	{1, 20, 8, 0, 19, 22, 35},
	{1, 24, 8, 8, 9, 12, 24},
	{1, 28, 8, 8, 25, 33, 52},
	{2, 8, 8, 0, 1, 1, 7},
	{2, 11, 9, 8, 1, 1, 8},
	{2, 14, 9, 0, 8, 9, 18},
	{2, 17, 10, 0, 36, 39, 52},
	{2, 20, 11, 0, 19, 22, 35},
	{2, 23, 13, 8, 9, 12, 26},
	{2, 26, 15, 8, 27, 32, 52},
	{3, 6, 12, 0, 1, 1, 6},
	{3, 8, 14, 0, 7, 7, 14},
	{3, 10, 16, 0, 15, 15, 24},
	{3, 12, 18, 0, 25, 25, 36},
	{3, 15, 21, 0, 37, 37, 51},
	{3, 20, 26, 16, 1, 1, 14},
	{3, 26, 32, 8, 1, 1, 20},
	{3, 32, 38, 8, 21, 27, 52},
	{3, 38, 44, 16, 15, 21, 52},
	{3, 44, 50, 24, 1, 1, 44},
	{4, 4, 8, 24, 47, 47, 50},
	{4, 6, 12, 0, 1, 1, 6},
	{4, 8, 14, 0, 7, 7, 14},
	{4, 10, 16, 0, 15, 15, 24},
	{4, 12, 18, 0, 25, 25, 36},
	{4, 15, 21, 0, 37, 37, 51},
	{4, 20, 26, 16, 1, 1, 14},
	{4, 26, 32, 8, 1, 1, 20},
	{4, 32, 38, 8, 21, 27, 52},
	{4, 38, 44, 16, 15, 21, 52},
	{4, 44, 50, 24, 1, 1, 44},
}};

static const SymbolInfo& DetermineSymbolInfo(const Matrix<Codeword>& cwMat, const std::array<int, 4>& rotFamHist [[maybe_unused]])
{
	auto rotFamMax = std::ranges::max_element(rotFamHist);
	int rotFam = *rotFamMax && *rotFamMax > Reduce(rotFamHist) / 2 ? static_cast<int>(rotFamMax - rotFamHist.begin()) * 8 : -1;
	// rotFam = -1; // uncomment to test symbol detection without rotation family filtering

	std::vector<int> sightsPerRow(cwMat.height(), 0);
	for (int y = 1; y < cwMat.height(); ++y)
		for (int x = 0; x < cwMat.width(); ++x)
			sightsPerRow[y] += cwMat(x, y).count;

	const SymbolInfo* bestSym = SYMBOLS.data();
	int minError = std::numeric_limits<int>::max();
	float meanCount = Reduce(cwMat, 0.f, [](float acc, const Codeword& e) { return acc + e.count; })
					  / std::ranges::count_if(cwMat, [](const auto& e) { return e.count > 0; });

	for (const auto& s : SYMBOLS) {
		if (s.nCols != cwMat.width() || (rotFam != -1 && s.rotFam != rotFam))
			continue;

		int error = s.nCWs() * meanCount; // with all inside filled and outside empty, this will result in 0 error
		for (int y = 1; y < cwMat.height(); ++y) {
			bool isOutside = y < s.startRow || y > s.lastRow();
			error += (isOutside ? 1 : -1) * sightsPerRow[y];
		}

		printf("symbol: %d, %2dx%2d, rotFam: %2d, firstRow: %2d, error: %d\n",
			   static_cast<int>(&s - SYMBOLS.data()), s.nCols, s.nRows, s.rotFam, s.startRow, error);

		if (error < minError) {
			minError = error;
			bestSym = &s;
		}
	}

	return *bestSym;
}

static BarcodeData ScanCandidate(const BitMatrix& image, const Cluster& lraps)
{
	auto inward = (lraps.back().y > lraps.front().y ? 1 : -1) * PointF(1, 0);
	RegressionLine lineL, lineR;
	lineL.setDirectionInward(inward);
	lineR.setDirectionInward(inward);
	for (auto& p : lraps) {
		lineL.add(PointF(p));
		lineR.add(PointF(p) + p.width * inward);
	}
	lineL.evaluate(2, true);
	lineR.evaluate(2, true);
	auto down = bresenhamDirection(right(lineL.normal()));
	printf("down: %s\n", ToString(down).c_str());
	BitMatrixModuleCursorF startCur(image, centered(lraps.front()), bresenhamDirection(lineR.normal()),
									lraps.front().width / (10. + 17. * LRAP_WITH_CW));
	startCur.step(-1);

	int nCols = DetermineNumCols(startCur, lraps);
	printf("nCols: %d\n", nCols);
	if (!nCols)
		return {};

	int failedTries = 0;
	while(failedTries < 10 && image.isIn(startCur.p - down)) {
		startCur.p += -down;
		auto cur = startCur;
		log(cur.p);
		if (!ReadRAP(cur, RAP::L))
			++failedTries;
	}

	startCur.p += failedTries * down;

	// Matrix with vector of codewords for each cell, to collect all codeword sightings and their counts.
	Matrix<std::vector<Codeword>> histMat(nCols, 52 + 1);
	// Histogram of rotation family votes (4 possible families) collected while scanning RAP pairs.
	std::array<int, 4> rotFamHist = {};
	Position pos;

	auto checkRAP = [&](int li, int ri) {
		auto rap = RAPPair(li, ri);
		if (!rap.isValid())
			return false;
		// printf("li: %2d, ri: %2d, ri-li: %2d, fam: %d, offset: %d\n", li, ri, rap.second - rap.first, rap.family, rap.offset);
		rotFamHist.at(rap.family/8)++;
		failedTries = 0;
		return true;
	};

	auto inSweepRange = [&](PointF p) { return dot(PointF(lraps.back()) - p, down) >= 0; };

	failedTries = 1;
	for (; image.isIn(startCur.p + down) && (inSweepRange(startCur.p) || failedTries < 10 * startCur.ms); startCur.p += down, failedTries++) {
		auto cur = startCur;
		log(cur.p);

		auto li = ReadRAP(cur, RAP::L);
		// printf("li: %2d @ (%f, %f)\n", li, cur.p.x, cur.p.y);
		if (!li)
			continue;
		startCur.ms = cur.ms;
		Codeword cw[4];
		int ci = 0, ri = 0, cluster = RAPCluster(li);

		cw[0] = ReadCodeword(cur, cluster);
		if (nCols == 2) {
			cw[1] = ReadCodeword(cur, cluster);
		} else if (nCols == 3) {
			ci = ReadRAP(cur, RAP::C);
			if (checkRAP(li, ci))
				cluster = RAPCluster(ci);
			cw[1] = ReadCodeword(cur, cluster);
			cw[2] = ReadCodeword(cur, cluster);
		} else if (nCols == 4) {
			cw[1] = ReadCodeword(cur, cluster);
			ci = ReadRAP(cur, RAP::C);
			if (checkRAP(li, ci))
				cluster = RAPCluster(ci);
			cw[2] = ReadCodeword(cur, cluster);
			cw[3] = ReadCodeword(cur, cluster);
		}
		ri = ReadRAP(cur, RAP::R);

		if (nCols <= 2) {
			checkRAP(li, ri);
		} else {
			checkRAP(li, ci);
			checkRAP(ci, ri);
		}

		if (ci || ri || cw[0] || cw[1] || cw[2] || cw[3]) {
			// collect crude approximation as fallback
			if (pos[0] == PointI()) {
				pos[0] = PointI(startCur.p);
				pos[1] = PointI(cur.p);
			} else {
				pos[2] = PointI(cur.p);
				pos[3] = PointI(startCur.p);
			}
		}

		auto rowOffset = [cluster = RAPCluster(li)](int nextCluster) mutable {
			int o = ((nextCluster - cluster + 9) / 3) % 3;
			if (o == 2)
				o = -1;
			cluster = nextCluster;
			return o;
		};

		printf("%2d/%d -> ", li, RAPCluster(li));
		for (int x = 0; x < nCols; ++x) {
			printf("%3d/%d ", cw[x].codeword, cw[x].cluster);
			if (cw[x]) {
				li += rowOffset(cw[x].cluster);
				if (li < 1 || li > 52)
					continue;
				auto& cell = histMat(x, li);
				auto cwPtr = std::ranges::find(cell, cw[x]);
				if (cwPtr != cell.end()) {
					cwPtr->count++;
					cwPtr->left += cw[x].left;
					cwPtr->right += cw[x].right;
				}
				else
					cell.push_back(cw[x]);
			}
		}
		printf("\n");
	}

	Matrix<Codeword> cwMat(nCols, 53, {});
	std::ranges::transform(histMat, cwMat.begin(), [](std::vector<Codeword>& hist) {
		if (hist.empty())
			return Codeword{};
		if (hist.size() == 1)
			return hist.front();

		auto best = std::ranges::max_element(hist, [](const auto& a, const auto& b) { return a.count < b.count; });
		bool bestIsUnique = std::ranges::all_of(hist, [&](const auto& a) { return &a == &*best || a.count < best->count; });

		return bestIsUnique ? *best : Codeword{};
	});

#ifdef PRINT_DEBUG
	for (int y = 0; y < cwMat.height(); ++y) {
		printf("%2d: ", y);
		for (int x = 0; x < cwMat.width(); ++x) {
			auto& e = cwMat(x, y);
			e.count ? printf("%3d %2d | ", e.codeword, e.count) : printf("       | ");
		}
		printf("\n");
	}
#endif

	auto si = DetermineSymbolInfo(cwMat, rotFamHist);

	std::vector<int> codewords(si.nCWs() + 1);
	// MicroPDF417 does not encode the number of codewords in the symbol but the DecodeCodewords() function expects the first element
	// to contain the number of codewords. The ReedSolomon algorithm can gracefully handle prepended zeros, the VerifyCodewordCount()
	// function will autocorrect the number of codewords if the first element is zero.
	codewords[0] = 0;
	for (int i = 0; i < si.nCWs(); ++i)
		codewords[i + 1] = (&cwMat(0, si.startRow))[i].codeword;

	std::vector<int> erasures;
	for (int i=0; i < Size(codewords); ++i)
		if (codewords[i] == -1)
			erasures.push_back(i);

	if (Size(erasures) > si.nECCs)
		return {};

	// TODO: implement proper handling of ECI Descriptor codeword at the start of the codeword sequence
	// (see ISO 24728:2006, section 5.2.4.2 ECI Descriptor codeword)
	DecoderResult decoderResult = Pdf417::DecodeCodewords(codewords, si.nECCs, erasures);
	printf("size: %dx%d, firstRow: %d, cws: %d, rotFamHist: %d/%d/%d/%d, rotFam: %d, nEECs: %d, erasures: %d, valid: %d\n", si.nCols,
		   si.nRows, si.startRow, si.nCWs(), rotFamHist[0], rotFamHist[1], rotFamHist[2], rotFamHist[3], si.rotFam, si.nECCs,
		   Size(erasures), decoderResult.isValid());

	// Extrapolate the symbol corners from a perspective transform created from detected codeword positions near the corners of the symbol.
	auto closestCorner = [&](PointI corner, PointI dir) -> PointI {
		for (int x = 0; x <= si.width() / 2; ++x)
			for (int y = 0; y < si.height() / 2; ++y) {
				auto offset = PointI{x, y} * dir;
				if (cwMat(corner + offset).count > 1)
					return offset;
			}
		return {};
	};
	PointI tlI = closestCorner({0, si.startRow}, {1, 1}), trI = closestCorner({si.nCols - 1, si.startRow}, {-1, 1}),
		   brI = closestCorner({si.nCols - 1, si.lastRow()}, {-1, -1}), blI = closestCorner({0, si.lastRow()}, {1, -1});
	PointI cell(17, 2);

	auto src = QuadrilateralF(PointI{10, 1} + tlI * cell, PointI{si.width() - 11, 1} + trI * cell,
							  PointI{si.width() - 11, si.height() - 1} + brI * cell, PointI{10, si.height() - 1} + blI * cell);
	auto dst =
		QuadrilateralF(cwMat(PointI{0, si.startRow} + tlI).leftPos(), cwMat(PointI{si.nCols - 1, si.startRow} + trI).rightPos(),
					   cwMat(PointI{si.nCols - 1, si.lastRow()} + brI).rightPos(), cwMat(PointI{0, si.lastRow()} + blI).leftPos());
	auto mod2pix = PerspectiveTransform(src, dst);
	if (mod2pix.isValid())
		pos = Position{mod2pix({0, 0}), mod2pix({(double)si.width(), 0}), mod2pix({(double)si.width(), (double)si.height()}),
					   mod2pix({0, (double)si.height()})};

	return MatrixBarcode(std::move(decoderResult), DetectorResult({}, std::move(pos)), BarcodeFormat::MicroPDF417);
}

BarcodesData Reader::read(const BinaryBitmap& image, int maxSymbols) const
{
	BarcodesData res;
	// TODO: implement proper isPure mode (performace)
	bool tryRotate = _opts.tryRotate() && !_opts.isPure();
	for (int rotate90 = 0; rotate90 <= static_cast<int>(tryRotate); ++rotate90) {
		// TODO: implement rotation support without full image rotation (performance)
		auto binImg = image.getBitMatrix(rotate90);
		if (!binImg)
			return {};

#ifdef PRINT_DEBUG
		LogMatrixWriter lmw(log, *binImg, 5, "mpdf-log.pnm");
#endif

		for (bool reversed : {false, true}) {
			for (const auto& x : FindCandidates(*binImg, _opts.tryHarder(), reversed)) {
				auto v = ScanCandidate(*binImg, x);
				if (rotate90)
					for (auto& p : v.position)
						p = {binImg->height() - 1 - p.y, p.x};
				if ((v.isValid() || _opts.returnErrors()) && !Contains(res, v))
					res.push_back(std::move(v));
				if (maxSymbols && Size(res) >= maxSymbols)
					return res;
			}
		}
	}

	return res;
}

} // namespace ZXing::MicroPdf417
