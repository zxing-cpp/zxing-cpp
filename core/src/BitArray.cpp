/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

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
