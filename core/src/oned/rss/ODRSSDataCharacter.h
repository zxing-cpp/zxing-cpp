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

#include <limits>

namespace ZXing {
namespace OneD {
namespace RSS {

class DataCharacter
{
	int _value = std::numeric_limits<int>::max();
	int _checksumPortion = 0;

public:
	DataCharacter() = default;
	DataCharacter(int value, int checksumPortion) : _value(value), _checksumPortion(checksumPortion) {}

	bool isValid() const {
		return _value != std::numeric_limits<int>::max();
	}

	int value() const {
		return _value;
	}

	int checksumPortion() const {
		return _checksumPortion;
	}

	bool operator==(const DataCharacter& other) const {
		return _value == other._value
			&& _checksumPortion == other._checksumPortion;
	}
	bool operator!=(const DataCharacter& other) const {
		return !(*this == other);
	}
};

} // RSS
} // OneD
} // ZXing
