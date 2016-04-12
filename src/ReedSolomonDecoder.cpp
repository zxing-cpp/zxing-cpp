#include "ReedSolomonDecoder.h"
#include "GenericGF.h"

namespace ZXing {

namespace {

// throws ReedSolomonException
void runEuclideanAlgorithm(const GenericGF& field, const GenericGFPoly& a, const GenericGFPoly& b, int R, GenericGFPoly& sigma, GenericGFPoly& omega)
{
	using std::swap;

	// Assume a's degree is >= b's
	if (a.degree() < b.degree()) {
		swap(a, b);
	}

	GenericGFPoly rLast = a;
	GenericGFPoly r = b;
	GenericGFPoly tLast = field.zero();
	GenericGFPoly t = field.one();

	// Run Euclidean algorithm until r's degree is less than R/2
	while (r.degree() >= R / 2) {
		GenericGFPoly rLastLast = rLast;
		GenericGFPoly tLastLast = tLast;
		rLast = r;
		tLast = t;

		// Divide rLastLast by rLast, with quotient in q and remainder in r
		if (rLast.isZero()) {
			// Oops, Euclidean algorithm already terminated?
			throw ReedSolomonException("r_{i-1} was zero");
		}
		r = rLastLast;
		GenericGFPoly q = field.zero();
		int denominatorLeadingTerm = rLast.coefficient(rLast.degree());
		int dltInverse = field.inverse(denominatorLeadingTerm);
		while (r.degree() >= rLast.degree() && !r.isZero()) {
			int degreeDiff = r.degree() - rLast.degree();
			int scale = field.multiply(r.coefficient(r.degree()), dltInverse);
			q = q.addOrSubtract(field.buildMonomial(degreeDiff, scale));
			r = r.addOrSubtract(rLast.multiplyByMonomial(degreeDiff, scale));
		}

		t = q.multiply(tLast).addOrSubtract(tLastLast);

		if (r.degree() >= rLast.degree()) {
			throw std::runtime_error("Division algorithm failed to reduce polynomial?");
		}
	}

	int sigmaTildeAtZero = t.coefficient(0);
	if (sigmaTildeAtZero == 0) {
		throw ReedSolomonException("sigmaTilde(0) was zero");
	}

	int inverse = field.inverse(sigmaTildeAtZero);
	sigma = t.multiply(inverse);
	omega = r.multiply(inverse);
}

private int[] findErrorLocations(GenericGFPoly errorLocator) throws ReedSolomonException {
	// This is a direct application of Chien's search
	int numErrors = errorLocator.getDegree();
	if (numErrors == 1) { // shortcut
		return new int[] { errorLocator.getCoefficient(1) };
	}
	int[] result = new int[numErrors];
	int e = 0;
	for (int i = 1; i < field.getSize() && e < numErrors; i++) {
		if (errorLocator.evaluateAt(i) == 0) {
			result[e] = field.inverse(i);
			e++;
		}
	}
	if (e != numErrors) {
		throw new ReedSolomonException("Error locator degree does not match number of roots");
	}
	return result;
}

private int[] findErrorMagnitudes(GenericGFPoly errorEvaluator, int[] errorLocations) {
	// This is directly applying Forney's Formula
	int s = errorLocations.length;
	int[] result = new int[s];
	for (int i = 0; i < s; i++) {
		int xiInverse = field.inverse(errorLocations[i]);
		int denominator = 1;
		for (int j = 0; j < s; j++) {
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
		result[i] = field.multiply(errorEvaluator.evaluateAt(xiInverse),
			field.inverse(denominator));
		if (field.getGeneratorBase() != 0) {
			result[i] = field.multiply(result[i], xiInverse);
		}
	}
	return result;
}


} // anonymous

void
ReedSolomonDecoder::decode(const std::vector<int>& received, int twoS) const
{
	GenericGFPoly poly(*_field, received);
	std::vector<int> syndromeCoefficients(twoS);
	bool noError = true;
	for (int i = 0; i < twoS; i++) {
		int eval = poly.evaluateAt(_field->exp(i + _field->generatorBase()));
		syndromeCoefficients[twoS - 1 - i] = eval;
		if (eval != 0) {
			noError = false;
		}
	}
	if (noError) {
		return;
	}

	GenericGFPoly syndrome(*_field, syndromeCoefficients);
	GenericGFPoly[] sigmaOmega = runEuclideanAlgorithm(field.buildMonomial(twoS, 1), syndrome, twoS);
	GenericGFPoly sigma = sigmaOmega[0];
	GenericGFPoly omega = sigmaOmega[1];
	int[] errorLocations = findErrorLocations(sigma);
	int[] errorMagnitudes = findErrorMagnitudes(omega, errorLocations);
	for (int i = 0; i < errorLocations.length; i++) {
		int position = received.length - 1 - field.log(errorLocations[i]);
		if (position < 0) {
			throw new ReedSolomonException("Bad error location");
		}
		received[position] = GenericGF.addOrSubtract(received[position], errorMagnitudes[i]);
	}
}

} // ZXing