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

#include "BitArray.h"

#include "ByteArray.h"

#include <cstddef>
#include <stdexcept>

#ifndef ZX_FAST_BIT_STORAGE
#include "BitHacks.h"
#include <algorithm>
#endif

namespace ZXing {

#ifndef ZX_FAST_BIT_STORAGE

bool
BitArray::isRange(int start, int end, bool value) const
{
	if (end < start || start < 0 || end > _size) {
		throw std::invalid_argument("BitArray::isRange(): Invalid range");
	}
	if (end == start) {
		return true; // empty range matches
	}
	end--; // will be easier to treat this as the last actually set bit -- inclusive
	int firstInt = start / 32;
	int lastInt = end / 32;
	for (int i = firstInt; i <= lastInt; i++) {
		int firstBit = i > firstInt ? 0 : start & 0x1F;
		int lastBit = i < lastInt ? 31 : end & 0x1F;
		// Ones from firstBit to lastBit, inclusive
		uint32_t mask = (2UL << lastBit) - (1UL << firstBit);
		// Return false if we're looking for 1s and the masked bits[i] isn't all 1s (that is,
		// equals the mask, or we're looking for 0s and the masked portion is not all 0s
		if ((_bits[i] & mask) != (value ? mask : 0U)) {
			return false;
		}
	}
	return true;
}

void
BitArray::appendBits(int value, int numBits)
{
	if (numBits < 0 || numBits > 32) {
		throw std::invalid_argument("BitArray::appendBits(): Num bits must be between 0 and 32");
	}
	int i = _size;
	_size += numBits;
	_bits.resize((_size + 31) / 32, 0);

	for (--numBits; numBits >= 0; --numBits, ++i) {
		_bits[i / 32] |= ((value >> numBits) & 1) << (i & 0x1F);
	}
}

void
BitArray::appendBit(bool bit)
{
	_bits.resize((_size + 1 + 31) / 32, 0);
	if (bit) {
		_bits[_size / 32] |= 1 << (_size & 0x1F);
	}
	_size++;
}


void
BitArray::appendBitArray(const BitArray& other)
{
	if (_bits.empty()) {
		*this = other.copy();
	}
	else if (other._size > 0) {
		unsigned offset = static_cast<unsigned>(_bits.size()) * 32 - _size;
		if (offset > 0) {
			auto buffer = other._bits;
			_bits.back() = (_bits.back() & (0xffffffff >> offset)) | (buffer.front() << (32 - offset));
			BitHacks::ShiftRight(buffer, offset);
			size_t prevBlockSize = _bits.size();
			_size += other._size;
			_bits.resize((_size + 31) / 32);
			std::copy_n(buffer.begin(), _bits.size() - prevBlockSize, _bits.begin() + prevBlockSize);
		}
		else {
			_size += other._size;
			_bits.insert(_bits.end(), other._bits.begin(), other._bits.end());
		}
	}
}
#endif

void
BitArray::bitwiseXOR(const BitArray& other)
{
	if (size() != other.size()) {
		throw std::invalid_argument("BitArray::xor(): Sizes don't match");
	}
	for (size_t i = 0; i < _bits.size(); i++) {
		// The last int could be incomplete (i.e. not have 32 bits in
		// it) but there is no problem since 0 XOR 0 == 0.
		_bits[i] ^= other._bits[i];
	}
}

ByteArray BitArray::toBytes(int bitOffset, int numBytes) const
{
	ByteArray res(numBytes == -1 ? (size() - bitOffset + 7) / 8 : numBytes);
	for (int i = 0; i < Size(res); i++)
		for (int j = 0; j < 8; j++)
			AppendBit(res[i], (numBytes != -1 || bitOffset < size()) ? get(bitOffset++) : 0);
	return res;
}

} // ZXing
