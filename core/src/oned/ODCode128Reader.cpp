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

#include "oned/ODCode128Reader.h"
#include "oned/ODCode128Patterns.h"
#include "Result.h"
#include "BitArray.h"
#include "DecodeHints.h"
#include "TextDecoder.h"
#include "ZXContainerAlgorithms.h"
#include "ZXStrConvWorkaround.h"

#include <algorithm>
#include <string>
#include <array>

namespace ZXing {
namespace OneD {

static const float MAX_AVG_VARIANCE = 0.25f;
static const float MAX_INDIVIDUAL_VARIANCE = 0.7f;

static const int CODE_SHIFT = 98;

static const int CODE_CODE_C = 99;
static const int CODE_CODE_B = 100;
static const int CODE_CODE_A = 101;

static const int CODE_FNC_1 = 102;
static const int CODE_FNC_2 = 97;
static const int CODE_FNC_3 = 96;
static const int CODE_FNC_4_A = 101;
static const int CODE_FNC_4_B = 100;

static const int CODE_START_A = 103;
static const int CODE_START_B = 104;
static const int CODE_START_C = 105;
static const int CODE_STOP = 106;

static BitArray::Range
FindStartPattern(const BitArray& row, int* startCode)
{
	assert(startCode != nullptr);

	using Counters = std::vector<int>;
	Counters counters(Code128::CODE_PATTERNS[CODE_START_A].size());

	return RowReader::FindPattern(
	    row.getNextSet(row.begin()), row.end(), counters,
	    [&row, startCode](BitArray::Iterator begin, BitArray::Iterator end, const Counters& counters) {
		    float bestVariance = MAX_AVG_VARIANCE;
		    for (int code = CODE_START_A; code <= CODE_START_C; code++) {
			    float variance =
			        RowReader::PatternMatchVariance(counters, Code128::CODE_PATTERNS[code], MAX_INDIVIDUAL_VARIANCE);
			    if (variance < bestVariance) {
				    bestVariance = variance;
				    *startCode = code;
			    }
		    }
		    // Look for whitespace before start pattern, >= 50% of width of start pattern
		    return bestVariance < MAX_AVG_VARIANCE && row.hasQuiteZone(begin, -(end - begin) / 2);
	    });
}

static bool DecodeCode(const std::vector<int>& counters, int* outCode)
{
	assert(outCode != nullptr);

	float bestVariance = MAX_AVG_VARIANCE; // worst variance we'll accept
	int bestMatch = -1;
	for (size_t d = 0; d < Code128::CODE_PATTERNS.size(); d++) {
		const auto& pattern = Code128::CODE_PATTERNS[d];
		float variance = RowReader::PatternMatchVariance(counters, pattern, MAX_INDIVIDUAL_VARIANCE);
		if (variance < bestVariance) {
			bestVariance = variance;
			bestMatch = static_cast<int>(d);
		}
	}
	// TODO We're overlooking the fact that the STOP pattern has 7 values, not 6.
	if (bestMatch >= 0) {
		*outCode = bestMatch;
		return true;
	}
	return false;
}

Code128Reader::Code128Reader(const DecodeHints& hints) :
	_convertFNC1(hints.shouldAssumeGS1())
{
}

Result
Code128Reader::decodeRow(int rowNumber, const BitArray& row, std::unique_ptr<DecodingState>&) const
{
	int startCode = 0;
	auto range = FindStartPattern(row, &startCode);
	if (!range) {
		return Result(DecodeStatus::NotFound);
	}

	float left = (range.begin - row.begin()) + 0.5f * range.size();
	ByteArray rawCodes;
	rawCodes.reserve(20);
	rawCodes.push_back(static_cast<uint8_t>(startCode));

	int codeSet;
	switch (startCode) {
	case CODE_START_A:
		codeSet = CODE_CODE_A;
		break;
	case CODE_START_B:
		codeSet = CODE_CODE_B;
		break;
	case CODE_START_C:
		codeSet = CODE_CODE_C;
		break;
	default:
		return Result(DecodeStatus::FormatError);
	}

	bool done = false;
	bool isNextShifted = false;

	std::string result;
	result.reserve(20);
	std::vector<int> counters(6, 0);

	int lastCode = 0;
	int code = 0;
	int checksumTotal = startCode;
	int multiplier = 0;
	bool lastCharacterWasPrintable = true;
	bool upperMode = false;
	bool shiftUpperMode = false;

	while (!done) {

		bool unshift = isNextShifted;
		isNextShifted = false;

		// Save off last code
		lastCode = code;

		range = RecordPattern(range.end, row.end(), counters);
		if (!range)
			return Result(DecodeStatus::NotFound);

		// Decode another code from image
		if (!DecodeCode(counters, &code))
			return Result(DecodeStatus::NotFound);

		rawCodes.push_back(static_cast<uint8_t>(code));

		// Remember whether the last code was printable or not (excluding CODE_STOP)
		if (code != CODE_STOP) {
			lastCharacterWasPrintable = true;
		}

		// Add to checksum computation (if not CODE_STOP of course)
		if (code != CODE_STOP) {
			multiplier++;
			checksumTotal += multiplier * code;
		}

		// Take care of illegal start codes
		switch (code) {
		case CODE_START_A:
		case CODE_START_B:
		case CODE_START_C:
			return Result(DecodeStatus::FormatError);
		}

		switch (codeSet) {

		case CODE_CODE_A:
			if (code < 64) {
				if (shiftUpperMode == upperMode) {
					result.push_back((char)(' ' + code));
				}
				else {
					result.push_back((char)(' ' + code + 128));
				}
				shiftUpperMode = false;
			}
			else if (code < 96) {
				if (shiftUpperMode == upperMode) {
					result.push_back((char)(code - 64));
				}
				else {
					result.push_back((char)(code + 64));
				}
				shiftUpperMode = false;
			}
			else {
				// Don't let CODE_STOP, which always appears, affect whether whether we think the last
				// code was printable or not.
				if (code != CODE_STOP) {
					lastCharacterWasPrintable = false;
				}
				switch (code) {
				case CODE_FNC_1:
					if (_convertFNC1) {
						if (result.empty()) {
							// GS1 specification 5.4.3.7. and 5.4.6.4. If the first char after the start code
							// is FNC1 then this is GS1-128. We add the symbology identifier.
							result.append("]C1");
						}
						else {
							// GS1 specification 5.4.7.5. Every subsequent FNC1 is returned as ASCII 29 (GS)
							result.push_back((char)29);
						}
					}
					break;
				case CODE_FNC_2:
				case CODE_FNC_3:
					// do nothing?
					break;
				case CODE_FNC_4_A:
					if (!upperMode && shiftUpperMode) {
						upperMode = true;
						shiftUpperMode = false;
					}
					else if (upperMode && shiftUpperMode) {
						upperMode = false;
						shiftUpperMode = false;
					}
					else {
						shiftUpperMode = true;
					}
					break;
				case CODE_SHIFT:
					isNextShifted = true;
					codeSet = CODE_CODE_B;
					break;
				case CODE_CODE_B:
					codeSet = CODE_CODE_B;
					break;
				case CODE_CODE_C:
					codeSet = CODE_CODE_C;
					break;
				case CODE_STOP:
					done = true;
					break;
				}
			}
			break;
		case CODE_CODE_B:
			if (code < 96) {
				if (shiftUpperMode == upperMode) {
					result.push_back((char)(' ' + code));
				}
				else {
					result.push_back((char)(' ' + code + 128));
				}
				shiftUpperMode = false;
			}
			else {
				if (code != CODE_STOP) {
					lastCharacterWasPrintable = false;
				}
				switch (code) {
				case CODE_FNC_1:
					if (_convertFNC1) {
						if (result.empty()) {
							// GS1 specification 5.4.3.7. and 5.4.6.4. If the first char after the start code
							// is FNC1 then this is GS1-128. We add the symbology identifier.
							result.append("]C1");
						}
						else {
							// GS1 specification 5.4.7.5. Every subsequent FNC1 is returned as ASCII 29 (GS)
							result.push_back((char)29);
						}
					}
					break;
				case CODE_FNC_2:
				case CODE_FNC_3:
					// do nothing?
					break;
				case CODE_FNC_4_B:
					if (!upperMode && shiftUpperMode) {
						upperMode = true;
						shiftUpperMode = false;
					}
					else if (upperMode && shiftUpperMode) {
						upperMode = false;
						shiftUpperMode = false;
					}
					else {
						shiftUpperMode = true;
					}
					break;
				case CODE_SHIFT:
					isNextShifted = true;
					codeSet = CODE_CODE_A;
					break;
				case CODE_CODE_A:
					codeSet = CODE_CODE_A;
					break;
				case CODE_CODE_C:
					codeSet = CODE_CODE_C;
					break;
				case CODE_STOP:
					done = true;
					break;
				}
			}
			break;
		case CODE_CODE_C:
			if (code < 100) {
				if (code < 10) {
					result.push_back('0');
				}
				result.append(std::to_string(code));
			}
			else {
				if (code != CODE_STOP) {
					lastCharacterWasPrintable = false;
				}
				switch (code) {
				case CODE_FNC_1:
					if (_convertFNC1) {
						if (result.empty()) {
							// GS1 specification 5.4.3.7. and 5.4.6.4. If the first char after the start code
							// is FNC1 then this is GS1-128. We add the symbology identifier.
							result.append("]C1");
						}
						else {
							// GS1 specification 5.4.7.5. Every subsequent FNC1 is returned as ASCII 29 (GS)
							result.push_back((char)29);
						}
					}
					break;
				case CODE_CODE_A:
					codeSet = CODE_CODE_A;
					break;
				case CODE_CODE_B:
					codeSet = CODE_CODE_B;
					break;
				case CODE_STOP:
					done = true;
					break;
				}
			}
			break;
		}

		// Unshift back to another code set if we were shifted
		if (unshift) {
			codeSet = codeSet == CODE_CODE_A ? CODE_CODE_B : CODE_CODE_A;
		}

	}

	// Check for ample whitespace following pattern, but, to do this we first need to remember that
	// we fudged decoding CODE_STOP since it actually has 7 bars, not 6. There is a black bar left
	// to read off. Would be slightly better to properly read. Here we just skip it:
	range.end = row.getNextUnset(range.end);
	if (!row.hasQuiteZone(range.end, range.size() / 2))
		return Result(DecodeStatus::NotFound);

	// Pull out from sum the value of the penultimate check code
	checksumTotal -= multiplier * lastCode;
	// lastCode is the checksum then:
	if (checksumTotal % 103 != lastCode) {
		return Result(DecodeStatus::ChecksumError);
	}

	// Need to pull out the check digits from string
	size_t resultLength = result.length();
	if (resultLength == 0) {
		// false positive
		return Result(DecodeStatus::NotFound);
	}

	// Only bother if the result had at least one character, and if the checksum digit happened to
	// be a printable character. If it was just interpreted as a control code, nothing to remove.
	if (resultLength > 0 && lastCharacterWasPrintable) {
		if (codeSet == CODE_CODE_C) {
			result.resize(resultLength >= 2 ? resultLength - 2 : 0);
		}
		else {
			result.resize(resultLength - 1);
		}
	}

	float right = (range.begin - row.begin()) + 0.5f * range.size();
	float ypos = static_cast<float>(rowNumber);
	return Result(TextDecoder::FromLatin1(result), std::move(rawCodes), { ResultPoint(left, ypos), ResultPoint(right, ypos) }, BarcodeFormat::CODE_128);
}

} // OneD
} // ZXing
