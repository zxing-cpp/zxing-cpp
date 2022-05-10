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

#include "MQRBitMatrixParser.h"

#include "BitArray.h"
#include "BitMatrix.h"
#include "ByteArray.h"
#include "MQRDataMask.h"
#include "MQRFormatInformationFactory.h"
#include "QRErrorCorrectionLevel.h"
#include "QRFormatInformation.h"
#include "QRVersion.h"

#include <utility>

namespace ZXing::MicroQRCode {

static bool getBit(const BitMatrix& bitMatrix, int x, int y, bool mirrored)
{
	return mirrored ? bitMatrix.get(y, x) : bitMatrix.get(x, y);
}

static bool hasValidDimension(const BitMatrix& bitMatrix)
{
	int dimension = bitMatrix.height();
	return dimension >= 11 && dimension <= 17 && (dimension % 2) == 1;
}

const QRCode::Version* ReadVersion(const BitMatrix& bitMatrix)
{
	if (!hasValidDimension(bitMatrix))
		return nullptr;

	int dimension = bitMatrix.height();

	int provisionalVersion = (dimension - 9) / 2;
	return QRCode::Version::VersionForNumber(provisionalVersion, true);
}

QRCode::FormatInformation ReadFormatInformation(const BitMatrix& bitMatrix, bool mirrored)
{
	if (!hasValidDimension(bitMatrix))
		return {};

	// Read top-left format info bits
	int formatInfoBits = 0;
	for (int x = 1; x < 9; x++)
		AppendBit(formatInfoBits, getBit(bitMatrix, x, 8, mirrored));
	for (int y = 7; y >= 1; y--)
		AppendBit(formatInfoBits, getBit(bitMatrix, 8, y, mirrored));

	return DecodeFormatInformation(formatInfoBits);
}

ByteArray ReadCodewords(const BitMatrix& bitMatrix, const QRCode::Version& version,
						const QRCode::FormatInformation& formatInformation, bool mirrored)
{
	if (!hasValidDimension(bitMatrix))
		return {};

	BitMatrix functionPattern = version.buildFunctionPattern();

	// D3 in a Version M1 symbol, D11 in a Version M3-L symbol and D9
	// in a Version M3-M symbol is a 2x2 square 4-module block.
	// See ISO 18004:2006 6.7.3.
	bool hasD4mBlock = version.versionNumber() % 2 == 1;
	int d4mBlockIndex =
		version.versionNumber() == 1 ? 3 : (formatInformation.errorCorrectionLevel() == QRCode::ErrorCorrectionLevel::Low ? 11 : 9);

	ByteArray result;
	result.reserve(version.totalCodewords());
	uint8_t currentByte = 0;
	bool readingUp = true;
	int bitsRead = 0;
	int dimension = bitMatrix.height();
	// Read columns in pairs, from right to left
	for (int x = dimension - 1; x > 0; x -= 2) {
		// Read alternatingly from bottom to top then top to bottom
		for (int row = 0; row < dimension; row++) {
			int y = readingUp ? dimension - 1 - row : row;
			for (int col = 0; col < 2; col++) {
				int xx = x - col;
				// Ignore bits covered by the function pattern
				if (!functionPattern.get(xx, y)) {
					// Read a bit
					AppendBit(currentByte, GetDataMaskBit(formatInformation.dataMask(), xx, y) !=
											   getBit(bitMatrix, xx, y, mirrored));
					++bitsRead;
					// If we've made a whole byte, save it off; save early if 2x2 data block.
					if (bitsRead == 8 || (bitsRead == 4 && hasD4mBlock && Size(result) == d4mBlockIndex - 1)) {
						result.push_back(std::exchange(currentByte, 0));
						bitsRead = 0;
					}
				}
			}
		}
		readingUp = !readingUp; // switch directions
	}
	if (Size(result) != version.totalCodewords())
		return {};

	return result;
}

} // namespace ZXing::MicroQRCode
