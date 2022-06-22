/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "ODRowReader.h"

#include <vector>

namespace ZXing {

class DecodeHints;

namespace OneD {

/**
* <p>Implements decoding of the ITF format, or Interleaved Two of Five.</p>
*
* <p>This Reader will scan ITF barcodes of certain lengths only.
* At the moment it reads length 6, 8, 10, 12, 14, 16, 18, 20, 24, and 44 as these have appeared "in the wild". Not all
* lengths are scanned, especially shorter ones, to avoid false positives. This in turn is due to a lack of
* required checksum function.</p>
*
* <p>The checksum is optional and is only applied by this Reader if the validateITFCheckSum hint is given.</p>
*
* <p><a href="http://en.wikipedia.org/wiki/Interleaved_2_of_5">http://en.wikipedia.org/wiki/Interleaved_2_of_5</a>
* is a great reference for Interleaved 2 of 5 information.</p>
*/
class ITFReader : public RowReader
{
public:
	explicit ITFReader(const DecodeHints& hints);
	Result decodePattern(int rowNumber, PatternView& next, std::unique_ptr<DecodingState>&) const override;

private:
	std::vector<int> _allowedLengths;
	bool _validateCheckSum;
};

} // OneD
} // ZXing
