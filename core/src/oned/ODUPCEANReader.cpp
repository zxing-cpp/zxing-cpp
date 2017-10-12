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

// These two values are critical for determining how permissive the decoding will be.
// We've arrived at these values through a lot of trial and error. Setting them any higher
// lets false positives creep in quickly.
static const float MAX_AVG_VARIANCE = 0.48f;
static const float MAX_INDIVIDUAL_VARIANCE = 0.7f;

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

	begin = whiteFirst ? row.getNextUnset(begin) : row.getNextSet(begin);

	return RowReader::FindPattern(
		begin, row.end(), counters,
		[&pattern, &length](BitArray::Iterator begin, BitArray::Iterator end, Counters& counters) {
			return RowReader::PatternMatchVariance(counters.data(), pattern, length, MAX_INDIVIDUAL_VARIANCE) < MAX_AVG_VARIANCE;
		});
}

BitArray::Range
UPCEANReader::FindStartGuardPattern(const BitArray& row)
{
	const auto& pattern = UPCEANCommon::START_END_PATTERN;
	using Counters = decltype(pattern);
	auto counters = Counters{};

	return RowReader::FindPattern(
		row.getNextSet(row.begin()), row.end(), counters,
		[&row, &pattern](BitArray::Iterator begin, BitArray::Iterator end, Counters& counters) {
			if (!(RowReader::PatternMatchVariance(counters, pattern, MAX_INDIVIDUAL_VARIANCE) < MAX_AVG_VARIANCE))
				return false;

			// Make sure there is a quiet zone at least as big as the start pattern before the barcode.
			// If this check would run off the left edge of the image, do not accept this barcode,
			// as it is very likely to be a false positive.
			int quietZoneWidth = end - begin;
			return begin - row.begin() >= quietZoneWidth && row.hasQuiteZone(begin, -quietZoneWidth);
		});
}

Result
UPCEANReader::decodeRow(int rowNumber, const BitArray& row, std::unique_ptr<DecodingState>& state) const
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
	//auto pointCallback = hints.resultPointCallback();
	//if (pointCallback != nullptr) {
	//	pointCallback(0.5f * (startGuardBegin + startGuardEnd), static_cast<float>(rowNumber));
	//}

	std::string result;
	result.reserve(20);
	int endStart = startGuard.end - row.begin();
	auto status = decodeMiddle(row, endStart, result);
	if (StatusIsError(status))
		return Result(status);

	/*if (pointCallback != nullptr) {
		pointCallback(static_cast<float>(endStart), static_cast<float>(rowNumber));
	}*/

	auto endRange = decodeEnd(row, row.iterAt(endStart));
	if (!endRange)
		return Result(DecodeStatus::NotFound);

	/*if (pointCallback != nullptr) {
		pointCallback(0.5f * (endRangeBegin + endRangeEnd), static_cast<float>(rowNumber));
	}*/

	// Make sure there is a quiet zone at least as big as the end pattern after the barcode. The
	// spec might want more whitespace, but in practice this is the maximum we can count on.
	int end = endRange.end - row.begin();
	int quietEnd = end + endRange.size();
	if (quietEnd >= row.size() || !row.isRange(end, quietEnd, false)) {
		return Result(DecodeStatus::NotFound);
	}

	// UPC/EAN should never be less than 8 chars anyway
	if (result.length() < 8) {
		return Result(DecodeStatus::FormatError);
	}
	status = checkChecksum(result);
	if (StatusIsError(status))
		return Result(status);

	float left = (startGuard.begin - row.begin()) + 0.5f * startGuard.size();
	float right = (endRange.begin - row.begin()) + 0.5f * endRange.size();
	BarcodeFormat format = expectedFormat();
	float ypos = static_cast<float>(rowNumber);

	Result decodeResult(TextDecoder::FromLatin1(result), ByteArray(), { ResultPoint(left, ypos), ResultPoint(right, ypos) }, format);
	int extensionLength = 0;
	Result extensionResult = UPCEANExtensionSupport::DecodeRow(rowNumber, row, endRange.end - row.begin());
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

/**
* Attempts to decode a single UPC/EAN-encoded digit.
*
* @param row row of black/white values to decode
* @param counters the counts of runs of observed black/white/black/... values
* @param rowOffset horizontal offset to start decoding from
* @param patterns the set of patterns to use to decode -- sometimes different encodings
* for the digits 0-9 are used, and this indicates the encodings for 0 to 9 that should
* be used
* @return horizontal offset of first pixel beyond the decoded digit
* @throws NotFoundException if digit cannot be decoded
*/
DecodeStatus
UPCEANReader::DecodeDigit(const BitArray& row, int rowOffset, const Digit* patterns, size_t patternCount, Digit& counters, int &resultOffset)
{
	DecodeStatus status = RowReader::RecordPattern(row, rowOffset, counters);
	if (StatusIsOK(status)) {
		float bestVariance = MAX_AVG_VARIANCE; // worst variance we'll accept
		int bestMatch = -1;
		for (size_t i = 0; i < patternCount; i++) {
			auto& pattern = patterns[i];
			float variance = PatternMatchVariance(counters, pattern, MAX_INDIVIDUAL_VARIANCE);
			if (variance < bestVariance) {
				bestVariance = variance;
				bestMatch = static_cast<int>(i);
			}
		}
		if (bestMatch >= 0) {
			resultOffset = bestMatch;
		}
		else {
			status = DecodeStatus::NotFound;
		}
	}
	return status;
}

} // OneD
} // ZXing
