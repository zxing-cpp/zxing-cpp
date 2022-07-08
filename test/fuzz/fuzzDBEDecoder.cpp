/*
 * Copyright 2021 Axel Waggershauser
 */
// SPDX-License-Identifier: Apache-2.0

#include <stdint.h>
#include <stddef.h>

#include "BitArray.h"
#include "oned/ODDataBarExpandedBitDecoder.h"

using namespace ZXing;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
	if (size < 2)
		return 0;

	BitArray bits;
	for (size_t i = 0; i < size; ++i)
		bits.appendBits(data[i], 8);

	OneD::DataBar::DecodeExpandedBits(bits);

	return 0;
}
