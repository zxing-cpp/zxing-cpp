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

#include "ODCode128Reader.h"
#include "ODCode128Patterns.h"
#include "Result.h"
#include "BitArray.h"
#include "DecodeHints.h"
#include "ZXContainerAlgorithms.h"
#include "ZXStrConvWorkaround.h"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

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

static const int CODE_START_A = 103;
static const int CODE_START_B = 104;
static const int CODE_START_C = 105;
static const int CODE_STOP = 106;

class Raw2TxtDecoder
{
	int codeSet = 0;
	bool _convertFNC1 = false;
	std::string txt;
	size_t lastTxtSize = 0;

	bool fnc4All = false;
	bool fnc4Next = false;
	bool shift = false;

	void fnc1()
	{
		if (_convertFNC1) {
			if (txt.empty()) {
				// GS1 specification 5.4.3.7. and 5.4.6.4. If the first char after the start code
				// is FNC1 then this is GS1-128. We add the symbology identifier.
				txt.append("]C1");
			}
			else {
				// GS1 specification 5.4.7.5. Every subsequent FNC1 is returned as ASCII 29 (GS)
				txt.push_back((char)29);
			}
		}
	};

public:
	Raw2TxtDecoder(int startCode, bool convertFNC1) : codeSet(204 - startCode), _convertFNC1(convertFNC1)
	{
		txt.reserve(20);
	}

	bool decode(int code)
	{
		lastTxtSize = txt.size();

		if (codeSet == CODE_CODE_C) {
			if (code < 100) {
				if (code < 10)
					txt.push_back('0');
				txt.append(std::to_string(code));
			} else if (code == CODE_FNC_1) {
				fnc1();
			} else {
				codeSet = code; // CODE_A / CODE_B
			}
		} else { // codeSet A or B
			bool unshift = shift;

			switch (code) {
			case CODE_FNC_1: fnc1(); break;
			case CODE_FNC_2:
			case CODE_FNC_3:
				// do nothing?
				break;
			case CODE_SHIFT:
				if (shift)
					return false; // two shifts in a row make no sense
				shift = true;
				codeSet = codeSet == CODE_CODE_A ? CODE_CODE_B : CODE_CODE_A;
				break;
			case CODE_CODE_A:
			case CODE_CODE_B:
				if (codeSet == code) {
					// FNC4
					if (fnc4Next)
						fnc4All = !fnc4All;
					fnc4Next = !fnc4Next;
				} else {
					codeSet = code;
				}
				break;
			case CODE_CODE_C: codeSet = CODE_CODE_C; break;

			default: {
				// code < 96 at this point
				int offset;
				if (codeSet == CODE_CODE_A && code >= 64)
					offset = fnc4All == fnc4Next ? -64 : +64;
				else
					offset = fnc4All == fnc4Next ? ' ' : ' ' + 128;
				txt.push_back((char)(code + offset));
				fnc4Next = false;
				break;
			}
			}

			// Unshift back to another code set if we were shifted
			if (unshift) {
				codeSet = codeSet == CODE_CODE_A ? CODE_CODE_B : CODE_CODE_A;
				shift = false;
			}
		}

		return true;
	}

	std::string text() const
	{
		// Need to pull out the check digit(s) from string (if the checksum code happened to
		// be a printable character).
		return txt.substr(0, lastTxtSize);
	}
};

static BitArray::Range
FindStartPattern(const BitArray& row, int* startCode)
{
	assert(startCode != nullptr);

	using Counters = std::vector<int>;
	Counters counters(Code128::CODE_PATTERNS[CODE_START_A].size());

	return RowReader::FindPattern(
	    row.getNextSet(row.begin()), row.end(), counters,
	    [&row, startCode](BitArray::Iterator begin, BitArray::Iterator end, const Counters& counters) {
			// Look for whitespace before start pattern, >= 50% of width of start pattern
			if (!row.hasQuiteZone(begin, static_cast<int>(-(end-begin)/2)))
				return false;
			float bestVariance = MAX_AVG_VARIANCE;
			for (int code : {CODE_START_A, CODE_START_B, CODE_START_C}) {
			    float variance =
			        RowReader::PatternMatchVariance(counters, Code128::CODE_PATTERNS[code], MAX_INDIVIDUAL_VARIANCE);
			    if (variance < bestVariance) {
				    bestVariance = variance;
				    *startCode = code;
			    }
		    }
		    return bestVariance < MAX_AVG_VARIANCE;
	    });
}

Code128Reader::Code128Reader(const DecodeHints& hints) :
	_convertFNC1(hints.assumeGS1())
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

	int xStart = static_cast<int>(range.begin - row.begin());
	ByteArray rawCodes;
	rawCodes.reserve(20);
	std::vector<int> counters(6, 0);
	Raw2TxtDecoder raw2txt(startCode, _convertFNC1);

	rawCodes.push_back(static_cast<uint8_t>(startCode));

	while (true) {
		range = RecordPattern(range.end, row.end(), counters);
		if (!range)
			return Result(DecodeStatus::NotFound);

		// Decode another code from image
		int code = RowReader::DecodeDigit(counters, Code128::CODE_PATTERNS, MAX_AVG_VARIANCE, MAX_INDIVIDUAL_VARIANCE);
		if (code == -1)
			return Result(DecodeStatus::NotFound);
		if (code == CODE_STOP)
			break;
		if (code >= CODE_START_A)
			return Result(DecodeStatus::FormatError);
		if (!raw2txt.decode(code))
			return Result(DecodeStatus::FormatError);

		rawCodes.push_back(static_cast<uint8_t>(code));
	}

	// Check for ample whitespace following pattern, but, to do this we first need to remember that
	// we fudged decoding CODE_STOP since it actually has 7 bars, not 6. There is a black bar left
	// to read off. Would be slightly better to properly read. Here we just skip it:
	range.end = row.getNextUnset(range.end);
	if (rawCodes.empty() || !row.hasQuiteZone(range.end, range.size() / 2))
		return Result(DecodeStatus::NotFound);

	int checksum = rawCodes.front();
	for (int i = 1; i < Size(rawCodes) - 1; ++i)
		checksum += i * rawCodes[i];
	// the last code is the checksum:
	if (checksum % 103 != rawCodes.back()) {
		return Result(DecodeStatus::ChecksumError);
	}

	int xStop = static_cast<int>(range.end - row.begin() - 1);
	return Result(raw2txt.text(), rowNumber, xStart, xStop, BarcodeFormat::CODE_128, std::move(rawCodes));
}

}

} // OneD
} // ZXing
