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

#include "oned/ODRowReader.h"
#include "Result.h"
#include "BitArray.h"

#include <algorithm>

namespace ZXing {
namespace OneD {

RowReader::~RowReader()
{
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
ErrorStatus
RowReader::RecordPattern(const BitArray& row, int start, int* counters, size_t length)
{
	std::fill_n(counters, length, 0);
	int end = row.size();
	if (start >= end) {
		return ErrorStatus::NotFound;
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
			if (counterPosition == length) {
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
	if (!(counterPosition == length || (counterPosition + 1 == length && i == end))) {
		return ErrorStatus::NotFound;
	}
	return ErrorStatus::NoError;
}

ErrorStatus
RowReader::RecordPatternInReverse(const BitArray& row, int start, int* counters, size_t length)
{
	// This could be more efficient I guess
	int numTransitionsLeft = static_cast<int>(length);
	bool last = row.get(start);
	while (start > 0 && numTransitionsLeft >= 0) {
		if (row.get(--start) != last) {
			numTransitionsLeft--;
			last = !last;
		}
	}
	if (numTransitionsLeft >= 0) {
		return ErrorStatus::NotFound;
	}
	return RecordPattern(row, start + 1, counters, length);
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
RowReader::PatternMatchVariance(const int *counters, const int* pattern, size_t length, float maxIndividualVariance)
{
	int total = 0;
	int patternLength = 0;
	for (size_t i = 0; i < length; i++) {
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
	for (size_t x = 0; x < length; x++) {
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
