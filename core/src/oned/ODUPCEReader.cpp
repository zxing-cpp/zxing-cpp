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

	int i = IndexOf(UPCEANCommon::NUMSYS_AND_CHECK_DIGIT_PATTERNS, lgPatternFound);
	if (i == -1)
		return notFound;

	resultString = std::to_string(i/10) + resultString + std::to_string(i % 10);
	return {begin, next.begin};
}

bool UPCEReader::checkChecksum(const std::string& s) const
{
	return UPCEANReader::checkChecksum(UPCEANCommon::ConvertUPCEtoUPCA(s));
}

BitArray::Range
UPCEReader::decodeEnd(const BitArray& row, BitArray::Iterator begin) const
{
	BitArray::Range next = {begin, row.end()};
	ReadGuardPattern(&next, UPCEANCommon::UPCE_END_PATTERN);
	return {begin, next.begin};
}

} // OneD
} // ZXing
