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

#include "ODRSSGenericAppIdDecoder.h"

#include "BitArray.h"
#include "DecodeStatus.h"
#include "ODRSSFieldParser.h"

#include <limits>
#include <stdexcept>
#include <utility>

namespace ZXing::OneD::DataBar {

struct DecodedValue
{
	int newPosition = std::numeric_limits<int>::max();
	
	DecodedValue() = default;
	explicit DecodedValue(int np) : newPosition(np) {}
	bool isValid() const { return newPosition != std::numeric_limits<int>::max(); }
};

/**
* @author Pablo Orduña, University of Deusto (pablo.orduna@deusto.es)
* @author Eduardo Castillejo, University of Deusto (eduardo.castillejo@deusto.es)
*/
struct DecodedChar : public DecodedValue
{
	static const char FNC1 = '$'; // It's not in Alphanumeric neither in ISO/IEC 646 charset

	char value = '\0';

	DecodedChar() = default;
	DecodedChar(int np, char c) : DecodedValue(np), value(c) {}
	
	bool isFNC1() const { return value == FNC1; }
};

/**
* @author Pablo Orduña, University of Deusto (pablo.orduna@deusto.es)
* @author Eduardo Castillejo, University of Deusto (eduardo.castillejo@deusto.es)
*/
struct DecodedInformation : public DecodedValue
{
	std::string newString;
	int remainingValue = -1;

	DecodedInformation() = default;
	DecodedInformation(int np, std::string s) : DecodedValue(np), newString(std::move(s)) {}
	DecodedInformation(int np, std::string s, int r) : DecodedValue(np), newString(std::move(s)), remainingValue(r) {}

	bool isRemaining() const { return remainingValue >= 0; }
};

/**
* @author Pablo Orduña, University of Deusto (pablo.orduna@deusto.es)
* @author Eduardo Castillejo, University of Deusto (eduardo.castillejo@deusto.es)
*/
struct DecodedNumeric : public DecodedValue
{
	static const int FNC1 = 10;

	int firstDigit = 0;
	int secondDigit = 0;

	DecodedNumeric() = default;
	DecodedNumeric(int newPosition, int first, int second) : DecodedValue(newPosition), firstDigit(first), secondDigit(second) {
		if (firstDigit < 0 || firstDigit > 10 || secondDigit < 0 || secondDigit > 10) {
			*this = DecodedNumeric();
		}
	}

	int value() const {
		return firstDigit * 10 + secondDigit;
	}

	bool isFirstDigitFNC1() const {
		return firstDigit == FNC1;
	}

	bool isSecondDigitFNC1() const {
		return secondDigit == FNC1;
	}

	bool isAnyFNC1() const {
		return firstDigit == FNC1 || secondDigit == FNC1;
	}

private:
};

struct ParsingState
{
	enum State { NUMERIC, ALPHA, ISO_IEC_646 };

	int position = 0;
	State encoding = NUMERIC;
};

static bool
IsStillAlpha(const BitArray& bits, int pos)
{
	if (pos + 5 > bits.size()) {
		return false;
	}

	// We now check if it's a valid 5-bit value (0..9 and FNC1)
	int fiveBitValue = ToInt(bits, pos, 5);
	if (fiveBitValue >= 5 && fiveBitValue < 16) {
		return true;
	}

	if (pos + 6 > bits.size()) {
		return false;
	}

	int sixBitValue = ToInt(bits, pos, 6);
	return sixBitValue >= 16 && sixBitValue < 63; // 63 not included
}

static bool
IsStillIsoIec646(const BitArray& bits, int pos)
{
	if (pos + 5 > bits.size()) {
		return false;
	}

	int fiveBitValue = ToInt(bits, pos, 5);
	if (fiveBitValue >= 5 && fiveBitValue < 16) {
		return true;
	}

	if (pos + 7 > bits.size()) {
		return false;
	}

	int sevenBitValue = ToInt(bits, pos, 7);
	if (sevenBitValue >= 64 && sevenBitValue < 116) {
		return true;
	}

	if (pos + 8 > bits.size()) {
		return false;
	}

	int eightBitValue = ToInt(bits, pos, 8);
	return eightBitValue >= 232 && eightBitValue < 253;
}

static bool
IsStillNumeric(const BitArray& bits, int pos)
{
	// It's numeric if it still has 7 positions
	// and one of the first 4 bits is "1".
	if (pos + 7 > bits.size()) {
		return pos + 4 <= bits.size();
	}
	auto bitIter = bits.iterAt(pos);
	for (int i = 0; i < 4; ++i, ++bitIter) {
		if (*bitIter) {
			return true;
		}
	}
	return false;
}

static DecodedChar
DecodeAlphanumeric(const BitArray& bits, int pos)
{
	int fiveBitValue = ToInt(bits, pos, 5);
	if (fiveBitValue == 15) {
		return DecodedChar(pos + 5, DecodedChar::FNC1);
	}

	if (fiveBitValue >= 5 && fiveBitValue < 15) {
		return DecodedChar(pos + 5, (char)('0' + fiveBitValue - 5));
	}

	int sixBitValue = ToInt(bits, pos, 6);

	if (sixBitValue >= 32 && sixBitValue < 58) {
		return DecodedChar(pos + 6, (char)(sixBitValue + 33));
	}

	if (sixBitValue < 58 || sixBitValue > 62)
		throw std::runtime_error("Decoding invalid alphanumeric value");

	constexpr char const* lut58to62 = R"(*,-./)";
	char c = lut58to62[sixBitValue - 58];

	return DecodedChar(pos + 6, c);
}

static bool
IsAlphaTo646ToAlphaLatch(const BitArray& bits, int pos)
{
	if (pos + 1 > bits.size()) {
		return false;
	}
	for (int i = 0; i < 5 && i + pos < bits.size(); ++i) {
		if (i == 2) {
			if (!bits.get(pos + 2)) {
				return false;
			}
		}
		else if (bits.get(pos + i)) {
			return false;
		}
	}
	return true;
}

static bool
IsAlphaOr646ToNumericLatch(const BitArray& bits, int pos)
{
	// Next is alphanumeric if there are 3 positions and they are all zeros
	if (pos + 3 > bits.size()) {
		return false;
	}
	auto bitIter = bits.iterAt(pos);
	for (int i = 0; i < 3; ++i, ++bitIter) {
		if (*bitIter) {
			return false;
		}
	}
	return true;
}

static bool
IsNumericToAlphaNumericLatch(const BitArray& bits, int pos)
{
	// Next is alphanumeric if there are 4 positions and they are all zeros, or
	// if there is a subset of this just before the end of the symbol
	if (pos + 1 > bits.size()) {
		return false;
	}

	auto bitIter = bits.iterAt(pos);
	for (int i = 0; i < 4 && i + pos < bits.size(); ++i, ++bitIter) {
		if (*bitIter) {
			return false;
		}
	}
	return true;
}

static DecodedInformation
ParseAlphaBlock(const BitArray& bits, ParsingState& state, std::string& buffer)
{
	while (IsStillAlpha(bits, state.position)) {
		DecodedChar alpha = DecodeAlphanumeric(bits, state.position);
		state.position = alpha.newPosition;

		if (alpha.isFNC1()) {
			// Allow for some generators incorrectly placing a numeric latch "000" after an FNC1
			if (state.position + 7 < bits.size() && ToInt(bits, state.position, 7) < 8) {
				state.position += 3;
			}
			state.encoding = ParsingState::NUMERIC; // FNC1 latches to numeric encodation
			return DecodedInformation(state.position, buffer); //end of the char block
		}
		buffer.push_back(alpha.value);
	}

	if (IsAlphaOr646ToNumericLatch(bits, state.position)) {
		state.position += 3;
		state.encoding = ParsingState::NUMERIC;
	}
	else if (IsAlphaTo646ToAlphaLatch(bits, state.position)) {
		if (state.position + 5 < bits.size()) {
			state.position += 5;
		}
		else {
			state.position = bits.size();
		}
		state.encoding = ParsingState::ISO_IEC_646;
	}
	return DecodedInformation();
}

static DecodedChar
DecodeIsoIec646(const BitArray& bits, int pos)
{
	int fiveBitValue = ToInt(bits,pos, 5);
	if (fiveBitValue == 15) {
		return DecodedChar(pos + 5, DecodedChar::FNC1);
	}

	if (fiveBitValue >= 5 && fiveBitValue < 15) {
		return DecodedChar(pos + 5, (char)('0' + fiveBitValue - 5));
	}

	int sevenBitValue = ToInt(bits, pos, 7);

	if (sevenBitValue >= 64 && sevenBitValue < 90) {
		return DecodedChar(pos + 7, (char)(sevenBitValue + 1));
	}

	if (sevenBitValue >= 90 && sevenBitValue < 116) {
		return DecodedChar(pos + 7, (char)(sevenBitValue + 7));
	}

	int eightBitValue = ToInt(bits, pos, 8);
	if (eightBitValue < 232 || eightBitValue > 252)
		throw std::runtime_error("Decoding invalid ISO-IEC-646 value");

	constexpr char const* lut232to252 = R"(!"%&'()*+,-./:;<=>?_ )";
	char c = lut232to252[eightBitValue - 232];

	return DecodedChar(pos + 8, c);
}

static DecodedInformation
ParseIsoIec646Block(const BitArray& bits, ParsingState& state, std::string& buffer)
{
	while (IsStillIsoIec646(bits, state.position)) {
		DecodedChar iso = DecodeIsoIec646(bits, state.position);
		state.position = iso.newPosition;
		if (iso.isFNC1()) {
			// Allow for some generators incorrectly placing a numeric latch "000" after an FNC1
			if (state.position + 7 < bits.size() && ToInt(bits, state.position, 7) < 8) {
				state.position += 3;
			}
			state.encoding = ParsingState::NUMERIC; // FNC1 latches to numeric encodation
			return DecodedInformation(state.position, buffer);
		}
		buffer.push_back(iso.value);
	}

	if (IsAlphaOr646ToNumericLatch(bits, state.position)) {
		state.position += 3;;
		state.encoding = ParsingState::NUMERIC;
	}
	else if (IsAlphaTo646ToAlphaLatch(bits, state.position)) {
		if (state.position + 5 < bits.size()) {
			state.position += 5;
		}
		else {
			state.position = bits.size();
		}
		state.encoding = ParsingState::ALPHA;
	}
	return DecodedInformation();
}

static DecodedNumeric
DecodeNumeric(const BitArray& bits, int pos)
{
	if (pos + 7 > bits.size()) {
		int numeric = ToInt(bits, pos, 4);
		if (numeric == 0) {
			return DecodedNumeric(bits.size(), DecodedNumeric::FNC1, DecodedNumeric::FNC1);
		}
		return DecodedNumeric(bits.size(), numeric - 1, DecodedNumeric::FNC1);
	}
	int numeric = ToInt(bits, pos, 7);
	int digit1 = (numeric - 8) / 11;
	int digit2 = (numeric - 8) % 11;

	return DecodedNumeric(pos + 7, digit1, digit2);
}

static DecodedInformation
ParseNumericBlock(const BitArray& bits, ParsingState& state, std::string& buffer)
{
	while (IsStillNumeric(bits, state.position)) {
		DecodedNumeric numeric = DecodeNumeric(bits, state.position);
		if (!numeric.isValid())
			break;
		state.position = numeric.newPosition;

		if (numeric.isFirstDigitFNC1()) {
			DecodedInformation information;
			if (numeric.isSecondDigitFNC1()) {
				return DecodedInformation(state.position, buffer);
			}
			else {
				return DecodedInformation(state.position, buffer, numeric.secondDigit);
			}
		}

		buffer.append(std::to_string(numeric.firstDigit));
		if (numeric.isSecondDigitFNC1()) {
			return DecodedInformation(state.position, buffer);
		}
		buffer.append(std::to_string(numeric.secondDigit));
	}

	if (IsNumericToAlphaNumericLatch(bits, state.position)) {
		state.encoding = ParsingState::ALPHA;
		state.position += 4;
	}
	return DecodedInformation();
}


static DecodedInformation
ParseBlocks(const BitArray& bits, ParsingState& state, std::string& buffer)
{
	while (true) {
		int initialPosition = state.position;
		auto result =
			state.encoding == ParsingState::ALPHA ?
			ParseAlphaBlock(bits, state, buffer) :
			(state.encoding == ParsingState::ISO_IEC_646 ?
				ParseIsoIec646Block(bits, state, buffer) :
				// else
				ParseNumericBlock(bits, state, buffer));
		if (result.isValid() || initialPosition == state.position)
		{
			return result;
		}
	}
}

static DecodedInformation
DoDecodeGeneralPurposeField(ParsingState& state, const BitArray& bits, std::string prefix)
{
	DecodedInformation lastDecoded = ParseBlocks(bits, state, prefix);
	if (lastDecoded.isValid() && lastDecoded.isRemaining()) {
		return DecodedInformation(state.position, prefix, lastDecoded.remainingValue);
	}
	return DecodedInformation(state.position, prefix);
}

DecodeStatus
DecodeAppIdGeneralPurposeField(const BitArray& bits, int& pos, int& remainingValue, std::string& result)
{
	try
	{
		ParsingState state;
		state.position = pos;
		DecodedInformation info = DoDecodeGeneralPurposeField(state, bits, std::string());
		result += info.newString;
		pos = state.position;
		remainingValue = info.remainingValue;
		return DecodeStatus::NoError;
	}
	catch (const std::exception &)
	{
	}
	return DecodeStatus::FormatError;
}

DecodeStatus
DecodeAppIdAllCodes(const BitArray& bits, int pos, int remainingValue, std::string& result)
{
	try
	{
		ParsingState state;
		std::string remaining;
		if (remainingValue != -1) {
			remaining = std::to_string(remainingValue);
		}
		while (true) {
			state.position = pos;
			DecodedInformation info = DoDecodeGeneralPurposeField(state, bits, remaining);
			std::string parsedFields;
			auto status = ParseFieldsInGeneralPurpose(info.newString, parsedFields);
			if (StatusIsError(status)) {
				if (result.empty() && remaining.empty()){
					result = info.newString;
					return DecodeStatus::NoError;
				} else
					return status;
			}
			result += parsedFields;
			if (info.isRemaining()) {
				remaining = std::to_string(info.remainingValue);
			}
			else {
				remaining.clear();
			}

			if (pos == info.newPosition) {// No step forward!
				break;
			}
			pos = info.newPosition;
		};
		return DecodeStatus::NoError;
	}
	catch (const std::exception &)
	{
	}
	return DecodeStatus::FormatError;
}

} // namespace ZXing::OneD::DataBar
