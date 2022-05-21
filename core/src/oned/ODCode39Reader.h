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
* <p>Decodes Code 39 barcodes. Supports "Full ASCII Code 39" if extendedMode is true.</p>
*
* @author Sean Owen
* @see Code93Reader
*/
class Code39Reader : public RowReader
{
public:
	/**
	* Creates a reader that can be configured to check the last character as a check digit,
	* or optionally attempt to decode "extended Code 39" sequences that are used to encode
	* the full ASCII character set.
	*
	* @param usingCheckDigit if true, treat the last data character as a check digit, not
	* data, and verify that the checksum passes.
	* @param extendedMode if true, will attempt to decode extended Code 39 sequences in the
	* text.
	*/
	explicit Code39Reader(const DecodeHints& hints);
	
	Result decodePattern(int rowNumber, PatternView& next, std::unique_ptr<DecodingState>&) const override;

private:
	bool _extendedMode;
	bool _validateCheckSum;
};

} // OneD
} // ZXing
