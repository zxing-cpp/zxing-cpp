#pragma once
/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

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
* <p>The checksum is optional and is not applied by this Reader. The consumer of the decoded
* value will have to apply a checksum if required.</p>
*
* <p><a href="http://en.wikipedia.org/wiki/Interleaved_2_of_5">http://en.wikipedia.org/wiki/Interleaved_2_of_5</a>
* is a great reference for Interleaved 2 of 5 information.</p>
*
* @author kevin.osullivan@sita.aero, SITA Lab.
*/
class ITFReader : public RowReader
{
public:
	explicit ITFReader(const DecodeHints& hints);
	Result decodeRow(int rowNumber, const BitArray& row, std::unique_ptr<DecodingState>& state) const override;
	Result decodePattern(int rowNumber, const PatternView& row, std::unique_ptr<DecodingState>&) const override;

private:
	std::vector<int> _allowedLengths;
};

} // OneD
} // ZXing
