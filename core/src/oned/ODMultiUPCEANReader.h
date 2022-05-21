/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "BarcodeFormat.h"
#include "DecodeHints.h"
#include "ODRowReader.h"

namespace ZXing {
namespace OneD {

/**
* <p>A reader that can read all available UPC/EAN formats. If a caller wants to try to
* read all such formats, it is most efficient to use this implementation rather than invoke
* individual readers.</p>
*
* @author Sean Owen
*/
class MultiUPCEANReader : public RowReader
{
public:
	explicit MultiUPCEANReader(const DecodeHints& hints);
	~MultiUPCEANReader() override;

	Result decodePattern(int rowNumber, PatternView& next, std::unique_ptr<DecodingState>&) const override;

private:
	DecodeHints _hints;
};

} // OneD
} // ZXing
