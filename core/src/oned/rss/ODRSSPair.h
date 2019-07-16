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

#include "oned/rss/ODRSSDataCharacter.h"
#include "oned/rss/ODRSSFinderPattern.h"

namespace ZXing {
namespace OneD {
namespace RSS {

class Pair : public DataCharacter
{
	FinderPattern _finderPattern;
	int _count = 0;

public:
	Pair() {}
	Pair(int value, int checksumPortion, const FinderPattern& finderPattern) : DataCharacter(value, checksumPortion), _finderPattern(finderPattern), _count(1) {}

	const FinderPattern& finderPattern() const {
		return _finderPattern;
	}

	int count() const {
		return _count;
	}

	void incrementCount() {
		_count++;
	}

	bool operator==(const Pair& other) const {
		return DataCharacter::operator==(other) && other.finderPattern().value() == finderPattern().value();
	}
};

} // RSS
} // OneD
} // ZXing
