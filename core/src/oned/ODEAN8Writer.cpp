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

#include "oned/ODEAN8Writer.h"
#include "oned/ODUPCEANCommon.h"
#include "oned/ODWriterHelper.h"

#include <vector>

namespace ZXing {
namespace OneD {

static const int CODE_WIDTH = 3 + // start guard
                              (7 * 4) + // left bars
                              5 + // middle guard
                              (7 * 4) + // right bars
                              3; // end guard

BitMatrix
EAN8Writer::encode(const std::wstring& contents, int width, int height) const
{
	size_t length = contents.length();
	if (length != 7 && length != 8) {
		throw std::invalid_argument("Requested contents should be 7 or 8 digits long");
	}

	std::array<int, 8> digits;
	for (size_t i = 0; i < length; ++i) {
		digits[i] = contents[i] - '0';
		if (digits[i] < 0 || digits[i] > 9) {
			throw std::invalid_argument("Contents should contain only digits: 0-9");
		}
	}

	if (length == 7) {
		digits[7] = UPCEANCommon::ComputeChecksum(digits);
	}
	else if (digits[7] != UPCEANCommon::ComputeChecksum(digits)) {
		throw std::invalid_argument("Contents do not pass checksum");
	}

	std::vector<bool> result(CODE_WIDTH, false);
	int pos = 0;

	pos += WriterHelper::AppendPattern(result, pos, UPCEANCommon::START_END_PATTERN, true);

	for (int i = 0; i <= 3; i++) {
		pos += WriterHelper::AppendPattern(result, pos, UPCEANCommon::L_PATTERNS[digits[i]], false);
	}

	pos += WriterHelper::AppendPattern(result, pos, UPCEANCommon::MIDDLE_PATTERN, false);

	for (int i = 4; i <= 7; i++) {
		pos += WriterHelper::AppendPattern(result, pos, UPCEANCommon::L_PATTERNS[digits[i]], true);
	}
	WriterHelper::AppendPattern(result, pos, UPCEANCommon::START_END_PATTERN, true);
	return WriterHelper::RenderResult(result, width, height, _sidesMargin >= 0 ? _sidesMargin : 9);
}

} // OneD
} // ZXing
