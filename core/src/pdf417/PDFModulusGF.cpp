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

#include "pdf417/PDFModulusGF.h"

namespace ZXing {
namespace Pdf417 {

ModulusGF::ModulusGF(int modulus, int generator) :
	_modulus(modulus),
	_zero(*this, { 0 }),
	_one(*this, { 1 })
{
	_expTable.resize(modulus, 0);
	_logTable.resize(modulus, 0);
	int x = 1;
	for (int i = 0; i < modulus; i++) {
		_expTable[i] = x;
		x = (x * generator) % modulus;
	}
	for (int i = 0; i < modulus - 1; i++) {
		_logTable[_expTable[i]] = i;
	}
	// logTable[0] == 0 but this should never be used
}

ModulusPoly
ModulusGF::buildMonomial(int degree, int coefficient) const
{
	if (degree < 0) {
		throw std::invalid_argument("degree < 0");
	}
	if (coefficient == 0) {
		return _zero;
	}
	std::vector<int> coefficients(degree + 1, 0);
	coefficients[0] = coefficient;
	return ModulusPoly(*this, coefficients);
}

} // Pdf417
} // ZXing
