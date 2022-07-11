/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "GlobalHistogramBinarizer.h"

#include "BitMatrix.h"
#include "ByteArray.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <functional>
#include <utility>

namespace ZXing {

static const int LUMINANCE_BITS = 5;
static const int LUMINANCE_SHIFT = 8 - LUMINANCE_BITS;
static const int LUMINANCE_BUCKETS = 1 << LUMINANCE_BITS;

GlobalHistogramBinarizer::GlobalHistogramBinarizer(const ImageView& buffer) : BinaryBitmap(buffer) {}

GlobalHistogramBinarizer::~GlobalHistogramBinarizer() = default;

// Return -1 on error
static int EstimateBlackPoint(const std::array<int, LUMINANCE_BUCKETS>& buckets)
{
	// Find the tallest peak in the histogram.
	auto firstPeakPos = std::max_element(buckets.begin(), buckets.end());
	int firstPeak = narrow_cast<int>(firstPeakPos - buckets.begin());
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

bool GlobalHistogramBinarizer::getPatternRow(int row, int rotation, PatternRow& res) const
{
	auto buffer = _buffer.rotated(rotation);

	if (buffer.width() < 3)
		return false; // special casing the code below for a width < 3 makes no sense

	res.clear();

	const uint8_t* luminances = buffer.data(0, row);
	const int pixStride = buffer.pixStride();
	std::array<int, LUMINANCE_BUCKETS> buckets = {};
	for (int x = 0; x < buffer.width(); x++)
		buckets[luminances[x * pixStride] >> LUMINANCE_SHIFT]++;

	int blackPoint = EstimateBlackPoint(buckets);
	if (blackPoint <= 0)
		return false;

	auto* lastPos = luminances;
	bool lastVal = luminances[0] < blackPoint;
	if (lastVal)
		res.push_back(0); // first value is number of white pixels, here 0

	auto process = [&](bool val, const uint8_t* p) {
		if (val != lastVal) {
			res.push_back(narrow_cast<PatternRow::value_type>((p - lastPos) / pixStride));
			lastVal = val;
			lastPos = p;
		}
	};

	for (auto *p = luminances + pixStride, *e = luminances + (buffer.width() - 1) * pixStride; p < e; p += pixStride)
		process((-*(p - pixStride) + (int(*p) * 4) - *(p + pixStride)) / 2 < blackPoint, p);

	auto* backPos = buffer.data(buffer.width() - 1, row);
	bool backVal = *backPos < blackPoint;
	process(backVal, backPos);

	res.push_back(narrow_cast<PatternRow::value_type>((backPos - lastPos) / pixStride + 1));

	if (backVal)
		res.push_back(0); // last value is number of white pixels, here 0

	assert(res.size() % 2 == 1);

	return true;
}

// Does not sharpen the data, as this call is intended to only be used by 2D Readers.
std::shared_ptr<const BitMatrix>
GlobalHistogramBinarizer::getBlackMatrix() const
{
	// Quickly calculates the histogram by sampling four rows from the image. This proved to be
	// more robust on the blackbox tests than sampling a diagonal as we used to do.
	std::array<int, LUMINANCE_BUCKETS> localBuckets = {};
	{
		for (int y = 1; y < 5; y++) {
			int row = height() * y / 5;
			const uint8_t* luminances = _buffer.data(0, row);
			int right = (width() * 4) / 5;
			for (int x = width() / 5; x < right; x++)
				localBuckets[luminances[x] >> LUMINANCE_SHIFT]++;
		}
	}

	int blackPoint = EstimateBlackPoint(localBuckets);
	if (blackPoint <= 0)
		return {};

	// We delay reading the entire image luminance until the black point estimation succeeds.
	// Although we end up reading four rows twice, it is consistent with our motto of
	// "fail quickly" which is necessary for continuous scanning.
	auto matrix = std::make_shared<BitMatrix>(width(), height());
	for(int y = 0; y < height(); ++y)
		for(int x = 0; x < width(); ++x)
			matrix->set(x, y, *_buffer.data(x, y) < blackPoint);

	return matrix;
}

} // ZXing
