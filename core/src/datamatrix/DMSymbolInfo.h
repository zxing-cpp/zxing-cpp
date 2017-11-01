#pragma once
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

namespace ZXing {
namespace DataMatrix {

enum class SymbolShape;

class SymbolInfo
{
	bool _rectangular;
	int _dataCapacity;
	int _errorCodewords;
	int _matrixWidth;
	int _matrixHeight;
	int _dataRegions;
	int _rsBlockData;
	int _rsBlockError;

public:
	SymbolInfo(bool rectangular, int dataCapacity, int errorCodewords, int matrixWidth, int matrixHeight, int dataRegions) :
		SymbolInfo(rectangular, dataCapacity, errorCodewords, matrixWidth, matrixHeight, dataRegions, dataCapacity, errorCodewords) {}

	SymbolInfo(bool rectangular, int dataCapacity, int errorCodewords, int matrixWidth, int matrixHeight, int dataRegions, int rsBlockData, int rsBlockError) :
		_rectangular(rectangular), _dataCapacity(dataCapacity), _errorCodewords(errorCodewords),
		_matrixWidth(matrixWidth), _matrixHeight(matrixHeight), _dataRegions(dataRegions),
		_rsBlockData(rsBlockData), _rsBlockError(rsBlockError)
	{
	}

	static const SymbolInfo* Lookup(int dataCodewords);
	static const SymbolInfo* Lookup(int dataCodewords, SymbolShape shape);
	static const SymbolInfo* Lookup(int dataCodewords, bool allowRectangular);
	static const SymbolInfo* Lookup(int dataCodewords, SymbolShape shape, int minWidth, int minHeight, int maxWidth, int maxHeight);

	int horizontalDataRegions() const;

	int verticalDataRegions() const;

	int symbolDataWidth() const {
		return horizontalDataRegions() * _matrixWidth;
	}

	int symbolDataHeight() const {
		return verticalDataRegions() * _matrixHeight;
	}

	int symbolWidth() const {
		return symbolDataWidth() + (horizontalDataRegions() * 2);
	}

	int symbolHeight() const {
		return symbolDataHeight() + (verticalDataRegions() * 2);
	}

	int matrixWidth() const {
		return _matrixWidth;
	}

	int matrixHeight() const {
		return _matrixHeight;
	}

	int codewordCount() const {
		return _dataCapacity + _errorCodewords;
	}

	int interleavedBlockCount() const {
		if (_rsBlockData > 0)
			return _dataCapacity / _rsBlockData;
		return 10; // Symbol 144
	}

	int dataCapacity() const {
		return _dataCapacity;
	}

	int errorCodewords() const {
		return _errorCodewords;
	}

	int dataLengthForInterleavedBlock(int index) const {
		if (_rsBlockData > 0)
			return _rsBlockData;
		return index <= 8 ? 156 : 155; // Symbol 144
	}

	int errorLengthForInterleavedBlock() const {
		return _rsBlockError;
	}
};

} // DataMatrix
} // ZXing
