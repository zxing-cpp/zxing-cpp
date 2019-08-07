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

static const int CODE_WIDTH = 3 + // start guard
                              (7 * 6) + // bars
                              6; // end guard

BitMatrix
UPCEWriter::encode(const std::wstring& contents, int width, int height) const
{
	auto digits = UPCEANCommon::DigitString2IntArray<8>(
		contents, UPCEANCommon::ComputeChecksum(UPCEANCommon::ConvertUPCEtoUPCA(contents), contents.size() == 8));

	int firstDigit = digits[0];
	if (firstDigit != 0 && firstDigit != 1) {
		throw std::invalid_argument("Number system must be 0 or 1");
	}

	int parities = UPCEANCommon::NUMSYS_AND_CHECK_DIGIT_PATTERNS[firstDigit * 10 + digits[7]];
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

	WriterHelper::AppendPattern(result, pos, UPCEANCommon::UPCE_END_PATTERN, false);
	return WriterHelper::RenderResult(result, width, height, _sidesMargin >= 0 ? _sidesMargin : 9);
}

} // OneD
} // ZXing
