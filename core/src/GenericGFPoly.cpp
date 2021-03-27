/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
* Copyright 2017 Axel Waggershauser
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
#include "ZXConfig.h"
#include "ZXContainerAlgorithms.h"

#include <algorithm>
#include <cassert>
#include <stdexcept>

namespace ZXing {

int
GenericGFPoly::evaluateAt(int a) const
{
	if (a == 0)
		// Just return the x^0 coefficient
		return constant();

	if (a == 1)
		// Just the sum of the coefficients
		return Reduce(_coefficients, 0, [](auto a, auto b) { return a ^ b; });

	int result = _coefficients[0];
	for (size_t i = 1; i < _coefficients.size(); ++i)
		result = _field->multiply(a, result) ^ _coefficients[i];
	return result;
}

GenericGFPoly& GenericGFPoly::addOrSubtract(GenericGFPoly& other)
{
	assert(_field == other._field); // "GenericGFPolys do not have same GenericGF field"
	
	if (isZero()) {
		swap(*this, other);
		return *this;
	}
	
	if (other.isZero())
		return *this;

	auto& smallerCoefs = other._coefficients;
	auto& largerCoefs = _coefficients;
	if (smallerCoefs.size() > largerCoefs.size())
		std::swap(smallerCoefs, largerCoefs);

	size_t lengthDiff = largerCoefs.size() - smallerCoefs.size();

	// high-order terms only found in higher-degree polynomial's coefficients stay untouched
	for (size_t i = lengthDiff; i < largerCoefs.size(); ++i)
		largerCoefs[i] ^= smallerCoefs[i - lengthDiff];

	normalize();
	return *this;
}

GenericGFPoly&
GenericGFPoly::multiply(const GenericGFPoly& other)
{
	assert(_field == other._field); // "GenericGFPolys do not have same GenericGF field"

	if (isZero() || other.isZero())
		return setMonomial(0);

	auto& a = _coefficients;
	auto& b = other._coefficients;

	// To disable the use of the malloc cache, simply uncomment:
	// Coefficients _cache;
	_cache.resize(a.size() + b.size() - 1);
	std::fill(_cache.begin(), _cache.end(), 0);
	for (size_t i = 0; i < a.size(); ++i)
		for (size_t j = 0; j < b.size(); ++j)
			_cache[i + j] ^= _field->multiply(a[i], b[j]);

	_coefficients.swap(_cache);

	normalize();
	return *this;
}

GenericGFPoly&
GenericGFPoly::multiplyByMonomial(int coefficient, int degree)
{
	assert(degree >= 0);

	if (coefficient == 0)
		return setMonomial(0);

	for (int& c : _coefficients)
		c = _field->multiply(c, coefficient);

	_coefficients.resize(_coefficients.size() + degree, 0);

	normalize();
	return *this;
}

GenericGFPoly&
GenericGFPoly::divide(const GenericGFPoly& other, GenericGFPoly& quotient)
{
	assert(_field == other._field); // "GenericGFPolys do not have same GenericGF field"

	if (other.isZero())
		throw std::invalid_argument("Divide by 0");

	quotient.setField(*_field);
	if (degree() < other.degree()) {
		// the remainder is this and the quotient is 0
		quotient.setMonomial(0);
		return *this;
	}

	// use Expanded Synthetic Division (see https://en.wikiversity.org/wiki/Reed%E2%80%93Solomon_codes_for_coders):
	// we use the memory from this (the dividend) and swap it with quotient, which will then accumulate the result as
	// [quotient : remainder]. we later copy back the remainder into this and shorten the quotient.
	std::swap(*this, quotient);
	auto& divisor = other._coefficients;
	auto& result = quotient._coefficients;
	auto normalizer = _field->inverse(divisor[0]);
	for (int i = 0; i < Size(result) - (Size(divisor) - 1); ++i) {
		auto& ci = result[i];
		if (ci == 0)
			continue;

		ci = _field->multiply(ci, normalizer);

		// we always skip the first coefficient of the divisor, because it's only used to normalize the dividend coefficient
		for (int j = 1; j < Size(divisor); ++j)
			result[i + j] ^= _field->multiply(divisor[j], ci); // equivalent to: result[i + j] += -divisor[j] * ci
	}

	// extract the normalized remainder from result
	auto firstNonZero = std::find_if(result.end() - other.degree(), result.end(), [](int c){ return c != 0; });
	if (firstNonZero == result.end()) {
		setMonomial(0);
	} else {
		_coefficients.resize(result.end() - firstNonZero);
		std::copy(firstNonZero, result.end(), _coefficients.begin());
	}
	// cut off the tail with the remainder to leave the quotient
	result.resize(result.size() - other.degree());

	return *this;
}

void GenericGFPoly::normalize()
{
	auto firstNonZero = FindIf(_coefficients, [](int c){ return c != 0; });
	// Leading term must be non-zero for anything except the constant polynomial "0"
	if (firstNonZero != _coefficients.begin()) {
		if (firstNonZero == _coefficients.end()) {
			_coefficients.resize(1, 0);
		} else {
			std::copy(firstNonZero, _coefficients.end(), _coefficients.begin());
			_coefficients.resize(_coefficients.end() - firstNonZero);
		}
	}
}

} // namespace ZXing
