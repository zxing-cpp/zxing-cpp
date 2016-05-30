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

#include "pdf417/PDFErrorCorrection.h"
#include "PDFModulusGF.h"

#include <cstdint>

namespace ZXing {
namespace Pdf417 {

static bool RunEuclideanAlgorithm(ModulusPoly a, ModulusPoly b, int R, ModulusPoly& sigma, ModulusPoly& omega)
{
	using std::swap;

	const ModulusGF& field = ModulusGF::PDF417();

	// Assume a's degree is >= b's
	if (a.degree() < b.degree()) {
		swap(a, b);
	}

	ModulusPoly rLast = a;
	ModulusPoly r = b;
	ModulusPoly tLast = field.zero();
	ModulusPoly t = field.one();

	// Run Euclidean algorithm until r's degree is less than R/2
	while (r.degree() >= R / 2) {
		ModulusPoly rLastLast = rLast;
		ModulusPoly tLastLast = tLast;
		rLast = r;
		tLast = t;

		// Divide rLastLast by rLast, with quotient in q and remainder in r
		if (rLast.isZero()) {
			// Oops, Euclidean algorithm already terminated?
			return false;
		}
		r = rLastLast;
		ModulusPoly q = field.zero();
		int denominatorLeadingTerm = rLast.coefficient(rLast.degree());
		int dltInverse = field.inverse(denominatorLeadingTerm);
		while (r.degree() >= rLast.degree() && !r.isZero()) {
			int degreeDiff = r.degree() - rLast.degree();
			int scale = field.multiply(r.coefficient(r.degree()), dltInverse);
			q = q.add(field.buildMonomial(degreeDiff, scale));
			r = r.subtract(rLast.multiplyByMonomial(degreeDiff, scale));
		}

		t = q.multiply(tLast).subtract(tLastLast).negative();
	}

	int sigmaTildeAtZero = t.coefficient(0);
	if (sigmaTildeAtZero == 0) {
		return false;
	}

	int inverse = field.inverse(sigmaTildeAtZero);
	sigma = t.multiply(inverse);
	omega = r.multiply(inverse);
	return true;
}

static bool FindErrorLocations(const ModulusPoly& errorLocator, std::vector<int>& result)
{
	const ModulusGF& field = ModulusGF::PDF417();
	// This is a direct application of Chien's search
	int numErrors = errorLocator.degree();
	result.resize(numErrors);
	int e = 0;
	for (int i = 1; i < field.size() && e < numErrors; i++) {
		if (errorLocator.evaluateAt(i) == 0) {
			result[e] = field.inverse(i);
			e++;
		}
	}
	return e == numErrors;
}

static void FindErrorMagnitudes(const ModulusPoly& errorEvaluator, const ModulusPoly& errorLocator, const std::vector<int>& errorLocations, std::vector<int>& result)
{
	const ModulusGF& field = ModulusGF::PDF417();
	int errorLocatorDegree = errorLocator.degree();
	std::vector<int> formalDerivativeCoefficients(errorLocatorDegree);
	for (int i = 1; i <= errorLocatorDegree; i++) {
		formalDerivativeCoefficients[errorLocatorDegree - i] = field.multiply(i, errorLocator.coefficient(i));
	}
	
	ModulusPoly formalDerivative(field, formalDerivativeCoefficients);
	// This is directly applying Forney's Formula
	size_t s = errorLocations.size();
	result.resize(s);
	for (size_t i = 0; i < s; i++) {
		int xiInverse = field.inverse(errorLocations[i]);
		int numerator = field.subtract(0, errorEvaluator.evaluateAt(xiInverse));
		int denominator = field.inverse(formalDerivative.evaluateAt(xiInverse));
		result[i] = field.multiply(numerator, denominator);
	}
}

bool
ErrorCorrection::Decode(std::vector<int>& received, int numECCodewords, const std::vector<int>& erasures, int& nbErrors)
{
	const ModulusGF& field = ModulusGF::PDF417();
	ModulusPoly poly(field, received);
	std::vector<int> S(numECCodewords);
	bool error = false;
	for (int i = numECCodewords; i > 0; i--) {
		int eval = poly.evaluateAt(field.exp(i));
		S[numECCodewords - i] = eval;
		if (eval != 0) {
			error = true;
		}
	}

	if (!error) {
		nbErrors = 0;
		return true;
	}

	ModulusPoly knownErrors = field.one();
	for (int erasure : erasures) {
		int b = field.exp(static_cast<int>(received.size()) - 1 - erasure);
		// Add (1 - bx) term:
		ModulusPoly term(field, {field.subtract(0, b), 1});
		knownErrors = knownErrors.multiply(term);
	}

	ModulusPoly syndrome(field, S);
	//syndrome = syndrome.multiply(knownErrors);

	ModulusPoly sigma, omega;
	if (!RunEuclideanAlgorithm(field.buildMonomial(numECCodewords, 1), syndrome, numECCodewords, sigma, omega)) {
		return false;
	}

	//sigma = sigma.multiply(knownErrors);

	std::vector<int> errorLocations;
	if (!FindErrorLocations(sigma, errorLocations)) {
		return false;
	}

	std::vector<int> errorMagnitudes;
	FindErrorMagnitudes(omega, sigma, errorLocations, errorMagnitudes);

	int receivedSize = static_cast<int>(received.size());
	for (size_t i = 0; i < errorLocations.size(); i++) {
		int position = receivedSize - 1 - field.log(errorLocations[i]);
		if (position < 0) {
			return false;
		}
		received[position] = field.subtract(received[position], errorMagnitudes[i]);
	}
	nbErrors = static_cast<int>(errorLocations.size());
	return true;
}

} // Pdf417
} // ZXing
