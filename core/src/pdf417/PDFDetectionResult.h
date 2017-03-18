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

#include "pdf417/PDFBarcodeMetadata.h"
#include "pdf417/PDFBoundingBox.h"
#include "pdf417/PDFDetectionResultColumn.h"
#include "ZXNullable.h"
#include <vector>

namespace ZXing {
namespace Pdf417 {

/**
* @author Guenther Grau
*/
class DetectionResult
{
	BarcodeMetadata _barcodeMetadata;
	std::vector<Nullable<DetectionResultColumn>> _detectionResultColumns;
	Nullable<BoundingBox> _boundingBox;

public:
	DetectionResult() {}
	DetectionResult(const BarcodeMetadata& barcodeMetadata, const Nullable<BoundingBox>& boundingBox);

	void init(const BarcodeMetadata& barcodeMetadata, const Nullable<BoundingBox>& boundingBox);

	const std::vector<Nullable<DetectionResultColumn>> & allColumns();

	int barcodeColumnCount() const {
		return _barcodeMetadata.columnCount();
	}

	int barcodeRowCount() const {
		return _barcodeMetadata.rowCount();
	}

	int barcodeECLevel() const {
		return _barcodeMetadata.errorCorrectionLevel();
	}

	void setBoundingBox(const BoundingBox& boundingBox) {
		_boundingBox = boundingBox;
	}

	const Nullable<BoundingBox> & getBoundingBox() const {
		return _boundingBox;
	}

	void setBoundingBox(const Nullable<BoundingBox>& box) {
		_boundingBox = box;
	}

	void setColumn(int barcodeColumn, const Nullable<DetectionResultColumn>& detectionResultColumn) {
		_detectionResultColumns[barcodeColumn] = detectionResultColumn;
	}

	const Nullable<DetectionResultColumn>& column(int barcodeColumn) const {
		return _detectionResultColumns[barcodeColumn];
	}

	Nullable<DetectionResultColumn>& column(int barcodeColumn) {
		return _detectionResultColumns[barcodeColumn];
	}

};

} // Pdf417
} // ZXing
