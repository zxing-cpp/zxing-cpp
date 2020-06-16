#pragma once
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

#include "GenericGFPoly.h"

#include <algorithm>
#include <cassert>
#include <vector>
#include <stdexcept>

namespace ZXing {

/**
* <p>This class contains utility methods for performing mathematical operations over
* the Galois Fields. Operations use a given primitive polynomial in calculations.</p>
*
* <p>Throughout this package, elements of the GF are represented as an {@code int}
* for convenience and speed (but at the cost of memory).
* </p>
*
* @author Sean Owen
* @author David Olivier
*/
class GenericGF
{
public:
	static const GenericGF& AztecData12();
	static const GenericGF& AztecData10();
	static const GenericGF& AztecData6();
	static const GenericGF& AztecParam();
	static const GenericGF& QRCodeField256();
	static const GenericGF& DataMatrixField256();
	static const GenericGF& AztecData8();
	static const GenericGF& MaxiCodeField64();

	/**
	* @return the monomial representing coefficient * x^degree
	*/
	GenericGFPoly& setMonomial(GenericGFPoly& poly, int degree, int coefficient) const
	{
		assert(degree >= 0);

		if (coefficient == 0)
			degree = 0;

		poly._field = this;
		poly._coefficients.resize(degree + 1);
		std::fill(poly._coefficients.begin(), poly._coefficients.end(), 0);
		poly._coefficients.front() = coefficient;

		return poly;
	}

	GenericGFPoly& setZero(GenericGFPoly& poly) const {
		return setMonomial(poly, 0, 0);
	}

	GenericGFPoly& setOne(GenericGFPoly& poly) const {
		return setMonomial(poly, 0, 1);
	}

	/**
	* Implements both addition and subtraction -- they are the same in GF(size).
	*
	* @return sum/difference of a and b
	*/
	int addOrSubtract(int a, int b) const noexcept {
		return a ^ b;
	}

	/**
	* @return 2 to the power of a in GF(size)
	*/
	int exp(int a) const {
		return _expTable.at(a);
	}

	/**
	* @return base 2 log of a in GF(size)
	*/
	int log(int a) const {
		if (a == 0) {
			throw std::invalid_argument("a == 0");
		}
		return _logTable.at(a);
	}

	/**
	* @return multiplicative inverse of a
	*/
	int inverse(int a) const {
		return _expTable[_size - log(a) - 1];
	}

	/**
	* @return product of a and b in GF(size)
	*/
	int multiply(int a, int b) const noexcept {
		if (a == 0 || b == 0) {
			return 0;
		}
		auto fast_mod = [](const int input, const int ceil) {
			// avoid using the '%' modulo operator => ReedSolomon computation is more than twice as fast
			// see also https://stackoverflow.com/a/33333636/2088798
			return input < ceil ? input : input - ceil;
		};
		return _expTable[fast_mod(_logTable[a] + _logTable[b], _size - 1)];
	}

	
	int size() const {
		return _size;
	}

	int generatorBase() const {
		return _generatorBase;
	}

private:
	const int _size;
	int _generatorBase;
	std::vector<int> _expTable;
	std::vector<int> _logTable;

	/**
	* Create a representation of GF(size) using the given primitive polynomial.
	*
	* @param primitive irreducible polynomial whose coefficients are represented by
	*  the bits of an int, where the least-significant bit represents the constant
	*  coefficient
	* @param size the size of the field
	* @param b the factor b in the generator polynomial can be 0- or 1-based
	*  (g(x) = (x+a^b)(x+a^(b+1))...(x+a^(b+2t-1))).
	*  In most cases it should be 1, but for QR code it is 0.
	*/
	GenericGF(int primitive, int size, int b);
};

} // ZXing
