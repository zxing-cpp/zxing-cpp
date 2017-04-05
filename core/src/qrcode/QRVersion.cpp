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

#include "qrcode/QRVersion.h"
#include "qrcode/QRECB.h"
#include "BitHacks.h"
#include "BitMatrix.h"

#include <limits>

namespace ZXing {
namespace QRCode {

namespace {

	/**
	* See ISO 18004:2006 Annex D.
	* Element i represents the raw version bits that specify version i + 7
	*/
	static const int VERSION_DECODE_INFO[] = {
		0x07C94, 0x085BC, 0x09A99, 0x0A4D3, 0x0BBF6,
		0x0C762, 0x0D847, 0x0E60D, 0x0F928, 0x10B78,
		0x1145D, 0x12A17, 0x13532, 0x149A6, 0x15683,
		0x168C9, 0x177EC, 0x18EC4, 0x191E1, 0x1AFAB,
		0x1B08E, 0x1CC1A, 0x1D33F, 0x1ED75, 0x1F250,
		0x209D5, 0x216F0, 0x228BA, 0x2379F, 0x24B0B,
		0x2542E, 0x26A64, 0x27541, 0x28C69
	};


} // anonymous

const Version *
Version::AllVersions()
{
	/**
	* See ISO 18004:2006 6.5.1 Table 9
	*/
	static const Version allVersions[] = {
		{1, {}, {
			7,  1, 19, 0, 0,
			10, 1, 16, 0, 0,
			13, 1, 13, 0, 0,
			17, 1, 9 , 0, 0
			}},
		{2, {6, 18}, {
			10, 1, 34, 0, 0,
			16, 1, 28, 0, 0,
			22, 1, 22, 0, 0,
			28, 1, 16, 0, 0,
			}},
		{3, {6, 22}, {
			15, 1, 55, 0, 0,
			26, 1, 44, 0, 0,
			18, 2, 17, 0, 0,
			22, 2, 13, 0, 0,
			}},
		{4, {6, 26}, {
			20, 1, 80, 0, 0,
			18, 2, 32, 0, 0,
			26, 2, 24, 0, 0,
			16, 4, 9 , 0, 0,
			}},
		{5, {6, 30}, {
			26, 1, 108, 0, 0,
			24, 2, 43 , 0, 0,
			18, 2, 15 , 2, 16,
			22, 2, 11 , 2, 12,
			}},
		{6, {6, 34}, {
			18, 2, 68, 0, 0,
			16, 4, 27, 0, 0,
			24, 4, 19, 0, 0,
			28, 4, 15, 0, 0,
			}},
		{7, {6, 22, 38}, {
			20, 2, 78, 0, 0,
			18, 4, 31, 0, 0,
			18, 2, 14, 4, 15,
			26, 4, 13, 1, 14,
			}},
		{8, {6, 24, 42}, {
			24, 2, 97, 0, 0,
			22, 2, 38, 2, 39,
			22, 4, 18, 2, 19,
			26, 4, 14, 2, 15,
			}},
		{9, {6, 26, 46}, {
			30, 2, 116, 0, 0,
			22, 3, 36, 2, 37,
			20, 4, 16, 4, 17,
			24, 4, 12, 4, 13,
			}},
		{10, {6, 28, 50}, {
			18, 2, 68, 2, 69,
			26, 4, 43, 1, 44,
			24, 6, 19, 2, 20,
			28, 6, 15, 2, 16,
			}},
		{11, {6, 30, 54}, {
			20, 4, 81, 0, 0,
			30, 1, 50, 4, 51,
			28, 4, 22, 4, 23,
			24, 3, 12, 8, 13,
			}},
		{12, {6, 32, 58}, {
			24, 2, 92, 2, 93,
			22, 6, 36, 2, 37,
			26, 4, 20, 6, 21,
			28, 7, 14, 4, 15,
			}},
		{13, {6, 34, 62}, {
			26, 4, 107, 0, 0,
			22, 8, 37, 1, 38,
			24, 8, 20, 4, 21,
			22, 12, 11, 4, 12,
			}},
		{14, {6, 26, 46, 66}, {
			30, 3, 115, 1, 116,
			24, 4, 40, 5, 41,
			20, 11, 16, 5, 17,
			24, 11, 12, 5, 13,
			}},
		{15, {6, 26, 48, 70}, {
			22, 5, 87, 1, 88,
			24, 5, 41, 5, 42,
			30, 5, 24, 7, 25,
			24, 11, 12, 7, 13,
			}},
		{16, {6, 26, 50, 74}, {
			24, 5, 98, 1, 99,
			28, 7, 45, 3, 46,
			24, 15, 19, 2, 20,
			30, 3, 15, 13, 16,
			}},
		{17, {6, 30, 54, 78}, {
			28, 1, 107, 5, 108,
			28, 10, 46, 1, 47,
			28, 1, 22, 15, 23,
			28, 2, 14, 17, 15,
			}},
		{18, {6, 30, 56, 82}, {
			30, 5, 120, 1, 121,
			26, 9, 43, 4, 44,
			28, 17, 22, 1, 23,
			28, 2, 14, 19, 15,
			}},
		{19, {6, 30, 58, 86}, {
			28, 3, 113, 4, 114,
			26, 3, 44, 11, 45,
			26, 17, 21, 4, 22,
			26, 9, 13, 16, 14,
			}},
		{20, {6, 34, 62, 90}, {
			28, 3, 107, 5, 108,
			26, 3, 41, 13, 42,
			30, 15, 24, 5, 25,
			28, 15, 15, 10, 16,
			}},
		{21, {6, 28, 50, 72, 94}, {
			28, 4, 116, 4, 117,
			26, 17, 42, 0, 0,
			28, 17, 22, 6, 23,
			30, 19, 16, 6, 17,
			}},
		{22, {6, 26, 50, 74, 98}, {
			28, 2, 111, 7, 112,
			28, 17, 46, 0, 0,
			30, 7, 24, 16, 25,
			24, 34, 13, 0, 0,
			}},
		{23, {6, 30, 54, 78, 102}, {
			30, 4, 121, 5, 122,
			28, 4, 47, 14, 48,
			30, 11, 24, 14, 25,
			30, 16, 15, 14, 16,
			}},
		{24, {6, 28, 54, 80, 106}, {
			30, 6, 117, 4, 118,
			28, 6, 45, 14, 46,
			30, 11, 24, 16, 25,
			30, 30, 16, 2, 17,
			}},
		{25, {6, 32, 58, 84, 110}, {
			26, 8, 106, 4, 107,
			28, 8, 47, 13, 48,
			30, 7, 24, 22, 25,
			30, 22, 15, 13, 16,
			}},
		{26, {6, 30, 58, 86, 114}, {
			28, 10, 114, 2, 115,
			28, 19, 46, 4, 47,
			28, 28, 22, 6, 23,
			30, 33, 16, 4, 17,
			}},
		{27, {6, 34, 62, 90, 118}, {
			30, 8, 122, 4, 123,
			28, 22, 45, 3, 46,
			30, 8, 23, 26, 24,
			30, 12, 15, 28, 16,
			}},
		{28, {6, 26, 50, 74, 98, 122}, {
			30, 3, 117, 10, 118,
			28, 3, 45, 23, 46,
			30, 4, 24, 31, 25,
			30, 11, 15, 31, 16,
			}},
		{29, {6, 30, 54, 78, 102, 126}, {
			30, 7, 116, 7, 117,
			28, 21, 45, 7, 46,
			30, 1, 23, 37, 24,
			30, 19, 15, 26, 16,
			}},
		{30, {6, 26, 52, 78, 104, 130}, {
			30, 5, 115, 10, 116,
			28, 19, 47, 10, 48,
			30, 15, 24, 25, 25,
			30, 23, 15, 25, 16,
			}},
		{31, {6, 30, 56, 82, 108, 134}, {
			30, 13, 115, 3, 116,
			28, 2, 46, 29, 47,
			30, 42, 24, 1, 25,
			30, 23, 15, 28, 16,
			}},
		{32, {6, 34, 60, 86, 112, 138}, {
			30, 17, 115, 0, 0,
			28, 10, 46, 23, 47,
			30, 10, 24, 35, 25,
			30, 19, 15, 35, 16,
			}},
		{33, {6, 30, 58, 86, 114, 142}, {
			30, 17, 115, 1, 116,
			28, 14, 46, 21, 47,
			30, 29, 24, 19, 25,
			30, 11, 15, 46, 16,
			}},
		{34, {6, 34, 62, 90, 118, 146}, {
			30, 13, 115, 6, 116,
			28, 14, 46, 23, 47,
			30, 44, 24, 7, 25,
			30, 59, 16, 1, 17,
			}},
		{35, {6, 30, 54, 78, 102, 126, 150}, {
			30, 12, 121, 7, 122,
			28, 12, 47, 26, 48,
			30, 39, 24, 14, 25,
			30, 22, 15, 41, 16,
			}},
		{36, {6, 24, 50, 76, 102, 128, 154}, {
			30, 6, 121, 14, 122,
			28, 6, 47, 34, 48,
			30, 46, 24, 10, 25,
			30, 2, 15, 64, 16,
			}},
		{37, {6, 28, 54, 80, 106, 132, 158}, {
			30, 17, 122, 4, 123,
			28, 29, 46, 14, 47,
			30, 49, 24, 10, 25,
			30, 24, 15, 46, 16,
			}},
		{38, {6, 32, 58, 84, 110, 136, 162}, {
			30, 4, 122, 18, 123,
			28, 13, 46, 32, 47,
			30, 48, 24, 14, 25,
			30, 42, 15, 32, 16,
			}},
		{39, {6, 26, 54, 82, 110, 138, 166}, {
			30, 20, 117, 4, 118,
			28, 40, 47, 7, 48,
			30, 43, 24, 22, 25,
			30, 10, 15, 67, 16,
			}},
		{40, {6, 30, 58, 86, 114, 142, 170}, {
			30, 19, 118, 6, 119,
			28, 18, 47, 31, 48,
			30, 34, 24, 34, 25,
			30, 20, 15, 61, 16
			}},
	};
	return allVersions;
}

Version::Version(int versionNumber, std::initializer_list<int> alignmentPatternCenters, const std::array<ECBlocks, 4> &ecBlocks) :
	_versionNumber(versionNumber),
	_alignmentPatternCenters(alignmentPatternCenters),
	_ecBlocks(ecBlocks)
{
	_totalCodewords = ecBlocks[0].totalDataCodewords();
}

const Version *
Version::VersionForNumber(int versionNumber)
{
	if (versionNumber < 1 || versionNumber > 40) {
		//throw std::invalid_argument("Version should be in range [1-40].");
		return nullptr;
	}
	return &AllVersions()[versionNumber - 1];
}

const Version *
Version::ProvisionalVersionForDimension(int dimension)
{
	if (dimension % 4 != 1) {
		//throw std::invalid_argument("Unexpected dimension");
		return nullptr;
	}
	return VersionForNumber((dimension - 17) / 4);
}


const Version *
Version::DecodeVersionInformation(int versionBits)
{
	int bestDifference = std::numeric_limits<int>::max();
	int bestVersion = 0;
	int i = 0;
	for (int targetVersion : VERSION_DECODE_INFO) {
		// Do the version info bits match exactly? done.
		if (targetVersion == versionBits) {
			return VersionForNumber(i + 7);
		}
		// Otherwise see if this is the closest to a real version info bit string
		// we have seen so far
		int bitsDifference = BitHacks::CountBitsSet(versionBits ^ targetVersion);
		if (bitsDifference < bestDifference) {
			bestVersion = i + 7;
			bestDifference = bitsDifference;
		}
		++i;
	}
	// We can tolerate up to 3 bits of error since no two version info codewords will
	// differ in less than 8 bits.
	if (bestDifference <= 3) {
		return VersionForNumber(bestVersion);
	}
	// If we didn't find a close enough match, fail
	return nullptr;
}

/**
* See ISO 18004:2006 Annex E
*/
void
Version::buildFunctionPattern(BitMatrix& bitMatrix) const
{
	int dimension = dimensionForVersion();
	bitMatrix = BitMatrix(dimension, dimension);

	// Top left finder pattern + separator + format
	bitMatrix.setRegion(0, 0, 9, 9);
	// Top right finder pattern + separator + format
	bitMatrix.setRegion(dimension - 8, 0, 8, 9);
	// Bottom left finder pattern + separator + format
	bitMatrix.setRegion(0, dimension - 8, 9, 8);

	// Alignment patterns
	size_t max = _alignmentPatternCenters.size();
	for (size_t x = 0; x < max; ++x) {
		int i = _alignmentPatternCenters[x] - 2;
		for (size_t y = 0; y < max; ++y) {
			if ((x == 0 && (y == 0 || y == max - 1)) || (x == max - 1 && y == 0)) {
				// No alignment patterns near the three finder paterns
				continue;
			}
			bitMatrix.setRegion(_alignmentPatternCenters[y] - 2, i, 5, 5);
		}
	}

	// Vertical timing pattern
	bitMatrix.setRegion(6, 9, 1, dimension - 17);
	// Horizontal timing pattern
	bitMatrix.setRegion(9, 6, dimension - 17, 1);

	if (_versionNumber > 6) {
		// Version info, top right
		bitMatrix.setRegion(dimension - 11, 0, 3, 6);
		// Version info, bottom left
		bitMatrix.setRegion(0, dimension - 11, 6, 3);
	}
}

} // QRCode
} // ZXing
