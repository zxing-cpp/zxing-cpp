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

#include "oned/ODEAN13Writer.h"
#include "oned/ODUPCEANCommon.h"
#include "oned/ODWriterHelper.h"
#include <array>

namespace ZXing {
namespace OneD {

static const int FIRST_DIGIT_ENCODINGS[] = {
	0x00, 0x0B, 0x0D, 0xE, 0x13, 0x19, 0x1C, 0x15, 0x16, 0x1A
};

static const int CODE_WIDTH = 3 + // start guard
                              (7 * 6) + // left bars
                              5 + // middle guard
                              (7 * 6) + // right bars
                              3; // end guard

BitMatrix
EAN13Writer::encode(const std::wstring& contents, int width, int height) const
{
	size_t length = contents.length();
	if (length != 12 && length != 13) {
		throw std::invalid_argument("Requested contents should be 12 or 13 digits long");
	}

	std::array<int, 13> digits;
	for (size_t i = 0; i < length; ++i) {
		digits[i] = contents[i] - '0';
		if (digits[i] < 0 || digits[i] > 9) {
			throw std::invalid_argument("Contents should contain only digits: 0-9");
		}
	}

	if (length == 12) {
		digits[12] = UPCEANCommon::ComputeChecksum(digits);
	}
	else if (digits[12] != UPCEANCommon::ComputeChecksum(digits)) {
		throw std::invalid_argument("Contents do not pass checksum");
	}

	int parities = FIRST_DIGIT_ENCODINGS[digits[0]];
	std::vector<bool> result(CODE_WIDTH, false);
	int pos = 0;

	pos += WriterHelper::AppendPattern(result, pos, UPCEANCommon::START_END_PATTERN, true);

	// See {@link #EAN13Reader} for a description of how the first digit & left bars are encoded
	for (int i = 1; i <= 6; i++) {
		int digit = digits[i];
		if ((parities >> (6 - i) & 1) == 1) {
			digit += 10;
		}
		pos += WriterHelper::AppendPattern(result, pos, UPCEANCommon::L_AND_G_PATTERNS[digit], false);
	}

	pos += WriterHelper::AppendPattern(result, pos, UPCEANCommon::MIDDLE_PATTERN, false);

	for (int i = 7; i <= 12; i++) {
		pos += WriterHelper::AppendPattern(result, pos, UPCEANCommon::L_PATTERNS[digits[i]], true);
	}
	WriterHelper::AppendPattern(result, pos, UPCEANCommon::START_END_PATTERN, true);
	return WriterHelper::RenderResult(result, width, height, _sidesMargin >= 0 ? _sidesMargin : 9);
}

} // OneD
} // ZXing
