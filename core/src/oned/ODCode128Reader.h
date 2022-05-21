/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "ODRowReader.h"

namespace ZXing {

class DecodeHints;

namespace OneD {

/**
* <p>Decodes Code 128 barcodes.</p>
*
* @author Sean Owen
*/
class Code128Reader : public RowReader
{
public:
	Result decodePattern(int rowNumber, PatternView& next, std::unique_ptr<DecodingState>&) const override;
};

} // OneD
} // ZXing
