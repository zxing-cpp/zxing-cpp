#pragma once
/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
* Copyright 2017 Axel Waggershauser
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

#include "BitHacks.h"

#include <cstdint>
#include <iterator>
#include <vector>
#include <algorithm>

namespace ZXing {

template <typename Iterator>
struct Range {
	Iterator begin, end;
	explicit operator bool() const { return begin < end; }
	int size() const { return end - begin; }
};

/**
* <p>A simple, fast array of bits, represented compactly by an array of ints internally.</p>
*
* @author Sean Owen
*/
class BitArray
{
	int _size = 0;
	std::vector<uint32_t> _bits;

	friend class BitMatrix;

	// Nothing wrong to support it, just to make it explicit, instead of by mistake.
	// Use copy() below.
	BitArray(const BitArray &) = default;
	BitArray& operator=(const BitArray &) = delete;

public:

	class Iterator : public std::iterator<std::bidirectional_iterator_tag, bool, int, bool*, bool>
	{
	public:
		bool operator*() const { return (*_value & _mask) != 0; }

		Iterator& operator++()
		{
			if ((_mask <<= 1) == 0) {
				_mask = 1;
				++_value;
			}
			return *this;
		}

		Iterator& operator--()
		{
			if ((_mask >>= 1) == 0) {
				_mask = 0x80000000;
				--_value;
			}
			return *this;
		}

		int operator-(const Iterator& rhs) const
		{
			return static_cast<int>(_value - rhs._value) * 32 + (_mask >= rhs._mask
																	 ? +BitHacks::CountBitsSet(_mask - rhs._mask)
																	 : -BitHacks::CountBitsSet(rhs._mask - _mask));
		}

		bool operator==(const Iterator& rhs) const { return _mask == rhs._mask && _value == rhs._value; }
		bool operator!=(const Iterator& rhs) const { return !(*this == rhs); }

		bool operator<(const Iterator& rhs) const
		{
			return _value < rhs._value || (_value == rhs._value && _mask < rhs._mask);
		}
		bool operator<=(const Iterator& rhs) const
		{
			return _value < rhs._value || (_value == rhs._value && _mask <= rhs._mask);
		}
		bool operator>(const Iterator& rhs) const { return !(*this <= rhs); }
		bool operator>=(const Iterator& rhs) const { return !(*this < rhs); }

	private:
		Iterator(std::vector<uint32_t>::const_iterator p, uint32_t m) : _value(p), _mask(m) {}
		std::vector<uint32_t>::const_iterator _value;
		uint32_t _mask;
		friend class BitArray;
	};

	using ReverseIterator = std::reverse_iterator<Iterator>;
	using Range = ZXing::Range<Iterator>;

	BitArray() {}

	explicit BitArray(int size) : _size(size), _bits((size + 31) / 32, 0) {}

	BitArray(BitArray&& other) noexcept : _size(other._size), _bits(std::move(other._bits)) {}

	BitArray& operator=(BitArray&& other) noexcept {
		_size = other._size;
		_bits = std::move(other._bits);
		return *this;
	}

	BitArray copy() const {
		return *this;
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
	// However, be extremly careful since there is no check whatsoever.
	// (Performance is the reason for the iterator to exist int the first place!)
	Iterator iterAt(int i) const noexcept { return {_bits.cbegin() + (i >> 5), 1U << (i & 0x1F)}; }

	Iterator begin() const noexcept { return iterAt(0); }
	Iterator end() const noexcept { return iterAt(_size); }
	ReverseIterator rbegin() const noexcept { return ReverseIterator(end()); }
	ReverseIterator rend() const noexcept { return ReverseIterator(begin()); }

	static Iterator getNextSetTo(Iterator begin, Iterator end, bool v) {
		auto i = begin;
		// reconstruct _bits.end()
		auto bitsEnd = end._mask == 0x1 ? end._value : std::next(end._value);
		if (i._value >= bitsEnd)
			return end;
		// mask off lesser bits first
		auto currentBits = (v ? *i._value : ~*i._value) & ~(i._mask - 1);
		while (currentBits == 0) {
			if (++i._value == bitsEnd) {
				return end;
			}
			currentBits = v ? *i._value : ~*i._value;
		}
		i._mask = 1 << BitHacks::NumberOfTrailingZeros(currentBits);
		return i;
	}

	Iterator getNextSetTo(Iterator i, bool v) const {
		return getNextSetTo(i, end(), v);
	}

	Iterator getNextSet(Iterator i) const { return getNextSetTo(i, true); }
	Iterator getNextUnset(Iterator i) const { return getNextSetTo(i, false); }

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
		for (auto& i : _bits) {
			i = ~i;
		}
	}

	/**
	* @param from first bit to check
	* @return index of first bit that is set, starting from the given index, or size if none are set
	*  at or beyond this given index
	* @see #getNextUnset(int)
	*/
	int getNextSet(int from) const {
		return getNextSet(iterAt(from)) - begin();
	}

	/**
	* @param from index to start looking for unset bit
	* @return index of next unset bit, or {@code size} if none are unset until the end
	* @see #getNextSet(int)
	*/
	int getNextUnset(int from) const {
		return getNextUnset(iterAt(from)) - begin();
	}


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
	void clearBits() {
		std::fill(_bits.begin(), _bits.end(), 0);
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

	// Little helper method to make common isRange use case more readable.
	// Pass positive zone size to look for quite zone after i and negative for zone in front of i.
	// Set allowClippedZone to false if clipping the zone at the image border is not acceptable.
	bool hasQuiteZone(Iterator i, int signedZoneSize, bool allowClippedZone = true) const {
		int index = i - begin();
		if (signedZoneSize > 0) {
			if (!allowClippedZone && index + signedZoneSize >= size())
				return false;
			return isRange(index, std::min(size(), index + signedZoneSize), false);
		} else {
			if (!allowClippedZone && index + signedZoneSize < 0)
				return false;
			return isRange(std::max(0, index + signedZoneSize), index, false);
		}
	}

	bool hasQuiteZone(ReverseIterator i, int signedZoneSize, bool allowClippedZone = true) const {
		return hasQuiteZone(i.base(), -signedZoneSize, allowClippedZone);
	}

	/**
	* Appends the least-significant bits, from value, in order from most-significant to
	* least-significant. For example, appending 6 bits from 0x000001E will append the bits
	* 0, 1, 1, 1, 1, 0 in that order.
	*
	* @param value {@code int} containing bits to append
	* @param numBits bits from value to append
	*/
	void appendBits(int value, int numBits);

	void appendBit(bool bit);

	void appendBitArray(const BitArray& other);

	void bitwiseXOR(const BitArray& other);

	/**
	*
	* @param bitOffset first bit to start writing
	* @param ouput array to write into. Bytes are written most-significant byte first. This is the opposite
	*  of the internal representation, which is exposed by {@link #getBitArray()}
	* @param numBytes how many bytes to write
	*/
	void toBytes(int bitOffset, uint8_t* output, int numBytes) const;

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
		return a._size == b._size && a._bits == b._bits;
	}
};

} // ZXing
