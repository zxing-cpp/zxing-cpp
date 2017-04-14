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
#include "GenericGF.h"

#include <cassert>
#include <stdexcept>

namespace ZXing {

int
GenericGFPoly::evaluateAt(int a) const
{
	if (a == 0) {
		// Just return the x^0 coefficient
		return coefficient(0);
	}
	if (a == 1) {
		// Just the sum of the coefficients
		int result = 0;
		for (int coef : _coefficients) {
			result = _field->addOrSubtract(result, coef);
		}
		return result;
	}
	int result = _coefficients[0];
	for (size_t i = 1; i < _coefficients.size(); ++i) {
		result = _field->addOrSubtract(_field->multiply(a, result), _coefficients[i]);
	}
	return result;
}

GenericGFPoly& GenericGFPoly::addOrSubtract(GenericGFPoly& other)
{
	assert(_field == other._field); // "GenericGFPolys do not have same GenericGF field"
	
	if (isZero()) {
		swap(*this, other);
		return *this;
	}
	
	if (other.isZero()) {
		return *this;
	}

	auto& smallerCoefs = other._coefficients;
	auto& largerCoefs = _coefficients;
	if (smallerCoefs.size() > largerCoefs.size())
		std::swap(smallerCoefs, largerCoefs);

	size_t lengthDiff = largerCoefs.size() - smallerCoefs.size();

	// high-order terms only found in higher-degree polynomial's coefficients stay untouched
	for (size_t i = lengthDiff; i < largerCoefs.size(); ++i)
		largerCoefs[i] = _field->addOrSubtract(smallerCoefs[i - lengthDiff], largerCoefs[i]);

	normalize();
	return *this;
}

GenericGFPoly&
GenericGFPoly::multiply(const GenericGFPoly& other)
{
	assert(_field == other._field); // "GenericGFPolys do not have same GenericGF field"

	if (isZero() || other.isZero())
		return _field->setZero(*this);

	auto& a = _coefficients;
	auto& b = other._coefficients;

	// To disable the use of the malloc cache, simply uncomment:
	// Coefficients _cache;
	_cache.resize(a.size() + b.size() - 1);
	std::fill(_cache.begin(), _cache.end(), 0);
	for (size_t i = 0; i < a.size(); ++i) {
		for (size_t j = 0; j < b.size(); ++j) {
			_cache[i + j] = _field->addOrSubtract(_cache[i + j], _field->multiply(a[i], b[j]));
		}
	}

	_coefficients.swap(_cache);

	normalize();
	return *this;
}

GenericGFPoly&
GenericGFPoly::multiply(int scalar)
{
	if (scalar == 0) {
		return _field->setZero(*this);
	}
	if (scalar == 1) {
		return *this;
	}

	for (int& c : _coefficients) {
		c = _field->multiply(c, scalar);
	}

	normalize();
	return *this;
}

GenericGFPoly&
GenericGFPoly::multiplyByMonomial(int degree, int coefficient)
{
	assert(degree >= 0);

	if (coefficient == 0) {
		return _field->setZero(*this);
	}
	size_t size = _coefficients.size();
	for (size_t i = 0; i < size; ++i) {
		_coefficients[i] = _field->multiply(_coefficients[i], coefficient);
	}
	_coefficients.resize(size + degree, 0);

	normalize();
	return *this;
}

GenericGFPoly&
GenericGFPoly::divide(const GenericGFPoly& other, GenericGFPoly& quotient)
{
	assert(_field == other._field); // "GenericGFPolys do not have same GenericGF field"

	if (other.isZero()) {
		throw std::invalid_argument("Divide by 0");
	}

	_field->setZero(quotient);
	auto& remainder = *this;

	int denominatorLeadingTerm = other.coefficient(other.degree());
	int inverseDenominatorLeadingTerm = _field->inverse(denominatorLeadingTerm);

	ZX_THREAD_LOCAL GenericGFPoly temp;

	while (remainder.degree() >= other.degree() && !remainder.isZero()) {
		int degreeDifference = remainder.degree() - other.degree();
		int scale = _field->multiply(remainder.coefficient(remainder.degree()), inverseDenominatorLeadingTerm);
		_field->setMonomial(temp, degreeDifference, scale);
		quotient.addOrSubtract(temp);
		temp = other;
		temp.multiplyByMonomial(degreeDifference, scale);
		remainder.addOrSubtract(temp);
	}

	return *this;
}

void GenericGFPoly::normalize()
{
	auto& coefs = _coefficients;
	auto firstNonZero = std::find_if(coefs.cbegin(), coefs.cend(), [](int c){ return c != 0; });
	// Leading term must be non-zero for anything except the constant polynomial "0"
	if (firstNonZero != coefs.cbegin())
	{
		if (firstNonZero == coefs.end()) {
			coefs.resize(1, 0);
		}
		else {
			std::copy(firstNonZero, coefs.cend(), coefs.begin());
			coefs.resize(coefs.cend() - firstNonZero);
		}
	}
}

} // ZXing

