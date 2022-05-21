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

/**
* <p>The main class which implements MaxiCode decoding -- as opposed to locating and extracting
* the MaxiCode from an image.</p>
*
* @author Manuel Kasten
*/
class Decoder
{
public:
	static DecoderResult Decode(const BitMatrix& bits, const std::string& characterSet);
};

} // MaxiCode
} // ZXing
