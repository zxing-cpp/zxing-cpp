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

#include "ReedSolomonEncoder.h"
#include "GenericGF.h"
#include "EncodeStatus.h"

namespace ZXing {

ReedSolomonEncoder::ReedSolomonEncoder(const GenericGF& field)
: _field(&field)
{
	_cachedGenerators.push_back(GenericGFPoly(field, { 1 }));
}

GenericGFPoly
ReedSolomonEncoder::buildGenerator(int degree)
{
	if (degree >= (int)_cachedGenerators.size()) {
		GenericGFPoly lastGenerator = _cachedGenerators.back();
		for (int d = _cachedGenerators.size(); d <= degree; d++) {
			GenericGFPoly nextGenerator = lastGenerator.multiply(GenericGFPoly(*_field, { 1, _field->exp(d - 1 + _field->generatorBase()) }));
			_cachedGenerators.push_back(nextGenerator);
			lastGenerator = nextGenerator;
		}
	}

	auto iter = _cachedGenerators.begin();
	std::advance(iter, degree);
	return *iter;
}

EncodeStatus
ReedSolomonEncoder::encode(std::vector<int>& toEncode, int ecBytes)
{
	if (ecBytes == 0) {
		return EncodeStatus::WithError("No error correction bytes");
	}
	int dataBytes = static_cast<int>(toEncode.size()) - ecBytes;
	if (dataBytes <= 0) {
		return EncodeStatus::WithError("No data bytes provided");
	}
	GenericGFPoly generator = buildGenerator(ecBytes);
	toEncode.resize(dataBytes);
	GenericGFPoly info = GenericGFPoly(*_field, toEncode);
	info = info.multiplyByMonomial(ecBytes, 1);
	GenericGFPoly _, remainder;
	info.divide(generator, _, remainder);
	auto& coefficients = remainder.coefficients();
	int numZeroCoefficients = ecBytes - (int)coefficients.size();
	toEncode.resize(dataBytes + ecBytes);
	for (int i = 0; i < numZeroCoefficients; i++) {
		toEncode[dataBytes + i] = 0;
	}
	std::copy(coefficients.begin(), coefficients.end(), toEncode.begin() + dataBytes + numZeroCoefficients);
}


} // ZXing
