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

#include "oned/ODUPCEReader.h"
#include "oned/ODUPCEANCommon.h"
#include "BarcodeFormat.h"
#include "BitArray.h"
#include "DecodeStatus.h"
#include "ZXContainerAlgorithms.h"

#include <array>

namespace ZXing {
namespace OneD {

/**
* The pattern that marks the middle, and end, of a UPC-E pattern.
* There is no "second half" to a UPC-E barcode.
*/
static const std::array<int, 6> MIDDLE_END_PATTERN = { 1, 1, 1, 1, 1, 1 };

/**
* See {@link #L_AND_G_PATTERNS}; these values similarly represent patterns of
* even-odd parity encodings of digits that imply both the number system (0 or 1)
* used (index / 10), and the check digit (index % 10).
*/
static const std::array<int, 20> NUMSYS_AND_CHECK_DIGIT_PATTERNS = {
	0x38, 0x34, 0x32, 0x31, 0x2C, 0x26, 0x23, 0x2A, 0x29, 0x25,
	0x07, 0x0B, 0x0D, 0x0E, 0x13, 0x19, 0x1C, 0x15, 0x16, 0x1A,
};

BarcodeFormat
UPCEReader::expectedFormat() const
{
	return BarcodeFormat::UPC_E;
}

BitArray::Range
UPCEReader::decodeMiddle(const BitArray& row, BitArray::Iterator begin, std::string& resultString) const
{
	BitArray::Range next = {begin, row.end()};
	const BitArray::Range notFound = {begin, begin};
	int lgPatternFound = 0;

	for (int x = 0; x < 6; x++) {
		int bestMatch = DecodeDigit(&next, UPCEANCommon::L_AND_G_PATTERNS, &resultString);
		if (bestMatch == -1)
			return notFound;

		if (bestMatch >= 10) {
			lgPatternFound |= 1 << (5 - x);
		}
	}

	int i = IndexOf(NUMSYS_AND_CHECK_DIGIT_PATTERNS, lgPatternFound);
	if (i == -1)
		return notFound;

	resultString = std::to_string(i/10) + resultString + std::to_string(i % 10);
	return {begin, next.begin};
}

DecodeStatus
UPCEReader::checkChecksum(const std::string& s) const
{
	return UPCEANReader::checkChecksum(UPCEANCommon::ConvertUPCEtoUPCA(s));
}

BitArray::Range
UPCEReader::decodeEnd(const BitArray& row, BitArray::Iterator begin) const
{
	return FindGuardPattern(row, begin, true, MIDDLE_END_PATTERN);
}

} // OneD
} // ZXing
