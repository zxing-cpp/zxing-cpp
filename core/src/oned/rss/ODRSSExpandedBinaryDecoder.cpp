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

/*
* These authors would like to acknowledge the Spanish Ministry of Industry,
* Tourism and Trade, for the support in the project TSI020301-2008-2
* "PIRAmIDE: Personalizable Interactions with Resources on AmI-enabled
* Mobile Dynamic Environments", led by Treelogic
* ( http://www.treelogic.com/ ):
*
*   http://www.piramidepse.com/
*/

#include "oned/rss/ODRSSExpandedBinaryDecoder.h"
#include "oned/rss/ODRSSGenericAppIdDecoder.h"
#include "BitArray.h"
#include "DecodeStatus.h"
#include "ZXStrConvWorkaround.h"

#include <functional>

namespace ZXing {
namespace OneD {
namespace RSS {

static const int AI01_GTIN_SIZE = 40;

static void
AI01AppendCheckDigit(std::string& buffer, int currentPos)
{
	int checkDigit = 0;
	for (int i = 0; i < 13; i++) {
		int digit = buffer[i + currentPos] - '0';
		checkDigit += (i & 0x01) == 0 ? 3 * digit : digit;
	}

	checkDigit = 10 - (checkDigit % 10);
	if (checkDigit == 10) {
		checkDigit = 0;
	}
	buffer.append(std::to_string(checkDigit));
}

static void
AI01EncodeCompressedGtinWithoutAI(std::string& buffer, const BitArray& bits, int currentPos, int initialBufferPosition)
{
	for (int i = 0; i < 4; ++i) {
		int currentBlock = GenericAppIdDecoder::ExtractNumeric(bits, currentPos + 10 * i, 10);
		if (currentBlock / 100 == 0) {
			buffer.push_back('0');
		}
		if (currentBlock / 10 == 0) {
			buffer.push_back('0');
		}
		buffer.append(std::to_string(currentBlock));
	}
	AI01AppendCheckDigit(buffer, initialBufferPosition);
}

static void
AI01EncodeCompressedGtin(std::string& buffer, const BitArray& bits, int currentPos)
{
	buffer.append("(01)");
	int initialPosition = static_cast<int>(buffer.length());
	buffer.push_back('9');
	AI01EncodeCompressedGtinWithoutAI(buffer, bits, currentPos, initialPosition);
}

typedef const std::function<void(std::string&, int)> AddWeightCodeFunc;
typedef const std::function<int(int)> CheckWeightFunc;

static void AI01EncodeCompressedWeight(std::string& buffer, const BitArray& bits, int currentPos, int weightSize,
	const AddWeightCodeFunc& addWeightCode, const CheckWeightFunc& checkWeight)
{
	int originalWeightNumeric = GenericAppIdDecoder::ExtractNumeric(bits, currentPos, weightSize);
	addWeightCode(buffer, originalWeightNumeric);

	int weightNumeric = checkWeight(originalWeightNumeric);

	int currentDivisor = 100000;
	for (int i = 0; i < 5; ++i) {
		if (weightNumeric / currentDivisor == 0) {
			buffer.push_back('0');
		}
		currentDivisor /= 10;
	}
	buffer.append(std::to_string(weightNumeric));
}

/**
* @author Pablo Orduña, University of Deusto (pablo.orduna@deusto.es)
* @author Eduardo Castillejo, University of Deusto (eduardo.castillejo@deusto.es)
*/
static std::string
DecodeAI01AndOtherAIs(const BitArray& bits)
{
	static const int HEADER_SIZE = 1 + 1 + 2; //first bit encodes the linkage flag,
													  //the second one is the encodation method, and the other two are for the variable length
	std::string buffer;
	buffer.append("(01)");
	int initialGtinPosition = static_cast<int>(buffer.length());
	int firstGtinDigit = GenericAppIdDecoder::ExtractNumeric(bits, HEADER_SIZE, 4);
	buffer.append(std::to_string(firstGtinDigit));

	AI01EncodeCompressedGtinWithoutAI(buffer, bits, HEADER_SIZE + 4, initialGtinPosition);
	if (StatusIsOK(GenericAppIdDecoder::DecodeAllCodes(bits, HEADER_SIZE + 44, buffer))) {
		return buffer;
	}
	return std::string();
}

static std::string
DecodeAnyAI(const BitArray& bits)
{
	static const int HEADER_SIZE = 2 + 1 + 2;
	std::string buffer;
	if (StatusIsOK(GenericAppIdDecoder::DecodeAllCodes(bits, HEADER_SIZE, buffer))) {
		return buffer;
	}
	return std::string();
}

static std::string
DecodeAI013103(const BitArray& bits)
{
	static const int HEADER_SIZE = 4 + 1;
	static const int WEIGHT_SIZE = 15;

	if (bits.size() != HEADER_SIZE + AI01_GTIN_SIZE + WEIGHT_SIZE) {
		return std::string();
	}

	std::string buffer;
	AI01EncodeCompressedGtin(buffer, bits, HEADER_SIZE);
	AI01EncodeCompressedWeight(buffer, bits, HEADER_SIZE + AI01_GTIN_SIZE, WEIGHT_SIZE,
		// addWeightCode
		[](std::string& buf, int) { buf.append("(3103)"); },
		// checkWeight
		[](int weight) { return weight; });

	return buffer;
}

static std::string
DecodeAI01320x(const BitArray& bits)
{
	static const int HEADER_SIZE = 4 + 1;
	static const int WEIGHT_SIZE = 15;

	if (bits.size() != HEADER_SIZE + AI01_GTIN_SIZE + WEIGHT_SIZE) {
		return std::string();
	}

	std::string buffer;
	AI01EncodeCompressedGtin(buffer, bits, HEADER_SIZE);
	AI01EncodeCompressedWeight(buffer, bits, HEADER_SIZE + AI01_GTIN_SIZE, WEIGHT_SIZE,
		// addWeightCode
		[](std::string& buf, int weight) { buf.append(weight < 10000 ? "(3202)" : "(3203)"); },
		// checkWeight
		[](int weight) { return weight < 10000 ? weight : weight - 10000; });

	return buffer;
}

static std::string
DecodeAI01392x(const BitArray& bits)
{
	static const int HEADER_SIZE = 5 + 1 + 2;
	static const int LAST_DIGIT_SIZE = 2;

	if (bits.size() < HEADER_SIZE + AI01_GTIN_SIZE) {
		return std::string();
	}

	std::string buffer;
	AI01EncodeCompressedGtin(buffer, bits, HEADER_SIZE);

	int lastAIdigit = GenericAppIdDecoder::ExtractNumeric(bits, HEADER_SIZE + AI01_GTIN_SIZE, LAST_DIGIT_SIZE);
	buffer.append("(392");
	buffer.append(std::to_string(lastAIdigit));
	buffer.push_back(')');

	if (StatusIsOK(GenericAppIdDecoder::DecodeGeneralPurposeField(bits, HEADER_SIZE + AI01_GTIN_SIZE + LAST_DIGIT_SIZE, buffer))) {
		return buffer;
	}
	return std::string();
}

static std::string
DecodeAI01393x(const BitArray& bits)
{
	static const int HEADER_SIZE = 5 + 1 + 2;
	static const int LAST_DIGIT_SIZE = 2;
	static const int FIRST_THREE_DIGITS_SIZE = 10;

	if (bits.size() < HEADER_SIZE + AI01_GTIN_SIZE) {
		return std::string();
	}

	std::string buffer;
	AI01EncodeCompressedGtin(buffer, bits, HEADER_SIZE);

	int lastAIdigit = GenericAppIdDecoder::ExtractNumeric(bits, HEADER_SIZE + AI01_GTIN_SIZE, LAST_DIGIT_SIZE);

	buffer.append("(393");
	buffer.append(std::to_string(lastAIdigit));
	buffer.push_back(')');

	int firstThreeDigits = GenericAppIdDecoder::ExtractNumeric(bits, HEADER_SIZE + AI01_GTIN_SIZE + LAST_DIGIT_SIZE, FIRST_THREE_DIGITS_SIZE);
	if (firstThreeDigits / 100 == 0) {
		buffer.push_back('0');
	}
	if (firstThreeDigits / 10 == 0) {
		buffer.push_back('0');
	}
	buffer.append(std::to_string(firstThreeDigits));

	if (StatusIsOK(GenericAppIdDecoder::DecodeGeneralPurposeField(bits, HEADER_SIZE + AI01_GTIN_SIZE + LAST_DIGIT_SIZE + FIRST_THREE_DIGITS_SIZE, buffer))) {
		return buffer;
	}
	return std::string();
}

static std::string
DecodeAI013x0x1x(const BitArray& bits, const char* firstAIdigits, const char* dateCode)
{
	static const int HEADER_SIZE = 7 + 1;
	static const int WEIGHT_SIZE = 20;
	static const int DATE_SIZE = 16;

	if (bits.size() != HEADER_SIZE + AI01_GTIN_SIZE + WEIGHT_SIZE + DATE_SIZE) {
		return std::string();
	}

	std::string buffer;
	AI01EncodeCompressedGtin(buffer, bits, HEADER_SIZE);
	AI01EncodeCompressedWeight(buffer, bits, HEADER_SIZE + AI01_GTIN_SIZE, WEIGHT_SIZE,
		// addWeightCode
		[firstAIdigits](std::string& buf, int weight) {
			buf.push_back('(');
			buf.append(firstAIdigits);
			buf.append(std::to_string(weight / 100000));
			buf.push_back(')');
		},
		// checkWeight
		[](int weight) {
			return weight % 100000;
		});

	// encode compressed date
	int numericDate = GenericAppIdDecoder::ExtractNumeric(bits, HEADER_SIZE + AI01_GTIN_SIZE + WEIGHT_SIZE, DATE_SIZE);
	if (numericDate != 38400) {
		buffer.push_back('(');
		buffer.append(dateCode);
		buffer.push_back(')');

		int day = numericDate % 32;
		numericDate /= 32;
		int month = numericDate % 12 + 1;
		numericDate /= 12;
		int year = numericDate;

		if (year / 10 == 0) {
			buffer.push_back('0');
		}
		buffer.append(std::to_string(year));
		if (month / 10 == 0) {
			buffer.push_back('0');
		}
		buffer.append(std::to_string(month));
		if (day / 10 == 0) {
			buffer.push_back('0');
		}
		buffer.append(std::to_string(day));
	}

	return buffer;
}

std::string
ExpandedBinaryDecoder::Decode(const BitArray& bits)
{
	if (bits.get(1)) {
		return DecodeAI01AndOtherAIs(bits);
	}
	if (!bits.get(2)) {
		return DecodeAnyAI(bits);
	}

	int fourBitEncodationMethod = GenericAppIdDecoder::ExtractNumeric(bits, 1, 4);

	switch (fourBitEncodationMethod) {
	case 4: return DecodeAI013103(bits);
	case 5: return DecodeAI01320x(bits);
	}

	int fiveBitEncodationMethod = GenericAppIdDecoder::ExtractNumeric(bits, 1, 5);
	switch (fiveBitEncodationMethod) {
	case 12: return DecodeAI01392x(bits);
	case 13: return DecodeAI01393x(bits);
	}

	int sevenBitEncodationMethod = GenericAppIdDecoder::ExtractNumeric(bits, 1, 7);
	switch (sevenBitEncodationMethod) {
	case 56: return DecodeAI013x0x1x(bits, "310", "11");
	case 57: return DecodeAI013x0x1x(bits, "320", "11");
	case 58: return DecodeAI013x0x1x(bits, "310", "13");
	case 59: return DecodeAI013x0x1x(bits, "320", "13");
	case 60: return DecodeAI013x0x1x(bits, "310", "15");
	case 61: return DecodeAI013x0x1x(bits, "320", "15");
	case 62: return DecodeAI013x0x1x(bits, "310", "17");
	case 63: return DecodeAI013x0x1x(bits, "320", "17");
	}

	return std::string();
	//throw new IllegalStateException("unknown decoder: " + information);
}

} // RSS
} // OneD
} // ZXing
