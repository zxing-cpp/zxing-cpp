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

static void Module(const std::vector<int>& codewords, ByteMatrix& bits, int row, int col, int pos, int bit)
{
	if (row < 0) {
		row += bits.height();
		col += 4 - ((bits.height() + 4) % 8);
	}
	if (col < 0) {
		col += bits.width();
		row += 4 - ((bits.width() + 4) % 8);
	}
	// Note the conversion:
	int v = codewords.at(pos);
	v &= 1 << (8 - bit);
	bits.set(col, row, v != 0);
}

/**
* Places the 8 bits of a utah-shaped symbol character in ECC200.
*
* @param row the row
* @param col the column
* @param pos character position
*/
static void Utah(const std::vector<int>& codewords, ByteMatrix& bits, int row, int col, int pos)
{
	Module(codewords, bits, row - 2, col - 2, pos, 1);
	Module(codewords, bits, row - 2, col - 1, pos, 2);
	Module(codewords, bits, row - 1, col - 2, pos, 3);
	Module(codewords, bits, row - 1, col - 1, pos, 4);
	Module(codewords, bits, row - 1, col, pos, 5);
	Module(codewords, bits, row, col - 2, pos, 6);
	Module(codewords, bits, row, col - 1, pos, 7);
	Module(codewords, bits, row, col, pos, 8);
}

static void Corner1(const std::vector<int>& codewords, ByteMatrix& bits, int pos)
{
	Module(codewords, bits, bits.height() - 1, 0, pos, 1);
	Module(codewords, bits, bits.height() - 1, 1, pos, 2);
	Module(codewords, bits, bits.height() - 1, 2, pos, 3);
	Module(codewords, bits, 0, bits.width() - 2, pos, 4);
	Module(codewords, bits, 0, bits.width() - 1, pos, 5);
	Module(codewords, bits, 1, bits.width() - 1, pos, 6);
	Module(codewords, bits, 2, bits.width() - 1, pos, 7);
	Module(codewords, bits, 3, bits.width() - 1, pos, 8);
}

static void Corner2(const std::vector<int>& codewords, ByteMatrix& bits, int pos)
{
	Module(codewords, bits, bits.height() - 3, 0, pos, 1);
	Module(codewords, bits, bits.height() - 2, 0, pos, 2);
	Module(codewords, bits, bits.height() - 1, 0, pos, 3);
	Module(codewords, bits, 0, bits.width() - 4, pos, 4);
	Module(codewords, bits, 0, bits.width() - 3, pos, 5);
	Module(codewords, bits, 0, bits.width() - 2, pos, 6);
	Module(codewords, bits, 0, bits.width() - 1, pos, 7);
	Module(codewords, bits, 1, bits.width() - 1, pos, 8);
}

static void Corner3(const std::vector<int>& codewords, ByteMatrix& bits, int pos)
{
	Module(codewords, bits, bits.height() - 3, 0, pos, 1);
	Module(codewords, bits, bits.height() - 2, 0, pos, 2);
	Module(codewords, bits, bits.height() - 1, 0, pos, 3);
	Module(codewords, bits, 0, bits.width() - 2, pos, 4);
	Module(codewords, bits, 0, bits.width() - 1, pos, 5);
	Module(codewords, bits, 1, bits.width() - 1, pos, 6);
	Module(codewords, bits, 2, bits.width() - 1, pos, 7);
	Module(codewords, bits, 3, bits.width() - 1, pos, 8);
}

static void Corner4(const std::vector<int>& codewords, ByteMatrix& bits, int pos)
{
	Module(codewords, bits, bits.height() - 1, 0, pos, 1);
	Module(codewords, bits, bits.height() - 1, bits.width() - 1, pos, 2);
	Module(codewords, bits, 0, bits.width() - 3, pos, 3);
	Module(codewords, bits, 0, bits.width() - 2, pos, 4);
	Module(codewords, bits, 0, bits.width() - 1, pos, 5);
	Module(codewords, bits, 1, bits.width() - 3, pos, 6);
	Module(codewords, bits, 1, bits.width() - 2, pos, 7);
	Module(codewords, bits, 1, bits.width() - 1, pos, 8);
}

void
DefaultPlacement::Place(const std::vector<int>& codewords, int numcols, int numrows, ByteMatrix& bits)
{
	bits.init(numcols, numrows, -1);

	int pos = 0;
	int row = 4;
	int col = 0;

	do {
		/* repeatedly first check for one of the special corner cases, then... */
		if ((row == numrows) && (col == 0)) {
			Corner1(codewords, bits, pos++);
		}
		if ((row == numrows - 2) && (col == 0) && ((numcols % 4) != 0)) {
			Corner2(codewords, bits, pos++);
		}
		if ((row == numrows - 2) && (col == 0) && (numcols % 8 == 4)) {
			Corner3(codewords, bits, pos++);
		}
		if ((row == numrows + 4) && (col == 2) && ((numcols % 8) == 0)) {
			Corner4(codewords, bits, pos++);
		}
		/* sweep upward diagonally, inserting successive characters... */
		do {
			if ((row < numrows) && (col >= 0) && bits.get(col, row) < 0) {
				Utah(codewords, bits, row, col, pos++);
			}
			row -= 2;
			col += 2;
		} while (row >= 0 && (col < numcols));
		row++;
		col += 3;

		/* and then sweep downward diagonally, inserting successive characters, ... */
		do {
			if ((row >= 0) && (col < numcols) && bits.get(col, row) < 0) {
				Utah(codewords, bits, row, col, pos++);
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
}

} // DataMatrix
} // ZXing
