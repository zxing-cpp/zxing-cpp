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
* <p>Decodes Codabar barcodes.</p>
*
* @author Bas Vijfwinkel
* @author David Walker
*/
class CodabarReader : public RowReader
{
public:
	explicit CodabarReader(const DecodeHints& hints);
	Result decodePattern(int rowNumber, PatternView& next, std::unique_ptr<DecodingState>& state) const override;

private:
	bool _returnStartEnd;
};

} // OneD
} // ZXing
