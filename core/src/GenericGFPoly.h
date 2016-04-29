#pragma once
/*
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

#include <vector>

namespace ZXing {

class GenericGF;

/**
* <p>Represents a polynomial whose coefficients are elements of a GF.
* Instances of this class are immutable.</p>
*
* <p>Much credit is due to William Rucklidge since portions of this code are an indirect
* port of his C++ Reed-Solomon implementation.</p>
*
* @author Sean Owen
*/
class GenericGFPoly
{
public:
	// Build a invalid object, so that this can be used in container or return by reference,
	// any access to invalid object is undefined behavior.
	GenericGFPoly() : _field(nullptr) {}

	/**
	* @param field the {@link GenericGF} instance representing the field to use
	* to perform computations
	* @param coefficients coefficients as ints representing elements of GF(size), arranged
	* from most significant (highest-power term) coefficient to least significant
	* @throws IllegalArgumentException if argument is null or empty,
	* or if leading coefficient is 0 and this is not a
	* constant polynomial (that is, it is not the monomial "0").
	*/
	GenericGFPoly(const GenericGF& field, const std::vector<int>& coefficients);

	const std::vector<int>& coefficients() const {
		return _coefficients;
	}

	/**
	* @return degree of this polynomial
	*/
	int degree() const {
		return _coefficients.size() - 1;
	}

	/**
	* @return true iff this polynomial is the monomial "0"
	*/
	bool isZero() const {
		return _coefficients[0] == 0;
	}

	/**
	* @return coefficient of x^degree term in this polynomial
	*/
	int coefficient(int degree) const {
		return _coefficients[_coefficients.size() - 1 - degree];
	}

	/**
	* @return evaluation of this polynomial at a given point
	*/
	int evaluateAt(int a) const;

	GenericGFPoly addOrSubtract(const GenericGFPoly& other) const;
	GenericGFPoly multiply(const GenericGFPoly& other) const;
	GenericGFPoly multiply(int scalar) const;
	GenericGFPoly multiplyByMonomial(int degree, int coefficient) const;
	void divide(const GenericGFPoly& other, GenericGFPoly& quotient, GenericGFPoly& remainder) const;


	friend void swap(GenericGFPoly& a, GenericGFPoly& b)
	{
		using std::swap;
		swap(a._field, b._field);
		swap(a._coefficients, b._coefficients);
	}

private:
	const GenericGF* _field;
	std::vector<int> _coefficients;
};

} // ZXing
