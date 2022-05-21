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

namespace Aztec {

/**
* This implementation can detect and decode Aztec codes in an image.
*
* @author David Olivier
*/
class Reader : public ZXing::Reader
{
public:
	explicit Reader(const DecodeHints& hints);
	Result decode(const BinaryBitmap& image) const override;

private:
	bool _isPure;
	std::string _characterSet;
};

} // Aztec
} // ZXing
