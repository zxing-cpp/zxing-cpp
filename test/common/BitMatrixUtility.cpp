/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2017 Axel Waggershauser
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
#include "ByteMatrixUtility.h"
#include "ByteMatrix.h"

#include <iostream>

namespace ZXing { namespace Utility {

void WriteBitMatrixAsPBM(const BitMatrix& matrix, std::ostream& out, int quiteZone)
{
	ByteMatrix bytes(matrix.width() + 2 * quiteZone, matrix.height() + 2 * quiteZone, 0);
	for (int y = 0; y < matrix.height(); ++y)
		for (int x = 0; x < matrix.width(); ++x)
			bytes.set(x + quiteZone, y + quiteZone, matrix.get(x, y));

	out << "P1\n" << bytes.width() << ' ' << bytes.height() << '\n';
	out << ToString(bytes);
}

std::string ToString(const BitMatrix& matrix)
{
	return ToString(matrix, 'X', ' ', true);
}

std::string ToString(const BitMatrix& matrix, char one, char zero, bool addSpace, bool printAsCString)
{
	std::string result;
	result.reserve((addSpace ? 2 : 1) * (matrix.width() * matrix.height()) + matrix.height());
	for (int y = 0; y < matrix.height(); ++y) {
		BitArray row;
		matrix.getRow(y, row);
		if (printAsCString)
			result += '"';
		for (auto bit : row) {
			result += bit ? one : zero;
			if (addSpace)
				result += ' ';
		}
		if (printAsCString)
			result += "\\n\"";
		result += '\n';
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
	if (lineLength == std::string::npos)
		return {};

	int height = static_cast<int>(str.length() / (lineLength + 1));
	int width = static_cast<int>(expectSpace ? lineLength / 2 : lineLength);
	BitMatrix mat(width, height);
	for (int y = 0; y < height; ++y) {
		size_t offset = y * (lineLength + 1);
		for (int x = 0; x < width; ++x) {
			if (str.at(offset) == one)
				mat.set(x, y);
			offset += expectSpace ? 2 : 1;
		}
	}
	return mat;
}

}} // ZXing::Utility
