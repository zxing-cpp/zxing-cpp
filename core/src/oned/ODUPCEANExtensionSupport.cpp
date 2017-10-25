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

/**
* @see UPCEANExtension2Support
*/
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

	static int
	DetermineCheckDigit(int lgPatternFound)
	{
		static const int CHECK_DIGIT_ENCODINGS[] = {
			0x18, 0x14, 0x12, 0x11, 0x0C, 0x06, 0x03, 0x0A, 0x09, 0x05
		};
		for (int d = 0; d < 10; d++) {
			if (lgPatternFound == CHECK_DIGIT_ENCODINGS[d]) {
				return d;
			}
		}
		return -1;
	}

	static BitArray::Range
	DecodeMiddle(const BitArray& row, BitArray::Iterator begin, std::string& resultString)
	{
		BitArray::Range next = {begin, row.end()};
		const BitArray::Range notFound = {begin, begin};
		int lgPatternFound = 0;

		for (int x = 0; x < 5 && next; x++) {
			int bestMatch = UPCEANReader::DecodeDigit(&next, UPCEANCommon::L_AND_G_PATTERNS, &resultString);
			if (bestMatch == -1)
				return notFound;

			if (bestMatch >= 10) {
				lgPatternFound |= 1 << (4 - x);
			}
			if (x != 4) {
				// Read off separator if not last
				next.begin = row.getNextSet(next.begin);
				next.begin = row.getNextUnset(next.begin);
			}
		}

		if (resultString.length() != 5) {
			return notFound;
		}

		int checkDigit = DetermineCheckDigit(lgPatternFound);
		if (checkDigit < 0 || ExtensionChecksum(resultString) != checkDigit) {
			return notFound;
		}

		return {begin, next.begin};
	}

	static std::string
	ParseExtension5String(const std::string& raw)
	{
		std::string currency;
		switch (raw.front()) {
		case '0':
			currency = "\xa3";
			break;
		case '5':
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

	/**
	* @param raw raw content of extension
	* @return formatted interpretation of raw content as a {@link Map} mapping
	*  one {@link ResultMetadataType} to appropriate value, or {@code null} if not known
	*/
	static void
	ParseExtensionString(const std::string& raw, Result& result) {
		if (raw.length() == 5) {
			std::string value = ParseExtension5String(raw);
			if (!value.empty()) {
				result.metadata().put(ResultMetadata::SUGGESTED_PRICE, TextDecoder::FromLatin1(value));
			}
		}
	}


	static Result
	DecodeRow(int rowNumber, const BitArray& row, int extStartRangeBegin, int extStartRangeEnd)
	{
		std::string resultString;
		auto range = DecodeMiddle(row, row.iterAt(extStartRangeEnd), resultString);
		if (!range)
			return Result(DecodeStatus::NotFound);

		float y = static_cast<float>(rowNumber);
		float x1 = 0.5f * static_cast<float>(extStartRangeBegin + extStartRangeEnd);
		float x2 = static_cast<float>(range.end - row.begin());
		Result result(TextDecoder::FromLatin1(resultString), ByteArray(), { ResultPoint(x1, y), ResultPoint(x2, y) }, BarcodeFormat::UPC_EAN_EXTENSION);
		ParseExtensionString(resultString, result);
		return result;
	}

} // UPCEANExtension5Support

namespace UPCEANExtension2Support
{
	static BitArray::Range
	DecodeMiddle(const BitArray& row, BitArray::Iterator begin, std::string& resultString)
	{
		BitArray::Range next = {begin, row.end()};
		const BitArray::Range notFound = {begin, begin};
		int lgPatternFound = 0;

		for (int x = 0; x < 2 && next; x++) {
			int bestMatch = UPCEANReader::DecodeDigit(&next, UPCEANCommon::L_AND_G_PATTERNS, &resultString);
			if (bestMatch == -1)
				return notFound;

			if (bestMatch >= 10) {
				lgPatternFound |= 1 << (1 - x);
			}
			if (x != 1) {
				// Read off separator if not last
				next.begin = row.getNextSet(next.begin);
				next.begin = row.getNextUnset(next.begin);
			}
		}

		if (resultString.length() != 2) {
			return notFound;
		}

		if (std::stoi(resultString) % 4 != lgPatternFound) {
			return notFound;
		}

		return {begin, next.begin};
	}

	static Result
	DecodeRow(int rowNumber, const BitArray& row, int extStartRangeBegin, int extStartRangeEnd)
	{
		std::string resultString;;
		auto range = DecodeMiddle(row, row.iterAt(extStartRangeEnd), resultString);
		if (!range)
			return Result(DecodeStatus::NotFound);

		float y = static_cast<float>(rowNumber);
		float x1 = 0.5f * static_cast<float>(extStartRangeBegin + extStartRangeEnd);
		float x2 = static_cast<float>(range.end - row.begin());

		Result result(TextDecoder::FromLatin1(resultString), ByteArray(), { ResultPoint(x1, y), ResultPoint(x2, y) }, BarcodeFormat::UPC_EAN_EXTENSION);
		if (resultString.length() == 2) {
			result.metadata().put(ResultMetadata::ISSUE_NUMBER, std::stoi(resultString));
		}
		return result;
	}

} // UPCEANExtension2Support

static const std::array<int, 3> EXTENSION_START_PATTERN = { 1,1,2 };

Result
UPCEANExtensionSupport::DecodeRow(int rowNumber, const BitArray& row, int rowOffset)
{
	auto extStartRange = UPCEANReader::FindGuardPattern(row, row.iterAt(rowOffset), false, EXTENSION_START_PATTERN);
	if (!extStartRange)
		return Result(DecodeStatus::NotFound);

	Result result = UPCEANExtension5Support::DecodeRow(rowNumber, row, extStartRange.begin - row.begin(), extStartRange.end - row.begin());
	if (!result.isValid()) {
		result = UPCEANExtension2Support::DecodeRow(rowNumber, row, extStartRange.begin - row.begin(), extStartRange.end - row.begin());
	}
	return result;
}

} // OneD
} // ZXing
