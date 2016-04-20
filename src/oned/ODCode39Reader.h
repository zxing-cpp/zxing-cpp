#pragma once
/*
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

#include "oned/ODReader.h"

namespace ZXing {

namespace OneD {

/**
* <p>Decodes Code 39 barcodes. This does not support "Full ASCII Code 39" yet.</p>
*
* @author Sean Owen
* @see Code93Reader
*/
class Code39Reader : public Reader
{
	bool _usingCheckDigit;
	bool _extendedMode;

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
	explicit Code39Reader(bool usingCheckDigit = false, bool extendedMode = false) : _usingCheckDigit(usingCheckDigit), _extendedMode(extendedMode) {}
	
	virtual Result decodeRow(int rowNumber, const BitArray& row, const DecodeHints* hints) const override;
};

} // OneD
} // ZXing
