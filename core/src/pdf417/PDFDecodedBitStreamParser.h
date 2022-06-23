/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <string>
#include <vector>

namespace ZXing {

class DecoderResult;

namespace Pdf417 {

/**
* <p>This class contains the methods for decoding the PDF417 codewords.</p>
*
* @author SITA Lab (kevin.osullivan@sita.aero)
* @author Guenther Grau
*/
class DecodedBitStreamParser
{
public:
	static DecoderResult Decode(const std::vector<int>& codewords, int ecLevel);
};

} // Pdf417
} // ZXing
