#pragma once
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

#include <cstdint>
#include <vector>
#include <cstring>

namespace ZXing {

/**
* <p>A simple, fast array of bits, represented compactly by an array of ints internally.</p>
*
* @author Sean Owen
*/
class BitArray
{
public:

	class Iterator
	{
	public:
		bool operator*() const { return (*_value & _mask) != 0; }
		void operator++() { if ((_mask <<= 1) == 0) { _mask = 1; ++_value; } }
	private:
		Iterator(const uint32_t* p, uint32_t m) : _value(p), _mask(m) {}
		const uint32_t* _value;
		uint32_t _mask;
		friend class BitArray;
	};

	class BackwardIterator
	{
	public:
		bool operator*() const { return (*_value & _mask) != 0; }
		void operator--() { if ((_mask >>= 1) == 0) { _mask = 0x80000000; --_value; } }
	private:
		BackwardIterator(const uint32_t* p, uint32_t m) : _value(p), _mask(m) {}
		const uint32_t* _value;
		uint32_t _mask;
		friend class BitArray;
	};


	BitArray() : _size(0) {}

	explicit BitArray(int size) : _size(size), _bits((size + 31) / 32, 0) {}
	
	BitArray(BitArray &&other) : _size(other._size), _bits(std::move(other._bits)) {}

	BitArray& operator=(BitArray &&other) {
		_size = other._size;
		_bits = std::move(other._bits);
		return *this;
	}

	// Nothing wrong to support it, just to make it explicit, instead of by mistake.
	// Use copyTo() below.
	BitArray(const BitArray &) = delete;
	BitArray& operator=(const BitArray &) = delete;

	void copyTo(BitArray& other) const {
		other._size = _size;
		other._bits = _bits;
	}

	void init(int size) {
		_size = size;
		_bits.resize((size + 31) / 32);
		std::memset(_bits.data(), 0, sizeof(uint32_t) * _bits.size());
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
		return (_bits.at(i >> 5) & (1 << (i & 0x1F))) != 0;
	}

	// If you know exactly how may bits you are going to iterate
	// and that you access bit in sequence, iterator is faster than get().
	// However, be extremly careful since there is no check so whatever
	// (performance is reason to theses iterators to exist at the first place!)
	Iterator iterAt(int i) const {
		return Iterator(_bits.data() + (i >> 5), 1 << (i & 0x1F));
	}

	BackwardIterator backIterAt(int i) const {
		return BackwardIterator(_bits.data() + (i >> 5), 1 << (i & 0x1F));
	}

	/**
	* Sets bit i.
	*
	* @param i bit to set
	*/
	void set(int i) {
		_bits.at(i >> 5) |= 1 << (i & 0x1F);
	}

	/**
	* Flips bit i.
	*
	* @param i bit to set
	*/
	void flip(int i) {
		_bits.at(i >> 5) ^= 1 << (i & 0x1F);
	}

	void flipAll() {
		for (auto it = _bits.begin(); it != _bits.end(); ++it) {
			*it = ~(*it);
		}
	}

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
	void clear() {
		std::memset(_bits.data(), 0, sizeof(uint32_t) * _bits.size());
	}

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

	//void xor(const BitArray& other);

	/**
	*
	* @param bitOffset first bit to start writing
	* @param array array to write into. Bytes are written most-significant byte first. This is the opposite
	*  of the internal representation, which is exposed by {@link #getBitArray()}
	* @param offset position in array to start writing
	* @param numBytes how many bytes to write
	*/
	//ByteArray toBytes(int bitOffset, int offset, int numBytes) const;

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

private:
	int _size;
	std::vector<uint32_t> _bits;

	static void ShiftRight(unsigned offset, std::vector<uint32_t>& bits);

	friend class BitMatrix;
};

} // ZXing
