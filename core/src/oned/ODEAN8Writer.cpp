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
#include "oned/ODUPCEANPatterns.h"
#include "oned/ODWriterHelper.h"

#include <vector>

namespace ZXing {
namespace OneD {

static const int CODE_WIDTH = 3 + // start guard
                              (7 * 4) + // left bars
                              5 + // middle guard
                              (7 * 4) + // right bars
                              3; // end guard

void
EAN8Writer::encode(const std::wstring& contents, int width, int height,  BitMatrix& output) const
{
	if (contents.length() != 8) {
		throw std::invalid_argument("Requested contents should be 8 digits long");
	}

	for (size_t i = 0; i < contents.length(); ++i) {
		if (contents[i] < '0' && contents[i] > '9') {
			throw std::invalid_argument("Contents should contain only digits: 0-9");
		}
	}

	std::vector<bool> result(CODE_WIDTH, false);
	int pos = 0;

	pos += WriterHelper::AppendPattern(result, pos, UPCEANPatterns::START_END_PATTERN, true);

	for (int i = 0; i <= 3; i++) {
		int digit = contents[i] - '0';
		pos += WriterHelper::AppendPattern(result, pos, UPCEANPatterns::L_PATTERNS[digit], false);
	}

	pos += WriterHelper::AppendPattern(result, pos, UPCEANPatterns::MIDDLE_PATTERN, false);

	for (int i = 4; i <= 7; i++) {
		int digit = contents[i] - '0';
		pos += WriterHelper::AppendPattern(result, pos, UPCEANPatterns::L_PATTERNS[digit], true);
	}
	WriterHelper::AppendPattern(result, pos, UPCEANPatterns::START_END_PATTERN, true);

	int sidesMargin = _sidesMargin;
	if (sidesMargin < 0)
	{
		sidesMargin = static_cast<int>(UPCEANPatterns::START_END_PATTERN.size());
	}
	WriterHelper::RenderResult(result, width, height, sidesMargin, output);
}

} // OneD
} // ZXing
