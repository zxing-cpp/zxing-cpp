/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Reader.h"
#include <string>

namespace ZXing {

class DecodeHints;

namespace DataMatrix {

class Reader : public ZXing::Reader
{
public:
	using ZXing::Reader::Reader;

	Result decode(const BinaryBitmap& image) const override;
#ifdef __cpp_impl_coroutine
	Results decode(const BinaryBitmap& image, int maxSymbols) const override;
#endif
};

} // DataMatrix
} // ZXing
