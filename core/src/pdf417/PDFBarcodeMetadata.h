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
class BarcodeMetadata
{
	int _columnCount = 0;
	int _errorCorrectionLevel = 0;
	int _rowCountUpperPart = 0;
	int _rowCountLowerPart = 0;

public:
	BarcodeMetadata() {}
	BarcodeMetadata(int columnCount, int rowCountUpperPart, int rowCountLowerPart, int errorCorrectionLevel)
	    : _columnCount(columnCount), _errorCorrectionLevel(errorCorrectionLevel), _rowCountUpperPart(rowCountUpperPart),
	      _rowCountLowerPart(rowCountLowerPart)
	{
	}

	int columnCount() const {
		return _columnCount;
	}

	int errorCorrectionLevel() const {
		return _errorCorrectionLevel;
	}

	int rowCount() const {
		return _rowCountUpperPart + _rowCountLowerPart;
	}

	int rowCountUpperPart() const {
		return _rowCountUpperPart;
	}

	int rowCountLowerPart() const {
		return _rowCountLowerPart;
	}

};

} // Pdf417
} // ZXing
