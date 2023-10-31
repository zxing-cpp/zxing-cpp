/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "HybridBinarizer.h"

#include "BitMatrix.h"
#include "Matrix.h"

#include <cstdint>
#include <memory>
#include <mutex>
#include <utility>

namespace ZXing {

// This class uses 5x5 blocks to compute local luminance, where each block is 8x8 pixels.
// So this is the smallest dimension in each axis we can accept.
static constexpr int BLOCK_SIZE = 8;
static constexpr int MINIMUM_DIMENSION = BLOCK_SIZE * 5;
static constexpr int MIN_DYNAMIC_RANGE = 24;

HybridBinarizer::HybridBinarizer(const ImageView& iv) : GlobalHistogramBinarizer(iv) {}

HybridBinarizer::~HybridBinarizer() = default;

bool HybridBinarizer::getPatternRow(int row, int rotation, PatternRow& res) const
{
#if 1
	// This is the original "hybrid" behavior: use GlobalHistogram for the 1D case
	return GlobalHistogramBinarizer::getPatternRow(row, rotation, res);
#else
	// This is an alternative that can be faster in general and perform better in unevenly lit sitations like
	// https://github.com/zxing-cpp/zxing-cpp/blob/master/test/samples/ean13-2/21.png. That said, it fairs
	// worse in borderline low resolution situations. With the current black box sample set we'd loose 94
	// test cases while gaining 53 others.
	auto bits = getBitMatrix();
	if (bits)
		GetPatternRow(*bits, row, res, rotation % 180 != 0);
	return bits != nullptr;
#endif
}

/**
* Calculates a single black point for each block of pixels and saves it away.
* See the following thread for a discussion of this algorithm:
*  http://groups.google.com/group/zxing/browse_thread/thread/d06efa2c35a7ddc0
*/
static Matrix<int> CalculateBlackPoints(const uint8_t* __restrict luminances, int subWidth, int subHeight, int width, int height,
										int rowStride)
{
	Matrix<int>	blackPoints(subWidth, subHeight);

	for (int y = 0; y < subHeight; y++) {
		int yoffset = std::min(y * BLOCK_SIZE, height - BLOCK_SIZE);
		for (int x = 0; x < subWidth; x++) {
			int xoffset = std::min(x * BLOCK_SIZE, width - BLOCK_SIZE);
			int sum = 0;
			uint8_t min = luminances[yoffset * rowStride + xoffset];
			uint8_t max = min;
			for (int yy = 0, offset = yoffset * rowStride + xoffset; yy < BLOCK_SIZE; yy++, offset += rowStride) {
				for (int xx = 0; xx < BLOCK_SIZE; xx++) {
					auto pixel = luminances[offset + xx];
					sum += pixel;
					if (pixel < min)
						min = pixel;
					if (pixel > max)
						max = pixel;
				}
				// short-circuit min/max tests once dynamic range is met
				if (max - min > MIN_DYNAMIC_RANGE) {
					// finish the rest of the rows quickly
					for (yy++, offset += rowStride; yy < BLOCK_SIZE; yy++, offset += rowStride) {
						for (int xx = 0; xx < BLOCK_SIZE; xx++) {
							sum += luminances[offset + xx];
						}
					}
				}
			}

			// The default estimate is the average of the values in the block.
			int average = sum / (BLOCK_SIZE * BLOCK_SIZE);
			if (max - min <= MIN_DYNAMIC_RANGE) {
				// If variation within the block is low, assume this is a block with only light or only
				// dark pixels. In that case we do not want to use the average, as it would divide this
				// low contrast area into black and white pixels, essentially creating data out of noise.
				//
				// The default assumption is that the block is light/background. Since no estimate for
				// the level of dark pixels exists locally, use half the min for the block.
				average = min / 2;

				if (y > 0 && x > 0) {
					// Correct the "white background" assumption for blocks that have neighbors by comparing
					// the pixels in this block to the previously calculated black points. This is based on
					// the fact that dark barcode symbology is always surrounded by some amount of light
					// background for which reasonable black point estimates were made. The bp estimated at
					// the boundaries is used for the interior.

					// The (min < bp) is arbitrary but works better than other heuristics that were tried.
					int averageNeighborBlackPoint =
						(blackPoints(x, y - 1) + (2 * blackPoints(x - 1, y)) + blackPoints(x - 1, y - 1)) / 4;
					if (min < averageNeighborBlackPoint) {
						average = averageNeighborBlackPoint;
					}
				}
			}
			blackPoints(x, y) = average;
		}
	}
	return blackPoints;
}


/**
* Applies a single threshold to a block of pixels.
*/
static void ThresholdBlock(const uint8_t* __restrict luminances, int xoffset, int yoffset, int threshold, int rowStride,
						   BitMatrix& matrix)
{
	for (int y = yoffset; y < yoffset + BLOCK_SIZE; ++y) {
		auto* src = luminances + y * rowStride + xoffset;
		auto* const dstBegin = matrix.row(y).begin() + xoffset;
		// TODO: fix pixelStride > 1 case
		for (auto* dst = dstBegin; dst < dstBegin + BLOCK_SIZE; ++dst, ++src)
			*dst = (*src <= threshold) * BitMatrix::SET_V;
	}
}

/**
* For each block in the image, calculate the average black point using a 5x5 grid
* of the blocks around it. Also handles the corner cases (fractional blocks are computed based
* on the last pixels in the row/column which are also used in the previous block).
*/
static std::shared_ptr<BitMatrix> CalculateMatrix(const uint8_t* __restrict luminances, int subWidth, int subHeight, int width,
												  int height, int rowStride, const Matrix<int>& blackPoints)
{
	auto matrix = std::make_shared<BitMatrix>(width, height);

	for (int y = 0; y < subHeight; y++) {
		int yoffset = std::min(y * BLOCK_SIZE, height - BLOCK_SIZE);
		for (int x = 0; x < subWidth; x++) {
			int xoffset = std::min(x * BLOCK_SIZE, width - BLOCK_SIZE);
			int left = std::clamp(x, 2, subWidth - 3);
			int top = std::clamp(y, 2, subHeight - 3);
			int sum = 0;
			for (int dy = -2; dy <= 2; ++dy) {
				for (int dx = -2; dx <= 2; ++dx) {
					sum += blackPoints(left + dx, top + dy);
				}
			}
			int average = sum / 25;
			ThresholdBlock(luminances, xoffset, yoffset, average, rowStride, *matrix);
		}
	}

	return matrix;
}

std::shared_ptr<const BitMatrix> HybridBinarizer::getBlackMatrix() const
{
	if (width() >= MINIMUM_DIMENSION && height() >= MINIMUM_DIMENSION) {
		const uint8_t* luminances = _buffer.data(0, 0);
		int subWidth = (width() + BLOCK_SIZE - 1) / BLOCK_SIZE; // ceil(width/BS)
		int subHeight = (height() + BLOCK_SIZE - 1) / BLOCK_SIZE; // ceil(height/BS)
		auto blackPoints =
			CalculateBlackPoints(luminances, subWidth, subHeight, width(), height(), _buffer.rowStride());

		return CalculateMatrix(luminances, subWidth, subHeight, width(), height(), _buffer.rowStride(), blackPoints);
	} else {
		// If the image is too small, fall back to the global histogram approach.
		return GlobalHistogramBinarizer::getBlackMatrix();
	}
}

} // ZXing
