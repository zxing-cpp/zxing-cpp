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

#include "oned/ODCode128Writer.h"
#include "oned/ODWriterHelper.h"
#include "oned/ODCode128Patterns.h"
#include "EncodeHints.h"
#include "EncodeStatus.h"

#include <list>
#include <numeric>

namespace ZXing {
namespace OneD {

static const int CODE_START_B = 104;
static const int CODE_START_C = 105;
static const int CODE_CODE_B = 100;
static const int CODE_CODE_C = 99;
static const int CODE_STOP = 106;

// Dummy characters used to specify control characters in input
static const char ESCAPE_FNC_1 = '\u00f1';
static const char ESCAPE_FNC_2 = '\u00f2';
static const char ESCAPE_FNC_3 = '\u00f3';
static const char ESCAPE_FNC_4 = '\u00f4';

static const int CODE_FNC_1 = 102;   // Code A, Code B, Code C
static const int CODE_FNC_2 = 97;    // Code A, Code B
static const int CODE_FNC_3 = 96;    // Code A, Code B
static const int CODE_FNC_4_B = 100; // Code B

static bool AreDigits(const std::wstring& value, int start, int length) {
	int end = start + length;
	int last = value.length();
	for (int i = start; i < end && i < last; i++) {
		int c = value[i];
		if (c < '0' || c > '9') {
			if (c != ESCAPE_FNC_1) {
				return false;
			}
			end++; // ignore FNC_1
		}
	}
	return end <= last; // end > last if we've run out of string
}

EncodeStatus
Code128Writer::Encode(const std::wstring& contents, int width, int height, const EncodeHints& hints, BitMatrix& output)
{
	// Check length
	int length = static_cast<int>(contents.length());
	if (length < 1 || length > 80) {
		return EncodeStatus::WithError("Contents length should be between 1 and 80 characters");
	}

	// Check content
	for (int i = 0; i < length; ++i) {
		int c = contents[i];
		if (c < ' ' || c > '~') {
			switch (c) {
			case ESCAPE_FNC_1:
			case ESCAPE_FNC_2:
			case ESCAPE_FNC_3:
			case ESCAPE_FNC_4:
				break;
			default:
				return EncodeStatus::WithError("Bad character in input: " + static_cast<char>(c));
			}
		}
	}

	std::list<std::vector<int>> patterns; // temporary storage for patterns
	int checkSum = 0;
	int checkWeight = 1;
	int codeSet = 0; // selected code (CODE_CODE_B or CODE_CODE_C)
	int position = 0; // position in contents

	while (position < length) {
		//Select code to use
		int requiredDigitCount = codeSet == CODE_CODE_C ? 2 : 4;
		int newCodeSet;
		if (AreDigits(contents, position, requiredDigitCount)) {
			newCodeSet = CODE_CODE_C;
		}
		else {
			newCodeSet = CODE_CODE_B;
		}

		//Get the pattern index
		int patternIndex;
		if (newCodeSet == codeSet) {
			// Encode the current character
			// First handle escapes
			switch (contents[position]) {
			case ESCAPE_FNC_1:
				patternIndex = CODE_FNC_1;
				break;
			case ESCAPE_FNC_2:
				patternIndex = CODE_FNC_2;
				break;
			case ESCAPE_FNC_3:
				patternIndex = CODE_FNC_3;
				break;
			case ESCAPE_FNC_4:
				patternIndex = CODE_FNC_4_B; // FIXME if this ever outputs Code A
				break;
			default:
				// Then handle normal characters otherwise
				if (codeSet == CODE_CODE_B) {
					patternIndex = contents[position] - ' ';
				}
				else { // CODE_CODE_C
					patternIndex = (contents[position] - '0') * 10 + (position+1 < length ? contents[position+1] - '0' : 0);
					position++; // Also incremented below
				}
			}
			position++;
		}
		else {
			// Should we change the current code?
			// Do we have a code set?
			if (codeSet == 0) {
				// No, we don't have a code set
				if (newCodeSet == CODE_CODE_B) {
					patternIndex = CODE_START_B;
				}
				else {
					// CODE_CODE_C
					patternIndex = CODE_START_C;
				}
			}
			else {
				// Yes, we have a code set
				patternIndex = newCodeSet;
			}
			codeSet = newCodeSet;
		}

		// Get the pattern
		patterns.push_back(Code128::CODE_PATTERNS[patternIndex]);

		// Compute checksum
		checkSum += patternIndex * checkWeight;
		if (position != 0) {
			checkWeight++;
		}
	}

	// Compute and append checksum
	checkSum %= 103;
	patterns.push_back(Code128::CODE_PATTERNS[checkSum]);

	// Append stop code
	patterns.push_back(Code128::CODE_PATTERNS[CODE_STOP]);

	// Compute code width
	int codeWidth = 0;
	for (const std::vector<int>& pattern : patterns) {
		codeWidth += std::accumulate(pattern.begin(), pattern.end(), 0);
	}

	// Compute result
	std::vector<bool> result(codeWidth, false);
	int pos = 0;
	for (const std::vector<int>& pattern : patterns) {
		pos += WriterHelper::AppendPattern(result, pos, pattern, true);
	}

	int sidesMargin = hints.margin();
	if (sidesMargin < 0)
	{
		sidesMargin = 10;
	}
	WriterHelper::RenderResult(result, width, height, sidesMargin, output);

	return EncodeStatus::Success();
}

} // OneD
} // ZXing
