/*
* Copyright 2016 Huy Cuong Nguyen
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

#include "qrcode/QRMatrixUtil.h"
#include "qrcode/QRErrorCorrectionLevel.h"
#include "qrcode/QRVersion.h"
#include "BitArray.h"
#include "ByteMatrix.h"
#include "BitHacks.h"
#include "ZXStrConvWorkaround.h"

#include <array>
#include <string>

namespace ZXing {
namespace QRCode {

static const std::array<std::array<int8_t, 7>, 7> POSITION_DETECTION_PATTERN = {
	1, 1, 1, 1, 1, 1, 1,
	1, 0, 0, 0, 0, 0, 1,
	1, 0, 1, 1, 1, 0, 1,
	1, 0, 1, 1, 1, 0, 1,
	1, 0, 1, 1, 1, 0, 1,
	1, 0, 0, 0, 0, 0, 1,
	1, 1, 1, 1, 1, 1, 1,
};

static const std::array<std::array<int8_t, 5>, 5> POSITION_ADJUSTMENT_PATTERN = {
	1, 1, 1, 1, 1,
	1, 0, 0, 0, 1,
	1, 0, 1, 0, 1,
	1, 0, 0, 0, 1,
	1, 1, 1, 1, 1,
};

// From Appendix E. Table 1, JIS0510X:2004 (p 71). The table was double-checked by komatsu.
static const std::array<std::array<int16_t, 7>, 40> POSITION_ADJUSTMENT_PATTERN_COORDINATE_TABLE = {
   -1, -1, -1, -1,  -1,  -1,  -1,  // Version 1
	6, 18, -1, -1,  -1,  -1,  -1,  // Version 2
	6, 22, -1, -1,  -1,  -1,  -1,  // Version 3
	6, 26, -1, -1,  -1,  -1,  -1,  // Version 4
	6, 30, -1, -1,  -1,  -1,  -1,  // Version 5
	6, 34, -1, -1,  -1,  -1,  -1,  // Version 6
	6, 22, 38, -1,  -1,  -1,  -1,  // Version 7
	6, 24, 42, -1,  -1,  -1,  -1,  // Version 8
	6, 26, 46, -1,  -1,  -1,  -1,  // Version 9
	6, 28, 50, -1,  -1,  -1,  -1,  // Version 10
	6, 30, 54, -1,  -1,  -1,  -1,  // Version 11
	6, 32, 58, -1,  -1,  -1,  -1,  // Version 12
	6, 34, 62, -1,  -1,  -1,  -1,  // Version 13
	6, 26, 46, 66,  -1,  -1,  -1,  // Version 14
	6, 26, 48, 70,  -1,  -1,  -1,  // Version 15
	6, 26, 50, 74,  -1,  -1,  -1,  // Version 16
	6, 30, 54, 78,  -1,  -1,  -1,  // Version 17
	6, 30, 56, 82,  -1,  -1,  -1,  // Version 18
	6, 30, 58, 86,  -1,  -1,  -1,  // Version 19
	6, 34, 62, 90,  -1,  -1,  -1,  // Version 20
	6, 28, 50, 72,  94,  -1,  -1,  // Version 21
	6, 26, 50, 74,  98,  -1,  -1,  // Version 22
	6, 30, 54, 78, 102,  -1,  -1,  // Version 23
	6, 28, 54, 80, 106,  -1,  -1,  // Version 24
	6, 32, 58, 84, 110,  -1,  -1,  // Version 25
	6, 30, 58, 86, 114,  -1,  -1,  // Version 26
	6, 34, 62, 90, 118,  -1,  -1,  // Version 27
	6, 26, 50, 74,  98, 122,  -1,  // Version 28
	6, 30, 54, 78, 102, 126,  -1,  // Version 29
	6, 26, 52, 78, 104, 130,  -1,  // Version 30
	6, 30, 56, 82, 108, 134,  -1,  // Version 31
	6, 34, 60, 86, 112, 138,  -1,  // Version 32
	6, 30, 58, 86, 114, 142,  -1,  // Version 33
	6, 34, 62, 90, 118, 146,  -1,  // Version 34
	6, 30, 54, 78, 102, 126, 150,  // Version 35
	6, 24, 50, 76, 102, 128, 154,  // Version 36
	6, 28, 54, 80, 106, 132, 158,  // Version 37
	6, 32, 58, 84, 110, 136, 162,  // Version 38
	6, 26, 54, 82, 110, 138, 166,  // Version 39
	6, 30, 58, 86, 114, 142, 170,  // Version 40
};

// Type info cells at the left top corner.
static const std::array<std::array<int8_t, 2>, 15> TYPE_INFO_COORDINATES = {
	8, 0,
	8, 1,
	8, 2,
	8, 3,
	8, 4,
	8, 5,
	8, 7,
	8, 8,
	7, 8,
	5, 8,
	4, 8,
	3, 8,
	2, 8,
	1, 8,
	0, 8,
};

// From Appendix D in JISX0510:2004 (p. 67)
static const int VERSION_INFO_POLY = 0x1f25;  // 1 1111 0010 0101

													  // From Appendix C in JISX0510:2004 (p.65).
static const int TYPE_INFO_POLY = 0x537;
static const int TYPE_INFO_MASK_PATTERN = 0x5412;

// Set all cells to -1.  -1 means that the cell is empty (not set yet).
//
// JAVAPORT: We shouldn't need to do this at all. The code should be rewritten to begin encoding
// with the ByteMatrix initialized all to zero.
static void makeEmpty(ByteMatrix& matrix)
{
	matrix.clear(-1);
}

// Check if "value" is empty.
static inline bool IsEmpty(int8_t value) {
	return value == -1;
}

static void EmbedTimingPatterns(ByteMatrix& matrix)
{
	// -8 is for skipping position detection patterns (size 7), and two horizontal/vertical
	// separation patterns (size 1). Thus, 8 = 7 + 1.
	for (int i = 8; i < matrix.width() - 8; ++i) {
		int bit = (i + 1) % 2;
		// Horizontal line.
		if (IsEmpty(matrix.get(i, 6))) {
			matrix.set(i, 6, bit);
		}
		// Vertical line.
		if (IsEmpty(matrix.get(6, i))) {
			matrix.set(6, i, bit);
		}
	}
}


// Note that we cannot unify the function with embedPositionDetectionPattern() despite they are
// almost identical, since we cannot write a function that takes 2D arrays in different sizes in
// C/C++. We should live with the fact.
static void EmbedPositionAdjustmentPattern(int xStart, int yStart, ByteMatrix& matrix)
{
	for (int y = 0; y < 5; ++y) {
		for (int x = 0; x < 5; ++x) {
			matrix.set(xStart + x, yStart + y, POSITION_ADJUSTMENT_PATTERN[y][x]);
		}
	}
}

// Embed position adjustment patterns if need be.
static void MaybeEmbedPositionAdjustmentPatterns(const Version& version, ByteMatrix& matrix)
{
	if (version.versionNumber() < 2) {  // The patterns appear if version >= 2
		return;
	}
	int index = version.versionNumber() - 1;
	auto& coordinates = POSITION_ADJUSTMENT_PATTERN_COORDINATE_TABLE[index];
	int numCoordinates = static_cast<int>(coordinates.size());
	for (int i = 0; i < numCoordinates; ++i) {
		for (int j = 0; j < numCoordinates; ++j) {
			int y = coordinates[i];
			int x = coordinates[j];
			if (x == -1 || y == -1) {
				continue;
			}
			// If the cell is unset, we embed the position adjustment pattern here.
			if (IsEmpty(matrix.get(x, y))) {
				// -2 is necessary since the x/y coordinates point to the center of the pattern, not the
				// left top corner.
				EmbedPositionAdjustmentPattern(x - 2, y - 2, matrix);
			}
		}
	}
}

static void EmbedPositionDetectionPattern(int xStart, int yStart, ByteMatrix& matrix)
{
	for (int y = 0; y < 7; ++y) {
		for (int x = 0; x < 7; ++x) {
			matrix.set(xStart + x, yStart + y, POSITION_DETECTION_PATTERN[y][x]);
		}
	}
}


static void EmbedHorizontalSeparationPattern(int xStart, int yStart, ByteMatrix& matrix) {
	for (int x = 0; x < 8; ++x) {
		if (!IsEmpty(matrix.get(xStart + x, yStart))) {
			throw std::invalid_argument("Unexpected input");
		}
		matrix.set(xStart + x, yStart, 0);
	}
}

static void EmbedVerticalSeparationPattern(int xStart, int yStart, ByteMatrix& matrix)
{
	for (int y = 0; y < 7; ++y) {
		if (!IsEmpty(matrix.get(xStart, yStart + y))) {
			throw std::invalid_argument("Unexpected input");
		}
		matrix.set(xStart, yStart + y, 0);
	}
}

// Embed position detection patterns and surrounding vertical/horizontal separators.
static void EmbedPositionDetectionPatternsAndSeparators(ByteMatrix& matrix)
{
	// Embed three big squares at corners.
	int pdpWidth = static_cast<int>(POSITION_DETECTION_PATTERN[0].size());
	// Left top corner.
	EmbedPositionDetectionPattern(0, 0, matrix);
	// Right top corner.
	EmbedPositionDetectionPattern(matrix.width() - pdpWidth, 0, matrix);
	// Left bottom corner.
	EmbedPositionDetectionPattern(0, matrix.width() - pdpWidth, matrix);

	// Embed horizontal separation patterns around the squares.
	int hspWidth = 8;
	// Left top corner.
	EmbedHorizontalSeparationPattern(0, hspWidth - 1, matrix);
	// Right top corner.
	EmbedHorizontalSeparationPattern(matrix.width() - hspWidth, hspWidth - 1, matrix);
	// Left bottom corner.
	EmbedHorizontalSeparationPattern(0, matrix.width() - hspWidth, matrix);

	// Embed vertical separation patterns around the squares.
	int vspSize = 7;
	// Left top corner.
	EmbedVerticalSeparationPattern(vspSize, 0, matrix);
	// Right top corner.
	EmbedVerticalSeparationPattern(matrix.height() - vspSize - 1, 0, matrix);
	// Left bottom corner.
	EmbedVerticalSeparationPattern(vspSize, matrix.height() - vspSize, matrix);
}


// Embed the lonely dark dot at left bottom corner. JISX0510:2004 (p.46)
static void EmbedDarkDotAtLeftBottomCorner(ByteMatrix& matrix)
{
	if (matrix.get(8, matrix.height() - 8) == 0) {
		throw std::invalid_argument("Unexpected input");
	}
	matrix.set(8, matrix.height() - 8, 1);
}

// Embed basic patterns. On success, modify the matrix and return true.
// The basic patterns are:
// - Position detection patterns
// - Timing patterns
// - Dark dot at the left bottom corner
// - Position adjustment patterns, if need be
static void EmbedBasicPatterns(const Version& version, ByteMatrix& matrix)
{
	// Let's get started with embedding big squares at corners.
	EmbedPositionDetectionPatternsAndSeparators(matrix);
	// Then, embed the dark dot at the left bottom corner.
	EmbedDarkDotAtLeftBottomCorner(matrix);

	// Position adjustment patterns appear if version >= 2.
	MaybeEmbedPositionAdjustmentPatterns(version, matrix);
	// Timing patterns should be embedded after position adj. patterns.
	EmbedTimingPatterns(matrix);
}


// Return the position of the most significant bit set (to one) in the "value". The most
// significant bit is position 32. If there is no bit set, return 0. Examples:
// - findMSBSet(0) => 0
// - findMSBSet(1) => 1
// - findMSBSet(255) => 8
static int FindMSBSet(unsigned value)
{
	return 32 - BitHacks::NumberOfLeadingZeros(value);
}

// Calculate BCH (Bose-Chaudhuri-Hocquenghem) code for "value" using polynomial "poly". The BCH
// code is used for encoding type information and version information.
// Example: Calculation of version information of 7.
// f(x) is created from 7.
//   - 7 = 000111 in 6 bits
//   - f(x) = x^2 + x^1 + x^0
// g(x) is given by the standard (p. 67)
//   - g(x) = x^12 + x^11 + x^10 + x^9 + x^8 + x^5 + x^2 + 1
// Multiply f(x) by x^(18 - 6)
//   - f'(x) = f(x) * x^(18 - 6)
//   - f'(x) = x^14 + x^13 + x^12
// Calculate the remainder of f'(x) / g(x)
//         x^2
//         __________________________________________________
//   g(x) )x^14 + x^13 + x^12
//         x^14 + x^13 + x^12 + x^11 + x^10 + x^7 + x^4 + x^2
//         --------------------------------------------------
//                              x^11 + x^10 + x^7 + x^4 + x^2
//
// The remainder is x^11 + x^10 + x^7 + x^4 + x^2
// Encode it in binary: 110010010100
// The return value is 0xc94 (1100 1001 0100)
//
// Since all coefficients in the polynomials are 1 or 0, we can do the calculation by bit
// operations. We don't care if cofficients are positive or negative.
static int CalculateBCHCode(int value, int poly) {
	// If poly is "1 1111 0010 0101" (version info poly), msbSetInPoly is 13. We'll subtract 1
	// from 13 to make it 12.
	int msbSetInPoly = FindMSBSet(poly);
	value <<= msbSetInPoly - 1;
	// Do the division business using exclusive-or operations.
	while (FindMSBSet(value) >= msbSetInPoly) {
		value ^= poly << (FindMSBSet(value) - msbSetInPoly);
	}
	// Now the "value" is the remainder (i.e. the BCH code)
	return value;
}

// Make bit vector of type information. On success, store the result in "bits" and return true.
// Encode error correction level and mask pattern. See 8.9 of
// JISX0510:2004 (p.45) for details.
static void MakeTypeInfoBits(ErrorCorrectionLevel ecLevel, int maskPattern, BitArray& bits)
{
	if (maskPattern < 0 || maskPattern >= MatrixUtil::NUM_MASK_PATTERNS) {
		throw std::invalid_argument("Invalid mask pattern");
	}

	int typeInfo = (BitsFromECLevel(ecLevel) << 3) | maskPattern;
	bits.appendBits(typeInfo, 5);

	int bchCode = CalculateBCHCode(typeInfo, TYPE_INFO_POLY);
	bits.appendBits(bchCode, 10);

	BitArray maskBits;
	maskBits.appendBits(TYPE_INFO_MASK_PATTERN, 15);
	bits.bitwiseXOR(maskBits);

	if (bits.size() != 15) {  // Just in case.
		throw std::logic_error("Should not happen but we got: " + std::to_string(bits.size()));
	}
}

// Embed type information. On success, modify the matrix.
static void EmbedTypeInfo(ErrorCorrectionLevel ecLevel, int maskPattern, ByteMatrix& matrix)
{
	BitArray typeInfoBits;
	MakeTypeInfoBits(ecLevel, maskPattern, typeInfoBits);

	for (int i = 0; i < typeInfoBits.size(); ++i) {
		// Place bits in LSB to MSB order.  LSB (least significant bit) is the last value in
		// "typeInfoBits".
		bool bit = typeInfoBits.get(typeInfoBits.size() - 1 - i);

		// Type info bits at the left top corner. See 8.9 of JISX0510:2004 (p.46).
		int x1 = TYPE_INFO_COORDINATES[i][0];
		int y1 = TYPE_INFO_COORDINATES[i][1];
		matrix.set(x1, y1, bit);

		if (i < 8) {
			// Right top corner.
			int x2 = matrix.width() - i - 1;
			int y2 = 8;
			matrix.set(x2, y2, bit);
		}
		else {
			// Left bottom corner.
			int x2 = 8;
			int y2 = matrix.height() - 7 + (i - 8);
			matrix.set(x2, y2, bit);
		}
	}
}

// Make bit vector of version information. On success, store the result in "bits" and return true.
// See 8.10 of JISX0510:2004 (p.45) for details.
static void MakeVersionInfoBits(const Version& version, BitArray& bits)
{
	bits.appendBits(version.versionNumber(), 6);
	int bchCode = CalculateBCHCode(version.versionNumber(), VERSION_INFO_POLY);
	bits.appendBits(bchCode, 12);

	if (bits.size() != 18) {  // Just in case.
		throw std::logic_error("Should not happen but we got: " + std::to_string(bits.size()));
	}
}

// Embed version information if need be. On success, modify the matrix and return true.
// See 8.10 of JISX0510:2004 (p.47) for how to embed version information.
static void MaybeEmbedVersionInfo(const Version& version, ByteMatrix& matrix)
{
	if (version.versionNumber() < 7) {  // Version info is necessary if version >= 7.
		return;  // Don't need version info.
	}

	BitArray versionInfoBits;
	MakeVersionInfoBits(version, versionInfoBits);

	int bitIndex = 6 * 3 - 1;  // It will decrease from 17 to 0.
	for (int i = 0; i < 6; ++i) {
		for (int j = 0; j < 3; ++j) {
			// Place bits in LSB (least significant bit) to MSB order.
			bool bit = versionInfoBits.get(bitIndex);
			bitIndex--;
			// Left bottom corner.
			matrix.set(i, matrix.height() - 11 + j, bit);
			// Right bottom corner.
			matrix.set(matrix.height() - 11 + j, i, bit);
		}
	}
}

/**
* Return the mask bit for "getMaskPattern" at "x" and "y". See 8.8 of JISX0510:2004 for mask
* pattern conditions.
*/
static bool GetDataMaskBit(int maskPattern, int x, int y)
{
	int intermediate;
	int temp;
	switch (maskPattern) {
	case 0:
		intermediate = (y + x) & 0x1;
		break;
	case 1:
		intermediate = y & 0x1;
		break;
	case 2:
		intermediate = x % 3;
		break;
	case 3:
		intermediate = (y + x) % 3;
		break;
	case 4:
		intermediate = ((y / 2) + (x / 3)) & 0x1;
		break;
	case 5:
		temp = y * x;
		intermediate = (temp & 0x1) + (temp % 3);
		break;
	case 6:
		temp = y * x;
		intermediate = ((temp & 0x1) + (temp % 3)) & 0x1;
		break;
	case 7:
		temp = y * x;
		intermediate = ((temp % 3) + ((y + x) & 0x1)) & 0x1;
		break;
	default:
		throw std::invalid_argument("Invalid mask pattern: " + std::to_string(maskPattern));
	}
	return intermediate == 0;
}

// Embed "dataBits" using "getMaskPattern". On success, modify the matrix and return true.
// For debugging purposes, it skips masking process if "getMaskPattern" is -1.
// See 8.7 of JISX0510:2004 (p.38) for how to embed data bits.
static void EmbedDataBits(const BitArray& dataBits, int maskPattern, ByteMatrix& matrix)
{
	int bitIndex = 0;
	int direction = -1;
	// Start from the right bottom cell.
	int x = matrix.width() - 1;
	int y = matrix.height() - 1;
	while (x > 0) {
		// Skip the vertical timing pattern.
		if (x == 6) {
			x -= 1;
		}
		while (y >= 0 && y < matrix.height()) {
			for (int i = 0; i < 2; ++i) {
				int xx = x - i;
				// Skip the cell if it's not empty.
				if (!IsEmpty(matrix.get(xx, y))) {
					continue;
				}
				bool bit;
				if (bitIndex < dataBits.size()) {
					bit = dataBits.get(bitIndex);
					++bitIndex;
				}
				else {
					// Padding bit. If there is no bit left, we'll fill the left cells with 0, as described
					// in 8.4.9 of JISX0510:2004 (p. 24).
					bit = false;
				}

				// Skip masking if mask_pattern is -1.
				if (maskPattern != -1 && GetDataMaskBit(maskPattern, xx, y)) {
					bit = !bit;
				}
				matrix.set(xx, y, bit);
			}
			y += direction;
		}
		direction = -direction;  // Reverse the direction.
		y += direction;
		x -= 2;  // Move to the left.
	}
	// All bits should be consumed.
	if (bitIndex != dataBits.size()) {
		throw std::invalid_argument("Not all bits consumed: " + std::to_string(bitIndex) + '/' + std::to_string(dataBits.size()));
	}
}

// Build 2D matrix of QR Code from "dataBits" with "ecLevel", "version" and "getMaskPattern". On
// success, store the result in "matrix" and return true.
void
MatrixUtil::BuildMatrix(const BitArray& dataBits, ErrorCorrectionLevel ecLevel, const Version& version, int maskPattern, ByteMatrix& matrix)
{
	makeEmpty(matrix);
	EmbedBasicPatterns(version, matrix);
	// Type information appear with any version.
	EmbedTypeInfo(ecLevel, maskPattern, matrix);
	// Version info appear if version >= 7.
	MaybeEmbedVersionInfo(version, matrix);
	// Data should be embedded at end.
	EmbedDataBits(dataBits, maskPattern, matrix);
}

} // QRCode
} // ZXing
