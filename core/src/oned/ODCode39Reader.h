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
	
	Result decodeRow(int rowNumber, const BitArray& row, std::unique_ptr<DecodingState>& state) const override;
	Result decodePattern(int rowNumber, const PatternView& row, std::unique_ptr<DecodingState>&) const override;

private:
	bool _extendedMode;
	bool _usingCheckDigit;
};

} // OneD
} // ZXing
