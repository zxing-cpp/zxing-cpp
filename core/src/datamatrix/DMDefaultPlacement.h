#pragma once
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

#include "BitMatrix.h"
#include "ByteArray.h"

#include <array>
#include <cstddef>

namespace ZXing {
namespace DataMatrix {

struct BitPos
{
	int row, col;
};

using BitPosArray = std::array<BitPos, 8>;

/**
 * VisitMatrix gets a functor/callback that is responsible for processing/visiting the bits in the matrix.
 * The return value contains a BitMatrix where each bit that has been visited is set.
 */
template <typename VisitFunc>
BitMatrix VisitMatrix(int numRows, int numCols, VisitFunc visit)
{
	// <p>See ISO 16022:2006, Figure F.3 to F.6</p>
	const BitPosArray CORNER1 = {{{-1, 0}, {-1, 1}, {-1, 2}, {0, -2}, {0, -1}, {1, -1}, {2, -1}, {3, -1}}};
	const BitPosArray CORNER2 = {{{-3, 0}, {-2, 0}, {-1, 0}, {0, -4}, {0, -3}, {0, -2}, {0, -1}, {1, -1}}};
	const BitPosArray CORNER3 = {{{-1, 0}, {-1,-1}, {0, -3}, {0, -2}, {0, -1}, {1, -3}, {1, -2}, {1, -1}}};
	const BitPosArray CORNER4 = {{{-3, 0}, {-2, 0}, {-1, 0}, {0, -2}, {0, -1}, {1, -1}, {2, -1}, {3, -1}}};

	BitMatrix visited(numCols, numRows);
	auto logAccess = [&visited](BitPos p){ visited.set(p.col, p.row); };

	int row = 4;
	int col = 0;

	auto corner = [&numRows, &numCols, logAccess](const BitPosArray& corner) {
		auto clamp = [](int i, int max) { return i < 0 ? i + max : i; };
		BitPosArray result;
		for (size_t bit = 0; bit < 8; ++bit) {
			result[bit] = {clamp(corner[bit].row, numRows), clamp(corner[bit].col, numCols)};
			logAccess(result[bit]);
		}
		return result;
	};

	auto utah = [&numRows, &numCols, logAccess](int row, int col) {
		const BitPosArray delta = {{{-2, -2}, {-2, -1}, {-1, -2}, {-1, -1}, {-1, 0}, {0, -2}, {0, -1}, {0, 0}}};

		BitPosArray result;
		for (size_t bit = 0; bit < 8; ++bit) {
			int r = row + delta[bit].row;
			int c = col + delta[bit].col;
			if (r < 0) {
				r += numRows;
				c += 4 - ((numRows + 4) % 8);
			}
			if (c < 0) {
				c += numCols;
				r += 4 - ((numCols + 4) % 8);
			}
			result[bit] = {r, c};
			logAccess(result[bit]);
		}
		return result;
	};

	do {
		// Check the four corner cases
		if ((row == numRows) && (col == 0))
			visit(corner(CORNER1));
		else if ((row == numRows - 2) && (col == 0) && (numCols % 4 != 0))
			visit(corner(CORNER2));
		else if ((row == numRows + 4) && (col == 2) && (numCols % 8 == 0))
			visit(corner(CORNER3));
		else if ((row == numRows - 2) && (col == 0) && (numCols % 8 == 4))
			visit(corner(CORNER4));

		// Sweep upward diagonally to the right
		do {
			if ((row < numRows) && (col >= 0) && !visited.get(col, row))
				visit(utah(row, col));

			row -= 2;
			col += 2;
		} while (row >= 0 && col < numCols);

		row += 1;
		col += 3;

		// Sweep downward diagonally to the left
		do {
			if ((row >= 0) && (col < numCols) && !visited.get(col, row))
				visit(utah(row, col));

			row += 2;
			col -= 2;
		} while ((row < numRows) && (col >= 0));

		row += 3;
		col += 1;
	} while ((row < numRows) || (col < numCols));

	return visited;
}

/**
* Symbol Character Placement Program. Adapted from Annex M.1 in ISO/IEC 16022:2000(E).
*/
class DefaultPlacement
{
public:
	static BitMatrix Place(const ByteArray& codewords, int numcols, int numrows);
};

} // DataMatrix
} // ZXing
