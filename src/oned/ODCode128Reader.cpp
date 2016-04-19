/*
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
#include "Result.h"
#include "BitArray.h"
#include "DecodeHints.h"

#include <algorithm>

namespace ZXing {

namespace OneD {

static const int CODE_PATTERNS_LENGTH = 107;
static const std::vector<int> CODE_PATTERNS[CODE_PATTERNS_LENGTH] = {
	{ 2, 1, 2, 2, 2, 2 }, // 0
	{ 2, 2, 2, 1, 2, 2 },
	{ 2, 2, 2, 2, 2, 1 },
	{ 1, 2, 1, 2, 2, 3 },
	{ 1, 2, 1, 3, 2, 2 },
	{ 1, 3, 1, 2, 2, 2 }, // 5
	{ 1, 2, 2, 2, 1, 3 },
	{ 1, 2, 2, 3, 1, 2 },
	{ 1, 3, 2, 2, 1, 2 },
	{ 2, 2, 1, 2, 1, 3 },
	{ 2, 2, 1, 3, 1, 2 }, // 10
	{ 2, 3, 1, 2, 1, 2 },
	{ 1, 1, 2, 2, 3, 2 },
	{ 1, 2, 2, 1, 3, 2 },
	{ 1, 2, 2, 2, 3, 1 },
	{ 1, 1, 3, 2, 2, 2 }, // 15
	{ 1, 2, 3, 1, 2, 2 },
	{ 1, 2, 3, 2, 2, 1 },
	{ 2, 2, 3, 2, 1, 1 },
	{ 2, 2, 1, 1, 3, 2 },
	{ 2, 2, 1, 2, 3, 1 }, // 20
	{ 2, 1, 3, 2, 1, 2 },
	{ 2, 2, 3, 1, 1, 2 },
	{ 3, 1, 2, 1, 3, 1 },
	{ 3, 1, 1, 2, 2, 2 },
	{ 3, 2, 1, 1, 2, 2 }, // 25
	{ 3, 2, 1, 2, 2, 1 },
	{ 3, 1, 2, 2, 1, 2 },
	{ 3, 2, 2, 1, 1, 2 },
	{ 3, 2, 2, 2, 1, 1 },
	{ 2, 1, 2, 1, 2, 3 }, // 30
	{ 2, 1, 2, 3, 2, 1 },
	{ 2, 3, 2, 1, 2, 1 },
	{ 1, 1, 1, 3, 2, 3 },
	{ 1, 3, 1, 1, 2, 3 },
	{ 1, 3, 1, 3, 2, 1 }, // 35
	{ 1, 1, 2, 3, 1, 3 },
	{ 1, 3, 2, 1, 1, 3 },
	{ 1, 3, 2, 3, 1, 1 },
	{ 2, 1, 1, 3, 1, 3 },
	{ 2, 3, 1, 1, 1, 3 }, // 40
	{ 2, 3, 1, 3, 1, 1 },
	{ 1, 1, 2, 1, 3, 3 },
	{ 1, 1, 2, 3, 3, 1 },
	{ 1, 3, 2, 1, 3, 1 },
	{ 1, 1, 3, 1, 2, 3 }, // 45
	{ 1, 1, 3, 3, 2, 1 },
	{ 1, 3, 3, 1, 2, 1 },
	{ 3, 1, 3, 1, 2, 1 },
	{ 2, 1, 1, 3, 3, 1 },
	{ 2, 3, 1, 1, 3, 1 }, // 50
	{ 2, 1, 3, 1, 1, 3 },
	{ 2, 1, 3, 3, 1, 1 },
	{ 2, 1, 3, 1, 3, 1 },
	{ 3, 1, 1, 1, 2, 3 },
	{ 3, 1, 1, 3, 2, 1 }, // 55
	{ 3, 3, 1, 1, 2, 1 },
	{ 3, 1, 2, 1, 1, 3 },
	{ 3, 1, 2, 3, 1, 1 },
	{ 3, 3, 2, 1, 1, 1 },
	{ 3, 1, 4, 1, 1, 1 }, // 60
	{ 2, 2, 1, 4, 1, 1 },
	{ 4, 3, 1, 1, 1, 1 },
	{ 1, 1, 1, 2, 2, 4 },
	{ 1, 1, 1, 4, 2, 2 },
	{ 1, 2, 1, 1, 2, 4 }, // 65
	{ 1, 2, 1, 4, 2, 1 },
	{ 1, 4, 1, 1, 2, 2 },
	{ 1, 4, 1, 2, 2, 1 },
	{ 1, 1, 2, 2, 1, 4 },
	{ 1, 1, 2, 4, 1, 2 }, // 70
	{ 1, 2, 2, 1, 1, 4 },
	{ 1, 2, 2, 4, 1, 1 },
	{ 1, 4, 2, 1, 1, 2 },
	{ 1, 4, 2, 2, 1, 1 },
	{ 2, 4, 1, 2, 1, 1 }, // 75
	{ 2, 2, 1, 1, 1, 4 },
	{ 4, 1, 3, 1, 1, 1 },
	{ 2, 4, 1, 1, 1, 2 },
	{ 1, 3, 4, 1, 1, 1 },
	{ 1, 1, 1, 2, 4, 2 }, // 80
	{ 1, 2, 1, 1, 4, 2 },
	{ 1, 2, 1, 2, 4, 1 },
	{ 1, 1, 4, 2, 1, 2 },
	{ 1, 2, 4, 1, 1, 2 },
	{ 1, 2, 4, 2, 1, 1 }, // 85
	{ 4, 1, 1, 2, 1, 2 },
	{ 4, 2, 1, 1, 1, 2 },
	{ 4, 2, 1, 2, 1, 1 },
	{ 2, 1, 2, 1, 4, 1 },
	{ 2, 1, 4, 1, 2, 1 }, // 90
	{ 4, 1, 2, 1, 2, 1 },
	{ 1, 1, 1, 1, 4, 3 },
	{ 1, 1, 1, 3, 4, 1 },
	{ 1, 3, 1, 1, 4, 1 },
	{ 1, 1, 4, 1, 1, 3 }, // 95
	{ 1, 1, 4, 3, 1, 1 },
	{ 4, 1, 1, 1, 1, 3 },
	{ 4, 1, 1, 3, 1, 1 },
	{ 1, 1, 3, 1, 4, 1 },
	{ 1, 1, 4, 1, 3, 1 }, // 100
	{ 3, 1, 1, 1, 4, 1 },
	{ 4, 1, 1, 1, 3, 1 },
	{ 2, 1, 1, 4, 1, 2 },
	{ 2, 1, 1, 2, 1, 4 },
	{ 2, 1, 1, 2, 3, 2 }, // 105
	{ 2, 3, 3, 1, 1, 1, 2 }
};

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

static ErrorStatus
FindStartPattern(const BitArray& row, int& begin, int& end, int& startCode)
{
	int width = row.size();
	int rowOffset = row.getNextSet(0);

	int counterPosition = 0;
	std::vector<int> counters(6, 0);
	int patternStart = rowOffset;
	bool isWhite = false;
	int patternLength = static_cast<int>(counters.size());

	for (int i = rowOffset; i < width; i++) {
		if (row.get(i) ^ isWhite) {
			counters[counterPosition]++;
		}
		else {
			if (counterPosition == patternLength - 1) {
				float bestVariance = MAX_AVG_VARIANCE;
				int bestMatch = -1;
				for (int startCode = CODE_START_A; startCode <= CODE_START_C; startCode++) {
					float variance = Reader::PatternMatchVariance(counters, CODE_PATTERNS[startCode], MAX_INDIVIDUAL_VARIANCE);
					if (variance < bestVariance) {
						bestVariance = variance;
						bestMatch = startCode;
					}
				}
				// Look for whitespace before start pattern, >= 50% of width of start pattern
				if (bestMatch >= 0 &&
					row.isRange(std::max(0, patternStart - (i - patternStart) / 2), patternStart, false)) {
					begin = patternStart;
					end = i;
					startCode = bestMatch;
					return ErrorStatus::NoError;
				}
				patternStart += counters[0] + counters[1];
				std::copy(counters.begin() + 2, counters.end(), counters.begin());
				counters[patternLength - 2] = 0;
				counters[patternLength - 1] = 0;
				counterPosition--;
			}
			else {
				counterPosition++;
			}
			counters[counterPosition] = 1;
			isWhite = !isWhite;
		}
	}
	return ErrorStatus::NotFound;
}

static ErrorStatus
DecodeCode(const BitArray &row, std::vector<int>& counters, int rowOffset, int& outCode)
{
	ErrorStatus status = Reader::RecordPattern(row, rowOffset, counters);
	if (StatusIsError(status)) {
		return status;
	}
	float bestVariance = MAX_AVG_VARIANCE; // worst variance we'll accept
	int bestMatch = -1;
	for (int d = 0; d < CODE_PATTERNS_LENGTH; d++) {
		auto& pattern = CODE_PATTERNS[d];
		float variance = Reader::PatternMatchVariance(counters, pattern, MAX_INDIVIDUAL_VARIANCE);
		if (variance < bestVariance) {
			bestVariance = variance;
			bestMatch = d;
		}
	}
	// TODO We're overlooking the fact that the STOP pattern has 7 values, not 6.
	if (bestMatch >= 0) {
		outCode = bestMatch;
		return ErrorStatus::NoError;
	}
	return ErrorStatus::NotFound;
}

Result
Code128Reader::decodeRow(int rowNumber, const BitArray& row, const DecodeHints* hints) const
{
	bool convertFNC1 = hints != nullptr && hints->getFlag(DecodeHint::ASSUME_GS1);

	int patternStart, patternEnd, startCode;
	ErrorStatus status = FindStartPattern(row, patternStart, patternEnd, startCode);
	if (StatusIsError(status)) {
		return Result(status);
	}

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
		return Result(ErrorStatus::FormatError);
	}

	bool done = false;
	bool isNextShifted = false;

	std::string result;
	result.reserve(20);

	int lastStart = patternStart;
	int nextStart = patternEnd;
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

		// Decode another code from image
		status = DecodeCode(row, counters, nextStart, code);
		if (StatusIsError(status)) {
			return Result(status);
		}

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

		// Advance to where the next code will to start
		lastStart = nextStart;
		for (int counter : counters) {
			nextStart += counter;
		}

		// Take care of illegal start codes
		switch (code) {
		case CODE_START_A:
		case CODE_START_B:
		case CODE_START_C:
			return Result(ErrorStatus::FormatError);
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
					if (convertFNC1) {
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
					if (convertFNC1) {
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
				result.push_back(code);
			}
			else {
				if (code != CODE_STOP) {
					lastCharacterWasPrintable = false;
				}
				switch (code) {
				case CODE_FNC_1:
					if (convertFNC1) {
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

	int lastPatternSize = nextStart - lastStart;

	// Check for ample whitespace following pattern, but, to do this we first need to remember that
	// we fudged decoding CODE_STOP since it actually has 7 bars, not 6. There is a black bar left
	// to read off. Would be slightly better to properly read. Here we just skip it:
	nextStart = row.getNextUnset(nextStart);
	if (!row.isRange(nextStart, std::min(row.size(), nextStart + (nextStart - lastStart) / 2), false)) {
		return Result(ErrorStatus::NotFound);
	}

	// Pull out from sum the value of the penultimate check code
	checksumTotal -= multiplier * lastCode;
	// lastCode is the checksum then:
	if (checksumTotal % 103 != lastCode) {
		return Result(ErrorStatus::ChecksumError);
	}

	// Need to pull out the check digits from string
	int resultLength = result.length();
	if (resultLength == 0) {
		// false positive
		return Result(ErrorStatus::NotFound);
	}

	// Only bother if the result had at least one character, and if the checksum digit happened to
	// be a printable character. If it was just interpreted as a control code, nothing to remove.
	if (resultLength > 0 && lastCharacterWasPrintable) {
		if (codeSet == CODE_CODE_C) {
			result.resize(resultLength - 2);
		}
		else {
			result.resize(resultLength - 1);
		}
	}

	float left = 0.5f * static_cast<float>(patternStart + patternEnd);
	float right = static_cast<float>(lastStart) + 0.5f * static_cast<float>(lastPatternSize);

	return Result(result, rawCodes, { ResultPoint(left, static_cast<float>(rowNumber)), ResultPoint(right, static_cast<float>(rowNumber)) }, BarcodeFormat::CODE_128);
}

} // OneD
} // ZXing
