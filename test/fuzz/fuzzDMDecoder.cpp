/*
 * Copyright 2021 Axel Waggershauser
 */
// SPDX-License-Identifier: Apache-2.0

#include <stdint.h>
#include <stddef.h>

#include "ByteArray.h"
#include "DecoderResult.h"

using namespace ZXing;

namespace ZXing::DataMatrix::DecodedBitStreamParser {
DecoderResult Decode(ByteArray&& bytes, const bool isDMRE);
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
	if (size < 2)
		return 0;

	ByteArray ba;
	ba.insert(ba.begin(), data, data + size);
	try {
		DataMatrix::DecodedBitStreamParser::Decode(std::move(ba), false);
	} catch (...) {
	}

	return 0;
}
