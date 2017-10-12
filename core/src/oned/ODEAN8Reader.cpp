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

DecodeStatus
EAN8Reader::decodeMiddle(const BitArray& row, int &rowOffset, std::string& resultString) const
{
	Digit counters = {};
	int end = row.size();
	DecodeStatus status;

	for (int x = 0; x < 4 && rowOffset < end; x++) {
		int bestMatch = 0;
		status = DecodeDigit(row, rowOffset, UPCEANCommon::L_PATTERNS, counters, bestMatch);
		if (StatusIsError(status)) {
			return status;
		}
		resultString.push_back((char)('0' + bestMatch));
		rowOffset = Accumulate(counters, rowOffset);
	}

	auto middleRange = FindGuardPattern(row, row.iterAt(rowOffset), true, UPCEANCommon::MIDDLE_PATTERN);
	if (!middleRange)
		return DecodeStatus::NotFound;

	rowOffset = middleRange.end - row.begin();
	for (int x = 0; x < 4 && rowOffset < end; x++) {
		int bestMatch = 0;
		status = DecodeDigit(row, rowOffset, UPCEANCommon::L_PATTERNS, counters, bestMatch);
		if (StatusIsError(status)) {
			return status;
		}
		resultString.push_back((char)('0' + bestMatch));
		rowOffset = Accumulate(counters, rowOffset);
	}
	return DecodeStatus::NoError;
}

} // OneD
} // ZXing
