/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
* Copyright 2020 Axel Waggershauser
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "PDFReader.h"
#include "PDFDetector.h"
#include "PDFScanningDecoder.h"
#include "PDFCodewordDecoder.h"
#include "PDFDecoderResultExtra.h"
#include "DecodeHints.h"
#include "DecoderResult.h"
#include "Result.h"

#include "BitMatrixCursor.h"
#include "BinaryBitmap.h"
#include "BitArray.h"
#include "DecodeStatus.h"
#include "Pattern.h"
#include "PDFDecodedBitStreamParser.h"
#include "BitMatrixIO.h"
#include <iostream>

#include <vector>
#include <cstdlib>
#include <algorithm>
#include <limits>
#include <utility>

namespace ZXing {
namespace Pdf417 {

static const int MODULES_IN_STOP_PATTERN = 18;

static int GetMinWidth(const Nullable<ResultPoint>& p1, const Nullable<ResultPoint>& p2)
{
	if (p1 == nullptr || p2 == nullptr) {
		// the division prevents an integer overflow (see below). 120 million is still sufficiently large.
		return std::numeric_limits<int>::max() / CodewordDecoder::MODULES_IN_CODEWORD;
	}
	return std::abs(static_cast<int>(p1.value().x()) - static_cast<int>(p2.value().x()));
}

static int GetMinCodewordWidth(const std::array<Nullable<ResultPoint>, 8>& p)
{
	return std::min(std::min(GetMinWidth(p[0], p[4]), GetMinWidth(p[6], p[2]) * CodewordDecoder::MODULES_IN_CODEWORD / MODULES_IN_STOP_PATTERN),
					std::min(GetMinWidth(p[1], p[5]), GetMinWidth(p[7], p[3]) * CodewordDecoder::MODULES_IN_CODEWORD / MODULES_IN_STOP_PATTERN));
}

static int GetMaxWidth(const Nullable<ResultPoint>& p1, const Nullable<ResultPoint>& p2)
{
	if (p1 == nullptr || p2 == nullptr) {
		return 0;
	}
	return std::abs(static_cast<int>(p1.value().x()) - static_cast<int>(p2.value().x()));
}

static int GetMaxCodewordWidth(const std::array<Nullable<ResultPoint>, 8>& p)
{
	return std::max(std::max(GetMaxWidth(p[0], p[4]), GetMaxWidth(p[6], p[2]) * CodewordDecoder::MODULES_IN_CODEWORD / MODULES_IN_STOP_PATTERN),
					std::max(GetMaxWidth(p[1], p[5]), GetMaxWidth(p[7], p[3]) * CodewordDecoder::MODULES_IN_CODEWORD / MODULES_IN_STOP_PATTERN));
}

DecodeStatus DoDecode(const BinaryBitmap& image, bool multiple, std::list<Result>& results)
{
	Detector::Result detectorResult;
	DecodeStatus status = Detector::Detect(image, multiple, detectorResult);
	if (StatusIsError(status)) {
		return status;
	}

	for (const auto& points : detectorResult.points) {
		DecoderResult decoderResult =
			ScanningDecoder::Decode(*detectorResult.bits, points[4], points[5], points[6], points[7],
									GetMinCodewordWidth(points), GetMaxCodewordWidth(points));
		if (decoderResult.isValid()) {
			auto point = [&](int i) { return points[i].value(); };
			Result result(std::move(decoderResult), {point(0), point(2), point(3), point(1)}, BarcodeFormat::PDF417);
			result.metadata().put(ResultMetadata::ERROR_CORRECTION_LEVEL, decoderResult.ecLevel());
			if (auto extra = decoderResult.extra()) {
				result.metadata().put(ResultMetadata::PDF417_EXTRA_METADATA, extra);
			}
			results.push_back(result);
			if (!multiple) {
				return DecodeStatus::NoError;
			}
		}
		else if (!multiple) {
			return decoderResult.errorCode();
		}
	}
	return results.empty() ? DecodeStatus::NotFound : DecodeStatus::NoError;
}

// new implementation (only for isPure use case atm.)

using Pattern417 = std::array<uint16_t, 8>;

struct CodeWord
{
	int cluster = -1;
	int code = -1;
	operator bool() const noexcept { return code != -1; }
};

struct SymbolInfo
{
	int width = 0;
	int height = 0;
	int nRows = 0;
	int nCols = 0;
	int ecLevel = -1;
	bool upsideDown = false;
	operator bool() const noexcept { return nRows >= 3 && nCols >= 1 && ecLevel != -1; }
};

template<typename POINT>
CodeWord ReadCodeWord(BitMatrixCursor<POINT>& cur, int expectedCluster = -1)
{
	auto readCodeWord = [expectedCluster](auto& cur) -> CodeWord {
		auto np     = NormalizedPattern<8, 17>(cur.template readPattern<Pattern417>());
		int cluster = (np[0] - np[2] + np[4] - np[6] + 9) % 9;
		int code = expectedCluster == -1 || cluster == expectedCluster ? CodewordDecoder::GetCodeword(ToInt(np)) : -1;

		return {cluster, code};
	};

	auto curBackup = cur;
	auto cw = readCodeWord(cur);
	if (!cw) {
		for (auto offset : {curBackup.left(), curBackup.right()}) {
			auto curAlt = curBackup;
			curAlt.p += offset;
			if (auto cwAlt = readCodeWord(curAlt)) {
				cur = curAlt;
				return cwAlt;
			}
		}
	}
	return cw;
}

static int Row(CodeWord rowIndicator)
{
	return (rowIndicator.code / 30) * 3 + rowIndicator.cluster / 3;
}

constexpr FixedPattern<8, 17> START_PATTERN = { 8, 1, 1, 1, 1, 1, 1, 3 };

#ifndef PRINT_DEBUG
#define printf(...){}
#endif

template<typename POINT>
SymbolInfo DetectSymbol(BitMatrixCursor<POINT> topCur, int width, int height)
{
	auto rowSkip = topCur.right();
	SymbolInfo res = {width, height};
	int clusterMask = 0;
	int lastRow = -1;
	int rows0, rows1;

	auto pat = BitMatrixCursor<POINT>(topCur).template readPatternFromBlack<Pattern417>(1, width / 3);
	if (!IsPattern(pat, START_PATTERN))
		return {};
	rowSkip = std::max(Reduce(pat) / 17, 1) * bresenhamDirection(rowSkip);

	for (auto startCur = topCur; clusterMask != 0b111 && maxAbsComponent(topCur.p - startCur.p) < height; startCur.p += rowSkip) {
		auto cur = startCur;
		if (!IsPattern(cur.template readPatternFromBlack<Pattern417>(1, width / 3), START_PATTERN))
			break;
		auto cw = ReadCodeWord(cur);
		printf("%3dx%3d:%2d: %4d.%d \n", cur.p.x, cur.p.y, Row(cw), cw.code, cw.cluster);
		if (!cw)
			continue;
		res.upsideDown = std::exchange(lastRow, Row(cw)) > lastRow;
		switch (cw.cluster) {
		case 0: rows0 = cw.code % 30; break;
		case 3: rows1 = cw.code % 3, res.ecLevel = (cw.code % 30) / 3; break;
		case 6: res.nCols = (cw.code % 30) + 1; break;
		default: continue;
		}
		clusterMask |= (1 << cw.cluster / 3);
	}
	if ((clusterMask & 0b11) == 0b11)
		res.nRows = 3 * rows0 + rows1 + 1;
	return res;
}

template<typename POINT>
std::vector<int> ReadCodeWords(BitMatrixCursor<POINT> topCur, SymbolInfo info)
{
	float rowHeight = float(info.height) / info.nRows;
	printf("rows: %d, cols: %d -> row height: %.1f\n", info.nRows, info.nCols, rowHeight);
	auto print = [](CodeWord c [[maybe_unused]]) { printf("%4d.%d ", c.code, c.cluster); };

	auto rowSkip = topCur.right();
	if (info.upsideDown) {
		topCur.p += (info.height - 1) * rowSkip;
		rowSkip = -rowSkip;
	}

	std::vector<int> codeWords;
	codeWords.reserve(info.nRows * info.nCols);
	for (int row = 0; row < info.nRows; ++row) {
		int cluster = (row % 3) * 3;
		auto cur = topCur;
		cur.p += int((row + 0.5f) * rowHeight) * rowSkip;
		// skip start pattern
		cur.stepToEdge(8 + cur.isWhite(), info.width / (info.nCols));
		// read off left row indicator column
		auto cw [[maybe_unused]] = ReadCodeWord(cur, cluster);
		printf("%3dx%3d:%2d: ", cur.p.x, cur.p.y, Row(cw));
		print(cw);

		for (int col = 0; col < info.nCols; ++col) {
			auto cw = ReadCodeWord(cur, cluster);
			codeWords.push_back(cw.code);
			print(cw);
		}

#ifdef PRINT_DEBUG
		print(ReadCodeWord(cur));
		printf("\n");
#endif
	}
	fflush(stdout);

	return codeWords;
}

// forward declaration from PDFScanningDecoder.cpp
DecoderResult DecodeCodewords(std::vector<int>& codewords, int ecLevel, const std::vector<int>& erasures);

static Result DecodePure(const BinaryBitmap& image_)
{
	auto pimage = image_.getBlackMatrix();
	if (!pimage)
		return Result(DecodeStatus::NotFound);
	auto& image = *pimage;

#ifdef PRINT_DEBUG
	SaveAsPBM(image, "weg.pbm");
#endif

	int left, top, width, height;
	if (!image.findBoundingBox(left, top, width, height, 9) || (width < 3 * 17 && height < 3 * 17))
		return Result(DecodeStatus::NotFound);
	int right  = left + width - 1;
	int bottom = top + height - 1;

	// counter intuitively, using a floating point cursor is about twice as fast an integer one (on an AVX architecture)
	BitMatrixCursorF cur(image, centered(PointI{left, top}), PointF{1, 0});
	SymbolInfo info;

	// try all 4 orientations
	for (int a = 0; a < 4; ++a) {
		info = DetectSymbol(cur, width, height);
		if (info)
			break;
		cur.step(width - 1);
		cur.turnRight();
		std::swap(width, height);
	}

	auto codeWords = ReadCodeWords(cur, info);
	if (codeWords.empty())
		return Result(DecodeStatus::NotFound);

	auto res = DecodeCodewords(codeWords, info.ecLevel, {});

	return Result(std::move(res), {{left, top}, {right, top}, {right, bottom}, {left, bottom}}, BarcodeFormat::PDF417);
}

Reader::Reader(const DecodeHints& hints) : _isPure(hints.isPure()) {}

Result
Reader::decode(const BinaryBitmap& image) const
{
	if (_isPure)
		return DecodePure(image);

	std::list<Result> results;
	DecodeStatus status = DoDecode(image, false, results);
	if (StatusIsOK(status)) {
		return results.front();
	}
	return Result(status);
}

std::list<Result>
Reader::decodeMultiple(const BinaryBitmap& image) const
{
	std::list<Result> results;
	DoDecode(image, true, results);
	return results;
}

} // Pdf417
} // ZXing
