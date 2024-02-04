/*
* Copyright 2026 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Reader.h"

namespace ZXing::MicroPdf417 {

/**
* This implementation can detect and decode Micro PDF417 codes in an image.
*/
class Reader : public ZXing::Reader
{
public:
	using ZXing::Reader::Reader;

	BarcodesData read(const BinaryBitmap& image, int maxSymbols) const override;
};

} // namespace ZXing::MicroPdf417
