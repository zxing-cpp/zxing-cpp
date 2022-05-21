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

namespace QRCode {

/**
* This implementation can detect and decode QR Codes in an image.
*
* @author Sean Owen
*/
class Reader : public ZXing::Reader
{
public:
	explicit Reader(const DecodeHints& hints);
	Result decode(const BinaryBitmap& image) const override;

	Results decode(const BinaryBitmap& image, int maxSymbols) const override;

private:
	bool _tryHarder, _isPure, _testQR, _testMQR;
	std::string _charset;
};

} // QRCode
} // ZXing
