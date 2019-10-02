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

#include "ResultPoint.h"

#include <array>

namespace ZXing {
namespace OneD {
namespace RSS {

class FinderPattern
{
	int _value = -1;
	int _start = -1;
	int _end = -1;
	std::array<ResultPoint, 2> _points;

public:
	FinderPattern() = default;
	FinderPattern(int value, int start, int end, const std::array<ResultPoint, 2>& points) :
		_value(value), _start(start), _end(end), _points(points) {}

	bool isValid() const {
		return _value >= 0;
	}

	int value() const {
		return _value;
	}

	int startPos() const {
		return _start;
	}

	int endPos() const {
		return _end;
	}

	const std::array<ResultPoint, 2>& points() const {
		return _points;
	}
};

} // RSS
} // OneD
} // ZXing
