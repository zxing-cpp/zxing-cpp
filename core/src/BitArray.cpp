/*
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
#include "BitHacks.h"

#include <algorithm>

namespace ZXing {

int
BitArray::getNextSet(int from) const
{
	if (from >= _size) {
		return _size;
	}
	int bitsOffset = from / 32;
	int32_t currentBits = _bits[bitsOffset];
	
	// mask off lesser bits first
	currentBits &= ~((1 << (from & 0x1F)) - 1);
	while (currentBits == 0) {
		if (++bitsOffset == _bits.size()) {
			return _size;
		}
		currentBits = _bits[bitsOffset];
	}
	int result = (bitsOffset * 32) + BitHacks::NumberOfTrailingZeros(currentBits);
	return result > _size ? _size : result;
}

int
BitArray::getNextUnset(int from) const
{
	if (from >= _size) {
		return _size;
	}
	int bitsOffset = from / 32;
	int32_t currentBits = ~_bits[bitsOffset];
	
	// mask off lesser bits first
	currentBits &= ~((1 << (from & 0x1F)) - 1);
	while (currentBits == 0) {
		if (++bitsOffset == _bits.size()) {
			return _size;
		}
		currentBits = ~_bits[bitsOffset];
	}
	int result = (bitsOffset * 32) + BitHacks::NumberOfTrailingZeros(currentBits);
	return result > _size ? _size : result;
}

void
BitArray::setRange(int start, int end)
{
	if (end < start) {
		throw std::invalid_argument("BitArray::setRange(): Invalid range");
	}
	if (end == start) {
		return;
	}
	end--; // will be easier to treat this as the last actually set bit -- inclusive
	int firstInt = start / 32;
	int lastInt = end / 32;
	for (int i = firstInt; i <= lastInt; i++) {
		int firstBit = i > firstInt ? 0 : start & 0x1F;
		int lastBit = i < lastInt ? 31 : end & 0x1F;
		int mask;
		if (firstBit == 0 && lastBit == 31) {
			mask = -1;
		}
		else {
			mask = 0;
			for (int j = firstBit; j <= lastBit; j++) {
				mask |= 1 << j;
			}
		}
		_bits[i] |= mask;
	}
}

void
BitArray::clear()
{
	std::fill(_bits.begin(), _bits.end(), 0);
}

bool
BitArray::isRange(int start, int end, bool value) const
{
	if (end < start) {
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
		int mask;
		if (firstBit == 0 && lastBit == 31) {
			mask = -1;
		}
		else {
			mask = 0;
			for (int j = firstBit; j <= lastBit; j++) {
				mask |= 1 << j;
			}
		}

		// Return false if we're looking for 1s and the masked bits[i] isn't all 1s (that is,
		// equals the mask, or we're looking for 0s and the masked portion is not all 0s
		if ((_bits[i] & mask) != (value ? mask : 0)) {
			return false;
		}
	}
	return true;
}

void
BitArray::appendBit(bool bit)
{
	ensureCapacity(_size + 1);
	if (bit) {
		_bits[_size / 32] |= 1 << (_size & 0x1F);
	}
	_size++;
}

void
BitArray::appendBits(int value, int numBits)
{
	if (numBits < 0 || numBits > 32) {
		throw std::invalid_argument("BitArray::appendBits(): Num bits must be between 0 and 32");
	}
	ensureCapacity(_size + numBits);
	for (int numBitsLeft = numBits; numBitsLeft > 0; numBitsLeft--) {
		appendBit(((value >> (numBitsLeft - 1)) & 0x01) == 1);
	}
}

void
BitArray::appendBitArray(const BitArray& other)
{
	int otherSize = other._size;
	ensureCapacity(_size + otherSize);
	for (int i = 0; i < otherSize; i++) {
		appendBit(other.get(i));
	}
}

void
BitArray::xor(const BitArray& other)
{
	if (_bits.size() != other._bits.size()) {
		throw std::invalid_argument("BitArray::xor(): Sizes don't match");
	}
	for (size_t i = 0; i < _bits.size(); i++) {
		_bits[i] ^= other._bits[i];
	}
}

ByteArray
BitArray::toBytes(int bitOffset, int offset, int numBytes) const
{
	ByteArray result(numBytes);
	for (int i = 0; i < numBytes; i++) {
		int theByte = 0;
		for (int j = 0; j < 8; j++) {
			if (get(bitOffset)) {
				theByte |= 1 << (7 - j);
			}
			bitOffset++;
		}
		result[offset + i] = (uint8_t)theByte;
	}
	return result;
}

void
BitArray::flipAll()
{
	for (auto it = _bits.begin(); it != _bits.end(); ++it) {
		*it = ~(*it);
	}
}

void
BitArray::reverse()
{
	// reverse all int's first
	std::reverse(_bits.begin(), _bits.end());
	std::transform(_bits.begin(), _bits.end(), _bits.begin(), [](uint32_t val) { return BitHacks::Reverse(val); });

	// now correct the int's if the bit size isn't a multiple of 32
	if (_size != _bits.size() * 32) {
		shiftLeft(static_cast<unsigned>(_bits.size()) * 32 - _size);
	}
}

void
BitArray::shiftLeft(unsigned offset)
{
	unsigned rightOffset = 32 - offset;
	for (size_t i = 0; i + 1 < _bits.size(); ++i) {
		_bits[i] = (_bits[i] << offset) | (_bits[i + 1] >> rightOffset);
	}
	_bits.back() <<= offset;
}


void
BitArray::getSubArray(int offset, int length, BitArray& result) const
{
	if (offset < 0 || offset + length > _size) {
		throw std::invalid_argument("Invalid range");
	}

	result._size = length;

	int startIndex = offset / 32;
	int endIndex = (offset + length + 31) / 32;
	
	result._bits.resize(endIndex - startIndex);
	std::copy_n(_bits.begin() + startIndex, result._bits.size(), result._bits.begin());

	unsigned leftOffset = offset % 32;
	if (leftOffset > 0) {
		result.shiftLeft(leftOffset);
		result._bits.resize((length + 31) / 32);
	}
}

} // ZXing
