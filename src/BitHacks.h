#pragma once
#include <cstdint>

namespace ZXing {

class BitHacks
{
public:
	/// <summary>
	/// Compute the number of zero bits on the right.
	/// </summary>
	static int NumberOfTrailingZeros(uint32_t v)
	{
		// See https://graphics.stanford.edu/~seander/bithacks.html#ZerosOnRightParallel
		int c = 32;
		v &= -int32_t(v);
		if (v) c--;
		if (v & 0x0000FFFF) c -= 16;
		if (v & 0x00FF00FF) c -= 8;
		if (v & 0x0F0F0F0F) c -= 4;
		if (v & 0x33333333) c -= 2;
		if (v & 0x55555555) c -= 1;
		return c;
	}

	static uint32_t Reverse(uint32_t v)
	{
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
	}
};

}