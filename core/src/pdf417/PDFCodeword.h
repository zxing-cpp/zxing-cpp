#pragma once
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

namespace ZXing {
namespace Pdf417 {

/**
* @author Guenther Grau
*/
class Codeword
{
	static const int BARCODE_ROW_UNKNOWN = -1;

	int _startX = 0;
	int _endX = 0;
	int _bucket = 0;
	int _value = 0;
	int _rowNumber = BARCODE_ROW_UNKNOWN;

public:
	Codeword() {}
	Codeword(int startX, int endX, int bucket, int value) : _startX(startX), _endX(endX), _bucket(bucket), _value(value) {}

	bool hasValidRowNumber() const {
		return isValidRowNumber(_rowNumber);
	}

	bool isValidRowNumber(int rowNumber) const {
		return rowNumber != BARCODE_ROW_UNKNOWN && _bucket == (rowNumber % 3) * 3;
	}

	void setRowNumberAsRowIndicatorColumn() {
		_rowNumber = (_value / 30) * 3 + _bucket / 3;
	}

	int width() const {
		return _endX - _startX;
	}

	int startX() const {
		return _startX;
	}

	int endX() const {
		return _endX;
	}

	int bucket() const {
		return _bucket;
	}

	int value() const {
		return _value;
	}

	int rowNumber() const {
		return _rowNumber;
	}

	void setRowNumber(int rowNumber) {
		_rowNumber = rowNumber;
	}
};

} // Pdf417
} // ZXing
