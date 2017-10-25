#pragma once
/*
* Copyright 2016 Huy Cuong Nguyen
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
#include "ODUPCEANReader.h"

#include <array>

namespace ZXing {
namespace OneD {

class UPCEANCommon
{
public:
   	/**
	* Start/end guard pattern.
	*/
	static const std::array<int, 3> START_END_PATTERN;

	/**
	* end guard pattern.
	*/
	static const std::array<int, 6> END_PATTERN;

	/**
	* "Odd", or "L" patterns used to encode UPC/EAN digits.
	*/
	static const std::array<UPCEANReader::Digit, 10> L_PATTERNS;

	/**
	* Pattern marking the middle of a UPC/EAN pattern, separating the two halves.
	*/
	static const std::array<int, 5> MIDDLE_PATTERN;

	/**
	* As above but also including the "even", or "G" patterns used to encode UPC/EAN digits.
	*/
	static const std::array<UPCEANReader::Digit, 20> L_AND_G_PATTERNS;


	template <size_t N>
	static int ComputeChecksum(const std::array<int, N>& digits)
	{
		int sum = 0;
		for (int i = N - 2; i >= 0; i -= 2) {
			sum += digits[i];
		}
		sum *= 3;
		for (int i = N - 3; i >= 0; i -= 2) {
			sum += digits[i];
		}
		return (10 - (sum % 10)) % 10;
	}

	/**
	* Expands a UPC-E value back into its full, equivalent UPC-A code value.
	*
	* @param upce UPC-E code as string of digits
	* @return equivalent UPC-A code as string of digits
	*/
	template <typename StringT>
	static StringT ConvertUPCEtoUPCA(const StringT& upce)
	{
		if (upce.length() < 7)
			return upce;

		auto upceChars = upce.substr(1, 6);

		StringT result;
		result.reserve(12);
		result += upce[0];
		auto lastChar = upceChars[5];
		switch (lastChar) {
		case '0':
		case '1':
		case '2':
			result += upceChars.substr(0, 2);
			result += lastChar;
			result += StringT(4, '0');
			result += upceChars.substr(2, 3);
			break;
		case '3':
			result += upceChars.substr(0, 3);
			result += StringT(5, '0');
			result += upceChars.substr(3, 2);
			break;
		case '4':
			result += upceChars.substr(0, 4);
			result += StringT(5, '0');;
			result += upceChars[4];
			break;
		default:
			result += upceChars.substr(0, 5);
			result += StringT(4, '0');
			result += lastChar;
			break;
		}
		// Only append check digit in conversion if supplied
		if (upce.length() >= 8) {
			result += upce[7];
		}
		return result;
	}
};

} // OneD
} // ZXing
