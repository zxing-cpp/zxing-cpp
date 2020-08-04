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

#include "BarcodeFormat.h"
#include "BitHacks.h"
#include "ZXContainerAlgorithms.h"

#include <algorithm>
#include <cctype>
#include <iterator>
#include <string>
#include <sstream>
#include <stdexcept>

namespace ZXing {

static const char* FORMAT_STR[] = {
	"NONE",
	"AZTEC",
	"CODABAR",
	"CODE_39",
	"CODE_93",
	"CODE_128",
	"DATA_MATRIX",
	"EAN_8",
	"EAN_13",
	"ITF",
	"MAXICODE",
	"PDF_417",
	"QR_CODE",
	"RSS_14",
	"RSS_EXPANDED",
	"UPC_A",
	"UPC_E",
	"UPC_EAN_EXTENSION",
};

static_assert(Size(FORMAT_STR) == (int)BarcodeFormats::bitIndex(BarcodeFormat::_max) + 1,
			  "FORMAT_STR array is out of sync with BarcodeFormat");

const char* ToString(BarcodeFormat format)
{
	return FORMAT_STR[BarcodeFormats::bitIndex(format)];
}

std::string ToString(BarcodeFormats formats)
{
	if (formats.testFlag(BarcodeFormat::NONE))
		return ToString(BarcodeFormat::NONE);
	std::string res;
	for (auto f : formats)
		res += ToString(f) + std::string("|");
	return res.substr(0, res.size() - 1);
}

static std::string NormalizeFormatString(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(), [](char c) { return (char)std::tolower(c); });
	str.erase(std::remove(str.begin(), str.end(), '_'), str.end());
	return str;
}

static BarcodeFormat ParseFormatString(const std::string& str)
{
	auto pos = std::find_if(std::begin(FORMAT_STR), std::end(FORMAT_STR),
							[str](auto fmt) { return NormalizeFormatString(fmt) == str; });
	return pos == std::end(FORMAT_STR) ? BarcodeFormat::NONE
									   : BarcodeFormat(1 << (std::distance(std::begin(FORMAT_STR), pos) - 1));
}

BarcodeFormat BarcodeFormatFromString(const std::string& str)
{
	return ParseFormatString(NormalizeFormatString(str));
}

BarcodeFormats BarcodeFormatsFromString(const std::string& str)
{
	auto normalized = NormalizeFormatString(str);
	std::replace_if(
		normalized.begin(), normalized.end(), [](char c) { return Contains(" ,", c); }, '|');
	std::istringstream input(normalized);
	BarcodeFormats res;
	for (std::string token; std::getline(input, token, '|');) {
		if(!token.empty()) {
			auto bc = ParseFormatString(token);
			if (bc == BarcodeFormat::NONE)
				throw std::invalid_argument("This is not a valid barcode format: " + token);
			res |= bc;
		}
	}
	return res;
}

} // ZXing
