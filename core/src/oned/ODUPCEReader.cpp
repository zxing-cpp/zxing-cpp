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
* used, and the check digit.
*/
static const std::array<std::array<int, 10>, 2> NUMSYS_AND_CHECK_DIGIT_PATTERNS = {
	0x38, 0x34, 0x32, 0x31, 0x2C, 0x26, 0x23, 0x2A, 0x29, 0x25,
	0x07, 0x0B, 0x0D, 0x0E, 0x13, 0x19, 0x1C, 0x15, 0x16, 0x1A,
};

BarcodeFormat
UPCEReader::expectedFormat() const
{
	return BarcodeFormat::UPC_E;
}

static DecodeStatus
DetermineNumSysAndCheckDigit(std::string& resultString, int lgPatternFound)
{
	for (size_t numSys = 0; numSys < NUMSYS_AND_CHECK_DIGIT_PATTERNS.size(); numSys++) {
		for (size_t d = 0; d < NUMSYS_AND_CHECK_DIGIT_PATTERNS[0].size(); d++) {
			if (lgPatternFound == NUMSYS_AND_CHECK_DIGIT_PATTERNS[numSys][d]) {
				resultString.insert(0, 1, (char)('0' + numSys));
				resultString.push_back((char)('0' + d));
				return DecodeStatus::NoError;
			}
		}
	}
	return DecodeStatus::NotFound;
}

DecodeStatus
UPCEReader::decodeMiddle(const BitArray& row, int &rowOffset, std::string& resultString) const
{
	std::array<int, 4> counters = {};
	int end = row.size();
	int lgPatternFound = 0;
	for (int x = 0; x < 6 && rowOffset < end; x++) {
		int bestMatch = 0;
		auto status = DecodeDigit(row, rowOffset, UPCEANCommon::L_AND_G_PATTERNS, counters, bestMatch);
		if (StatusIsError(status)) {
			return status;
		}
		resultString.push_back((char)('0' + bestMatch % 10));
		for (int counter : counters) {
			rowOffset += counter;
		}
		if (bestMatch >= 10) {
			lgPatternFound |= 1 << (5 - x);
		}
	}
	return DetermineNumSysAndCheckDigit(resultString, lgPatternFound);
}

DecodeStatus
UPCEReader::checkChecksum(const std::string& s) const
{
	return UPCEANReader::checkChecksum(UPCEANCommon::ConvertUPCEtoUPCA(s));
}

DecodeStatus
UPCEReader::decodeEnd(const BitArray& row, int endStart, int& begin, int& end) const
{
	return FindGuardPattern(row, endStart, true, MIDDLE_END_PATTERN, begin, end);
}

} // OneD
} // ZXing
