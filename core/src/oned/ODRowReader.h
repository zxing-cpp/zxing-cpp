#pragma once
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

#include "BitArray.h"
#include "DecodeStatus.h"

#include <cstddef>
#include <memory>

namespace ZXing {

class Result;

namespace OneD {

/**
* Encapsulates functionality and implementation that is common to all families
* of one-dimensional barcodes.
*
* @author dswitkin@google.com (Daniel Switkin)
* @author Sean Owen
*/
class RowReader
{
public:

	struct DecodingState
	{
		virtual ~DecodingState() {}
	};

	Result decodeSingleRow(int rowNumber, const BitArray& row) const;

	virtual ~RowReader() {}

	/**
	* <p>Attempts to decode a one-dimensional barcode format given a single row of
	* an image.</p>
	*
	* @param rowNumber row number from top of the row
	* @param row the black/white pixel data of the row
	* @param hints decode hints
	* @return {@link Result} containing encoded string and start/end of barcode
	* @throws NotFoundException if no potential barcode is found
	* @throws ChecksumException if a potential barcode is found but does not pass its checksum
	* @throws FormatException if a potential barcode is found but format is invalid
	*/
	virtual Result decodeRow(int rowNumber, const BitArray& row, std::unique_ptr<DecodingState>& state) const = 0;

	/**
	* Scans the given bit range for a pattern identified by evaluating the function object match for each
	* successive run of counters.size() bars. If the pattern is found, it returns the bit range with the
	* pattern. Otherwise an empty range is returned.
	*
	* @param begin/end bit range to scan for pattern
	* @param counters array into which to record counts
	* @param match predicate that gets evaluated to identify the pattern
	* @throws NotFoundException if counters cannot be filled entirely from row before running out
	*  of pixels
	*/
	template <typename Iterator, typename Container, typename Predicate>
	static Range<Iterator> FindPattern(Iterator begin, Iterator end, Container& counters, Predicate match) {
		if (begin == end)
			return {end, end};
		bool lastValue = *begin;
		auto currentCounter = counters.begin();
		*currentCounter = 1;
		for (auto i = std::next(begin); i != end; ++i) {
			if (*i == lastValue) {
				++*currentCounter;
			}
			else {
				if (++currentCounter == counters.end()) {
					if (match(begin, i, counters)) {
						return {begin, i};
					}
					std::advance(begin, counters[0] + counters[1]);
					std::copy(counters.begin() + 2, counters.end(), counters.begin());
					std::fill_n(counters.rbegin(), 2, 0);
					std::advance(currentCounter, -2);
				}
				*currentCounter = 1;
				lastValue = !lastValue;
			}
		}
		return {end, end};
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
	template <typename Iterator, typename Container>
	static Range<Iterator> RecordPattern(Iterator begin, Iterator end, Container& counters) {
		// mark the last counter-slot as empty
		counters.back() = 0;

		auto range = FindPattern(begin, end, counters, [](Iterator, Iterator, Container&) { return true; });

		// If we reached the end iterator but touched the last counter-slot, we accept the result.
		if (range.end == end && counters.back() != 0)
			return {begin, end};
		else
			return range;
	}

	template <typename Iterator, typename Container>
	static Range<Iterator> RecordPatternInReverse(Iterator begin, Iterator end, Container& counters) {
		std::reverse_iterator<Iterator> rbegin(end), rend(begin);
		auto range = RecordPattern(rbegin, rend, counters);
		std::reverse(counters.begin(), counters.end());
		return {range.end.base(), range.begin.base()};
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
	template <typename Container>
	static float PatternMatchVariance(const Container& counters, const Container& pattern, float maxIndividualVariance) {
		assert(counters.size() <= pattern.size()); //TODO: this should test for equality, see ODCode128Reader.cpp:93
		return PatternMatchVariance(counters.data(), pattern.data(), counters.size(), maxIndividualVariance);
	}

	/**
	* Attempts to decode a sequence of black/white lines into single
	* digit.
	*
	* @param counters the counts of runs of observed black/white/black/... values
	* @param patterns the list of patterns to compare the contens of counters to
	* @param requireUnambiguousMatch the 'best match' must be better than all other matches
	* @return The decoded digit index, -1 if no pattern matched
	*/
	template <typename Patterns, typename Counters>
	static int DecodeDigit(const Counters& counters, const Patterns& patterns, float maxAvgVariance,
						   float maxIndividualVariance, bool requireUnambiguousMatch = true)
	{
		float bestVariance = maxAvgVariance; // worst variance we'll accept
		constexpr int INVALID_MATCH = -1;
		int bestMatch = INVALID_MATCH;
		for (size_t i = 0; i < patterns.size(); i++) {
			float variance = PatternMatchVariance(counters, patterns[i], maxIndividualVariance);
			if (variance < bestVariance) {
				bestVariance = variance;
				bestMatch = static_cast<int>(i);
			} else if (requireUnambiguousMatch && variance == bestVariance) {
				// if we find a second 'best match' with the same variance, we can not reliably report to have a suitable match
				bestMatch = INVALID_MATCH;
			}
		}
		return bestMatch;
	}

public:
	static float PatternMatchVariance(const int *counters, const int* pattern, size_t length, float maxIndividualVariance);
};

} // OneD
} // ZXing
