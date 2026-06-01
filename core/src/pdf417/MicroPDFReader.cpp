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
#include "PDFCodewordDecoder.h"
#include "PDFScanningDecoder.h"
#include "Pattern.h"

#include <list>
#include <map>
#include <vector>

#ifndef PRINT_DEBUG
#define printf(...){}
#endif

namespace ZXing::MicroPdf417 {

using Pattern417 = std::array<uint16_t, 8>;

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
		res[i] = ToInt(in[i]);
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

struct CodeWord
{
	int cluster = -1;
	int code = -1;
	operator bool() const noexcept { return code != -1 && cluster % 3 == 0; }
};

template <typename POINT>
class BitMatrixModuleCursor : public BitMatrixCursor<POINT>
{
public:
	float ms;

	BitMatrixModuleCursor(const BitMatrix& image, POINT p, POINT d, float ms) : BitMatrixCursor<POINT>(image, p, d), ms(ms) {}
};

using BitMatrixModuleCursorF = BitMatrixModuleCursor<PointF>;

template <typename POINT>
CodeWord ReadCodeWord(BitMatrixModuleCursor<POINT>& cur, int expectedCluster = -1)
{
	auto readCodeWord = [expectedCluster](auto& cur) -> CodeWord {
		auto np = NormalizedPattern<8, 17>(cur.template readPattern<Pattern417>(cur.ms * 20));
		int cluster = (np[0] - np[2] + np[4] - np[6] + 9) % 9;
		int code = expectedCluster == -1 || cluster == expectedCluster ? Pdf417::CodewordDecoder::GetCodeword(ToInt(np)) : -1;

		return {cluster, code};
	};

	auto curBackup = cur;
	auto cw = readCodeWord(cur);
	if (!cw) {
		for (auto offset : {curBackup.left(), curBackup.right()}) {
			auto curAlt = curBackup;
			curAlt.p += offset;
			if (!curAlt.isIn()) // curBackup might be the first or last image row
				continue;
			if (auto cwAlt = readCodeWord(curAlt)) {
				cur = curAlt;
				return cwAlt;
			}
		}
	}
	return cw;
}

using PatternRAP = std::array<uint16_t, 6>;

static int ReadRAP(BitMatrixModuleCursorF& cur, RAP type)
{
	int res = RAPIndex(ToInt(NormalizedPattern<6, 10>(cur.readPatternFromBlack<PatternRAP>(3, cur.ms * 15))), type);
	if (type == RAP::R) {
		auto c = cur;
		if (cur.stepToEdge(1, cur.ms * 2) == 0 || ((c = cur).stepToEdge(1, cur.ms * 2) != 0 && c.isIn()))
			return 0;
	}
	return res;
}

static int IsLRAP(const PatternView& view)
{
	int l = view.sum(6);
	int r = view.subView(6).sum(8);

	// nominal l:r ratio is 10:17
	if (l < 10 || r < 17 || l * 20 < r * 10 || l * 14 > r * 10 || (!view.isAtFirstBar() && view[-1] < l / 10))
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
	auto v = RAPIndex(ToInt(NormalizedPattern<6, 10>(view)), RAP::L);

	return v;
};

static std::tuple<PatternView, int> FindLRAP(const PatternView& view)
{
	constexpr int minSize = 6 + 8 + 6; // 1 column
	auto window = view.subView(0, 6 + 8);
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
			log(centered(p), 2);

			// Try to attach this LRAP to an existing cluster by proximity in x/y; prune stale short
			// clusters and create a new cluster if no suitable match was found.
			for (auto pCluster = res.begin(); pCluster != res.end();) {
				auto diff = p - pCluster->back();
				if (diff.y <= 2 * skip && std::abs(diff.x) <= std::max(diff.y, 2)) {
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

		// remove non-monotonic elements from the front and the back
		while (segs.size() > 1 && segs.begin()->idx > std::next(segs.begin())->idx)
			segs.pop_front();
		while (segs.size() > 1 && segs.rbegin()->idx < std::next(segs.rbegin())->idx)
			segs.pop_back();

		// remove complete segment if too small, too spread out or contains non-monotonic sequence
		if (Size(segs) < 3 || std::abs(segs.back().idx - segs.front().idx) > 2 * Size(segs)
			|| !std::ranges::is_sorted(segs, {}, &Segment::idx))
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
			printf("%d @ %dx%d\n", lrap.idx, lrap.x, lrap.y);
		printf("\n");
	}
#endif
	return res;
}

static int DetermineNumCols(BitMatrixModuleCursorF start, const Cluster& lraps)
{
	auto r = [&](BitMatrixModuleCursorF cur) { return ReadRAP(cur, RAP::R) != 0; };
	auto cw_r = [&](BitMatrixModuleCursorF cur) { return ReadCodeWord(cur) && r(cur); };
	auto c_cw_cw_r = [&](BitMatrixModuleCursorF cur) { return (ReadRAP(cur, RAP::C) != 0) && ReadCodeWord(cur) && cw_r(cur); };
	auto cw_c_cw_cw_r = [&](BitMatrixModuleCursorF cur) { return ReadCodeWord(cur) && c_cw_cw_r(cur); };

	for (auto& p : lraps) {
		auto cur = start;
		cur.p = centered(p);

		auto i = ReadRAP(cur, RAP::L);
		if (!i)
			continue;
		if (!ReadCodeWord(cur, RAPCluster(i)))
			continue;

		if (cw_c_cw_cw_r(cur))
			return 4;
		if (c_cw_cw_r(cur))
			return 3;
		if (cw_r(cur))
			return 2;
		if (r(cur))
			return 1;
	}

	return 0;
}

struct SymbolInfo
{
	int nCols = 0, nRows = 0, nECCs = 0, rotFam = 0, startRow = 0, rowB = 0, rowE = 0;
	int nCWs() const { return nCols * nRows; }
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

static const SymbolInfo& DetermineSymbolInfo(const Matrix<int>& mat, const std::array<int, 4>& rotFamHist)
{
	SymbolInfo res;
	res.nCols = mat.width();
	int rotFam = (std::ranges::max_element(rotFamHist) - rotFamHist.begin()) * 8;
	std::array<int, SYMBOLS.size()> symHist = {};

	for (int y = 1; y < mat.height(); ++y) {
		if (std::any_of(&mat(0, y), &mat(0, y) + mat.width(), [](auto i) { return i != -1; }))
			for (const auto& s : SYMBOLS) {
				if (s.nCols == res.nCols && s.rotFam == rotFam && s.rowB <= y && y <= s.rowE)
					symHist[&s - &SYMBOLS.front()]++;
			}
	}

	int symIdx = std::ranges::max_element(symHist) - symHist.begin();
	if (symHist[symIdx] < 1)
		return SYMBOLS.front();
	return SYMBOLS[symIdx];
}

static BarcodeData ScanCandidate(const BitMatrix& image, const Cluster& lraps)
{
	BitMatrixModuleCursorF startCur(image, centered(lraps.front()), PointF(lraps.back() - lraps.front()), lraps.front().width / 27.f);
	startCur.turnLeft();
	startCur.step(-1);

	int nCols = DetermineNumCols(startCur, lraps);
	printf("nCols: %d\n", nCols);
	if (!nCols)
		return {};

	int failedTries = 0;
	while(failedTries < 10 && image.isIn(startCur.p + startCur.left())) {
		startCur.p += startCur.left();
		auto cur = startCur;
		auto i = ReadRAP(cur, RAP::L);
		failedTries = i ? 0 : failedTries + 1;
	}

	startCur.p += failedTries * startCur.right();

	// Per-column histogram of observed RAP deltas (0..52), used to stabilize row/column pattern decoding.
	Matrix<std::map<int, int>> histMat(nCols, 52 + 1);
	// Histogram of rotation family votes (4 possible families) collected while scanning RAP pairs.
	std::array<int, 4> rotFamHist = {};
	Position pos;

	auto checkRAP = [&](int li, int ri, int scale = 1) {
		int famIdx = ((52 + ri - li) % 52 + 4 * scale) / (8 * scale);
		if (ri == 0 || famIdx > 3)
			return false;
		rotFamHist.at(famIdx)++;
		failedTries = 0;
		return true;
	};

	auto inSweepRange = [&, sweepDir = startCur.right()](PointF p) { return dot(PointF(lraps.back()) - p, sweepDir) >= 0; };

	failedTries = 1;
	for (; image.isIn(startCur.p + startCur.right()) && (inSweepRange(startCur.p) || failedTries < 10);
		 startCur.p += startCur.right(), failedTries++) {
		auto cur = startCur;
		log(cur.p);

		auto li = ReadRAP(cur, RAP::L);
		if (!li)
			continue;
		CodeWord cw[4];
		int ci = 0, ri = 0;

		cw[0] = ReadCodeWord(cur);
		if (nCols == 2) {
			cw[1] = ReadCodeWord(cur);
		} else if (nCols == 3) {
			ci = ReadRAP(cur, RAP::C);
			cw[1] = ReadCodeWord(cur);
			cw[2] = ReadCodeWord(cur);
		} else if (nCols == 4) {
			cw[1] = ReadCodeWord(cur);
			ci = ReadRAP(cur, RAP::C);
			cw[2] = ReadCodeWord(cur);
			cw[3] = ReadCodeWord(cur);
		}
		ri = ReadRAP(cur, RAP::R);

		if (nCols <= 2) {
			checkRAP(li, ri, 1);
		} else {
			checkRAP(li, ci, 1);
			checkRAP(li, ri, 2);
		}

		if (ri) {
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

		printf("%d/%d -> ", li, RAPCluster(li));
		for (int x = 0; x < nCols; ++x) {
			printf("%d/%d ", cw[x].code, cw[x].cluster);
			if (cw[x]) {
				li += rowOffset(cw[x].cluster);
				if (li < 1 || li > 52)
					break;
				histMat(x, li)[cw[x].code]++;
			}
		}
		printf("\n");
	}

	Matrix<int> mat(nCols, 53, -1);
	std::ranges::transform(histMat, mat.begin(), [](std::map<int, int>& hist) {
		return hist.empty() ? -1 : std::ranges::max_element(hist, {}, &std::pair<const int, int>::second)->first;
	});

	auto si = DetermineSymbolInfo(mat, rotFamHist);

	std::vector<int> codewords(si.nCWs() + 1);
	codewords[0] = Size(codewords); // see DecodeCodewords
	std::copy_n(&mat(0, si.startRow), si.nCWs(), codewords.begin() + 1);

	DecoderResult decoderResult = Pdf417::DecodeCodewords(codewords, si.nECCs);
	printf("cws: %d, rotFam: %d, valid: %d\n", si.nCWs(), si.rotFam, decoderResult.isValid());

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
