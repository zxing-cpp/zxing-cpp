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

#include "ByteMatrix.h"

#include <array>

namespace ZXing {

class ByteArray;

namespace DataMatrix {

struct BitPos
{
	int row, col;
};

using BitPosArray = std::array<BitPos, 8>;

/**
 * <p>See ISO 16022:2006, Figure F.3 to F.6</p>
 */
constexpr BitPosArray CORNER1 = {{{-1, 0}, {-1, 1}, {-1, 2}, {0, -2}, {0, -1}, {1, -1}, {2, -1}, {3, -1}}};
constexpr BitPosArray CORNER2 = {{{-3, 0}, {-2, 0}, {-1, 0}, {0, -4}, {0, -3}, {0, -2}, {0, -1}, {1, -1}}};
constexpr BitPosArray CORNER3 = {{{-1, 0}, {-1, -1}, {0, -3}, {0, -2}, {0, -1}, {1, -3}, {1, -2}, {1, -1}}};
constexpr BitPosArray CORNER4 = {{{-3, 0}, {-2, 0}, {-1, 0}, {0, -2}, {0, -1}, {1, -1}, {2, -1}, {3, -1}}};

template <class Matrix>
BitPos GetCornerBitPos(const Matrix& matrix, int bit, const BitPosArray& corner)
{
	auto clamp = [](int i, int max) { return i < 0 ? i + max : i; };
	return {clamp(corner[bit].row, matrix.height()), clamp(corner[bit].col, matrix.width())};
}

template <class Matrix>
BitPos GetUtahBitPos(const Matrix& matrix, int bit, int row, int col)
{
	const BitPosArray delta = {{{-2, -2}, {-2, -1}, {-1, -2}, {-1, -1}, {-1, 0}, {0, -2}, {0, -1}, {0, 0}}};

	row += delta[bit].row;
	col += delta[bit].col;
	if (row < 0) {
		row += matrix.height();
		col += 4 - ((matrix.height() + 4) % 8);
	}
	if (col < 0) {
		col += matrix.width();
		row += 4 - ((matrix.width() + 4) % 8);
	}
	return {row, col};
}

/**
* Symbol Character Placement Program. Adapted from Annex M.1 in ISO/IEC 16022:2000(E).
*/
class DefaultPlacement
{
public:
	static ByteMatrix Place(const ByteArray& codewords, int numcols, int numrows);
};

} // DataMatrix
} // ZXing
