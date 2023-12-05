/*
 * Copyright 2021 Axel Waggershauser
 */
// SPDX-License-Identifier: Apache-2.0

#include <stdint.h>
#include <stddef.h>

#include "BitArray.h"
#include "DecoderResult.h"

using namespace ZXing;

namespace ZXing::Aztec {
DecoderResult Decode(const BitArray& bits);
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
	if (size < 2)
		return 0;

	BitArray bits;
	for (size_t i = 1; i < size - 1; ++i)
		bits.appendBits(data[i], 8);

	bits.appendBits(data[size - 1], (data[0] & 0x7) + 1);

	Aztec::Decode(bits);

	return 0;
}
