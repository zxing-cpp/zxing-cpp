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

#ifdef __clang__
#define ZX_HAS_GCC_BUILTINS
#endif

namespace ZXing {

/**
* The code below is taken from https://graphics.stanford.edu/~seander/bithacks.html
* All credits go to Sean Eron Anderson and other authors mentioned in that page.
*/
class BitHacks
{
public:
	/// <summary>
	/// Compute the number of zero bits on the left.
	/// </summary>
	static int NumberOfLeadingZeros(uint32_t x)
	{
		if (x == 0)
			return 32;
#ifdef ZX_HAS_GCC_BUILTINS
		return __builtin_ctz(x);
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
	static int NumberOfTrailingZeros(uint32_t v)
	{
#ifdef ZX_HAS_GCC_BUILTINS
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

	static uint32_t Reverse(uint32_t v)
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

	static int CountBitsSet(uint32_t v)
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
	static int HighestBitSet(uint32_t v)
	{
		uint32_t r;
		uint32_t shift;
		r =     (v > 0xFFFF) << 4; v >>= r;
		shift = (v > 0xFF  ) << 3; v >>= shift; r |= shift;
		shift = (v > 0xF   ) << 2; v >>= shift; r |= shift;
		shift = (v > 0x3   ) << 1; v >>= shift; r |= shift;
		                                        r |= (v >> 1);
		return r;
	}

	// shift a whole array of bits by offset bits to the right
	static void ShiftRight(std::vector<uint32_t>& bits, size_t offset)
	{
		assert(offset < 32);

		if (offset == 0 || bits.empty())
			return;

		size_t leftOffset = 32 - offset;
		for (size_t i = 0; i + 1 < bits.size(); ++i) {
			bits[i] = (bits[i] >> offset) | (bits[i + 1] << leftOffset);
		}
		bits.back() >>= offset;
	}

	// reverse a whole array of bits. padding is the number of 'dummy' bits at the end of the array
	static void Reverse(std::vector<uint32_t>& bits, size_t padding)
	{			
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
};

} // ZXing
