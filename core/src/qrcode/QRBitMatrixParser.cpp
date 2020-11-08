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

#include "QRBitMatrixParser.h"

#include "BitArray.h"
#include "BitMatrix.h"
#include "ByteArray.h"
#include "QRDataMask.h"
#include "QRFormatInformation.h"
#include "QRVersion.h"

namespace ZXing::QRCode {

static bool getBit(const BitMatrix& bitMatrix, int x, int y, bool mirrored)
{
	return mirrored ? bitMatrix.get(y, x) : bitMatrix.get(x, y);
}

static bool hasValidDimension(const BitMatrix& bitMatrix)
{
	int dimension = bitMatrix.height();
	return dimension >= 21 && dimension <= 177 && (dimension % 4) == 1;
}

const Version* ReadVersion(const BitMatrix& bitMatrix)
{
	if (!hasValidDimension(bitMatrix))
		return nullptr;

	int dimension = bitMatrix.height();

	int provisionalVersion = (dimension - 17) / 4;
	if (provisionalVersion <= 6)
		return Version::VersionForNumber(provisionalVersion);

	for (bool mirror : {false, true}) {
		// Read top-right/bottom-left version info: 3 wide by 6 tall (depending on mirrored)
		int versionBits = 0;
		for (int y = 5; y >= 0; --y)
			for (int x = dimension - 9; x >= dimension - 11; --x)
				AppendBit(versionBits, getBit(bitMatrix, x, y, mirror));

		auto theParsedVersion = Version::DecodeVersionInformation(versionBits);
		// TODO: why care for the contents of the version bits if we know the dimension already?
		if (theParsedVersion != nullptr && theParsedVersion->dimensionForVersion() == dimension)
			return theParsedVersion;
	}

	return nullptr;
}

FormatInformation ReadFormatInformation(const BitMatrix& bitMatrix, bool mirrored)
{
	if (!hasValidDimension(bitMatrix))
		return {};

	// Read top-left format info bits
	int formatInfoBits1 = 0;
	for (int x = 0; x < 6; x++)
		AppendBit(formatInfoBits1, getBit(bitMatrix, x, 8, mirrored));
	// .. and skip a bit in the timing pattern ...
	AppendBit(formatInfoBits1, getBit(bitMatrix, 7, 8, mirrored));
	AppendBit(formatInfoBits1, getBit(bitMatrix, 8, 8, mirrored));
	AppendBit(formatInfoBits1, getBit(bitMatrix, 8, 7, mirrored));
	// .. and skip a bit in the timing pattern ...
	for (int y = 5; y >= 0; y--)
		AppendBit(formatInfoBits1, getBit(bitMatrix, 8, y, mirrored));

	// Read the top-right/bottom-left pattern too
	int dimension = bitMatrix.height();
	int formatInfoBits2 = 0;
	for (int y = dimension - 1; y >= dimension - 7; y--)
		AppendBit(formatInfoBits2, getBit(bitMatrix, 8, y, mirrored));
	for (int x = dimension - 8; x < dimension; x++)
		AppendBit(formatInfoBits2, getBit(bitMatrix, x, 8, mirrored));

	return FormatInformation::DecodeFormatInformation(formatInfoBits1, formatInfoBits2);
}

ByteArray ReadCodewords(const BitMatrix& bitMatrix, const Version& version, int maskIndex, bool mirrored)
{
	if (!hasValidDimension(bitMatrix))
		return {};

	BitMatrix functionPattern = version.buildFunctionPattern();

	ByteArray result;
	result.reserve(version.totalCodewords());
	uint8_t currentByte = 0;
	bool readingUp = true;
	int bitsRead = 0;
	int dimension = bitMatrix.height();
	// Read columns in pairs, from right to left
	for (int x = dimension - 1; x > 0; x -= 2) {
		// Skip whole column with vertical timing pattern.
		if (x == 6)
			x--;
		// Read alternatingly from bottom to top then top to bottom
		for (int row = 0; row < dimension; row++) {
			int y = readingUp ? dimension - 1 - row : row;
			for (int col = 0; col < 2; col++) {
				int xx = x - col;
				// Ignore bits covered by the function pattern
				if (!functionPattern.get(xx, y)) {
					// Read a bit
					AppendBit(currentByte, GetDataMaskBit(maskIndex, xx, y) != getBit(bitMatrix, xx, y, mirrored));
					// If we've made a whole byte, save it off
					if (++bitsRead % 8 == 0)
						result.push_back(std::exchange(currentByte, 0));
				}
			}
		}
		readingUp = !readingUp; // switch directions
	}
	if (Size(result) != version.totalCodewords())
		return {};

	return result;
}

} // namespace ZXing::QRCode
