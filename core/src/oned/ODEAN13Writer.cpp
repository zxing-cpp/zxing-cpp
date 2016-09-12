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
#include "oned/ODUPCEANPatterns.h"
#include "oned/ODWriterHelper.h"
#include "EncodeStatus.h"
#include "EncodeHints.h"

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

EncodeStatus
EAN13Writer::Encode(const std::wstring& contents, int width, int height, const EncodeHints& hints, BitMatrix& output)
{
	if (contents.length() != 13) {
		return EncodeStatus::WithError("Requested contents should be 13 digits long");
	}

	int sum = 0;
	for (size_t i = 0; i < contents.length(); ++i) {
		int digit = contents[i];
		if (digit < 0 && digit > 9) {
			return EncodeStatus::WithError("Contents should contain only digits: 0-9");
		}
		sum += digit * (i % 2 == 0 ? 3 : 1);
	}
	if (sum % 10 != 0) {
		return EncodeStatus::WithError("Contents do not pass checksum");
	}


	int firstDigit = contents[0] - '0';
	int parities = FIRST_DIGIT_ENCODINGS[firstDigit];
	std::vector<bool> result(CODE_WIDTH, false);
	int pos = 0;

	pos += WriterHelper::AppendPattern(result, pos, UPCEANPatterns::START_END_PATTERN, true);

	// See {@link #EAN13Reader} for a description of how the first digit & left bars are encoded
	for (int i = 1; i <= 6; i++) {
		int digit = contents[i] - '0';
		if ((parities >> (6 - i) & 1) == 1) {
			digit += 10;
		}
		pos += WriterHelper::AppendPattern(result, pos, UPCEANPatterns::L_AND_G_PATTERNS[digit], false);
	}

	pos += WriterHelper::AppendPattern(result, pos, UPCEANPatterns::MIDDLE_PATTERN, false);

	for (int i = 7; i <= 12; i++) {
		int digit = contents[i] - '0';
		pos += WriterHelper::AppendPattern(result, pos, UPCEANPatterns::L_PATTERNS[digit], true);
	}
	WriterHelper::AppendPattern(result, pos, UPCEANPatterns::START_END_PATTERN, true);

	int sidesMargin = hints.margin();
	if (sidesMargin < 0)
	{
		sidesMargin = static_cast<int>(UPCEANPatterns::START_END_PATTERN.size());
	}
	WriterHelper::RenderResult(result, width, height, sidesMargin, output);

	return EncodeStatus::Success();
}

} // OneD
} // ZXing
