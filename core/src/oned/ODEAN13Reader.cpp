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

#include "oned/ODEAN13Reader.h"
#include "oned/ODUPCEANCommon.h"
#include "BarcodeFormat.h"
#include "BitArray.h"
#include "DecodeStatus.h"
#include "ZXContainerAlgorithms.h"

namespace ZXing {
namespace OneD {

// For an EAN-13 barcode, the first digit is represented by the parities used
// to encode the next six digits, according to the table below. For example,
// if the barcode is 5 123456 789012 then the value of the first digit is
// signified by using odd for '1', even for '2', even for '3', odd for '4',
// odd for '5', and even for '6'. See http://en.wikipedia.org/wiki/EAN-13
//
//                Parity of next 6 digits
//    Digit   0     1     2     3     4     5
//       0    Odd   Odd   Odd   Odd   Odd   Odd
//       1    Odd   Odd   Even  Odd   Even  Even
//       2    Odd   Odd   Even  Even  Odd   Even
//       3    Odd   Odd   Even  Even  Even  Odd
//       4    Odd   Even  Odd   Odd   Even  Even
//       5    Odd   Even  Even  Odd   Odd   Even
//       6    Odd   Even  Even  Even  Odd   Odd
//       7    Odd   Even  Odd   Even  Odd   Even
//       8    Odd   Even  Odd   Even  Even  Odd
//       9    Odd   Even  Even  Odd   Even  Odd
//
// Note that the encoding for '0' uses the same parity as a UPC barcode. Hence
// a UPC barcode can be converted to an EAN-13 barcode by prepending a 0.
//
// The encoding is represented by the following array, which is a bit pattern
// using Odd = 0 and Even = 1. For example, 5 is represented by:
//
//              Odd Even Even Odd Odd Even
// in binary:
//                0    1    1   0   0    1   == 0x19
//
static const int FIRST_DIGIT_ENCODINGS[] = {
	0x00, 0x0B, 0x0D, 0xE, 0x13, 0x19, 0x1C, 0x15, 0x16, 0x1A
};

BarcodeFormat
EAN13Reader::expectedFormat() const
{
	return BarcodeFormat::EAN_13;
}

BitArray::Range EAN13Reader::decodeMiddle(const BitArray& row, BitArray::Iterator begin, std::string& resultString) const
{
	int lgPatternFound = 0;
	BitArray::Range next = {begin, row.end()};
	const BitArray::Range notFound = {begin, begin};

	for (int x = 0; x < 6; x++) {
		int bestMatch = DecodeDigit(&next, UPCEANCommon::L_AND_G_PATTERNS, &resultString);
		if (bestMatch == -1)
			return notFound;

		if (bestMatch >= 10) {
			lgPatternFound |= 1 << (5 - x);
		}
	}

	/**
	* Based on pattern of odd-even ('L' and 'G') patterns used to encoded the explicitly-encoded
	* digits in a barcode, determines the implicitly encoded first digit and adds it to the
	* result string.
	*/
	int index = IndexOf(FIRST_DIGIT_ENCODINGS, lgPatternFound);
	if (index == -1)
		return notFound;
	resultString.insert(0, 1, (char)('0' + index));

	if (!ReadGuardPattern(&next, UPCEANCommon::MIDDLE_PATTERN))
		return notFound;

	for (int x = 0; x < 6; x++) {
		if (DecodeDigit(&next, UPCEANCommon::L_PATTERNS, &resultString) == -1)
			return notFound;
	}
	return {begin, next.begin};
}

} // OneD
} // ZXing
