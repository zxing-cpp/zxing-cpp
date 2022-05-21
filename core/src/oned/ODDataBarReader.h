/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
* Copyright 2020 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "DecodeHints.h"
#include "ODRowReader.h"

namespace ZXing {
namespace OneD {

/**
* Decodes DataBar (formerly known as RSS) sybmols, including truncated and stacked variants. See ISO/IEC 24724:2006.
*/
class DataBarReader : public RowReader
{

public:
	explicit DataBarReader(const DecodeHints& hints);
	~DataBarReader() override;

	Result decodePattern(int rowNumber, PatternView& next, std::unique_ptr<DecodingState>& state) const override;
};

} // OneD
} // ZXing
