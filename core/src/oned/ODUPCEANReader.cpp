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

#include "oned/ODUPCEANReader.h"
#include "oned/ODUPCEANExtensionSupport.h"
#include "oned/ODEANManufacturerOrgSupport.h"
#include "oned/ODUPCEANCommon.h"
#include "Result.h"
#include "BitArray.h"
#include "DecodeHints.h"
#include "TextDecoder.h"
#include "ZXContainerAlgorithms.h"

#include <algorithm>

namespace ZXing {

namespace OneD {

UPCEANReader::UPCEANReader(const DecodeHints& hints) :
	_allowedExtensions(hints.allowedEanExtensions())
{
}

/**
* @param row row of black/white values to search
* @param rowOffset position to start search
* @param whiteFirst if true, indicates that the pattern specifies white/black/white/...
* pixel counts, otherwise, it is interpreted as black/white/black/...
* @param pattern pattern of counts of number of black and white pixels that are being
* searched for as a pattern
* @return start/end horizontal offset of guard pattern, as an array of two ints
* @throws NotFoundException if pattern is not found
*/
BitArray::Range
UPCEANReader::FindGuardPattern(const BitArray& row, BitArray::Iterator begin, bool whiteFirst, const int* pattern, size_t length)
{
	using Counters = std::vector<int>;
	Counters counters(length, 0);

	return RowReader::FindPattern(
		row.getNextSetTo(begin, !whiteFirst), row.end(), counters,
		[pattern, length](BitArray::Iterator, BitArray::Iterator, const Counters& cntrs) {
			return RowReader::PatternMatchVariance(cntrs.data(), pattern, length, MAX_INDIVIDUAL_VARIANCE) < MAX_AVG_VARIANCE;
		});
}

BitArray::Range
UPCEANReader::FindStartGuardPattern(const BitArray& row)
{
#if 0
	// this is the way the upstream JAVA project implemented it: scan for a
	// pattern 111 and look for a quite-zone of 3 after the fact. If there is no
	// quite zone, then skip the whole pattern und start over. This fails to identify
	// the valid start guard given the input: 49333... (falsepositives-2/14.png
	// line 471)

	BitArray::Range range{row.begin(), row.begin()};
	while(range.end != row.end()) {
		range = FindGuardPattern(row, range.end, false, UPCEANCommon::START_END_PATTERN.data(),
								 UPCEANCommon::START_END_PATTERN.size());

		// Make sure there is a quiet zone at least as big as the start pattern before the barcode.
		// If this check would run off the left edge of the image, do not accept this barcode,
		// as it is very likely to be a false positive.
		if (row.hasQuiteZone(range.begin, -range.size(), false))
			return range;
	}
	return {row.end(), row.end()};
#else
	// this is the 'right' way to do it: scan for a pattern of the form 3111, where 3 is the quitezone
	const auto& pattern = UPCEANCommon::START_END_PATTERN;
	using Counters = std::decay<decltype(pattern)>::type;
	Counters counters{};

	return RowReader::FindPattern(
		row.getNextSet(row.begin()), row.end(), counters,
		[&row, &pattern](BitArray::Iterator begin, BitArray::Iterator end, const Counters& cntrs) {
			if (!(RowReader::PatternMatchVariance(cntrs, pattern, MAX_INDIVIDUAL_VARIANCE) < MAX_AVG_VARIANCE))
				return false;

			// Make sure there is a quiet zone at least as big as the start pattern before the barcode.
			// If this check would run off the left edge of the image, do not accept this barcode,
			// as it is very likely to be a false positive.
			return row.hasQuiteZone(begin, -(end - begin), false);
		});
#endif
}

Result
UPCEANReader::decodeRow(int rowNumber, const BitArray& row, std::unique_ptr<DecodingState>&) const
{
	auto range = FindStartGuardPattern(row);
	if (!range)
		return Result(DecodeStatus::NotFound);

	return decodeRow(rowNumber, row, range);
}

BitArray::Range
UPCEANReader::decodeEnd(const BitArray& row, BitArray::Iterator begin) const
{
	return FindGuardPattern(row, begin, false, UPCEANCommon::START_END_PATTERN);
}

Result
UPCEANReader::decodeRow(int rowNumber, const BitArray& row, BitArray::Range startGuard) const
{
	std::string result;
	result.reserve(20);
	auto range = decodeMiddle(row, startGuard.end, result);
	if (!range)
		return Result(DecodeStatus::NotFound);

	auto stopGuard = decodeEnd(row, range.end);
	if (!stopGuard)
		return Result(DecodeStatus::NotFound);

	// Make sure there is a quiet zone at least as big as the end pattern after the barcode. The
	// spec might want more whitespace, but in practice this is the maximum we can count on.
	if (!row.hasQuiteZone(stopGuard.end, stopGuard.size(), false))
		return Result(DecodeStatus::NotFound);

	// UPC/EAN should never be less than 8 chars anyway
	if (result.length() < 8) {
		return Result(DecodeStatus::FormatError);
	}
	auto status = checkChecksum(result);
	if (StatusIsError(status))
		return Result(status);

	float left = (startGuard.begin - row.begin()) + 0.5f * startGuard.size();
	float right = (stopGuard.begin - row.begin()) + 0.5f * stopGuard.size();
	BarcodeFormat format = expectedFormat();
	float ypos = static_cast<float>(rowNumber);

	Result decodeResult(TextDecoder::FromLatin1(result), ByteArray(), { ResultPoint(left, ypos), ResultPoint(right, ypos) }, format);
	int extensionLength = 0;
	Result extensionResult = UPCEANExtensionSupport::DecodeRow(rowNumber, row, stopGuard.end - row.begin());
	if (extensionResult.isValid())
	{
		decodeResult.metadata().put(ResultMetadata::UPC_EAN_EXTENSION, extensionResult.text());
		decodeResult.metadata().putAll(extensionResult.metadata());
		decodeResult.addResultPoints(extensionResult.resultPoints());
		extensionLength = static_cast<int>(extensionResult.text().length());
	}

	if (!_allowedExtensions.empty() && !Contains(_allowedExtensions, extensionLength)) {
		return Result(DecodeStatus::NotFound);
	}

	if (format == BarcodeFormat::EAN_13 || format == BarcodeFormat::UPC_A) {
		std::string countryID = EANManufacturerOrgSupport::LookupCountryIdentifier(result);
		if (!countryID.empty()) {
			decodeResult.metadata().put(ResultMetadata::POSSIBLE_COUNTRY, TextDecoder::FromLatin1(countryID));
		}
	}

	return decodeResult;
}

/**
* Computes the UPC/EAN checksum on a string of digits, and reports
* whether the checksum is correct or not.
*
* @param s string of digits to check
* @return true iff string of digits passes the UPC/EAN checksum algorithm
* @throws FormatException if the string does not contain only digits
*/
DecodeStatus
UPCEANReader::checkChecksum(const std::string& s) const
{
	int length = static_cast<int>(s.length());
	if (length == 0) {
		return DecodeStatus::ChecksumError;
	}

	int sum = 0;
	for (int i = length - 2; i >= 0; i -= 2) {
		int digit = s[i] - '0';
		if (digit < 0 || digit > 9) {
			return DecodeStatus::FormatError;
		}
		sum += digit;
	}
	sum *= 3;
	for (int i = length - 1; i >= 0; i -= 2) {
		int digit = s[i] - '0';
		if (digit < 0 || digit > 9) {
			return DecodeStatus::FormatError;
		}
		sum += digit;
	}
	return sum % 10 == 0 ? DecodeStatus::NoError : DecodeStatus::ChecksumError;
}

} // OneD
} // ZXing
