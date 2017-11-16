/*
* Copyright 2016 Huy Cuong Nguyen
* Copyright 2006 Jeremias Maerki.
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

#include "datamatrix/DMDefaultPlacement.h"
#include "ByteMatrix.h"
#include "ByteArray.h"

#include <array>

namespace ZXing {
namespace DataMatrix {

ByteMatrix DefaultPlacement::Place(const ByteArray& codewords, int numCols, int numRows)
{
	ByteMatrix result(numCols, numRows, -1);

	auto codeword = codewords.begin();

	VisitMatrix(numRows, numCols, [&codeword, &result](const BitPosArray& bitPos) {
		// Places the 8 bits of a corner or the utah-shaped symbol character in the result matrix
		uint8_t mask = 0x80;
		for (auto& p : bitPos) {
			bool value = *codeword & mask;
			result.set(p.col, p.row, value);
			mask >>= 1;
		}
		++codeword;
	});

	if (codeword != codewords.end())
		return {};

	// Lastly, if the lower righthand corner is untouched, fill in fixed pattern
	if (result.get(numCols - 1, numRows - 1) < 0) {
		result.set(numCols - 1, numRows - 1, true);
		result.set(numCols - 2, numRows - 2, true);
	}

	return result;
}

} // DataMatrix
} // ZXing
