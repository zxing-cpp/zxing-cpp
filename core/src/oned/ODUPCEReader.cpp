/*
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
#include "BarcodeFormat.h"
#include "BitArray.h"
#include "ErrorStatus.h"

#include <array>

namespace ZXing {

namespace OneD {

// For an UPC-E barcode, the final digit is represented by the parities used
// to encode the middle six digits, according to the table below.
//
//                Parity of next 6 digits
//    Digit   0     1     2     3     4     5
//       0    Even   Even  Even Odd  Odd   Odd
//       1    Even   Even  Odd  Even Odd   Odd
//       2    Even   Even  Odd  Odd  Even  Odd
//       3    Even   Even  Odd  Odd  Odd   Even
//       4    Even   Odd   Even Even Odd   Odd
//       5    Even   Odd   Odd  Even Even  Odd
//       6    Even   Odd   Odd  Odd  Even  Even
//       7    Even   Odd   Even Odd  Even  Odd
//       8    Even   Odd   Even Odd  Odd   Even
//       9    Even   Odd   Odd  Even Odd   Even
//
// The encoding is represented by the following array, which is a bit pattern
// using Odd = 0 and Even = 1. For example, 5 is represented by:
//
//              Odd Even Even Odd Odd Even
// in binary:
//                0    1    1   0   0    1   == 0x19
//
//static final int[] CHECK_DIGIT_ENCODINGS = {
//	0x38, 0x34, 0x32, 0x31, 0x2C, 0x26, 0x23, 0x2A, 0x29, 0x25
//};

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

static ErrorStatus
DetermineNumSysAndCheckDigit(std::string& resultString, int lgPatternFound)
{
	for (size_t numSys = 0; numSys < NUMSYS_AND_CHECK_DIGIT_PATTERNS.size(); numSys++) {
		for (size_t d = 0; d < NUMSYS_AND_CHECK_DIGIT_PATTERNS[0].size(); d++) {
			if (lgPatternFound == NUMSYS_AND_CHECK_DIGIT_PATTERNS[numSys][d]) {
				resultString.insert(0, 1, (char)('0' + numSys));
				resultString.push_back((char)('0' + d));
				return ErrorStatus::NoError;
			}
		}
	}
	return ErrorStatus::NotFound;
}

ErrorStatus
UPCEReader::decodeMiddle(const BitArray& row, int &rowOffset, std::string& resultString)
{
	std::array<int, 4> counters = {};
	int end = row.size();
	int lgPatternFound = 0;
	for (int x = 0; x < 6 && rowOffset < end; x++) {
		int bestMatch = 0;
		auto status = DecodeDigit(row, rowOffset, L_AND_G_PATTERNS, counters, bestMatch);
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

ErrorStatus
UPCEReader::checkChecksum(const std::string& s)
{
	return UPCEANReader::checkChecksum(ConvertUPCEtoUPCA(s));
}

ErrorStatus
UPCEReader::decodeEnd(const BitArray& row, int endStart, int& begin, int& end)
{
	return FindGuardPattern(row, endStart, true, MIDDLE_END_PATTERN, begin, end);
}

std::string
UPCEReader::ConvertUPCEtoUPCA(const std::string& upce)
{
	if (upce.length() < 8)
		return upce;

	auto upceChars = upce.substr(1, 6);

	std::string result;
	result.reserve(12);
	result += upce[0];
	char lastChar = upceChars[5];
	switch (lastChar) {
	case '0':
	case '1':
	case '2':
		result += upceChars.substr(0, 2);
		result += lastChar;
		result += "0000";
		result += upceChars.substr(2, 3);
		break;
	case '3':
		result += upceChars.substr(0, 3);
		result += "00000";
		result += upceChars.substr(3, 2);
		break;
	case '4':
		result += upceChars.substr(0, 4);
		result += "00000";
		result += upceChars[4];
		break;
	default:
		result += upceChars.substr(0, 5);
		result += "0000";
		result += lastChar;
		break;
	}
	result += upce[7];
	return result;
}

} // OneD
} // ZXing
