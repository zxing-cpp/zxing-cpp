/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "ODRowReader.h"

namespace ZXing::OneD {

/**
* @brief A reader that can read all available UPC/EAN formats.
*/
class MultiUPCEANReader : public RowReader
{
	bool readEAN8, readEAN13, readUPCA, readUPCE;
public:
	explicit MultiUPCEANReader(const ReaderOptions& opts)
		: RowReader(opts),
		  readEAN8(opts.hasFormat(BarcodeFormat::EAN8)),
		  readEAN13(opts.hasFormat(BarcodeFormat::EAN13)),
		  readUPCA(opts.hasFormat(BarcodeFormat::UPCA)),
		  readUPCE(opts.hasFormat(BarcodeFormat::UPCE))
	{}

	BarcodeData decodePattern(int rowNumber, PatternView& next, std::unique_ptr<DecodingState>&) const override;
};

} // namespace ZXing::OneD

