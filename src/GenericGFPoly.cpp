#include "GenericGFPoly.h"
#include "GenericGF.h"

#include <stdexcept>

namespace ZXing {

GenericGFPoly::GenericGFPoly(const GenericGF& field, const std::vector<int>& coefs) :
	_field(&field)
{
	if (coefs.empty() == 0) {
		throw std::invalid_argument("GenericGFPoly: coefficients cannot be empty.");
	}

	int coefficientsLength = static_cast<int>(coefs.size());
	if (coefficientsLength > 1 && coefs[0] == 0)
	{
		// Leading term must be non-zero for anything except the constant polynomial "0"
		int firstNonZero = 1;
		while (firstNonZero < coefficientsLength && coefs[firstNonZero] == 0) {
			firstNonZero++;
		}
		if (firstNonZero == coefficientsLength) {
			_coefficients.resize(1, 0);
		}
		else {
			_coefficients.resize(coefficientsLength - firstNonZero);
			std::copy(coefs.begin() + firstNonZero, coefs.end(), _coefficients.begin());
		}
	}
	else
	{
		_coefficients = coefs;
	}
}

int
GenericGFPoly::evaluateAt(int a) const
{
	if (a == 0) {
		// Just return the x^0 coefficient
		return coefficient(0);
	}
	int size = static_cast<int>(_coefficients.size());
	if (a == 1) {
		// Just the sum of the coefficients
		int result = 0;
		for (int coef : _coefficients) {
			result = GenericGF::AddOrSubtract(result, coef);
		}
		return result;
	}
	int result = _coefficients[0];
	for (int i = 1; i < size; i++) {
		result = GenericGF::AddOrSubtract(_field->multiply(a, result), _coefficients[i]);
	}
	return result;
}

GenericGFPoly
GenericGFPoly::addOrSubtract(const GenericGFPoly& other) const
{
	using std::swap;

	if (_field != other._field) {
		throw std::invalid_argument("GenericGFPolys do not have same GenericGF field");
	}
	
	if (isZero()) {
		return other;
	}
	
	if (other.isZero()) {
		return *this;
	}

	auto smallerCoefficients = &_coefficients;
	auto largerCoefficients = &other._coefficients;
	if (smallerCoefficients->size() > largerCoefficients->size()) {
		swap(smallerCoefficients, largerCoefficients);
	}

	std::vector<int> sumDiff(largerCoefficients->size());
	int lengthDiff = static_cast<int>(largerCoefficients->size() - smallerCoefficients->size());

	// Copy high-order terms only found in higher-degree polynomial's coefficients
	std::copy(largerCoefficients->begin(), largerCoefficients->begin() + lengthDiff, sumDiff.begin());

	for (size_t i = lengthDiff; i < largerCoefficients->size(); ++i)
	{
		sumDiff[i] = GenericGF::AddOrSubtract((*smallerCoefficients)[i - lengthDiff], (*largerCoefficients)[i]);
	}

	return GenericGFPoly(*_field, sumDiff);
}

GenericGFPoly
GenericGFPoly::multiply(const GenericGFPoly& other) const
{
	if (_field != other._field) {
		throw std::invalid_argument("GenericGFPolys do not have same GenericGF field");
	}
	if (isZero() || other.isZero()) {
		return _field->zero();
	}
	auto& aCoefficients = _coefficients;
	size_t aLength = aCoefficients.size();
	auto& bCoefficients = other._coefficients;
	size_t bLength = bCoefficients.size();
	std::vector<int> product(aLength + bLength - 1);
	for (size_t i = 0; i < aLength; ++i) {
		int aCoeff = aCoefficients[i];
		for (size_t j = 0; j < bLength; ++j) {
			product[i + j] = GenericGF::AddOrSubtract(product[i + j], _field->multiply(aCoeff, bCoefficients[j]));
		}
	}

	return GenericGFPoly(*_field, product);
}

GenericGFPoly
GenericGFPoly::multiply(int scalar) const
{
	if (scalar == 0) {
		return _field->zero();
	}
	if (scalar == 1) {
		return *this;
	}
	size_t size = _coefficients.size();
	std::vector<int> product(size);
	for (size_t i = 0; i < size; ++i) {
		product[i] = _field->multiply(_coefficients[i], scalar);
	}
	return GenericGFPoly(*_field, product);
}

GenericGFPoly
GenericGFPoly::multiplyByMonomial(int degree, int coefficient) const
{
	if (degree < 0) {
		throw std::invalid_argument("GenericGF::buildMonomial: degree cannot be negative.");
	}
	if (coefficient == 0) {
		return _field->zero();
	}
	size_t size = _coefficients.size();
	std::vector<int> product(size + degree, 0);
	for (size_t i = 0; i < size; ++i) {
		product[i] = _field->multiply(_coefficients[i], coefficient);
	}
	return GenericGFPoly(*_field, product);
}

void
GenericGFPoly::divide(const GenericGFPoly& other, GenericGFPoly& quotient, GenericGFPoly& remainder) const
{
	if (_field != other._field) {
		throw std::invalid_argument("GenericGFPolys do not have same GenericGF field");
	}
	if (other.isZero()) {
		throw std::invalid_argument("Divide by 0");
	}

	quotient = _field->zero();
	remainder = *this;

	int denominatorLeadingTerm = other.coefficient(other.degree());
	int inverseDenominatorLeadingTerm = _field->inverse(denominatorLeadingTerm);

	while (remainder.degree() >= other.degree() && !remainder.isZero()) {
		int degreeDifference = remainder.degree() - other.degree();
		int scale = _field->multiply(remainder.coefficient(remainder.degree()), inverseDenominatorLeadingTerm);
		auto term = other.multiplyByMonomial(degreeDifference, scale);
		auto iterationQuotient = _field->buildMonomial(degreeDifference, scale);
		quotient = quotient.addOrSubtract(iterationQuotient);
		remainder = remainder.addOrSubtract(term);
	}
}

//@Override
//public String toString() {
//	StringBuilder result = new StringBuilder(8 * getDegree());
//	for (int degree = getDegree(); degree >= 0; degree--) {
//		int coefficient = getCoefficient(degree);
//		if (coefficient != 0) {
//			if (coefficient < 0) {
//				result.append(" - ");
//				coefficient = -coefficient;
//			}
//			else {
//				if (result.length() > 0) {
//					result.append(" + ");
//				}
//			}
//			if (degree == 0 || coefficient != 1) {
//				int alphaPower = field.log(coefficient);
//				if (alphaPower == 0) {
//					result.append('1');
//				}
//				else if (alphaPower == 1) {
//					result.append('a');
//				}
//				else {
//					result.append("a^");
//					result.append(alphaPower);
//				}
//			}
//			if (degree != 0) {
//				if (degree == 1) {
//					result.append('x');
//				}
//				else {
//					result.append("x^");
//					result.append(degree);
//				}
//			}
//		}
//	}
//	return result.toString();
//}


} // ZXing

