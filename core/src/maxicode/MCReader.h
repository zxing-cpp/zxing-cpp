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

namespace MaxiCode {

class Reader : public ZXing::Reader
{
public:
	using ZXing::Reader::Reader;

	Result decode(const BinaryBitmap& image) const override;
};

} // MaxiCode
} // ZXing
