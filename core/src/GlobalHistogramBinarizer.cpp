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

#include "GlobalHistogramBinarizer.h"

#include "BitArray.h"
#include "BitMatrix.h"
#include "ByteArray.h"
#include "LuminanceSource.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <functional>
#include <mutex>
#include <utility>

namespace ZXing {

static const int LUMINANCE_BITS = 5;
static const int LUMINANCE_SHIFT = 8 - LUMINANCE_BITS;
static const int LUMINANCE_BUCKETS = 1 << LUMINANCE_BITS;


struct GlobalHistogramBinarizer::DataCache
{
	std::once_flag once;
	std::shared_ptr<const BitMatrix> matrix;
};

GlobalHistogramBinarizer::GlobalHistogramBinarizer(std::shared_ptr<const LuminanceSource> source) :
	_source(std::move(source)),
	_cache(new DataCache)
{
}

GlobalHistogramBinarizer::~GlobalHistogramBinarizer() = default;

int
GlobalHistogramBinarizer::width() const
{
	return _source->width();
}

int
GlobalHistogramBinarizer::height() const
{
	return _source->height();
}


// Return -1 on error
static int EstimateBlackPoint(const std::array<int, LUMINANCE_BUCKETS>& buckets)
{
	// Find the tallest peak in the histogram.
	auto firstPeakPos = std::max_element(buckets.begin(), buckets.end());
	int firstPeak = static_cast<int>(firstPeakPos - buckets.begin());
	int firstPeakSize = *firstPeakPos;
	int maxBucketCount = firstPeakSize;

	// Find the second-tallest peak which is somewhat far from the tallest peak.
	int secondPeak = 0;
	int secondPeakScore = 0;
	for (int x = 0; x < LUMINANCE_BUCKETS; x++) {
		int distanceToBiggest = x - firstPeak;
		// Encourage more distant second peaks by multiplying by square of distance.
		int score = buckets[x] * distanceToBiggest * distanceToBiggest;
		if (score > secondPeakScore) {
			secondPeak = x;
			secondPeakScore = score;
		}
	}

	// Make sure firstPeak corresponds to the black peak.
	if (firstPeak > secondPeak) {
		std::swap(firstPeak, secondPeak);
	}

	// If there is too little contrast in the image to pick a meaningful black point, throw rather
	// than waste time trying to decode the image, and risk false positives.
	if (secondPeak - firstPeak <= LUMINANCE_BUCKETS / 16) {
		return -1;
	}

	// Find a valley between them that is low and closer to the white peak.
	int bestValley = secondPeak - 1;
	int bestValleyScore = -1;
	for (int x = secondPeak - 1; x > firstPeak; x--) {
		int fromFirst = x - firstPeak;
		int score = fromFirst * fromFirst * (secondPeak - x) * (maxBucketCount - buckets[x]);
		if (score > bestValleyScore) {
			bestValley = x;
			bestValleyScore = score;
		}
	}

	return bestValley << LUMINANCE_SHIFT;
}

bool GlobalHistogramBinarizer::getPatternRow(int y, PatternRow& res) const
{
	int width = _source->width();
	if (width < 3)
		return false; // special casing the code below for a width < 3 makes no sense

	res.clear();

	ByteArray buffer;
	const uint8_t* luminances = _source->getRow(y, buffer);
	std::array<int, LUMINANCE_BUCKETS> buckets = {};
	for (int x = 0; x < width; x++) {
		buckets[luminances[x] >> LUMINANCE_SHIFT]++;
	}
	int blackPoint = EstimateBlackPoint(buckets);
	if (blackPoint <= 0)
		return false;

	auto* lastPos = luminances;
	bool lastVal = luminances[0] < blackPoint;
	if (lastVal)
		res.push_back(0); // first value is number of white pixels, here 0

	auto process = [&](bool val, const uint8_t* p) {
		if (val != lastVal) {
			res.push_back(static_cast<PatternRow::value_type>(p - lastPos));
			lastVal = val;
			lastPos = p;
		}
	};

	for (auto* p = luminances + 1; p < luminances + width - 1; ++p)
		process((-*(p - 1) + (int(*p) * 4) - *(p + 1)) / 2 < blackPoint, p);

	auto* backPos = luminances + width - 1;
	bool backVal = *backPos < blackPoint;
	process(backVal, backPos);

	res.push_back(static_cast<PatternRow::value_type>(backPos - lastPos + 1));

	if (backVal)
		res.push_back(0); // last value is number of white pixels, here 0

	assert(res.size() % 2 == 1);

	return true;
}

static void InitBlackMatrix(const LuminanceSource& source, std::shared_ptr<const BitMatrix>& outMatrix)
{
	int width = source.width();
	int height = source.height();
	auto matrix = std::make_shared<BitMatrix>(width, height);

	// Quickly calculates the histogram by sampling four rows from the image. This proved to be
	// more robust on the blackbox tests than sampling a diagonal as we used to do.
	std::array<int, LUMINANCE_BUCKETS> localBuckets = {};
	{
		ByteArray buffer;
		for (int y = 1; y < 5; y++) {
			int row = height * y / 5;
			const uint8_t* luminances = source.getRow(row, buffer);
			int right = (width * 4) / 5;
			for (int x = width / 5; x < right; x++) {
				localBuckets[luminances[x] >> LUMINANCE_SHIFT]++;
			}
		}
	}

	int blackPoint = EstimateBlackPoint(localBuckets);
	if (blackPoint <= 0)
		return;

	// We delay reading the entire image luminance until the black point estimation succeeds.
	// Although we end up reading four rows twice, it is consistent with our motto of
	// "fail quickly" which is necessary for continuous scanning.
	ByteArray buffer;
	int stride;
	const uint8_t* luminances = source.getMatrix(buffer, stride);
	for (int y = 0; y < height; y++) {
		int offset = y * stride;
		for (int x = 0; x < width; x++) {
			if (luminances[offset + x] < blackPoint) {
				matrix->set(x, y);
			}
		}
	}
	outMatrix = matrix;
}

// Does not sharpen the data, as this call is intended to only be used by 2D Readers.
std::shared_ptr<const BitMatrix>
GlobalHistogramBinarizer::getBlackMatrix() const
{
	std::call_once(_cache->once, &InitBlackMatrix, std::cref(*_source), std::ref(_cache->matrix));
	return _cache->matrix;
}

bool
GlobalHistogramBinarizer::canCrop() const
{
	return _source->canCrop();
}

std::shared_ptr<BinaryBitmap>
GlobalHistogramBinarizer::cropped(int left, int top, int width, int height) const
{
	return newInstance(_source->cropped(left, top, width, height));
}

bool
GlobalHistogramBinarizer::canRotate() const
{
	return _source->canRotate();
}

std::shared_ptr<BinaryBitmap>
GlobalHistogramBinarizer::rotated(int degreeCW) const
{
	return newInstance(_source->rotated(degreeCW));
}

std::shared_ptr<BinaryBitmap>
GlobalHistogramBinarizer::newInstance(const std::shared_ptr<const LuminanceSource>& source) const
{
	return std::make_shared<GlobalHistogramBinarizer>(source);
}


} // ZXing
