/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
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

#include "HybridBinarizer.h"
#include "LuminanceSource.h"
#include "ByteArray.h"
#include "BitArray.h"
#include "BitMatrix.h"
#include "DecodeStatus.h"
#include "ZXNumeric.h"

#include <cassert>
#include <array>
#include <mutex>

namespace ZXing {

// This class uses 5x5 blocks to compute local luminance, where each block is 8x8 pixels.
// So this is the smallest dimension in each axis we can accept.
static const int BLOCK_SIZE_POWER = 3;
static const int BLOCK_SIZE = 1 << BLOCK_SIZE_POWER; // ...0100...00
static const int BLOCK_SIZE_MASK = BLOCK_SIZE - 1;   // ...0011...11
static const int MINIMUM_DIMENSION = BLOCK_SIZE * 5;
static const int MIN_DYNAMIC_RANGE = 24;

template <typename T>
class Matrix : std::vector<T>
{
	int _width, _height;

public:
	Matrix(int width, int height) : std::vector<T>(width*height), _width(width), _height(height) {}

	T& operator()(int x, int y)
	{
		assert(x < _width && y < _height);
		return std::vector<T>::operator[](y* _width + x);
	}
	const T& operator()(int x, int y) const
	{
		assert(x < _width && y < _height);
		return std::vector<T>::operator[](y* _width + x);
	}
};

struct HybridBinarizer::DataCache
{
	std::once_flag once;
	std::shared_ptr<const BitMatrix> matrix;
};

HybridBinarizer::HybridBinarizer(const std::shared_ptr<const LuminanceSource>& source, bool pureBarcode) :
	GlobalHistogramBinarizer(source, pureBarcode),
	_cache(new DataCache)
{
}

HybridBinarizer::~HybridBinarizer()
{
}

/**
* Calculates a single black point for each block of pixels and saves it away.
* See the following thread for a discussion of this algorithm:
*  http://groups.google.com/group/zxing/browse_thread/thread/d06efa2c35a7ddc0
*/
static Matrix<int> CalculateBlackPoints(const uint8_t* luminances, int subWidth, int subHeight, int width, int height, int stride)
{
	Matrix<int>	blackPoints(subWidth, subHeight);

	for (int y = 0; y < subHeight; y++) {
		int yoffset = y << BLOCK_SIZE_POWER;
		int maxYOffset = height - BLOCK_SIZE;
		if (yoffset > maxYOffset) {
			yoffset = maxYOffset;
		}
		for (int x = 0; x < subWidth; x++) {
			int xoffset = x << BLOCK_SIZE_POWER;
			int maxXOffset = width - BLOCK_SIZE;
			if (xoffset > maxXOffset) {
				xoffset = maxXOffset;
			}
			int sum = 0;
			int min = 0xFF;
			int max = 0;
			for (int yy = 0, offset = yoffset * stride + xoffset; yy < BLOCK_SIZE; yy++, offset += stride) {
				for (int xx = 0; xx < BLOCK_SIZE; xx++) {
					int pixel = luminances[offset + xx];
					sum += pixel;
					// still looking for good contrast
					if (pixel < min) {
						min = pixel;
					}
					if (pixel > max) {
						max = pixel;
					}
				}
				// short-circuit min/max tests once dynamic range is met
				if (max - min > MIN_DYNAMIC_RANGE) {
					// finish the rest of the rows quickly
					for (yy++, offset += stride; yy < BLOCK_SIZE; yy++, offset += stride) {
						for (int xx = 0; xx < BLOCK_SIZE; xx++) {
							sum += luminances[offset + xx];
						}
					}
				}
			}

			// The default estimate is the average of the values in the block.
			int average = sum >> (BLOCK_SIZE_POWER * 2);
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
static void ThresholdBlock(const uint8_t* luminances, int xoffset, int yoffset, int threshold, int stride, BitMatrix& matrix)
{
	for (int y = 0, offset = yoffset * stride + xoffset; y < BLOCK_SIZE; y++, offset += stride) {
		for (int x = 0; x < BLOCK_SIZE; x++) {
			// Comparison needs to be <= so that black == 0 pixels are black even if the threshold is 0.
			if (luminances[offset + x] <= threshold) {
				matrix.set(xoffset + x, yoffset + y);
			}
		}
	}
}

/**
* For each block in the image, calculate the average black point using a 5x5 grid
* of the blocks around it. Also handles the corner cases (fractional blocks are computed based
* on the last pixels in the row/column which are also used in the previous block).
*/
static void CalculateThresholdForBlock(const uint8_t* luminances, int subWidth, int subHeight, int width, int height,
                                       int stride, const Matrix<int>& blackPoints, BitMatrix& matrix)
{
	for (int y = 0; y < subHeight; y++) {
		int yoffset = y << BLOCK_SIZE_POWER;
		int maxYOffset = height - BLOCK_SIZE;
		if (yoffset > maxYOffset) {
			yoffset = maxYOffset;
		}
		for (int x = 0; x < subWidth; x++) {
			int xoffset = x << BLOCK_SIZE_POWER;
			int maxXOffset = width - BLOCK_SIZE;
			if (xoffset > maxXOffset) {
				xoffset = maxXOffset;
			}
			int left = Clamp(x, 2, subWidth - 3);
			int top = Clamp(y, 2, subHeight - 3);
			int sum = 0;
			for (int dy = -2; dy <= 2; ++dy) {
				for (int dx = -2; dx <= 2; ++dx) {
					sum += blackPoints(left + dx, top + dy);
				}
			}
			int average = sum / 25;
			ThresholdBlock(luminances, xoffset, yoffset, average, stride, matrix);
		}
	}
}


/**
* Calculates the final BitMatrix once for all requests. This could be called once from the
* constructor instead, but there are some advantages to doing it lazily, such as making
* profiling easier, and not doing heavy lifting when callers don't expect it.
*/
static void InitBlackMatrix(const LuminanceSource& source, std::shared_ptr<const BitMatrix>& outMatrix)
{
	int width = source.width();
	int height = source.height();
	ByteArray buffer;
	int stride;
	const uint8_t* luminances = source.getMatrix(buffer, stride);
	int subWidth = width >> BLOCK_SIZE_POWER;
	if ((width & BLOCK_SIZE_MASK) != 0) {
		subWidth++;
	}
	int subHeight = height >> BLOCK_SIZE_POWER;
	if ((height & BLOCK_SIZE_MASK) != 0) {
		subHeight++;
	}
	auto blackPoints = CalculateBlackPoints(luminances, subWidth, subHeight, width, height, stride);

	auto matrix = std::make_shared<BitMatrix>(width, height);
	CalculateThresholdForBlock(luminances, subWidth, subHeight, width, height, stride, blackPoints, *matrix);
	outMatrix = matrix;
}

std::shared_ptr<const BitMatrix>
HybridBinarizer::getBlackMatrix() const
{
	int width = _source->width();
	int height = _source->height();
	if (width >= MINIMUM_DIMENSION && height >= MINIMUM_DIMENSION) {
		std::call_once(_cache->once, &InitBlackMatrix, std::cref(*_source), std::ref(_cache->matrix));
		return _cache->matrix;
	}
	else {
		// If the image is too small, fall back to the global histogram approach.
		return GlobalHistogramBinarizer::getBlackMatrix();
	}
}

std::shared_ptr<BinaryBitmap>
HybridBinarizer::newInstance(const std::shared_ptr<const LuminanceSource>& source) const
{
	return std::make_shared<HybridBinarizer>(source, _pureBarcode);
}

} // ZXing
