/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "ODRowReader.h"

namespace ZXing::OneD {

class Code93Reader : public RowReader
{
public:
	using RowReader::RowReader;

	Result decodePattern(int rowNumber, PatternView& next, std::unique_ptr<DecodingState>&) const override;
};

} // namespace ZXing::OneD
