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

#include "pdf417/PDFBoundingBox.h"
#include "pdf417/PDFCodeword.h"
#include "ZXNullable.h"
#include <vector>
#include <memory>

namespace ZXing {
namespace Pdf417 {

class BarcodeMetadata;

/**
* @author Guenther Grau
*/
class DetectionResultColumn
{
public:
	enum class RowIndicator {
		None,
		Left,
		Right,
	};

	DetectionResultColumn() {}
	explicit DetectionResultColumn(const BoundingBox& boundingBox, RowIndicator rowInd = RowIndicator::None);

	bool isRowIndicator() const {
		return _rowIndicator != RowIndicator::None;
	}

	bool isLeftRowIndicator() const {
		return _rowIndicator == RowIndicator::Left;
	}

	Nullable<Codeword> codewordNearby(int imageRow) const;

	int imageRowToCodewordIndex(int imageRow) const {
		return imageRow - _boundingBox.minY();
	}

	void setCodeword(int imageRow, Codeword codeword) {
		_codewords[imageRowToCodewordIndex(imageRow)] = codeword;
	}

	Nullable<Codeword> codeword(int imageRow) const {
		return _codewords[imageRowToCodewordIndex(imageRow)];
	}

	const BoundingBox& boundingBox() const {
		return _boundingBox;
	}

	const std::vector<Nullable<Codeword>>& allCodewords() const {
		return _codewords;
	}

	std::vector<Nullable<Codeword>>& allCodewords() {
		return _codewords;
	}

	void adjustCompleteIndicatorColumnRowNumbers(const BarcodeMetadata& barcodeMetadata);
	bool getRowHeights(std::vector<int>& result); // not const, since it modifies object's state
	bool getBarcodeMetadata(BarcodeMetadata& result); // not const, since it modifies object's state

private:
	BoundingBox _boundingBox;
	std::vector<Nullable<Codeword>> _codewords;
	RowIndicator _rowIndicator = RowIndicator::None;

	void setRowNumbers();
	void adjustIncompleteIndicatorColumnRowNumbers(const BarcodeMetadata& barcodeMetadata);
};

} // Pdf417
} // ZXing
