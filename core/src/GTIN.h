#pragma once
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

#include "BarcodeFormat.h"
#include "ZXContainerAlgorithms.h"

#include <string>

namespace ZXing {

class Result;

namespace GTIN {

template <typename T>
T ComputeCheckDigit(const std::basic_string<T>& digits, bool skipTail = false)
{
	int sum = 0, N = Size(digits) - skipTail;
	for (int i = N - 1; i >= 0; i -= 2)
		sum += digits[i] - '0';
	sum *= 3;
	for (int i = N - 2; i >= 0; i -= 2)
		sum += digits[i] - '0';
	return ((10 - (sum % 10)) % 10) + '0';
}

template <typename T>
bool IsCheckDigitValid(const std::basic_string<T>& s)
{
	return ComputeCheckDigit(s, true) == s.back();
}

/**
 * Evaluate the prefix of the GTIN to estimate the country of origin. See
 * <a href="https://www.gs1.org/standards/id-keys/company-prefix">
 * https://www.gs1.org/standards/id-keys/company-prefix</a> and
 * <a href="https://en.wikipedia.org/wiki/List_of_GS1_country_codes">
 * https://en.wikipedia.org/wiki/List_of_GS1_country_codes</a>.
 *
 * `format` required for EAN-8 (UPC-E assumed if not given)
 */
std::string LookupCountryIdentifier(const std::string& GTIN, const BarcodeFormat format = BarcodeFormat::None);

std::string EanAddOn(const Result& result);

std::string IssueNr(const std::string& ean2AddOn);
std::string Price(const std::string& ean5AddOn);

} // namespace GTIN
} // namespace ZXing
