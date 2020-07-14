/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
* Copyright 2020 Axel Waggershauser
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

#include "ODMultiUPCEANReader.h"
#include "ODUPCEANReader.h"
#include "ODUPCEANCommon.h"
#include "ODEANManufacturerOrgSupport.h"
#include "ODUPCEANExtensionSupport.h"
#include "ODEAN13Reader.h"
#include "ODEAN8Reader.h"
#include "ODUPCAReader.h"
#include "ODUPCEReader.h"
#include "GTIN.h"
#include "DecodeHints.h"
#include "BarcodeFormat.h"
#include "TextDecoder.h"
#include "Result.h"

#include <cmath>

namespace ZXing {

namespace OneD {

MultiUPCEANReader::MultiUPCEANReader(const DecodeHints& hints) : _hints(hints)
{
	_canReturnUPCA = _hints.hasNoFormat() || _hints.hasFormat(BarcodeFormat::UPC_A);
	if (_hints.hasNoFormat()) {
		_hints.setFormats(BarcodeFormat::EAN_13 | BarcodeFormat::EAN_8 | BarcodeFormat::UPC_E);
		// UPC-A is covered by EAN-13
	}

	if (_hints.hasFormat(BarcodeFormat::EAN_13)) {
		_readers.emplace_back(new EAN13Reader(hints));
	}
	else if (_hints.hasFormat(BarcodeFormat::UPC_A)) {
		_readers.emplace_back(new UPCAReader(hints));
	}
	if (_hints.hasFormat(BarcodeFormat::EAN_8)) {
		_readers.emplace_back(new EAN8Reader(hints));
	}
	if (_hints.hasFormat(BarcodeFormat::UPC_E)) {
		_readers.emplace_back(new UPCEReader(hints));
	}

	if (_hints.hasFormat(BarcodeFormat::UPC_EAN_EXTENSION))
		_hints.setAllowedEanExtensions({2,5});
}

MultiUPCEANReader::~MultiUPCEANReader() = default;

Result
MultiUPCEANReader::decodeRow(int rowNumber, const BitArray& row, std::unique_ptr<DecodingState>&) const
{
	// Compute this location once and reuse it on multiple implementations
	auto range = UPCEANReader::FindStartGuardPattern(row);
	if (!range)
		return Result(DecodeStatus::NotFound);

	for (auto& reader : _readers) {
		Result result = reader->decodeRow(rowNumber, row, range);
		if (!result.isValid())
			continue;

		// Special case: a 12-digit code encoded in UPC-A is identical to a "0"
		// followed by those 12 digits encoded as EAN-13. Each will recognize such a code,
		// UPC-A as a 12-digit string and EAN-13 as a 13-digit string starting with "0".
		// Individually these are correct and their readers will both read such a code
		// and correctly call it EAN-13, or UPC-A, respectively.
		//
		// In this case, if we've been looking for both types, we'd like to call it
		// a UPC-A code. But for efficiency we only run the EAN-13 decoder to also read
		// UPC-A. So we special case it here, and convert an EAN-13 result to a UPC-A
		// result if appropriate.
		//
		// But, don't return UPC-A if UPC-A was not a requested format!
		const std::wstring& resultText = result.text();
		bool ean13MayBeUPCA = result.format() == BarcodeFormat::EAN_13 && !resultText.empty() && resultText[0] == '0';
		if (ean13MayBeUPCA && _canReturnUPCA) {
			result.setText(resultText.substr(1));
			result.setFormat(BarcodeFormat::UPC_A);
		}
		return result;
	}
	return Result(DecodeStatus::NotFound);
}

constexpr int CHAR_LEN = 4;

constexpr auto END_PATTERN = FixedPattern<3, 3>{1, 1, 1};
constexpr auto MID_PATTERN = FixedPattern<5, 5>{1, 1, 1, 1, 1};
constexpr auto UPCE_END_PATTERN = FixedPattern<6, 6>{1, 1, 1, 1, 1, 1};
constexpr auto EXT_START_PATTERN = FixedPattern<3, 4>{1, 1, 2};
constexpr auto EXT_SEPARATOR_PATTERN = FixedPattern<2, 2>{1, 1};

static const int FIRST_DIGIT_ENCODINGS[] = {
	0x00, 0x0B, 0x0D, 0x0E, 0x13, 0x19, 0x1C, 0x15, 0x16, 0x1A
};

// The GS1 specification has the following to say about quite zones
// Type: EAN-13 | EAN-8 | UPC-A | UPC-E | EAN Add-on | UPC Add-on
// QZ L:   11   |   7   |   9   |   9   |     7-12   |     9-12
// QZ R:    7   |   7   |   9   |   7   |        5   |        5

constexpr float QUIET_ZONE_LEFT = 6;
constexpr float QUIET_ZONE_RIGHT = 6;

// There is a single sample (ean13-1/12.png) that fails to decode with these (new) settings because
// it has a right-side quite zone of only about 4.5 modules, which is clearly out of spec.

static bool DecodeDigit(const PatternView& view, std::string& txt, int* lgPattern = nullptr)
{
#if 1
	int bestMatch = lgPattern
						? RowReader::DecodeDigit(view, UPCEANCommon::L_AND_G_PATTERNS, UPCEANReader::MAX_AVG_VARIANCE,
												 UPCEANReader::MAX_INDIVIDUAL_VARIANCE, false)
						: RowReader::DecodeDigit(view, UPCEANCommon::L_PATTERNS, UPCEANReader::MAX_AVG_VARIANCE,
												 UPCEANReader::MAX_INDIVIDUAL_VARIANCE, false);
	txt += '0' + (bestMatch % 10);
	if (lgPattern)
		(*lgPattern <<= 1) |= (bestMatch >= 10);

	return bestMatch != -1;
#else
	constexpr int CHAR_SUM = 7;
	auto pattern = RowReader::OneToFourBitPattern<CHAR_LEN, CHAR_SUM>(view);

	// remove first and last bit
	pattern = (~pattern >> 1) & 0b11111;

	// clang-format off
/* pattern now contains the central 5 bits of the L/G/R code
 * L/G-codes always wart with 1 and end with 0, R-codes are simply
 * inverted L-codes.

		L-Code  G-Code  R-Code
	___________________________________
	0 	00110 	10011 	11001
	1 	01100 	11001 	10011
	2 	01001 	01101 	10110
	3 	11110 	10000 	00001
	4 	10001 	01110 	01110
	5 	11000 	11100 	00111
	6 	10111 	00010 	01000
	7 	11101 	01000 	00010
	8 	11011 	00100 	00100
	9 	00101 	01011 	11010
*/
	constexpr char I = -1; // invalid pattern

	const char digit[] = {I,    I,    0x16, I,    0x18, 0x09, 0x00, I,
                          0x17, 0x02, I,    0x19, 0x01, 0x12, 0x14, I,
                          0x13, 0x04, I,    0x10, I,    I,    I,    0x06,
                          0x05, 0x11, I,    0x08, 0x15, 0x07, 0x03, I};
	// clang-format on

	char d = digit[pattern];
	txt += '0' + (d & 0xf);
	if (lgPattern)
		(*lgPattern <<= 1) |= (d >> 4) & 1;

	return d != I;
#endif
}

static bool DecodeDigits(int digitCount, PatternView& next, std::string& txt, int* lgPattern = nullptr)
{
	for (int j = 0; j < digitCount; ++j, next.skipSymbol())
		if (!DecodeDigit(next, txt, lgPattern))
			return false;
	return true;
}

struct PartialResult
{
	std::string txt;
	PatternView end;
	BarcodeFormat format = BarcodeFormat::NONE;

	PartialResult() { txt.reserve(14); }
	bool isValid() const { return format != BarcodeFormat::NONE; }
};

inline bool _ret_false_debug_helper()
{
	return false;
}
#define CHECK(A) if(!(A)) return _ret_false_debug_helper();

static bool EAN13(PartialResult& res, PatternView begin)
{
	auto mid = begin.subView(27, MID_PATTERN.size());
	auto end = begin.subView(56, END_PATTERN.size());

	CHECK(end.isValid() && IsRightGuard(end, END_PATTERN, QUIET_ZONE_RIGHT) && IsPattern(mid, MID_PATTERN));

	auto next = begin.subView(END_PATTERN.size(), CHAR_LEN);
	res.txt = " "; // make space for lgPattern character
	int lgPattern = 0;

	CHECK(DecodeDigits(6, next, res.txt, &lgPattern));

	next = next.subView(MID_PATTERN.size(), CHAR_LEN);

	CHECK(DecodeDigits(6, next, res.txt));

	res.txt[0] = '0' + IndexOf(FIRST_DIGIT_ENCODINGS, lgPattern);
	CHECK(res.txt[0] != '0' - 1);

	res.end = end;
	res.format = BarcodeFormat::EAN_13;
	return true;
}

static bool EAN8(PartialResult& res, PatternView begin)
{
	auto mid = begin.subView(19, MID_PATTERN.size());
	auto end = begin.subView(40, END_PATTERN.size());

	CHECK(end.isValid() && IsRightGuard(end, END_PATTERN, QUIET_ZONE_RIGHT) && IsPattern(mid, MID_PATTERN));

	auto next = begin.subView(END_PATTERN.size(), CHAR_LEN);
	res.txt.clear();

	CHECK(DecodeDigits(4, next, res.txt));

	next = next.subView(MID_PATTERN.size(), CHAR_LEN);

	CHECK(DecodeDigits(4, next, res.txt));

	res.end = end;
	res.format = BarcodeFormat::EAN_8;
	return true;
}

static bool UPCE(PartialResult& res, PatternView begin)
{
	auto end = begin.subView(27, UPCE_END_PATTERN.size());

	CHECK(end.isValid() && IsRightGuard(end, UPCE_END_PATTERN, QUIET_ZONE_RIGHT));

	// additional plausibilty check for the module size: it has to be about the same for both
	// the guard patterns and the payload/data part. This speeds up the falsepositives use case
	// about 2x and brings the misread count down to 0
	float moduleSizeGuard = (begin.sum() + end.sum()) / 9.f;
	float moduleSizeData = begin.subView(3, 6*4).sum() / (6*7.f);
	CHECK(std::abs(moduleSizeData / moduleSizeGuard - 1) < 0.2f);

	auto next = begin.subView(END_PATTERN.size(), CHAR_LEN);
	int lgPattern = 0;
	res.txt = " "; // make space for lgPattern character

	CHECK(DecodeDigits(6, next, res.txt, &lgPattern));

	int i = IndexOf(UPCEANCommon::NUMSYS_AND_CHECK_DIGIT_PATTERNS, lgPattern);
	CHECK(i != -1);

	res.txt[0] = '0' + i / 10;
	res.txt += '0' + i % 10;

	res.end = end;
	res.format = BarcodeFormat::UPC_E;
	return true;
}

static bool Extension(PartialResult& res, PatternView begin, int digitCount)
{
	auto ext = begin.subView(0, 3 + digitCount * 4 + (digitCount - 1) * 2);
	CHECK(ext.isValid());
	auto moduleSize = IsPattern(ext, EXT_START_PATTERN);
	CHECK(moduleSize);

	res.end = ext;
	ext = ext.subView(EXT_START_PATTERN.size(), CHAR_LEN);
	int lgPattern = 0;
	res.txt.clear();

	for (int i = 0; i < digitCount; ++i) {
		CHECK(DecodeDigit(ext, res.txt, &lgPattern));
		ext.skipSymbol();
		if (i < digitCount - 1) {
			CHECK(IsPattern(ext, EXT_SEPARATOR_PATTERN, 0, 0, moduleSize));
			ext.skipPair();
		}
	}

	//TODO: check right quite zone

	if (digitCount == 2) {
		CHECK(std::stoi(res.txt) % 4 == lgPattern);
	} else {
		constexpr int CHECK_DIGIT_ENCODINGS[] = {0x18, 0x14, 0x12, 0x11, 0x0C, 0x06, 0x03, 0x0A, 0x09, 0x05};
		CHECK(UPCEANExtension5Support::ExtensionChecksum(res.txt) == IndexOf(CHECK_DIGIT_ENCODINGS, lgPattern));
	}
	res.format = BarcodeFormat::UPC_EAN_EXTENSION;
	return true;
}

Result MultiUPCEANReader::decodePattern(int rowNumber, const PatternView& row, std::unique_ptr<RowReader::DecodingState>&) const
{
//	return Result(DecodeStatus::_internal);

	const int minSize = 3 + 6*4 + 6; // UPC-E

	auto begin = FindLeftGuard(row, minSize, END_PATTERN, QUIET_ZONE_LEFT);
	if (!begin.isValid())
		return Result(DecodeStatus::NotFound);

	PartialResult res;

	if (!((_hints.hasFormat(BarcodeFormat::EAN_13) && EAN13(res, begin)) ||
		  (_hints.hasFormat(BarcodeFormat::EAN_8) && EAN8(res, begin)) ||
		  (_hints.hasFormat(BarcodeFormat::UPC_E) && UPCE(res, begin))))
		return Result(DecodeStatus::NotFound);

	if (!GTIN::IsCheckDigitValid(res.format == BarcodeFormat::UPC_E ? UPCEANCommon::ConvertUPCEtoUPCA(res.txt) : res.txt))
		return Result(DecodeStatus::ChecksumError);

	// If UPC-A was a requested format and we deteced a EAN-13 code with a leading '0', then we drop the '0' and call it
	// a UPC-A code.
	// TODO: this is questionable
	if (_canReturnUPCA && res.format == BarcodeFormat::EAN_13 && res.txt.front() == '0') {
		res.txt = res.txt.substr(1);
		res.format = BarcodeFormat::UPC_A;
	}

	Result result(res.txt, rowNumber, begin.pixelsInFront(), res.end.pixelsTillEnd(), res.format);

	auto expectExtension =
		[this](int n) { return _hints.allowedEanExtensions().empty() || Contains(_hints.allowedEanExtensions(), n); };

	auto ext = res.end;
	PartialResult extRes;
	if (ext.skipSymbol() && ext.skipSingle(begin.sum() * 3.5) &&
		((expectExtension(5) && Extension(extRes, ext, 5)) || (expectExtension(2) && Extension(extRes, ext, 2)))) {

		//TODO: extend position in include extension

		result.metadata().put(ResultMetadata::UPC_EAN_EXTENSION, TextDecoder::FromLatin1(extRes.txt));

		if (Size(extRes.txt) == 2) {
			result.metadata().put(ResultMetadata::ISSUE_NUMBER, std::stoi(extRes.txt));
		} else {
			std::string price = UPCEANExtension5Support::ParseExtension5String(extRes.txt);
			if (!price.empty())
				result.metadata().put(ResultMetadata::SUGGESTED_PRICE, TextDecoder::FromLatin1(price));
		}
	}

	if (!_hints.allowedEanExtensions().empty() && !extRes.isValid())
		return Result(DecodeStatus::NotFound);

	if (res.format == BarcodeFormat::EAN_13 || res.format == BarcodeFormat::UPC_A) {
		std::string countryID = EANManufacturerOrgSupport::LookupCountryIdentifier(res.txt);
		if (!countryID.empty())
			result.metadata().put(ResultMetadata::POSSIBLE_COUNTRY, TextDecoder::FromLatin1(countryID));
	}

	return result;
}

} // OneD
} // ZXing
