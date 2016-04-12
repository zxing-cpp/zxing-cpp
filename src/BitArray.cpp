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
		// The last byte could be incomplete (i.e. not have 8 bits in
		// it) but there is no problem since 0 XOR 0 == 0.
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
BitArray::reverse()
{
	std::vector<uint32_t> newBits(_bits.size(), 0);
	// reverse all int's first
	int len = (_size - 1) / 32;
	int oldBitsLen = len + 1;
	for (int i = 0; i < oldBitsLen; i++) {
		newBits[len - i] = BitHacks::Reverse(_bits[i]);
	}
	// now correct the int's if the bit size isn't a multiple of 32
	if (_size != oldBitsLen * 32) {
		int leftOffset = oldBitsLen * 32 - _size;
		int mask = 1;
		for (int i = 0; i < 31 - leftOffset; i++) {
			mask = (mask << 1) | 1;
		}
		uint32_t currentInt = (newBits[0] >> leftOffset) & mask;
		for (int i = 1; i < oldBitsLen; i++) {
			int nextInt = newBits[i];
			currentInt |= nextInt << (32 - leftOffset);
			newBits[i - 1] = currentInt;
			currentInt = (nextInt >> leftOffset) & mask;
		}
		newBits[oldBitsLen - 1] = currentInt;
	}
	_bits = newBits;
}

}