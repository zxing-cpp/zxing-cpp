/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <string>

namespace ZXing {

class DecoderResult;
class BitMatrix;

namespace MaxiCode {

class Decoder
{
public:
	static DecoderResult Decode(const BitMatrix& bits, const std::string& characterSet);
};

} // MaxiCode
} // ZXing
