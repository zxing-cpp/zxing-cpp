/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Reader.h"

#include <list>

namespace ZXing::Pdf417 {

/**
* This implementation can detect and decode PDF417 codes in an image.
*
* @author Guenther Grau
*/
class Reader : public ZXing::Reader
{
public:
	using ZXing::Reader::Reader;

	Result decode(const BinaryBitmap& image) const override;
	Results decode(const BinaryBitmap& image, int maxSymbols) const override;
};

} // namespace ZXing::Pdf417
