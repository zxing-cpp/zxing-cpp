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
#include "BitMatrixUtility.h"
#include "BitMatrix.h"
#include "BitArray.h"

#include <iostream>

namespace ZXing { namespace Utility {

void WriteBitMatrixAsPBM(const BitMatrix& matrix, std::ostream& out)
{
	int width = matrix.width();
	out << "P1\n" << width << ' ' << matrix.height() << '\n';
	for (int y = 0; y < matrix.height(); ++y) {
		BitArray row;
		matrix.getRow(y, row);
		auto iter = row.begin();
		for (int i = 0; i < width; ++i, ++iter) {
			out << (*iter != 0 ? '1' : '0') << (i+1 < width ? ' ' : '\n');
		}
	}
}

std::string ToString(const BitMatrix& matrix)
{
	return ToString(matrix, 'X', '.', false);
}

std::string ToString(const BitMatrix& matrix, char one, char zero, bool addSpace)
{
	std::string result;
	result.reserve((addSpace ? 2 : 1) * (matrix.width() * matrix.height()) + matrix.height());
	int width = matrix.width();
	for (int y = 0; y < matrix.height(); ++y) {
		BitArray row;
		matrix.getRow(y, row);
		auto iter = row.begin();
		for (int i = 0; i < width; ++i, ++iter) {
			result.push_back(*iter != 0 ? one : zero);
			if (addSpace) {
				result.push_back(' ');
			}
		}
		result.push_back('\n');
	}
	return result;
}

BitMatrix ParseBitMatrix(const std::string& str)
{
	return ParseBitMatrix(str, 'X', false);
}

BitMatrix ParseBitMatrix(const std::string& str, char one, bool expectSpace)
{
	auto lineLength = str.find('\n');
	if (lineLength != std::string::npos) {
		int height = str.length() / (lineLength + 1);
		int width = static_cast<int>(expectSpace ? lineLength / 2 : lineLength);
		BitMatrix mat(width, height);
		BitArray row(width);
		for (int y = 0; y < height; ++y) {
			int offset = y*(lineLength + 1);
			auto iter = row.begin();
			for (int x = 0; x < width; ++x, ++iter) {
				if (str.at(offset) == one) {
					row.set(x);
				}
				offset += expectSpace ? 2 : 1;
			}
			mat.setRow(y, row);
			row.clearBits();
		}
		return mat;
	}
	return BitMatrix();
}

}} // ZXing::Utility
