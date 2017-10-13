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

#include "oned/ODEAN8Reader.h"
#include "oned/ODUPCEANCommon.h"
#include "BitArray.h"
#include "BarcodeFormat.h"
#include "DecodeStatus.h"
#include "ZXContainerAlgorithms.h"

namespace ZXing {
namespace OneD {

BarcodeFormat
EAN8Reader::expectedFormat() const
{
	return BarcodeFormat::EAN_8;
}

BitArray::Range
EAN8Reader::decodeMiddle(const BitArray& row, BitArray::Iterator begin, std::string& resultString) const
{
	BitArray::Range next = {begin, row.end()};
	const BitArray::Range notFound = {begin, begin};

	for (int x = 0; x < 4 && next; x++) {
		if (DecodeDigit(&next, UPCEANCommon::L_PATTERNS, &resultString) == -1)
			return notFound;
	}

	auto middleRange = FindGuardPattern(row, next.begin, true, UPCEANCommon::MIDDLE_PATTERN);
	if (!middleRange)
		return notFound;
	next.begin = middleRange.end;

	for (int x = 0; x < 4 && next; x++) {
		if (DecodeDigit(&next, UPCEANCommon::L_PATTERNS, &resultString) == -1)
			return notFound;
	}
	return {begin, next.begin};
}

} // OneD
} // ZXing
