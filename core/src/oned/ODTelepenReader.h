/*
* Copyright 2026 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "ODRowReader.h"

namespace ZXing::OneD {

class TelepenReader : public RowReader
{
	bool readAlpha, readNumeric;
public:
	explicit TelepenReader(const ReaderOptions& opts)
		: RowReader(opts),
		  readAlpha(opts.hasFormat(BarcodeFormat::TelepenAlpha)),
		  readNumeric(opts.hasFormat(BarcodeFormat::TelepenNumeric))
	{}

	BarcodeData decodePattern(int rowNumber, PatternView& next, std::unique_ptr<DecodingState>&) const override;
};

} // namespace ZXing::OneD
