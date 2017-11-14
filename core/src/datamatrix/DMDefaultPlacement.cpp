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

namespace {

struct Pos
{
	int row, col;
};

using PosArray = std::array<Pos, 8>;

} // namespace

inline static bool GetBit(uint8_t codeword, int bit)
{
	return codeword & (1 << (8 - bit));
}

/**
* Places the 8 bits of one of the special corner symbols.
*
* @param pos the array describing the position of the 8 consecutive bits.
*            Negative numbers mean 'counting from the right/bottom'
*/
static void Corner(uint8_t codeword, ByteMatrix& bits, const PosArray& pos)
{
	auto clamp = [](int i, int max) { return i < 0 ? i + max : i; };
	int bit = 1;
	for (auto p : pos)
		bits.set(clamp(p.col, bits.width()), clamp(p.row, bits.height()), GetBit(codeword, bit++));
}

/**
* Places the 8 bits of a utah-shaped symbol character in ECC200.
*/
static void Utah(uint8_t codeword, ByteMatrix& bits, const int row, const int col)
{
	const PosArray deltas = {{{-2, -2}, {-2, -1}, {-1, -2}, {-1, -1}, {-1, 0}, {0, -2}, {0, -1}, {0, 0}}};

	int bit = 1;
	for (auto delta : deltas) {
		int r = row + delta.row;
		int c = col + delta.col;
		if (r < 0) {
			r += bits.height();
			c += 4 - ((bits.height() + 4) % 8);
		}
		if (c < 0) {
			c += bits.width();
			r += 4 - ((bits.width() + 4) % 8);
		}
		bits.set(c, r, GetBit(codeword, bit++));
	}
}

ByteMatrix DefaultPlacement::Place(const ByteArray& codewords, int numcols, int numrows)
{
	ByteMatrix bits(numcols, numrows, -1);

	auto codeword = codewords.begin();
	auto nextCW = [&codeword](){ return *codeword++; };
	int row = 4;
	int col = 0;

	do {
		/* repeatedly first check for one of the special corner cases, then... */
		if ((row == numrows) && (col == 0)) {
			Corner(nextCW(), bits, {{{-1, 0}, {-1, 1}, {-1, 2}, {0, -2}, {0, -1}, {1, -1}, {2, -1}, {3, -1}}});
		}
		if ((row == numrows - 2) && (col == 0) && ((numcols % 4) != 0)) {
			Corner(nextCW(), bits, {{{-3, 0}, {-2, 0}, {-1, 0}, {0, -4}, {0, -3}, {0, -2}, {0, -1}, {1, -1}}});
		}
		if ((row == numrows - 2) && (col == 0) && (numcols % 8 == 4)) {
			Corner(nextCW(), bits, {{{-3, 0}, {-2, 0}, {-1, 0}, {0, -2}, {0, -1}, {1, -1}, {2, -1}, {3, -1}}});
		}
		if ((row == numrows + 4) && (col == 2) && ((numcols % 8) == 0)) {
			Corner(nextCW(), bits, {{{-1, 0}, {-1, -1}, {0, -3}, {0, -2}, {0, -1}, {1, -3}, {1, -2}, {1, -1}}});
		}
		/* sweep upward diagonally, inserting successive characters... */
		do {
			if ((row < numrows) && (col >= 0) && bits.get(col, row) < 0) {
				Utah(nextCW(), bits, row, col);
			}
			row -= 2;
			col += 2;
		} while (row >= 0 && (col < numcols));
		row++;
		col += 3;

		/* and then sweep downward diagonally, inserting successive characters, ... */
		do {
			if ((row >= 0) && (col < numcols) && bits.get(col, row) < 0) {
				Utah(nextCW(), bits, row, col);
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
