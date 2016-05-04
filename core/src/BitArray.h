#pragma once
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

#include <cstdint>
#include <vector>

namespace ZXing {

class ByteArray;

/**
* <p>A simple, fast array of bits, represented compactly by an array of ints internally.</p>
*
* @author Sean Owen
*/
class BitArray
{
	int _size;
	std::vector<uint32_t> _bits;
	
	friend class BitMatrix;

public:

	BitArray() : _size(0) {}

	explicit BitArray(int size) : _size(size), _bits((size + 31) / 32, 0) {}

	void init(int size) {
		_size = size;
		_bits.resize((size + 31) / 32);
		clear();
	}

	int size() const {
		return _size;
	}
	
	int sizeInBytes() const {
		return (_size + 7) / 8;
	}

	/**
	* @param i bit to get
	* @return true iff bit i is set
	*/
	bool get(int i) const {
		return (_bits[i / 32] & (1 << (i & 0x1F))) != 0;
	}

	/**
	* Sets bit i.
	*
	* @param i bit to set
	*/
	void set(int i) {
		_bits[i / 32] |= 1 << (i & 0x1F);
	}

	/**
	* Flips bit i.
	*
	* @param i bit to set
	*/
	void flip(int i) {
		_bits[i / 32] ^= 1 << (i & 0x1F);
	}

	void flipAll();

	/**
	* @param from first bit to check
	* @return index of first bit that is set, starting from the given index, or size if none are set
	*  at or beyond this given index
	* @see #getNextUnset(int)
	*/
	int getNextSet(int from) const;

	/**
	* @param from index to start looking for unset bit
	* @return index of next unset bit, or {@code size} if none are unset until the end
	* @see #getNextSet(int)
	*/
	int getNextUnset(int from) const;

	/**
	* Sets a block of 32 bits, starting at bit i.
	*
	* @param i first bit to set
	* @param newBits the new value of the next 32 bits. Note again that the least-significant bit
	* corresponds to bit i, the next-least-significant to i+1, and so on.
	*/
	//void setBulk(int i, uint32_t newBits) {
	//	_bits[i / 32] = newBits;
	//}

	void getSubArray(int offset, int length, BitArray& result) const;

	/**
	* Sets a range of bits.
	*
	* @param start start of range, inclusive.
	* @param end end of range, exclusive
	*/
	void setRange(int start, int end);

	/**
	* Clears all bits (sets to false).
	*/
	void clear();

	/**
	* Efficient method to check if a range of bits is set, or not set.
	*
	* @param start start of range, inclusive.
	* @param end end of range, exclusive
	* @param value if true, checks that bits in range are set, otherwise checks that they are not set
	* @return true iff all bits are set or not set in range, according to value argument
	* @throws IllegalArgumentException if end is less than or equal to start
	*/
	bool isRange(int start, int end, bool value) const;


	void appendBit(bool bit);

	/**
	* Appends the least-significant bits, from value, in order from most-significant to
	* least-significant. For example, appending 6 bits from 0x000001E will append the bits
	* 0, 1, 1, 1, 1, 0 in that order.
	*
	* @param value {@code int} containing bits to append
	* @param numBits bits from value to append
	*/
	void appendBits(int value, int numBits);

	void appendBitArray(const BitArray& other);

	void xor(const BitArray& other);

	/**
	*
	* @param bitOffset first bit to start writing
	* @param array array to write into. Bytes are written most-significant byte first. This is the opposite
	*  of the internal representation, which is exposed by {@link #getBitArray()}
	* @param offset position in array to start writing
	* @param numBytes how many bytes to write
	*/
	ByteArray toBytes(int bitOffset, int offset, int numBytes) const;

	/**
	* @return underlying array of ints. The first element holds the first 32 bits, and the least
	*         significant bit is bit 0.
	*/
	//const std::vector<uint32_t>& bitArray() const { return _bits; }

	/**
	* Reverses all bits in the array.
	*/
	void reverse();

	friend bool operator==(const BitArray& a, const BitArray& b)
	{
		return a._size == b._size && b._bits == b._bits;
	}

	//public String toString() {
	//	StringBuilder result = new StringBuilder(size);
	//	for (int i = 0; i < size; i++) {
	//		if ((i & 0x07) == 0) {
	//			result.append(' ');
	//		}
	//		result.append(get(i) ? 'X' : '.');
	//	}
	//	return result.toString();
	//}

private:
	void ensureCapacity(int size)
	{
		_bits.reserve((size + 31) / 32);
	}

	void shiftLeft(unsigned offset);
};

} // ZXing
