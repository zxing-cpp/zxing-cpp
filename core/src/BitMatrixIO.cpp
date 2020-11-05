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

#include "BitMatrixIO.h"

#include "BitArray.h"

#include <fstream>

namespace ZXing {

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

BitMatrix ParseBitMatrix(const std::string& str, char one, bool expectSpace)
{
	auto lineLength = str.find('\n');
	if (lineLength == std::string::npos)
		return {};

	int strStride = expectSpace ? 2 : 1;
	int height = static_cast<int>(str.length() / (lineLength + 1));
	int width = static_cast<int>(lineLength / strStride);
	BitMatrix mat(width, height);
	for (int y = 0; y < height; ++y) {
		size_t offset = y * (lineLength + 1);
		for (int x = 0; x < width; ++x, offset += strStride) {
			if (str[offset] == one)
				mat.set(x, y);
		}
	}
	return mat;
}

void SaveAsPBM(const BitMatrix& matrix, const std::string filename, int quiteZone)
{
	auto out = Inflate(matrix.copy(), 0, 0, quiteZone);
	std::ofstream file(filename);
	file << "P1\n" << out.width() << ' ' << out.height() << '\n';
	file << ToString(out, '1', '0', true);
}

} // ZXing
