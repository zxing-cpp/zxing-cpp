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

ByteMatrix DefaultPlacement::Place(const ByteArray& codewords, int numcols, int numrows)
{
	ByteMatrix bits(numcols, numrows, -1);

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
		if ((row == numrows) && (col == 0)) {
			placeCorner(CORNER1);
		}
		if ((row == numrows - 2) && (col == 0) && ((numcols % 4) != 0)) {
			placeCorner(CORNER2);
		}
		if ((row == numrows + 4) && (col == 2) && ((numcols % 8) == 0)) {
			placeCorner(CORNER3);
		}
		if ((row == numrows - 2) && (col == 0) && (numcols % 8 == 4)) {
			placeCorner(CORNER4);
		}
		/* sweep upward diagonally, inserting successive characters... */
		do {
			if ((row < numrows) && (col >= 0) && bits.get(col, row) < 0) {
				placeUtah(row, col);
			}
			row -= 2;
			col += 2;
		} while (row >= 0 && (col < numcols));
		row++;
		col += 3;

		/* and then sweep downward diagonally, inserting successive characters, ... */
		do {
			if ((row >= 0) && (col < numcols) && bits.get(col, row) < 0) {
				placeUtah(row, col);
			}
			row += 2;
			col -= 2;
		} while ((row < numrows) && (col >= 0));
		row += 3;
		col++;

		/* ...until the entire array is scanned */
	} while ((row < numrows) || (col < numcols));

	/* Lastly, if the lower righthand corner is untouched, fill in fixed pattern */
	if (bits.get(numcols - 1, numrows - 1) < 0) {
		bits.set(numcols - 1, numrows - 1, true);
		bits.set(numcols - 2, numrows - 2, true);
	}
	return bits;
}

} // DataMatrix
} // ZXing
