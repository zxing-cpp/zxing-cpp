/*
* Copyright 2016 Huy Cuong Nguyen
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

#include "datamatrix/DMWriter.h"
#include "datamatrix/DMSymbolShape.h"
#include "datamatrix/DMHighLevelEncoder.h"
#include "datamatrix/DMSymbolInfo.h"
#include "datamatrix/DMECEncoder.h"
#include "datamatrix/DMDefaultPlacement.h"
#include "BitMatrix.h"
#include "ByteMatrix.h"
#include "ByteArray.h"
#include "ZXStrConvWorkaround.h"

#include <stdexcept>
#include <string>
#include <algorithm>

namespace ZXing {
namespace DataMatrix {

/**
* Encode the given symbol info to a bit matrix.
*
* @param placement  The DataMatrix placement.
* @param symbolInfo The symbol info to encode.
* @return The bit matrix generated.
*/
static BitMatrix EncodeLowLevel(const ByteMatrix& placement, const SymbolInfo& symbolInfo) {
	int symbolWidth = symbolInfo.symbolDataWidth();
	int symbolHeight = symbolInfo.symbolDataHeight();

	ByteMatrix matrix(symbolInfo.symbolWidth(), symbolInfo.symbolHeight());
	int matrixY = 0;
	for (int y = 0; y < symbolHeight; y++) {
		// Fill the top edge with alternate 0 / 1
		int matrixX;
		if ((y % symbolInfo.matrixHeight()) == 0) {
			matrixX = 0;
			for (int x = 0; x < matrix.width(); x++) {
				matrix.set(matrixX, matrixY, (x % 2) == 0);
				matrixX++;
			}
			matrixY++;
		}
		matrixX = 0;
		for (int x = 0; x < symbolWidth; x++) {
			// Fill the right edge with full 1
			if ((x % symbolInfo.matrixWidth()) == 0) {
				matrix.set(matrixX, matrixY, true);
				matrixX++;
			}
			matrix.set(matrixX, matrixY, placement.get(x, y) == 1);
			matrixX++;
			// Fill the right edge with alternate 0 / 1
			if ((x % symbolInfo.matrixWidth()) == symbolInfo.matrixWidth() - 1) {
				matrix.set(matrixX, matrixY, (y % 2) == 0);
				matrixX++;
			}
		}
		matrixY++;
		// Fill the bottom edge with full 1
		if ((y % symbolInfo.matrixHeight()) == symbolInfo.matrixHeight() - 1) {
			matrixX = 0;
			for (int x = 0; x < matrix.width(); x++) {
				matrix.set(matrixX, matrixY, true);
				matrixX++;
			}
			matrixY++;
		}
	}

	// Zero is white in the bytematrix
	return BitMatrix(matrix, 1);
}

Writer::Writer() :
	_shapeHint(SymbolShape::NONE),
	_minWidth(-1),
	_minHeight(-1),
	_maxWidth(-1),
	_maxHeight(-1)
{
}

BitMatrix
Writer::encode(const std::wstring& contents, int width, int height) const
{
	if (contents.empty()) {
		throw std::invalid_argument("Found empty contents");
	}

	if (width < 0 || height < 0) {
		throw std::invalid_argument("Requested dimensions are invalid");
	}

	//1. step: Data encodation
	auto encoded = HighLevelEncoder::Encode(contents, _shapeHint, _minWidth, _minHeight, _maxWidth, _maxHeight);
	const SymbolInfo* symbolInfo = SymbolInfo::Lookup(static_cast<int>(encoded.size()), _shapeHint, _minWidth, _minHeight, _maxWidth, _maxHeight);
	if (symbolInfo == nullptr) {
		throw std::invalid_argument("Can't find a symbol arrangement that matches the message. Data codewords: " + std::to_string(encoded.size()));
	}

	//2. step: ECC generation
	ECEncoder::EncodeECC200(encoded, *symbolInfo);

	//3. step: Module placement in Matrix
	ByteMatrix placement = DefaultPlacement::Place(encoded, symbolInfo->symbolDataWidth(), symbolInfo->symbolDataHeight());

	//4. step: low-level encoding
	BitMatrix result = EncodeLowLevel(placement, *symbolInfo);

	//5. step: scale-up to requested size, minimum required quite zone is 1
	return Inflate(std::move(result), width, height, 0);
}

} // DataMatrix
} // ZXing
