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

//inline static bool getBit(const ThreeStateBoolArray& bits, int col, int row)
//{
//	return bits[row * numcols + col] == 1;
//}
//
//inline static void setBit(ThreeStateBoolArray& bits, int col, int row, boolean bit) {
//	bits[row * numcols + col] = (byte)(bit ? 1 : 0);
//}
//
//final boolean hasBit(int col, int row) {
//	return bits[row * numcols + col] >= 0;
//}

template <class GetBitPosFunc>
static void PlaceCodeword(uint8_t codeword, ByteMatrix& matrix, GetBitPosFunc getBitPos)
{
	for (int bit = 0; bit < 8; ++bit) {
		BitPos p = getBitPos(matrix, bit);
		bool value = codeword & (1 << (7 - bit));
		matrix.set(p.col, p.row, value);
	}
}

ByteMatrix DefaultPlacement::Place(const ByteArray& codewords, int numCols, int numRows)
{
	ByteMatrix bits(numCols, numRows, -1);

	auto codeword = codewords.begin();
	// Places the 8 bits of one of the special corner symbols.
	auto placeCorner = [&](const BitPosArray& corner) {
		PlaceCodeword(*codeword++, bits, [&](const ByteMatrix& matrix, int bit){
			return GetCornerBitPos(matrix, bit, corner);
		});
	};
	// Places the 8 bits of a utah-shaped symbol character in ECC200.
	auto placeUtah = [&](int row, int col) {
		PlaceCodeword(*codeword++, bits, [&](const ByteMatrix& matrix, int bit){
			return GetUtahBitPos(matrix, bit, row, col);
		});
	};

	int row = 4;
	int col = 0;

	do {
		/* repeatedly first check for one of the special corner cases, then... */
		if ((row == numRows) && (col == 0))
			placeCorner(CORNER1);
		else if ((row == numRows - 2) && (col == 0) && (numCols % 4 != 0))
			placeCorner(CORNER2);
		else if ((row == numRows + 4) && (col == 2) && (numCols % 8 == 0))
			placeCorner(CORNER3);
		else if ((row == numRows - 2) && (col == 0) && (numCols % 8 == 4))
			placeCorner(CORNER4);

		/* sweep upward diagonally, inserting successive characters... */
		do {
			if ((row < numRows) && (col >= 0) && bits.get(col, row) < 0) {
				placeUtah(row, col);
			}
			row -= 2;
			col += 2;
		} while (row >= 0 && col < numCols);
		row += 1;
		col += 3;

		/* and then sweep downward diagonally, inserting successive characters, ... */
		do {
			if ((row >= 0) && (col < numCols) && bits.get(col, row) < 0) {
				placeUtah(row, col);
			}
			row += 2;
			col -= 2;
		} while ((row < numRows) && (col >= 0));
		row += 3;
		col += 1;
	} while ((row < numRows) || (col < numCols));

	/* Lastly, if the lower righthand corner is untouched, fill in fixed pattern */
	if (bits.get(numCols - 1, numRows - 1) < 0) {
		bits.set(numCols - 1, numRows - 1, true);
		bits.set(numCols - 2, numRows - 2, true);
	}
	return bits;
}

} // DataMatrix
} // ZXing
