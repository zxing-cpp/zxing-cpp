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

#include "ODRSSDataCharacter.h"
#include "ODRSSFinderPattern.h"

namespace ZXing {
namespace OneD {
namespace RSS {

/**
* @author Pablo Orduña, University of Deusto (pablo.orduna@deusto.es)
*/
class ExpandedPair
{
	DataCharacter _leftChar;
	DataCharacter _rightChar;
	FinderPattern _finderPattern;
	bool _mayBeLast = false;

public:
	ExpandedPair() = default;
	ExpandedPair(const DataCharacter& leftChar, const DataCharacter& rightChar, const FinderPattern& finderPattern, bool mayBeLast) :
		_leftChar(leftChar), _rightChar(rightChar), _finderPattern(finderPattern), _mayBeLast(mayBeLast) {}

	bool mayBeLast() const {
		return _mayBeLast;
	}

	const DataCharacter& leftChar() const {
		return _leftChar;
	}

	const DataCharacter& rightChar() const {
		return _rightChar;
	}

	const FinderPattern& finderPattern() const {
		return _finderPattern;
	}

	bool mustBeLast() const {
		return !_rightChar.isValid();
	}

	bool operator==(const ExpandedPair& other) const {
		return _leftChar == other._leftChar
			&& _rightChar == other._rightChar
			&& _finderPattern.value() == other._finderPattern.value();
	}
	
	bool operator!=(const ExpandedPair& other) const {
		return !(*this == other);
	}	
};

} // RSS
} // OneD
} // ZXing
