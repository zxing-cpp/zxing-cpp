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
			// Make sure there is a quiet zone at least as big as the start pattern before the barcode.
			// If this check would run off the left edge of the image, do not accept this barcode,
			// as it is very likely to be a false positive.
			return row.hasQuiteZone(begin, -(end - begin), false) &&
				RowReader::PatternMatchVariance(cntrs, pattern, MAX_INDIVIDUAL_VARIANCE) < MAX_AVG_VARIANCE;
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
	BitArray::Range next = {begin, row.end()};
	ReadGuardPattern(&next, UPCEANCommon::START_END_PATTERN);
	return {begin, next.begin};
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

	if (!checkChecksum(result))
		return Result(DecodeStatus::ChecksumError);

	BarcodeFormat format = expectedFormat();
	int xStart = startGuard.begin - row.begin();
	int xStop = stopGuard.end - row.begin() - 1;

	Result decodeResult(result, rowNumber, xStart, xStop, format);
	Result extensionResult = UPCEANExtensionSupport::DecodeRow(rowNumber, row, stopGuard.end);
	if (extensionResult.isValid())
	{
		decodeResult.metadata().put(ResultMetadata::UPC_EAN_EXTENSION, extensionResult.text());
		decodeResult.metadata().putAll(extensionResult.metadata());
		decodeResult.addResultPoints(extensionResult.resultPoints());
	}

	if (!_allowedExtensions.empty() && !Contains(_allowedExtensions, extensionResult.text().size())) {
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
bool
UPCEANReader::checkChecksum(const std::string& s) const
{
	return UPCEANCommon::ComputeChecksum(s, 1) == s.back() - '0';
}

} // OneD
} // ZXing
