/*
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

#include "oned/ODReader.h"
#include "Result.h"
#include "BitArray.h"
#include "BinaryBitmap.h"
#include "DecodeHints.h"

#include <algorithm>

namespace ZXing {

namespace OneD {

/**
* We're going to examine rows from the middle outward, searching alternately above and below the
* middle, and farther out each time. rowStep is the number of rows between each successive
* attempt above and below the middle. So we'd scan row middle, then middle - rowStep, then
* middle + rowStep, then middle - (2 * rowStep), etc.
* rowStep is bigger as the image is taller, but is always at least 1. We've somewhat arbitrarily
* decided that moving up and down by about 1/16 of the image is pretty good; we try more of the
* image if "trying harder".
*
* @param image The image to decode
* @param hints Any hints that were requested
* @return The contents of the decoded barcode
* @throws NotFoundException Any spontaneous errors which occur
*/
Result
Reader::doDecode(const BinaryBitmap& image, const DecodeHints* hints) const
{
	int width = image.width();
	int height = image.height();
	BitArray row(width);

	int middle = height >> 1;
	bool tryHarder = hints != nullptr && hints->getFlag(DecodeHint::TRY_HARDER);
	int rowStep = std::max(1, height >> (tryHarder ? 8 : 5));
	int maxLines;
	if (tryHarder) {
		maxLines = height; // Look at the whole image, not just the center
	}
	else {
		maxLines = 15; // 15 rows spaced 1/32 apart is roughly the middle half of the image
	}

	DecodeHints copyHints;

	for (int x = 0; x < maxLines; x++) {

		// Scanning from the middle out. Determine which row we're looking at next:
		int rowStepsAboveOrBelow = (x + 1) / 2;
		bool isAbove = (x & 0x01) == 0; // i.e. is x even?
		int rowNumber = middle + rowStep * (isAbove ? rowStepsAboveOrBelow : -rowStepsAboveOrBelow);
		if (rowNumber < 0 || rowNumber >= height) {
			// Oops, if we run off the top or bottom, stop
			break;
		}

		// Estimate black point for this row and load it:
		if (!image.getBlackRow(rowNumber, row))
			continue;

		// While we have the image data in a BitArray, it's fairly cheap to reverse it in place to
		// handle decoding upside down barcodes.
		for (int attempt = 0; attempt < 2; attempt++) {
			if (attempt == 1) { // trying again?
				row.reverse(); // reverse the row and continue
							   // This means we will only ever draw result points *once* in the life of this method
							   // since we want to avoid drawing the wrong points after flipping the row, and,
							   // don't want to clutter with noise from every single row scan -- just the scans
							   // that start on the center line.
				if (hints != nullptr && hints->getPointCallback(DecodeHint::NEED_RESULT_POINT_CALLBACK) != nullptr) {
					copyHints = *hints;
					copyHints.remove(DecodeHint::NEED_RESULT_POINT_CALLBACK);
					hints = &copyHints;
				}
			}
			// Look for a barcode
			Result result = decodeRow(rowNumber, row, hints);
			if (result.isValid()) {
				// We found our barcode
				if (attempt == 1) {
					// But it was upside down, so note that
					result.metadata().put(ResultMetadata::ORIENTATION, 180);
					// And remember to flip the result points horizontally.
					auto points = result.resultPoints();
					if (!points.empty()) {
						for (auto& p : points) {
							p = ResultPoint(width - p.x() - 1, p.y());
						}
						result.setResultPoints(points);
					}
				}
				return result;
			}
		}
	}
	return Result();
}


// Note that we don't try rotation without the try harder flag, even if rotation was supported.
Result
Reader::decode(const BinaryBitmap& image, const DecodeHints* hints) const
{
	Result result = doDecode(image, hints);
	if (result.isValid()) {
		return result;
	}

	bool tryHarder = hints != nullptr && hints->getFlag(DecodeHint::TRY_HARDER);
	if (tryHarder && image.canRotate()) {
		BinaryBitmap rotatedImage = image.rotatedCCW90();
		result = doDecode(rotatedImage, hints);
		if (result.isValid()) {
			// Record that we found it rotated 90 degrees CCW / 270 degrees CW
			auto& metadata = result.metadata();
			metadata.put(ResultMetadata::ORIENTATION, (270 + metadata.getInt(ResultMetadata::ORIENTATION)) % 360);
			// Update result points
			auto points = result.resultPoints();
			if (!points.empty()) {
				int height = rotatedImage.height();
				for (auto& p : points) {
					p = ResultPoint(height - p.y() - 1, p.x());
				}
				result.setResultPoints(points);
			}
			return result;
		}
	}
	return result;
}

/**
* Records the size of successive runs of white and black pixels in a row, starting at a given point.
* The values are recorded in the given array, and the number of runs recorded is equal to the size
* of the array. If the row starts on a white pixel at the given start point, then the first count
* recorded is the run of white pixels starting from that point; likewise it is the count of a run
* of black pixels if the row begin on a black pixels at that point.
*
* @param row row to count from
* @param start offset into row to start at
* @param counters array into which to record counts
* @throws NotFoundException if counters cannot be filled entirely from row before running out
*  of pixels
*/
bool
Reader::RecordPattern(const BitArray& row, int start, std::vector<int>& counters)
{
	size_t numCounters = counters.size();
	std::fill_n(counters.begin(), numCounters, 0);
	int end = row.size();
	if (start >= end) {
		return false;
	}
	bool isWhite = !row.get(start);
	size_t counterPosition = 0;
	int i = start;
	while (i < end) {
		if (row.get(i) ^ isWhite) { // that is, exactly one is true
			counters[counterPosition]++;
		}
		else {
			counterPosition++;
			if (counterPosition == numCounters) {
				break;
			}
			else {
				counters[counterPosition] = 1;
				isWhite = !isWhite;
			}
		}
		i++;
	}
	// If we read fully the last section of pixels and filled up our counters -- or filled
	// the last counter but ran off the side of the image, OK. Otherwise, a problem.
	if (!(counterPosition == numCounters || (counterPosition == numCounters - 1 && i == end))) {
		return false;
	}
	return true;
}

bool
Reader::RecordPatternInReverse(const BitArray& row, int start, std::vector<int>& counters)
{
	// This could be more efficient I guess
	int numTransitionsLeft = static_cast<int>(counters.size());
	bool last = row.get(start);
	while (start > 0 && numTransitionsLeft >= 0) {
		if (row.get(--start) != last) {
			numTransitionsLeft--;
			last = !last;
		}
	}
	if (numTransitionsLeft >= 0) {
		return false;
	}
	return RecordPattern(row, start + 1, counters);
}

/**
* Determines how closely a set of observed counts of runs of black/white values matches a given
* target pattern. This is reported as the ratio of the total variance from the expected pattern
* proportions across all pattern elements, to the length of the pattern.
*
* @param counters observed counters
* @param pattern expected pattern
* @param maxIndividualVariance The most any counter can differ before we give up
* @return ratio of total variance between counters and pattern compared to total pattern size
*/
float
Reader::PatternMatchVariance(const std::vector<int>& counters, const std::vector<int>& pattern, float maxIndividualVariance)
{
	size_t numCounters = counters.size();
	int total = 0;
	int patternLength = 0;
	for (size_t i = 0; i < numCounters; i++) {
		total += counters[i];
		patternLength += pattern[i];
	}
	if (total < patternLength) {
		// If we don't even have one pixel per unit of bar width, assume this is too small
		// to reliably match, so fail:
		return std::numeric_limits<float>::infinity();
	}

	float unitBarWidth = (float)total / patternLength;
	maxIndividualVariance *= unitBarWidth;

	float totalVariance = 0.0f;
	for (size_t x = 0; x < numCounters; x++) {
		int counter = counters[x];
		float scaledPattern = pattern[x] * unitBarWidth;
		float variance = counter > scaledPattern ? counter - scaledPattern : scaledPattern - counter;
		if (variance > maxIndividualVariance) {
			return std::numeric_limits<float>::infinity();
		}
		totalVariance += variance;
	}
	return totalVariance / total;
}

} // OneD
} // ZXing
