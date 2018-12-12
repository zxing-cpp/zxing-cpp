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

#include "qrcode/QRDataMask.h"
#include "BitMatrix.h"
#include "ZXContainerAlgorithms.h"

#include <stdexcept>

namespace ZXing {
namespace QRCode {

namespace {

/**
* 000: mask bits for which (x + y) mod 2 == 0
*/
bool DataMask000(int i, int j)
{
	return ((i + j) & 0x01) == 0;
}

/**
* 001: mask bits for which x mod 2 == 0
*/
bool DataMask001(int i, int)
{
	return (i & 0x01) == 0;
}

/**
* 010: mask bits for which y mod 3 == 0
*/
bool DataMask010(int, int j)
{
	return j % 3 == 0;
}

/**
* 011: mask bits for which (x + y) mod 3 == 0
*/
bool DataMask011(int i, int j)
{
	return (i + j) % 3 == 0;
}

/**
* 100: mask bits for which (x/2 + y/3) mod 2 == 0
*/
bool DataMask100(int i, int j)
{
	return (((i / 2) + (j / 3)) & 0x01) == 0;
}

/**
* 101: mask bits for which xy mod 2 + xy mod 3 == 0
* equivalently, such that xy mod 6 == 0
*/
bool DataMask101(int i, int j)
{
	return (i * j) % 6 == 0;
}

/**
* 110: mask bits for which (xy mod 2 + xy mod 3) mod 2 == 0
* equivalently, such that xy mod 6 < 3
*/
bool DataMask110(int i, int j)
{
	return ((i * j) % 6) < 3;
}

/**
* 111: mask bits for which ((x+y)mod 2 + xy mod 3) mod 2 == 0
* equivalently, such that (x + y + xy mod 3) mod 2 == 0
*/
bool DataMask111(int i, int j)
{
	return ((i + j + ((i * j) % 3)) & 0x01) == 0;
}

using IsMaskedFunc = bool (*)(int, int);

/**
* See ISO 18004:2006 6.8.1
*/
static const IsMaskedFunc DATA_MASKS[] = {
	DataMask000,
	DataMask001,
	DataMask010,
	DataMask011,
	DataMask100,
	DataMask101,
	DataMask110,
	DataMask111,
};

} // anonymous

DataMask::DataMask(int reference)
{
	if (reference < 0 || reference >= Length(DATA_MASKS)) {
		throw std::invalid_argument("Invalid data mask");
	}
	_isMasked = DATA_MASKS[reference];
}

void
DataMask::unmaskBitMatrix(BitMatrix& bits, int dimension) const
{
	for (int i = 0; i < dimension; i++) {
		for (int j = 0; j < dimension; j++) {
			if (_isMasked(i, j)) {
				bits.flip(j, i);
			}
		}
	}
}

} // QRCode
} // ZXing
