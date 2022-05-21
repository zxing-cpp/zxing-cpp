/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "ODRowReader.h"

namespace ZXing {
namespace OneD {

/**
* <p>Decodes Code 93 barcodes.</p>
*
* @author Sean Owen
* @see Code39Reader
*/
class Code93Reader : public RowReader
{
public:
	Result decodePattern(int rowNumber, PatternView& next, std::unique_ptr<DecodingState>&) const override;
};

} // OneD
} // ZXing
