/*
* Copyright 2017 Huy Cuong Nguyen
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
#include "ByteMatrixUtility.h"
#include "ByteMatrix.h"

#include <iostream>

namespace ZXing { namespace Utility {

std::string ToString(const ByteMatrix& matrix)
{
	return ToString(matrix, '1', '0', ' ', true);
}

std::string ToString(const ByteMatrix& matrix, char one, char zero, char other, bool addSpace)
{
	std::string result;
	result.reserve((addSpace ? 2 : 1) * (matrix.width() * matrix.height()) + matrix.height());
	for (int y = 0; y < matrix.height(); ++y) {
		for (int x = 0; x < matrix.width(); ++x) {
			auto c = matrix.get(x, y);
			if (c == 1)
				result.push_back(one);
			else if (c == 0)
				result.push_back(zero);
			else
				result.push_back(other);
			if (addSpace)
				result.push_back(' ');
		}
		result.push_back('\n');
	}
	return result;
}

}} // ZXing::Utility
