/*
 * Copyright 2021 Axel Waggershauser
 */
// SPDX-License-Identifier: Apache-2.0

#include <stdint.h>
#include <stddef.h>

#include "ByteArray.h"
#include "datamatrix/DMHighLevelEncoder.h"

using namespace ZXing;
using namespace ZXing::DataMatrix;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    std::wstring txt(data, data + size);
	try {
		Encode(txt);
	} catch (...) {
	}

	return 0;
}
