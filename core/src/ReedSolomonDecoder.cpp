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

#include "ReedSolomonDecoder.h"
#include "GenericGF.h"
#include "DecodeStatus.h"

#include <memory>

namespace ZXing {

// May throw ReedSolomonException
static bool
RunEuclideanAlgorithm(const GenericGF& field, std::vector<int>&& rCoefs, int R, GenericGFPoly& sigma, GenericGFPoly& omega)
{
	GenericGFPoly r(field, std::move(rCoefs));
	GenericGFPoly& tLast = omega;
	GenericGFPoly& t = sigma;
	ZX_THREAD_LOCAL GenericGFPoly q, rLast;

	field.setMonomial(rLast, R, 1);
	field.setZero(tLast);
	field.setOne(t);

	// Assume r's degree is < rLast's
	if (r.degree() >= rLast.degree()) {
		swap(r, rLast);
	}

	// Run Euclidean algorithm until r's degree is less than R/2
	while (r.degree() >= R / 2) {
		swap(tLast, t);
		swap(rLast, r);

		// Divide rLastLast by rLast, with quotient in q and remainder in r
		if (rLast.isZero()) {
			// Oops, Euclidean algorithm already terminated?
			//throw ReedSolomonException("r_{i-1} was zero");
			return false;
		}

		r.divide(rLast, q);

		q.multiply(tLast);
		q.addOrSubtract(t);
		swap(t, q); // t = q

		if (r.degree() >= rLast.degree()) {
			throw std::runtime_error("Division algorithm failed to reduce polynomial?");
		}
	}

	int sigmaTildeAtZero = t.coefficient(0);
	if (sigmaTildeAtZero == 0) {
		return false;
	}

	int inverse = field.inverse(sigmaTildeAtZero);
	t.multiply(inverse);
	r.multiply(inverse);

	// sigma is t
	omega = std::move(r);
	return true;
}

// May throw ReedSolomonException
static std::vector<int>
FindErrorLocations(const GenericGF& field, const GenericGFPoly& errorLocator)
{
	// This is a direct application of Chien's search
	int numErrors = errorLocator.degree();
	std::vector<int> outLocations(numErrors);
	if (numErrors == 1) { // shortcut
		outLocations[0] = errorLocator.coefficient(1);
	}
	int e = 0;
	for (int i = 1; i < field.size() && e < numErrors; i++) {
		if (errorLocator.evaluateAt(i) == 0) {
			outLocations[e] = field.inverse(i);
			e++;
		}
	}
	if (e != numErrors) {
		//throw ReedSolomonException("Error locator degree does not match number of roots");
		return {};
	}
	return outLocations;
}

static std::vector<int>
FindErrorMagnitudes(const GenericGF& field, const GenericGFPoly& errorEvaluator, const std::vector<int>& errorLocations)
{
	// This is directly applying Forney's Formula
	size_t s = errorLocations.size();
	std::vector<int> outMagnitudes(s);
	for (size_t i = 0; i < s; ++i) {
		int xiInverse = field.inverse(errorLocations[i]);
		int denominator = 1;
		for (size_t j = 0; j < s; ++j) {
			if (i != j) {
				//denominator = field.multiply(denominator,
				//    GenericGF.addOrSubtract(1, field.multiply(errorLocations[j], xiInverse)));
				// Above should work but fails on some Apple and Linux JDKs due to a Hotspot bug.
				// Below is a funny-looking workaround from Steven Parkes
				int term = field.multiply(errorLocations[j], xiInverse);
				int termPlus1 = (term & 0x1) == 0 ? term | 1 : term & ~1;
				denominator = field.multiply(denominator, termPlus1);
			}
		}
		outMagnitudes[i] = field.multiply(errorEvaluator.evaluateAt(xiInverse), field.inverse(denominator));
		if (field.generatorBase() != 0) {
			outMagnitudes[i] = field.multiply(outMagnitudes[i], xiInverse);
		}
	}
	return outMagnitudes;
}


bool
ReedSolomonDecoder::Decode(const GenericGF& field, std::vector<int>& received, int twoS)
{
	GenericGFPoly poly(field, received);
	std::vector<int> syndromeCoefficients(twoS, 0);
	bool noError = true;
	for (int i = 0; i < twoS; i++) {
		int eval = poly.evaluateAt(field.exp(i + field.generatorBase()));
		syndromeCoefficients[twoS - 1 - i] = eval;
		if (eval != 0) {
			noError = false;
		}
	}
	if (noError) {
		return true;
	}

	ZX_THREAD_LOCAL GenericGFPoly sigma, omega;

	if (!RunEuclideanAlgorithm(field, std::move(syndromeCoefficients), twoS, sigma, omega))
		return false;

	auto errorLocations = FindErrorLocations(field, sigma);
	if (errorLocations.empty())
		return false;

	auto errorMagnitudes = FindErrorMagnitudes(field, omega, errorLocations);

	int receivedCount = static_cast<int>(received.size());
	for (size_t i = 0; i < errorLocations.size(); ++i) {
		int position = receivedCount - 1 - field.log(errorLocations[i]);
		if (position < 0)
			return false;

		received[position] = field.addOrSubtract(received[position], errorMagnitudes[i]);
	}
	return true;
}

} // ZXing
