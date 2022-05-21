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

/**
* This implementation can detect and decode Data Matrix codes in an image.
*
* @author bbrown@google.com (Brian Brown)
*/
class Reader : public ZXing::Reader
{
	bool _tryRotate, _tryHarder, _isPure;
	std::string _characterSet;
public:
	explicit Reader(const DecodeHints& hints);
	Result decode(const BinaryBitmap& image) const override;
};

} // DataMatrix
} // ZXing
