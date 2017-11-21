#pragma once
/*
* Copyright 2016 Huy Cuong Nguyen
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

#include <cstdlib>
#include <type_traits>
#include <vector>
#include <string>
#include <iosfwd>

namespace ZXing {

/**
* All credits on BigInteger below go to Matt McCutchen, as the code below is extracted/modified from his C++ Big Integer Library (https://mattmccutchen.net/bigint/)
*/
class BigInteger
{
public:
	using Block = size_t;

	// The number of bits in a block
	//static const unsigned int N = 8 * sizeof(Block);

	// Constructs zero.
	BigInteger() = default;

	template <typename T>
	BigInteger(T x, typename std::enable_if<std::is_integral<T>::value && std::is_unsigned<T>::value>::type* = nullptr) : mag(1, x) {}

	template <typename T>
	BigInteger(T x, typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value>::type* = nullptr) : negative(x < 0), mag(1, std::abs(x)) {}

	static bool TryParse(const std::string& str, BigInteger& result);
	static bool TryParse(const std::wstring& str, BigInteger& result);

	bool isZero() const { return mag.empty(); }

	std::string toString() const;
	int toInt() const;

	inline BigInteger& operator+=(BigInteger&& a) {
		if (mag.empty())
			*this = std::move(a);
		else
			Add(*this, a, *this);
		return *this;
	}

	friend inline BigInteger operator+(const BigInteger& a, const BigInteger& b) {
		BigInteger c;
		BigInteger::Add(a, b, c);
		return c;
	}

	friend inline BigInteger operator-(const BigInteger& a, const BigInteger& b) {
		BigInteger c;
		BigInteger::Subtract(a, b, c);
		return c;
	}

	friend inline BigInteger operator*(const BigInteger& a, const BigInteger& b) {
		BigInteger c;
		BigInteger::Multiply(a, b, c);
		return c;
	}

	friend inline std::ostream& operator<<(std::ostream& out, const BigInteger& x) {
		return out << x.toString();
	}

	static void Add(const BigInteger& a, const BigInteger &b, BigInteger& c);
	static void Subtract(const BigInteger& a, const BigInteger &b, BigInteger& c);
	static void Multiply(const BigInteger& a, const BigInteger &b, BigInteger& c);
	static void Divide(const BigInteger& a, const BigInteger &b, BigInteger& quotient, BigInteger& remainder);

private:
	bool negative = false;
	std::vector<Block> mag;
};


} // ZXing
