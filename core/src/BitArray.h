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

#include "ZXConfig.h"
#include "ZXContainerAlgorithms.h"
#ifndef ZX_FAST_BIT_STORAGE
#include "BitHacks.h"
#endif

#include <cstdint>
#include <iterator>
#include <vector>
#include <algorithm>
#include <utility>

namespace ZXing {

class ByteArray;

template <typename Iterator>
struct Range {
	Iterator begin, end;
	explicit operator bool() const { return begin < end; }
	int size() const { return static_cast<int>(end - begin); }
};

/**
* A simple, fast array of bits.
*/
class BitArray
{
#ifdef ZX_FAST_BIT_STORAGE
	std::vector<uint8_t> _bits;
#else
	int _size = 0;
	std::vector<uint32_t> _bits;
#endif

	friend class BitMatrix;

	// Nothing wrong to support it, just to make it explicit, instead of by mistake.
	// Use copy() below.
	BitArray(const BitArray &) = default;
	BitArray& operator=(const BitArray &) = delete;

public:

#ifdef ZX_FAST_BIT_STORAGE
	using Iterator = std::vector<uint8_t>::const_iterator;
#else
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
#endif

	using ReverseIterator = std::reverse_iterator<Iterator>;
	using Range = ZXing::Range<Iterator>;

	BitArray() = default;

	explicit BitArray(int size) :
#ifdef ZX_FAST_BIT_STORAGE
								  _bits(size, 0) {}
#else
								  _size(size), _bits((size + 31) / 32, 0) {}
#endif

	BitArray(BitArray&& other) noexcept :
#ifndef ZX_FAST_BIT_STORAGE
										  _size(other._size),
#endif
										  _bits(std::move(other._bits)) {}

	BitArray& operator=(BitArray&& other) noexcept {
#ifndef ZX_FAST_BIT_STORAGE
		_size = other._size;
#endif
		_bits = std::move(other._bits);
		return *this;
	}

	BitArray copy() const {
		return *this;
	}

	int size() const noexcept {
#ifdef ZX_FAST_BIT_STORAGE
		return Size(_bits);
#else
		return _size;
#endif
	}
	
	int sizeInBytes() const noexcept {
		return (size() + 7) / 8;
	}

	/**
	* @param i bit to get
	* @return true iff bit i is set
	*/
	bool get(int i) const {
#ifdef ZX_FAST_BIT_STORAGE
		return _bits.at(i) != 0;
#else
		return (_bits.at(i >> 5) & (1 << (i & 0x1F))) != 0;
#endif
	}

	// If you know exactly how may bits you are going to iterate
	// and that you access bit in sequence, iterator is faster than get().
	// However, be extremly careful since there is no check whatsoever.
	// (Performance is the reason for the iterator to exist in the first place.)
#ifdef ZX_FAST_BIT_STORAGE
	Iterator iterAt(int i) const noexcept { return {_bits.cbegin() + i}; }
	Iterator begin() const noexcept { return _bits.cbegin(); }
	Iterator end() const noexcept { return _bits.cend(); }

	template <typename ITER>
	static ITER getNextSetTo(ITER begin, ITER end, bool v) noexcept {
		while( begin != end && *begin != static_cast<int>(v) )
			++begin;
		return begin;
	}
#else
	Iterator iterAt(int i) const noexcept { return {_bits.cbegin() + (i >> 5), 1U << (i & 0x1F)}; }
	Iterator begin() const noexcept { return iterAt(0); }
	Iterator end() const noexcept { return iterAt(_size); }

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

	static ReverseIterator getNextSetTo(ReverseIterator begin, ReverseIterator end, bool v) {
		while( begin != end && *begin != v )
			++begin;
		return begin;
	}
#endif

	Iterator getNextSetTo(Iterator i, bool v) const {
		return getNextSetTo(i, end(), v);
	}

	Iterator getNextSet(Iterator i) const { return getNextSetTo(i, true); }
	Iterator getNextUnset(Iterator i) const { return getNextSetTo(i, false); }

	ReverseIterator rbegin() const noexcept { return ReverseIterator(end()); }
	ReverseIterator rend() const noexcept { return ReverseIterator(begin()); }

	/**
	* Sets bit i.
	*
	* @param i bit to set
	*/
	void set(int i) {
#ifdef ZX_FAST_BIT_STORAGE
#if 0
		_bits[i] = 1;
#else
		_bits.at(i) = 1;
#endif
#else
		_bits.at(i >> 5) |= 1 << (i & 0x1F);
#endif
	}

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
#ifdef ZX_FAST_BIT_STORAGE
	bool isRange(int start, int end, bool value) const {
		return std::all_of(std::begin(_bits) + start, std::begin(_bits) + end, [value](uint8_t v) { return v == static_cast<int>(value); });
	}
#else
	bool isRange(int start, int end, bool value) const;
#endif

	// Little helper method to make common isRange use case more readable.
	// Pass positive zone size to look for quite zone after i and negative for zone in front of i.
	// Set allowClippedZone to false if clipping the zone at the image border is not acceptable.
	bool hasQuiteZone(Iterator i, int signedZoneSize, bool allowClippedZone = true) const {
		int index = static_cast<int>(i - begin());
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
#ifdef ZX_FAST_BIT_STORAGE
	void appendBits(int value, int numBits) {
		for (; numBits; --numBits)
			_bits.push_back((value >> (numBits-1)) & 1);
	}

	void appendBit(bool bit) {
		_bits.push_back(bit);
	}

	void appendBitArray(const BitArray& other) {
		_bits.insert(_bits.end(), other.begin(), other.end());
	}

	/**
	* Reverses all bits in the array.
	*/
	void reverse() {
		std::reverse(_bits.begin(), _bits.end());
	}
#else
	void appendBits(int value, int numBits);

	void appendBit(bool bit);

	void appendBitArray(const BitArray& other);

	/**
	* Reverses all bits in the array.
	*/
	void reverse() {
		BitHacks::Reverse(_bits, _bits.size() * 32 - _size);
	}
#endif

	void bitwiseXOR(const BitArray& other);

	/**
	*
	* @param bitOffset first bit to start writing
	* @param numBytes how many bytes to write
	* @return Bytes are written most-significant bit first. This is the opposite
	*  of the internal representation, which is exposed by {@link #getBitArray()}
	*/
	ByteArray toBytes(int bitOffset, int numBytes) const;

	friend bool operator==(const BitArray& a, const BitArray& b)
	{
		return
#ifndef ZX_FAST_BIT_STORAGE
			a._size == b._size &&
#endif
			a._bits == b._bits;
	}
};


} // ZXing
