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

#include "BitSource.h"
#include "ByteArray.h"

#include <stdexcept>

namespace ZXing {

int
BitSource::available() const
{
	return 8 * (_bytes.length() - _byteOffset) - _bitOffset;
}

int
BitSource::readBits(int numBits)
{
	if (numBits < 1 || numBits > 32 || numBits > available()) {
		throw std::out_of_range("BitSource::readBits: out of range");
	}

	int result = 0;

	// First, read remainder from current byte
	if (_bitOffset > 0) {
		int bitsLeft = 8 - _bitOffset;
		int toRead = numBits < bitsLeft ? numBits : bitsLeft;
		int bitsToNotRead = bitsLeft - toRead;
		int mask = (0xFF >> (8 - toRead)) << bitsToNotRead;
		result = (_bytes[_byteOffset] & mask) >> bitsToNotRead;
		numBits -= toRead;
		_bitOffset += toRead;
		if (_bitOffset == 8) {
			_bitOffset = 0;
			_byteOffset++;
		}
	}

	// Next read whole bytes
	if (numBits > 0) {
		while (numBits >= 8) {
			result = (result << 8) | _bytes[_byteOffset];
			_byteOffset++;
			numBits -= 8;
		}

		// Finally read a partial byte
		if (numBits > 0) {
			int bitsToNotRead = 8 - numBits;
			int mask = (0xFF >> bitsToNotRead) << bitsToNotRead;
			result = (result << numBits) | ((_bytes[_byteOffset] & mask) >> bitsToNotRead);
			_bitOffset += numBits;
		}
	}

	return result;
}

} // ZXing
