#pragma once
/*
* Copyright 2016 Huy Cuong Nguyen
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

#include <cstdint>
#include <cassert>
#include <vector>

#if defined(__clang__) || defined(__GNUC__)
#define ZX_HAS_GCC_BUILTINS
#endif

namespace ZXing {
namespace BitHacks {

/**
* The code below is taken from https://graphics.stanford.edu/~seander/bithacks.html
* All credits go to Sean Eron Anderson and other authors mentioned in that page.
*/

/// <summary>
/// Compute the number of zero bits on the left.
/// </summary>
inline int NumberOfLeadingZeros(uint32_t x)
{
	if (x == 0)
		return 32;
#ifdef ZX_HAS_GCC_BUILTINS
	return __builtin_clz(x);
#else
	int n = 0;
	if ((x & 0xFFFF0000) == 0) { n = n + 16; x = x << 16; }
	if ((x & 0xFF000000) == 0) { n = n + 8; x = x << 8; }
	if ((x & 0xF0000000) == 0) { n = n + 4; x = x << 4; }
	if ((x & 0xC0000000) == 0) { n = n + 2; x = x << 2; }
	if ((x & 0x80000000) == 0) { n = n + 1; }
	return n;
#endif
}

/// <summary>
/// Compute the number of zero bits on the right.
/// </summary>
inline int NumberOfTrailingZeros(uint32_t v)
{
#ifdef ZX_HAS_GCC_BUILTINS
	assert(v != 0);
	return __builtin_ctz(v);
#else
	int c = 32;
	v &= -int32_t(v);
	if (v) c--;
	if (v & 0x0000FFFF) c -= 16;
	if (v & 0x00FF00FF) c -= 8;
	if (v & 0x0F0F0F0F) c -= 4;
	if (v & 0x33333333) c -= 2;
	if (v & 0x55555555) c -= 1;
	return c;
#endif
}

inline uint32_t Reverse(uint32_t v)
{
#if 0
	return __builtin_bitreverse32(v);
#else
	v = ((v >> 1) & 0x55555555) | ((v & 0x55555555) << 1);
	// swap consecutive pairs
	v = ((v >> 2) & 0x33333333) | ((v & 0x33333333) << 2);
	// swap nibbles ...
	v = ((v >> 4) & 0x0F0F0F0F) | ((v & 0x0F0F0F0F) << 4);
	// swap bytes
	v = ((v >> 8) & 0x00FF00FF) | ((v & 0x00FF00FF) << 8);
	// swap 2-byte long pairs
	v = (v >> 16) | (v << 16);
	return v;
#endif
}

inline int CountBitsSet(uint32_t v)
{
#ifdef ZX_HAS_GCC_BUILTINS
	return __builtin_popcount(v);
#else
	v = v - ((v >> 1) & 0x55555555);							// reuse input as temporary
	v = (v & 0x33333333) + ((v >> 2) & 0x33333333);				// temp
	return (((v + (v >> 4)) & 0xF0F0F0F) * 0x1010101) >> 24;	// count
#endif
}

// this is the same as log base 2 of v
inline int HighestBitSet(uint32_t v)
{
	return 31 - NumberOfLeadingZeros(v);
}

// shift a whole array of bits by offset bits to the right (thinking of the array as a contiguous stream of bits
// starting with the LSB of the first int and ending with the MSB of the last int, this is actually a left shift)
template <typename T>
void ShiftRight(std::vector<T>& bits, std::size_t offset)
{
	assert(offset < sizeof(T) * 8);

	if (offset == 0 || bits.empty())
		return;

	std::size_t leftOffset = sizeof(T) * 8 - offset;
	for (std::size_t i = 0; i < bits.size() - 1; ++i) {
		bits[i] = (bits[i] >> offset) | (bits[i + 1] << leftOffset);
	}
	bits.back() >>= offset;
}

// reverse a whole array of bits. padding is the number of 'dummy' bits at the end of the array
template <typename T>
void Reverse(std::vector<T>& bits, std::size_t padding)
{
	static_assert(sizeof(T) == sizeof(uint32_t), "Reverse only implemented for 32 bit types");

	// reverse all int's first (reversing the ints in the array and the bits in the ints at the same time)
	auto first = bits.begin(), last = bits.end();
	for (; first < --last; ++first) {
		auto t = *first;
		*first = BitHacks::Reverse(*last);
		*last = BitHacks::Reverse(t);
	}
	if (first == last)
		*last = BitHacks::Reverse(*last);

	// now correct the int's if the bit size isn't a multiple of 32
	ShiftRight(bits, padding);
}

} // BitHacks
} // ZXing
