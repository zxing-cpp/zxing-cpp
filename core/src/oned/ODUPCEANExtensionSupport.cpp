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

#include "oned/ODUPCEANExtensionSupport.h"
#include "oned/ODUPCEANReader.h"
#include "oned/ODUPCEANCommon.h"
#include "Result.h"
#include "BitArray.h"
#include "TextDecoder.h"
#include "ZXContainerAlgorithms.h"
#include "ZXStrConvWorkaround.h"

#include <array>
#include <sstream>
#include <iomanip>

namespace ZXing {
namespace OneD {

namespace UPCEANExtension5Support
{
	static int
	ExtensionChecksum(const std::string& s)
	{
		int length = static_cast<int>(s.length());
		int sum = 0;
		for (int i = length - 2; i >= 0; i -= 2) {
			sum += (int)s[i] - (int) '0';
		}
		sum *= 3;
		for (int i = length - 1; i >= 0; i -= 2) {
			sum += (int)s[i] - (int) '0';
		}
		sum *= 3;
		return sum % 10;
	}

	static std::string
	ParseExtension5String(const std::string& raw)
	{
		std::string currency;
		switch (raw.front()) {
		case '0':
		case '1':
			currency = "\xa3";
			break;
		case '3': // AUS
		case '4': // NZ
		case '5': // US
		case '6': // CA
			currency = "$";
			break;
		case '9':
			// Reference: http://www.jollytech.com
			if (raw == "90000") {
				// No suggested retail price
				return std::string();
			}
			if (raw == "99991") {
				// Complementary
				return "0.00";
			}
			if (raw == "99990") {
				return "Used";
			}
			// Otherwise... unknown currency?
			currency = "";
			break;
		default:
			currency = "";
			break;
		}
		int rawAmount = std::stoi(raw.substr(1));
		std::stringstream buf;
		buf << currency << std::fixed << std::setprecision(2) << (float(rawAmount) / 100);
		return buf.str();
	}

} // UPCEANExtension5Support


static std::string
DecodeMiddle(BitArray::Range* next_, int N)
{
	assert(N == 2 || N == 5);
	int lgPatternFound = 0;
	auto next = *next_;
	std::string resultString;

	for (int x = 0; x < N; x++) {
		int bestMatch = UPCEANReader::DecodeDigit(&next, UPCEANCommon::L_AND_G_PATTERNS, &resultString);
		if (bestMatch == -1)
			return {};

		// Read off separator if not last
		if (x != N - 1 && !UPCEANReader::ReadGuardPattern(&next, std::array<int, 2>{1, 1}))
			return {};

		if (bestMatch >= 10)
			lgPatternFound |= 1 << (N - 1 - x);
	}

	if (N == 2) {
		if (std::stoi(resultString) % 4 != lgPatternFound)
			return {};
	} else {
		constexpr int CHECK_DIGIT_ENCODINGS[] = {0x18, 0x14, 0x12, 0x11, 0x0C, 0x06, 0x03, 0x0A, 0x09, 0x05};
		if (UPCEANExtension5Support::ExtensionChecksum(resultString) != IndexOf(CHECK_DIGIT_ENCODINGS, lgPatternFound))
			return {};
	}

	*next_ = next;
	return resultString;
}

static const std::array<int, 3> EXTENSION_START_PATTERN = { 1,1,2 };

Result
UPCEANExtensionSupport::DecodeRow(int rowNumber, const BitArray& row, BitArray::Iterator begin)
{
	BitArray::Range next = {row.getNextSet(begin), row.end()};

	int xStart = next.begin - row.begin();

	if (!UPCEANReader::ReadGuardPattern(&next, EXTENSION_START_PATTERN))
		return Result(DecodeStatus::NotFound);

	auto resultString = DecodeMiddle(&next, 5);
	if (resultString.empty())
		resultString = DecodeMiddle(&next, 2);

	if (resultString.empty())
		return Result(DecodeStatus::NotFound);

	int xStop = next.begin - row.begin() - 1;

	Result result(resultString, rowNumber, xStart, xStop, BarcodeFormat::UPC_EAN_EXTENSION);

	if (resultString.size() == 2) {
		result.metadata().put(ResultMetadata::ISSUE_NUMBER, std::stoi(resultString));
	} else {
		std::string price = UPCEANExtension5Support::ParseExtension5String(resultString);
		if (!price.empty())
			result.metadata().put(ResultMetadata::SUGGESTED_PRICE, TextDecoder::FromLatin1(price));
	}

	return result;
}

} // OneD
} // ZXing
