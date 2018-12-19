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

#include "oned/ODUPCEWriter.h"
#include "oned/ODUPCEANCommon.h"
#include "oned/ODWriterHelper.h"

#include <vector>

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
static const std::array<std::array<int, 10>, 2> NUMSYS_AND_CHECK_DIGIT_PATTERNS = {
	0x38, 0x34, 0x32, 0x31, 0x2C, 0x26, 0x23, 0x2A, 0x29, 0x25,
	0x07, 0x0B, 0x0D, 0x0E, 0x13, 0x19, 0x1C, 0x15, 0x16, 0x1A,
};

static const int CODE_WIDTH = 3 + // start guard
                              (7 * 6) + // bars
                              6; // end guard

template <size_t N>
static void GetDigits(const std::wstring& contents, std::array<int, N>& digits)
{
	for (size_t i = 0; i < contents.length(); ++i) {
		digits[i] = contents[i] - '0';
		if (digits[i] < 0 || digits[i] > 9) {
			throw std::invalid_argument("Contents should contain only digits: 0-9");
		}
	}
}

BitMatrix
UPCEWriter::encode(const std::wstring& contents, int width, int height) const
{
	size_t length = contents.length();
	if (length != 7 && length != 8) {
		throw std::invalid_argument("Requested contents should be 7 or 8 digits long");
	}

	std::array<int, 8> digits;
	GetDigits(contents, digits);

	if (length == 7) {
		std::array<int, 12> upceDigits;
		GetDigits(UPCEANCommon::ConvertUPCEtoUPCA(contents), upceDigits);
		digits[7] = UPCEANCommon::ComputeChecksum(upceDigits);
	}
	else if (digits[7] != UPCEANCommon::ComputeChecksum(digits)) {
		throw std::invalid_argument("Contents do not pass checksum");
	}
	
	int firstDigit = digits[0];
	if (firstDigit != 0 && firstDigit != 1) {
		throw std::invalid_argument("Number system must be 0 or 1");
	}

	int parities = NUMSYS_AND_CHECK_DIGIT_PATTERNS[firstDigit][digits[7]];
	std::vector<bool> result(CODE_WIDTH, false);
	int pos = 0;

	pos += WriterHelper::AppendPattern(result, pos, UPCEANCommon::START_END_PATTERN, true);

	for (int i = 1; i <= 6; i++) {
		int digit = digits[i];
		if ((parities >> (6 - i) & 1) == 1) {
			digit += 10;
		}
		pos += WriterHelper::AppendPattern(result, pos, UPCEANCommon::L_AND_G_PATTERNS[digit], false);
	}

	WriterHelper::AppendPattern(result, pos, UPCEANCommon::END_PATTERN, false);
	return WriterHelper::RenderResult(result, width, height, _sidesMargin >= 0 ? _sidesMargin : 9);
}

} // OneD
} // ZXing
