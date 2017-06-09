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

namespace ZXing {

ReedSolomonEncoder::ReedSolomonEncoder(const GenericGF& field)
: _field(&field)
{
	_cachedGenerators.push_back(GenericGFPoly(field, { 1 }));
}

const GenericGFPoly&
ReedSolomonEncoder::buildGenerator(int degree)
{
	int cachedGenSize = static_cast<int>(_cachedGenerators.size());
	if (degree >= cachedGenSize) {
		GenericGFPoly lastGenerator = _cachedGenerators.back();
		for (int d = cachedGenSize; d <= degree; d++) {
			lastGenerator.multiply(GenericGFPoly(*_field, { 1, _field->exp(d - 1 + _field->generatorBase()) }));
			_cachedGenerators.push_back(lastGenerator);
		}
	}

	return *std::next(_cachedGenerators.begin(), degree);
}

void
ReedSolomonEncoder::encode(std::vector<int>& toEncode, const int ecBytes)
{
	if (ecBytes == 0) {
		throw std::invalid_argument("No error correction bytes");
	}
	int dataBytes = static_cast<int>(toEncode.size()) - ecBytes;
	if (dataBytes <= 0) {
		throw std::invalid_argument("No data bytes provided");
	}
	GenericGFPoly info = GenericGFPoly(*_field, std::vector<int>(toEncode.begin(), toEncode.begin() + dataBytes));
	info.multiplyByMonomial(ecBytes, 1);
	GenericGFPoly _;
	info.divide(buildGenerator(ecBytes), _);
	auto& coefficients = info.coefficients();
	int numZeroCoefficients = ecBytes - static_cast<int>(coefficients.size());
	std::fill_n(toEncode.begin() + dataBytes, numZeroCoefficients, 0);
	std::copy(coefficients.begin(), coefficients.end(), toEncode.begin() + dataBytes + numZeroCoefficients);
}


} // ZXing
