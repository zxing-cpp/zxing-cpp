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
#include "GTIN.h"

#include <array>
#include <cstddef>
#include <stdexcept>
#include <string>

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

	/**
	* UPCE end guard pattern (== MIDDLE_PATTERN + single module black bar)
	*/
	static const std::array<int, 6> UPCE_END_PATTERN;

	/**
	* See {@link #L_AND_G_PATTERNS}; these values similarly represent patterns of
	* even-odd parity encodings of digits that imply both the number system (0 or 1)
	* used (index / 10), and the check digit (index % 10).
	*/
	static const std::array<int, 20> NUMSYS_AND_CHECK_DIGIT_PATTERNS;

	template <size_t N, typename T>
	static std::array<int, N> DigitString2IntArray(const std::basic_string<T>& in, int checkDigit = -1)
	{
		static_assert(N == 8 || N == 13, "invalid UPC/EAN length");

		if (in.size() != N && in.size() != N-1)
			throw std::invalid_argument("Invalid input string length");

		std::array<int, N> out = {};
		for (size_t i = 0; i < in.size(); ++i) {
			out[i] = in[i] - '0';
			if (out[i] < 0 || out[i] > 9)
				throw std::invalid_argument("Contents must contain only digits: 0-9");
		}

		if (checkDigit == -1)
			checkDigit = GTIN::ComputeCheckDigit(in, N == in.size());

		if (in.size() == N-1)
			out.back() = checkDigit - '0';
		else if (in.back() != checkDigit)
			throw std::invalid_argument("Checksum error");

		return out;
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
