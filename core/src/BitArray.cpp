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
#include "BitHacks.h"

#include <algorithm>
#include <stdexcept>

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
		if (++bitsOffset == (int)_bits.size()) {
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
		if (++bitsOffset == (int)_bits.size()) {
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
		uint32_t mask;
		if (firstBit == 0 && lastBit == 31) {
			mask = 0xffffffff;
		}
		else {
			mask = 0;
			for (int j = firstBit; j <= lastBit; j++) {
				mask |= 1 << j;
			}
		}

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
		other.copyTo(*this);
	}
	else if (other._size > 0) {
		unsigned offset = static_cast<unsigned>(_bits.size()) * 32 - _size;
		if (offset > 0) {
			auto buffer = other._bits;
			_bits.back() = (_bits.back() & (0xffffffff >> offset)) | (buffer.front() << (32 - offset));
			ShiftRight(offset, buffer);
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

void
BitArray::bitwiseXOR(const BitArray& other)
{
	if (_bits.size() != other._bits.size()) {
		throw std::invalid_argument("BitArray::xor(): Sizes don't match");
	}
	for (size_t i = 0; i < _bits.size(); i++) {
		_bits[i] ^= other._bits[i];
	}
}

void
BitArray::toBytes(int bitOffset, uint8_t* output, int numBytes) const
{
	for (int i = 0; i < numBytes; i++) {
		int theByte = 0;
		for (int j = 0; j < 8; j++) {
			if (get(bitOffset)) {
				theByte |= 1 << (7 - j);
			}
			bitOffset++;
		}
		output[i] = (uint8_t)theByte;
	}
}

void
BitArray::reverse()
{
	// reverse all int's first
	std::reverse(_bits.begin(), _bits.end());
	std::transform(_bits.begin(), _bits.end(), _bits.begin(), [](uint32_t val) { return BitHacks::Reverse(val); });

	// now correct the int's if the bit size isn't a multiple of 32
	unsigned offset = static_cast<unsigned>(_bits.size()) * 32 - _size;
	if (offset > 0) {
		ShiftRight(offset, _bits);
	}
}

/**
* I't matter of convention that "right" here makes sense.
* If you see consider the array from left to right, the shift here is left-shift from we move bits from right to left.
* However since the least significant bit is at right, it's more clear to see the array element starts from right, goes to left.
* In that case, the shift is right-shift.
*/
void
BitArray::ShiftRight(unsigned offset, std::vector<uint32_t>& bits)
{
	if (!bits.empty()) {
		unsigned leftOffset = 32 - offset;
		for (size_t i = 0; i + 1 < bits.size(); ++i) {
			bits[i] = (bits[i] >> offset) | (bits[i + 1] << leftOffset);
		}
		bits.back() >>= offset;
	}
}

void
BitArray::getSubArray(int offset, int length, BitArray& result) const
{
	if (offset < 0 || offset + length > _size) {
		throw std::invalid_argument("Invalid range");
	}
	if (length < 0) {
		length = _size - offset;
	}
	if (length == 0) {
		result._size = 0;
		result._bits.clear();
	}
	else {
		result._size = length;

		int startIndex = offset / 32;
		int endIndex = (offset + length + 31) / 32;

		result._bits.resize(endIndex - startIndex);
		std::copy_n(_bits.begin() + startIndex, result._bits.size(), result._bits.begin());

		unsigned rightOffset = offset % 32;
		if (rightOffset > 0) {
			ShiftRight(rightOffset, result._bits);
			result._bits.resize((length + 31) / 32);
		}
		result._bits.back() &= (0xffffffff >> (result._bits.size() * 32 - result._size));
	}
}

//std::string
//BitArray::toString() const
//{
//	std::string result;
//	result.reserve(_size);
//	for (int i = 0; i < _size; ++i) {
//		if ((i & 0x07) == 0) {
//			result.push_back(' ');
//		}
//		result.push_back(get(i) ? 'X' : '.');
//	}
//	return result;
//}
//

} // ZXing
