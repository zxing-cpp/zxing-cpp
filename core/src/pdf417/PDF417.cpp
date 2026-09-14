/*
 * Copyright 2026 Axel Waggershauser
 */
// SPDX-License-Identifier: Apache-2.0

#include "PDF417.h"

#include "Log.h"
#include "PDFCodewordDecoder.h"

namespace ZXing::PDF417 {

#if 0
// upstream PDF417 PatternView -> codeword implementation (uses "pick the closest pattern" approach and is worse than the below implementation)
using Pattern417I = std::array<int, Pdf417::CodewordDecoder::BARS_IN_MODULE>;
static Pattern417I GetPdf417PatternFromBits(uint32_t value)
{
	Pattern417I result{};
	for (int i = 7; i >= 0; --i)
		value >>= (result[i] = std::countr_one(value) + std::countr_zero(value));
	return result;
}

Codeword ReadCodeword(BitMatrixModuleCursorF& cur)
{
	auto pattern = cur.template readPatternFromBlack<Pattern417I>(cur.ms / 2, cur.ms * (17 + MS_THR), cur.ms * (17 - MS_THR));
	auto symbol = Pdf417::CodewordDecoder::GetDecodedValue(pattern);
	int cluster = CodewordCluster(GetPdf417PatternFromBits(symbol));
	int codeword = Pdf417::CodewordDecoder::GetCodeword(symbol);
	return {cluster, codeword};
}
#else
Codeword ReadCodeword(BitMatrixModuleCursorF& cur)
{
	auto start = cur.p;
	auto pattern = cur.template readPatternFromBlack<Pattern417>(cur.ms / 2, cur.ms * (17 + MS_THR), cur.ms * (17 - MS_THR));
	auto np = NormalizedPattern<8, 17>(pattern);
	int cluster = CodewordCluster(np);
#if 0
	int codeword = Pdf417::CodewordDecoder::GetCodeword(ToInt(np));
#else
	int codeword = Pdf417::CodewordDecoder::GetCodeword(NormalizedE2EPattern<8, 17>(pattern));
#endif
	return {codeword, cluster, 1, PointT<float>(start), PointT<float>(cur.p)};
}
#endif

Codeword ReadCodeword(BitMatrixModuleCursorF& cur, int expectedCluster)
{
	log(cur.p, LOG_R);
	auto start = cur;
	auto cw = ReadCodeword(cur);
	if (!cw || cw.cluster != expectedCluster) {
		for (auto offset : {start.left(), start.right()}) {
			auto curAlt = start.movedBy(cur.ms / 2 * offset);
			if (auto cwAlt = ReadCodeword(curAlt)) {
				if (!cw || cwAlt.cluster == expectedCluster) {
					cw = cwAlt;
					if (cwAlt.cluster == expectedCluster)
						break;
				}
			}
		}
	}
	if (cw)
		cur.ms = dot(cur.p - start.p, mainDirection(cur.d)) / 17.f;
	else {
		cur = start;
		// find the closest white edge near the expected position within the threshold
		if (cur.step(17 * cur.ms) && (cur.isBlack() || !cur.edgeAtFront())) {
			auto back = cur, front = cur;
			for (int step = 0; step < cur.ms * MS_THR; ++step) {
				if (back.step(-1) && back.isWhite() && back.edgeAtFront()) {
					cur = back;
					break;
				}
				if (front.step(1) && front.isWhite() && front.edgeAtFront()) {
					cur = front;
					break;
				}
			}
		}
	}
	log_t("| %3d/%d @ %4.0fx%4.0f %3.0f ", cw.codeword, cw.cluster, cur.p.x * 5, cur.p.y * 5, cur.ms * 5);

	return cw;
}

bool SkipCodeword(BitMatrixModuleCursorF& cur)
{
	int min = cur.ms * (17 - MS_THR), max = cur.ms * (17 + MS_THR);
	int steps = cur.stepToEdge(8, max);
	int totalSteps = steps;
	while (totalSteps < min && steps) {
		steps = cur.stepToEdge(2, max - totalSteps);
		totalSteps += steps;
	}
	cur.ms = totalSteps / 17.f;
	return totalSteps >= min && max > 0;
}

} // namespace ZXing::PDF417
