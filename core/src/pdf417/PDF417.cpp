/*
 * Copyright 2026 Axel Waggershauser
 */
// SPDX-License-Identifier: Apache-2.0

#include "PDF417.h"

#include "LogMatrix.h"
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
	log(cur.p, 4);
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
	printf("%3d/%d, ms: %.1f, @ %5.1fx%5.1f | ", cw.codeword, cw.cluster, cur.ms, cur.p.x, cur.p.y);

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
