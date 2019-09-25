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

#include "ODUPCEANReader.h"

namespace ZXing {
namespace OneD {

/**
* <p>Implements decoding of the EAN-13 format.</p>
*
* @author dswitkin@google.com (Daniel Switkin)
* @author Sean Owen
* @author alasdair@google.com (Alasdair Mackintosh)
*/
class EAN13Reader : public UPCEANReader
{
public:
	explicit EAN13Reader(const DecodeHints& hints) : UPCEANReader(hints) {}

	BarcodeFormat expectedFormat() const override;
	BitArray::Range decodeMiddle(const BitArray& row, BitArray::Iterator begin, std::string& resultString) const override;
};

} // OneD
} // ZXing
