/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
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

#include "datamatrix/DMBitMatrixParser.h"
#include "datamatrix/DMDefaultPlacement.h"
#include "datamatrix/DMVersion.h"
#include "BitMatrix.h"
#include "DecodeStatus.h"
#include "ByteArray.h"

namespace ZXing {
namespace DataMatrix {

//private final BitMatrix mappingBitMatrix;
//private final BitMatrix readMappingMatrix;
//private final Version version;

const Version*
BitMatrixParser::ReadVersion(const BitMatrix& bits)
{
	return Version::VersionForDimensions(bits.height(), bits.width());
}

/**
* <p>Extracts the data region from a {@link BitMatrix} that contains
* alignment patterns.</p>
*
* @param bitMatrix Original {@link BitMatrix} with alignment patterns
* @return BitMatrix that has the alignment patterns removed
*/
static BitMatrix ExtractDataRegion(const Version& version, const BitMatrix& bitMatrix)
{
	int symbolSizeRows = version.symbolSizeRows();
	int symbolSizeColumns = version.symbolSizeColumns();

	if (bitMatrix.height() != symbolSizeRows) {
		throw std::invalid_argument("Dimension of bitMarix must match the version size");
	}

	int dataRegionSizeRows = version.dataRegionSizeRows();
	int dataRegionSizeColumns = version.dataRegionSizeColumns();

	int numDataRegionsRow = symbolSizeRows / dataRegionSizeRows;
	int numDataRegionsColumn = symbolSizeColumns / dataRegionSizeColumns;

	int sizeDataRegionRow = numDataRegionsRow * dataRegionSizeRows;
	int sizeDataRegionColumn = numDataRegionsColumn * dataRegionSizeColumns;

	BitMatrix result(sizeDataRegionColumn, sizeDataRegionRow);
	for (int dataRegionRow = 0; dataRegionRow < numDataRegionsRow; ++dataRegionRow) {
		int dataRegionRowOffset = dataRegionRow * dataRegionSizeRows;
		for (int dataRegionColumn = 0; dataRegionColumn < numDataRegionsColumn; ++dataRegionColumn) {
			int dataRegionColumnOffset = dataRegionColumn * dataRegionSizeColumns;
			for (int i = 0; i < dataRegionSizeRows; ++i) {
				int readRowOffset = dataRegionRow * (dataRegionSizeRows + 2) + 1 + i;
				int writeRowOffset = dataRegionRowOffset + i;
				for (int j = 0; j < dataRegionSizeColumns; ++j) {
					int readColumnOffset = dataRegionColumn * (dataRegionSizeColumns + 2) + 1 + j;
					if (bitMatrix.get(readColumnOffset, readRowOffset)) {
						int writeColumnOffset = dataRegionColumnOffset + j;
						result.set(writeColumnOffset, writeRowOffset);
					}
				}
			}
		}
	}
	return result;
}

template <class GetBitPosFunc>
static uint8_t ReadCodeword(const BitMatrix& matrix, BitMatrix& logMatrix, GetBitPosFunc getBitPos)
{
	uint8_t codeword = 0;
	for (int bit = 0; bit < 8; ++bit) {
		BitPos p = getBitPos(matrix, bit);
		logMatrix.set(p.col, p.row);
		if (matrix.get(p.col, p.row))
			codeword |= (1 << (7 - bit));
	}
	return codeword;
}

/**
* <p>Reads the bits in the {@link BitMatrix} representing the mapping matrix (No alignment patterns)
* in the correct order in order to reconstitute the codewords bytes contained within the
* Data Matrix Code.</p>
*
* @return bytes encoded within the Data Matrix Code
* @throws FormatException if the exact number of bytes expected is not read
*/
ByteArray
BitMatrixParser::ReadCodewords(const BitMatrix& bits)
{
	const Version* version = ReadVersion(bits);
	if (version == nullptr) {
		return {};
	}

	BitMatrix mappingBitMatrix = ExtractDataRegion(*version, bits);
	BitMatrix readMappingMatrix(mappingBitMatrix.width(), mappingBitMatrix.height());

	ByteArray result(version->totalCodewords());
	auto codeword = result.begin();

	int numRows = mappingBitMatrix.height();
	int numCols = mappingBitMatrix.width();

	// Read the 8 bits of one of the special corner symbols.
	auto readCorner = [&](const BitPosArray& corner) {
		return ReadCodeword(mappingBitMatrix, readMappingMatrix, [&](const BitMatrix& matrix, int bit){
			return GetCornerBitPos(matrix, bit, corner);
		});
	};
	// Read the 8 bits of a utah-shaped symbol character in ECC200.
	auto readUtah = [&](int row, int col) {
		return ReadCodeword(mappingBitMatrix, readMappingMatrix, [&](const BitMatrix& matrix, int bit){
			return GetUtahBitPos(matrix, bit, row, col);
		});
	};

	int row = 4;
	int col = 0;

	do {
		// Check the four corner cases
		if ((row == numRows) && (col == 0))
			*codeword++ = readCorner(CORNER1);
		else if ((row == numRows - 2) && (col == 0) && (numCols % 4 != 0))
			*codeword++ = readCorner(CORNER2);
		else if ((row == numRows + 4) && (col == 2) && (numCols % 8 == 0))
			*codeword++ = readCorner(CORNER3);
		else if ((row == numRows - 2) && (col == 0) && (numCols % 8 == 4))
			*codeword++ = readCorner(CORNER4);

		// Sweep upward diagonally to the right
		do {
			if ((row < numRows) && (col >= 0) && !readMappingMatrix.get(col, row)) {
				*codeword++ = readUtah(row, col);
			}
			row -= 2;
			col += 2;
		} while (row >= 0 && col < numCols);
		row += 1;
		col += 3;

		// Sweep downward diagonally to the left
		do {
			if ((row >= 0) && (col < numCols) && !readMappingMatrix.get(col, row)) {
				*codeword++ = readUtah(row, col);
			}
			row += 2;
			col -= 2;
		} while ((row < numRows) && (col >= 0));
		row += 3;
		col += 1;
	} while ((row < numRows) || (col < numCols));

	if (codeword != result.end())
		return {};

	return result;
}

} // DataMatrix
} // ZXing
