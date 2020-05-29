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

#include "ODRSSExpandedPair.h"

#include <algorithm>
#include <vector>

namespace ZXing {
namespace OneD {
namespace RSS {

/**
* One row of an RSS Expanded Stacked symbol, consisting of 1+ expanded pairs.
*/
class ExpandedRow
{
	std::vector<ExpandedPair> _pairs;
	int _rowNumber;
	/** Did this row of the image have to be reversed (mirrored) to recognize the pairs? */
	bool _wasReversed;

public:
	template <typename U>
	ExpandedRow(const U& pairs, int rowNumber, bool wasReversed) : _rowNumber(rowNumber), _wasReversed(wasReversed) {
		_pairs.reserve(pairs.size());
		_pairs.assign(pairs.begin(), pairs.end());
	}

	const std::vector<ExpandedPair>& pairs() const {
		return _pairs;
	}

	int rowNumber() const {
		return _rowNumber;
	}

	bool wasReversed() const {
		return _wasReversed;
	}

	template <typename U>
	bool isEquivalent(const U& list) const {
		return _pairs.size() == list.size() && std::equal(_pairs.begin(), _pairs.end(), list.begin());
	}
};

} // RSS
} // OneD
} // ZXing
