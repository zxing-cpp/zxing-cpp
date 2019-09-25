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

#include "DMECB.h"

namespace ZXing {
namespace DataMatrix {

/**
* The Version object encapsulates attributes about a particular
* size Data Matrix Code.
*
* @author bbrown@google.com (Brian Brown)
*/
class Version
{
public:
	int versionNumber() const {
		return _versionNumber;
	}

	int symbolSizeRows() const {
		return _symbolSizeRows;
	}

	int symbolSizeColumns() const {
		return _symbolSizeColumns;
	}

	int dataRegionSizeRows() const {
		return _dataRegionSizeRows;
	}

	int dataRegionSizeColumns() const {
		return _dataRegionSizeColumns;
	}

	int totalCodewords() const {
		return _ecBlocks.totalDataCodewords();
	}

	const ECBlocks& ecBlocks() const {
		return _ecBlocks;
	}

	/**
	* <p>Deduces version information from Data Matrix dimensions.</p>
	*
	* @param numRows Number of rows in modules
	* @param numColumns Number of columns in modules
	* @return Version for a Data Matrix Code of those dimensions
	* @throws FormatException if dimensions do correspond to a valid Data Matrix size
	*/
	static const Version* VersionForDimensions(int numRows, int numColumns);

private:
	int _versionNumber;
	int _symbolSizeRows;
	int _symbolSizeColumns;
	int _dataRegionSizeRows;
	int _dataRegionSizeColumns;
	ECBlocks _ecBlocks;

	Version(int versionNumber, int symbolSizeRows, int symbolSizeColumns, int dataRegionSizeRows, int dataRegionSizeColumns, const ECBlocks& ecBlocks);
};

} // DataMatrix
} // ZXing
