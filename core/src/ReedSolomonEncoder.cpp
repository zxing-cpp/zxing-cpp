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

#include <algorithm>
#include <iterator>
#include <stdexcept>

namespace ZXing {

ReedSolomonEncoder::ReedSolomonEncoder(const GenericGF& field)
: _field(&field)
{
	_cachedGenerators.push_back(GenericGFPoly(field, { 1 }));
}

const GenericGFPoly&
ReedSolomonEncoder::buildGenerator(int degree)
{
	int cachedGenSize = Size(_cachedGenerators);
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
ReedSolomonEncoder::encode(std::vector<int>& message, const int numECCodeWords)
{
	if (numECCodeWords == 0 || numECCodeWords >= Size(message))
		throw std::invalid_argument("Invalid number of error correction code words");

	GenericGFPoly info = GenericGFPoly(*_field, std::vector<int>(message.begin(), message.end() - numECCodeWords));
	info.multiplyByMonomial(1, numECCodeWords);
	GenericGFPoly _;
	info.divide(buildGenerator(numECCodeWords), _);
	auto& coefficients = info.coefficients();
	int numZeroCoefficients = numECCodeWords - Size(coefficients);
	std::fill_n(message.end() - numECCodeWords, numZeroCoefficients, 0);
	std::copy(coefficients.begin(), coefficients.end(), message.end() - numECCodeWords + numZeroCoefficients);
}

} // ZXing
