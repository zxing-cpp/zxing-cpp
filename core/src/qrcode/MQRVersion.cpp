/*
 *  Version.cpp
 *  zxing
 *
 *  Created by Christian Brunschen on 14/05/2008.
 *  Copyright 2008 ZXing authors All rights reserved.
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

#include "MQRVersion.h"

#include "BitMatrix.h"

namespace ZXing::MicroQRCode {

const Version* Version::AllVersions()
{
	/**
	 * See ISO 18004:2006 6.5.1 Table 9
	 */
	static const Version allVersions[] = {{1, {2, 1, 3, 0, 0}},
										  {2, {5, 1, 5, 0, 0, 6, 1, 4, 0, 0}},
										  {3, {6, 1, 11, 0, 0, 8, 1, 9, 0, 0}},
										  {4, {8, 1, 16, 0, 0, 10, 1, 14, 0, 0, 14, 1, 10, 0, 0}}};
	return allVersions;
}

Version::Version(int versionNumber, const std::array<QRCode::ECBlocks, 4>& ecBlocks)
	: _versionNumber(versionNumber), _ecBlocks(ecBlocks)
{
	_totalCodewords = ecBlocks[0].totalDataCodewords();
}

const Version* Version::VersionForNumber(int versionNumber)
{
	if (versionNumber < 1 || versionNumber > 4) {
		return nullptr;
	}
	return &AllVersions()[versionNumber - 1];
}

const Version* Version::ProvisionalVersionForDimension(int dimension)
{
	if (dimension % 2 != 1) {
		return nullptr;
	}
	return VersionForNumber((dimension - 9) / 2);
}

/**
 * See ISO 18004:2006 5.3.4, 6.9.2 and Annex E
 */
BitMatrix Version::buildFunctionPattern() const
{
	int dimension = dimensionForVersion();
	BitMatrix functionPattern(dimension, dimension);

	// Top left finder pattern + separator + format
	functionPattern.setRegion(0, 0, 9, 9);

	// Vertical timing pattern
	functionPattern.setRegion(9, 0, dimension - 9, 1);

	// Horizontal timing pattern
	functionPattern.setRegion(0, 9, 1, dimension - 9);

	return functionPattern;
}

} // namespace ZXing::MicroQRCode
