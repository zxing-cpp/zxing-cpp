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

#include "QRECB.h"
#include "QRErrorCorrectionLevel.h"

#include <array>
#include <initializer_list>
#include <vector>

namespace ZXing {

class BitMatrix;

namespace QRCode {

/**
* See ISO 18004:2006 Annex D
*
* @author Sean Owen
*/
class Version
{
public:
	int versionNumber() const { return _versionNumber; }

	const std::vector<int>& alignmentPatternCenters() const { return _alignmentPatternCenters; }

	int totalCodewords() const { return _totalCodewords; }

	int dimensionForVersion() const { return DimensionOfVersion(_versionNumber, _isMicro); }

	const ECBlocks& ecBlocksForLevel(ErrorCorrectionLevel ecLevel) const { return _ecBlocks[(int)ecLevel]; }

	BitMatrix buildFunctionPattern() const;

	bool isMicroQRCode() const { return _isMicro; }

	static constexpr int DimensionStep(bool isMicro) { return std::array{4, 2}[isMicro]; }
	static constexpr int DimensionOffset(bool isMicro) { return std::array{17, 9}[isMicro]; }
	static constexpr int DimensionOfVersion(int version, bool isMicro)
	{
		return DimensionOffset(isMicro) + DimensionStep(isMicro) * version;
	}

	/**
	* <p>Deduces version information purely from micro QR or QR Code dimensions.</p>
	*
	* @param dimension dimension in modules
	* @return Version for a QR Code of that dimension
	*/
	static const Version* ProvisionalVersionForDimension(int dimension, bool isMicro = false);
	
	static const Version* VersionForNumber(int versionNumber, bool isMicro = false);

	static const Version* DecodeVersionInformation(int versionBits);
private:
	int _versionNumber;
	std::vector<int> _alignmentPatternCenters;
	std::array<ECBlocks, 4> _ecBlocks;
	int _totalCodewords;
	bool _isMicro;

	Version(int versionNumber, std::initializer_list<int> alignmentPatternCenters, const std::array<ECBlocks, 4> &ecBlocks);
	Version(int versionNumber, const std::array<ECBlocks, 4>& ecBlocks);
	static const Version* AllVersions();
	static const Version* AllMicroVersions();
};

} // QRCode
} // ZXing
