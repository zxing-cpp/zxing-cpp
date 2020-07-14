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
#include "ZXContainerAlgorithms.h"

#include <string>

namespace ZXing {
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

//TODO: move EANManufacturerSupport code here

} // namespace GTIN
} // namespace ZXing
