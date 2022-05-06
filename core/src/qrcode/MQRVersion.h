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

#include "QRErrorCorrectionLevel.h"
#include "qrcode/QRECB.h"

#include <array>

namespace ZXing {

class BitMatrix;

namespace MicroQRCode {

/**
 * See ISO 18004:2006 Annex D
 *
 * @author Sean Owen
 */
class Version
{
public:
	int versionNumber() const { return _versionNumber; }

	int totalCodewords() const { return _totalCodewords; }

	int dimensionForVersion() const { return 9 + 2 * _versionNumber; }

	const QRCode::ECBlocks& ecBlocksForLevel(QRCode::ErrorCorrectionLevel ecLevel) const
	{
		return _ecBlocks[(int)ecLevel];
	}

	BitMatrix buildFunctionPattern() const;

	/**
	 * <p>Deduces version information purely from micro QR Code dimensions.</p>
	 *
	 * @param dimension dimension in modules
	 * @return Version for a micro QR Code of that dimension
	 */
	static const Version* ProvisionalVersionForDimension(int dimension);

	static const Version* VersionForNumber(int versionNumber);

private:
	int _versionNumber;
	std::array<QRCode::ECBlocks, 4> _ecBlocks;
	int _totalCodewords;

	Version(int versionNumber, const std::array<QRCode::ECBlocks, 4>& ecBlocks);
	static const Version* AllVersions();
};

} // namespace MicroQRCode

} // namespace ZXing
