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
#include "Result.h"
#include "BitArray.h"
#include "DecodeHints.h"
#include "TextCodec.h"

#include <algorithm>

namespace ZXing {

namespace OneD {

// These two values are critical for determining how permissive the decoding will be.
// We've arrived at these values through a lot of trial and error. Setting them any higher
// lets false positives creep in quickly.
static const float MAX_AVG_VARIANCE = 0.48f;
static const float MAX_INDIVIDUAL_VARIANCE = 0.7f;

/**
* Start/end guard pattern.
*/
static const std::array<int, 3> START_END_PATTERN = { 1, 1, 1 };

const std::array<int, 5> UPCEANReader::MIDDLE_PATTERN = { 1, 1, 1, 1, 1 };

/**
* end guard pattern.
*/
//static const std::array<int, 6> END_PATTERN = { 1, 1, 1, 1, 1, 1 };

const std::array<std::array<int, 4>, 10>
UPCEANReader::L_PATTERNS = {
	3, 2, 1, 1, // 0
	2, 2, 2, 1, // 1
	2, 1, 2, 2, // 2
	1, 4, 1, 1, // 3
	1, 1, 3, 2, // 4
	1, 2, 3, 1, // 5
	1, 1, 1, 4, // 6
	1, 3, 1, 2, // 7
	1, 2, 1, 3, // 8
	3, 1, 1, 2, // 9
};

const std::array<std::array<int, 4>, 20>
UPCEANReader::L_AND_G_PATTERNS = {
	3, 2, 1, 1, // 0
	2, 2, 2, 1, // 1
	2, 1, 2, 2, // 2
	1, 4, 1, 1, // 3
	1, 1, 3, 2, // 4
	1, 2, 3, 1, // 5
	1, 1, 1, 4, // 6
	1, 3, 1, 2, // 7
	1, 2, 1, 3, // 8
	3, 1, 1, 2, // 9
	// reversed
	1, 1, 2, 3, // 10
	1, 2, 2, 2, // 11
	2, 2, 1, 2, // 12
	1, 1, 4, 1, // 13
	2, 3, 1, 1, // 14
	1, 3, 2, 1, // 15
	4, 1, 1, 1, // 16
	2, 1, 3, 1, // 17
	3, 1, 2, 1, // 18
	2, 1, 1, 3, // 19
};


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
* @param counters array of counters, as long as pattern, to re-use
* @return start/end horizontal offset of guard pattern, as an array of two ints
* @throws NotFoundException if pattern is not found
*/
ErrorStatus
UPCEANReader::DoFindGuardPattern(const BitArray& row, int rowOffset, bool whiteFirst, const int* pattern, int* counters, size_t length, int& begin, int& end)
{
	int width = row.size();
	bool isWhite = whiteFirst;
	rowOffset = whiteFirst ? row.getNextUnset(rowOffset) : row.getNextSet(rowOffset);
	int counterPosition = 0;
	int patternStart = rowOffset;
	auto bitIter = row.iterAt(rowOffset);
	for (; rowOffset < width; ++rowOffset, ++bitIter) {
		if (*bitIter ^ isWhite) {
			counters[counterPosition]++;
		}
		else {
			if (counterPosition == int(length) - 1) {
				if (PatternMatchVariance(counters, pattern, length, MAX_INDIVIDUAL_VARIANCE) < MAX_AVG_VARIANCE) {
					begin = patternStart;
					end = rowOffset;
					return ErrorStatus::NoError;
				}
				patternStart += counters[0] + counters[1];
				for (size_t i = 2; i < length; ++i) {
					counters[i - 2] = counters[i];
				}
				counters[length - 2] = 0;
				counters[length - 1] = 0;
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

ErrorStatus
UPCEANReader::FindGuardPattern(const BitArray& row, int rowOffset, bool whiteFirst, const int* pattern, size_t length, int& begin, int& end)
{
	std::vector<int> counters(length, 0);
	return DoFindGuardPattern(row, rowOffset, whiteFirst, pattern, counters.data(), length, begin, end);
}

ErrorStatus
UPCEANReader::FindStartGuardPattern(const BitArray& row, int& begin, int& end)
{
	bool foundStart = false;
	int start = 0;
	int nextStart = 0;
	std::vector<int> counters(START_END_PATTERN.size());
	while (!foundStart) {
		std::fill(counters.begin(), counters.end(), 0);
		auto status = DoFindGuardPattern(row, nextStart, false, START_END_PATTERN.data(), counters.data(), START_END_PATTERN.size(), start, nextStart);
		if (StatusIsError(status)) {
			return status;
		}
		// Make sure there is a quiet zone at least as big as the start pattern before the barcode.
		// If this check would run off the left edge of the image, do not accept this barcode,
		// as it is very likely to be a false positive.
		int quietStart = start - (nextStart - start);
		if (quietStart >= 0) {
			foundStart = row.isRange(quietStart, start, false);
		}
	}
	begin = start;
	end = nextStart;
	return ErrorStatus::NoError;
}

Result
UPCEANReader::decodeRow(int rowNumber, const BitArray& row, std::unique_ptr<DecodingState>& state) const
{
	int begin, end;
	auto status = FindStartGuardPattern(row, begin, end);
	if (StatusIsError(status))
		return Result(status);

	return decodeRow(rowNumber, row, begin, end);
}

ErrorStatus
UPCEANReader::decodeEnd(const BitArray& row, int endStart, int& begin, int& end) const
{
	return FindGuardPattern(row, endStart, false, START_END_PATTERN.data(), START_END_PATTERN.size(), begin, end);
}

Result
UPCEANReader::decodeRow(int rowNumber, const BitArray& row, int startGuardBegin, int startGuardEnd) const
{
	//auto pointCallback = hints.resultPointCallback();
	//if (pointCallback != nullptr) {
	//	pointCallback(0.5f * (startGuardBegin + startGuardEnd), static_cast<float>(rowNumber));
	//}

	std::string result;
	result.reserve(20);
	int endStart = startGuardEnd;
	auto status = decodeMiddle(row, endStart, result);
	if (StatusIsError(status))
		return Result(status);

	/*if (pointCallback != nullptr) {
		pointCallback(static_cast<float>(endStart), static_cast<float>(rowNumber));
	}*/

	int endRangeBegin, endRangeEnd;
	status = decodeEnd(row, endStart, endRangeBegin, endRangeEnd);
	if (StatusIsError(status))
		return Result(status);

	/*if (pointCallback != nullptr) {
		pointCallback(0.5f * (endRangeBegin + endRangeEnd), static_cast<float>(rowNumber));
	}*/

	// Make sure there is a quiet zone at least as big as the end pattern after the barcode. The
	// spec might want more whitespace, but in practice this is the maximum we can count on.
	int end = endRangeEnd;
	int quietEnd = end + (end - endRangeBegin);
	if (quietEnd >= row.size() || !row.isRange(end, quietEnd, false)) {
		return Result(ErrorStatus::NotFound);
	}

	// UPC/EAN should never be less than 8 chars anyway
	if (result.length() < 8) {
		return Result(ErrorStatus::FormatError);
	}
	status = checkChecksum(result);
	if (StatusIsError(status))
		return Result(status);

	float left = 0.5f * static_cast<float>(startGuardBegin + startGuardEnd);
	float right = 0.5f * static_cast<float>(endRangeBegin + endRangeEnd);
	BarcodeFormat format = expectedFormat();
	float ypos = static_cast<float>(rowNumber);

	Result decodeResult(TextCodec::FromLatin1(result), ByteArray(), { ResultPoint(left, ypos), ResultPoint(right, ypos) }, format);
	int extensionLength = 0;
	Result extensionResult = UPCEANExtensionSupport::DecodeRow(rowNumber, row, endRangeEnd);
	if (extensionResult.isValid())
	{
		decodeResult.metadata().put(ResultMetadata::UPC_EAN_EXTENSION, extensionResult.text());
		decodeResult.metadata().putAll(extensionResult.metadata());
		decodeResult.addResultPoints(extensionResult.resultPoints());
		extensionLength = static_cast<int>(extensionResult.text().length());
	}

	if (!_allowedExtensions.empty()) {
		bool valid = false;
		for (int length : _allowedExtensions) {
			if (extensionLength == length) {
				valid = true;
				break;
			}
		}
		if (!valid) {
			return Result(ErrorStatus::NotFound);
		}
	}

	if (format == BarcodeFormat::EAN_13 || format == BarcodeFormat::UPC_A) {
		std::string countryID = EANManufacturerOrgSupport::LookupCountryIdentifier(result);
		if (!countryID.empty()) {
			decodeResult.metadata().put(ResultMetadata::POSSIBLE_COUNTRY, TextCodec::FromLatin1(countryID));
		}
	}

	return decodeResult;
}

ErrorStatus
UPCEANReader::checkChecksum(const std::string& s) const
{
	return CheckStandardUPCEANChecksum(s);
}

/**
* Computes the UPC/EAN checksum on a string of digits, and reports
* whether the checksum is correct or not.
*
* @param s string of digits to check
* @return true iff string of digits passes the UPC/EAN checksum algorithm
* @throws FormatException if the string does not contain only digits
*/
ErrorStatus
UPCEANReader::CheckStandardUPCEANChecksum(const std::string& s)
{
	int length = static_cast<int>(s.length());
	if (length == 0) {
		return ErrorStatus::ChecksumError;
	}

	int sum = 0;
	int zero = static_cast<int>('0');
	for (int i = length - 2; i >= 0; i -= 2) {
		int digit = static_cast<int>(s[i]) - zero;
		if (digit < 0 || digit > 9) {
			return ErrorStatus::FormatError;
		}
		sum += digit;
	}
	sum *= 3;
	for (int i = length - 1; i >= 0; i -= 2) {
		int digit = static_cast<int>(s[i]) - zero;
		if (digit < 0 || digit > 9) {
			return ErrorStatus::FormatError;
		}
		sum += digit;
	}
	return sum % 10 == 0 ? ErrorStatus::NoError : ErrorStatus::ChecksumError;
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
ErrorStatus
UPCEANReader::DecodeDigit(const BitArray& row, int rowOffset, const std::array<int, 4>* patterns, size_t patternCount, std::array<int, 4>& counters, int &resultOffset)
{
	ErrorStatus status = RowReader::RecordPattern(row, rowOffset, counters);
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
			status = ErrorStatus::NotFound;
		}
	}
	return status;
}

} // OneD
} // ZXing
