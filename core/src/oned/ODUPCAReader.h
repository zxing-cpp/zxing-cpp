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

#include "oned/ODEAN13Reader.h"

namespace ZXing {
namespace OneD {

/**
* <p>Implements decoding of the UPC-A format.</p>
*
* @author dswitkin@google.com (Daniel Switkin)
* @author Sean Owen
*/
class UPCAReader : public UPCEANReader
{
public:
	explicit UPCAReader(const DecodeHints& hints) : UPCEANReader(hints), _reader(hints) {}

	virtual Result decodeRow(int rowNumber, const BitArray& row, std::unique_ptr<DecodingState>& state) const override;
	virtual Result decodeRow(int rowNumber, const BitArray& row, int startGuardBegin, int startGuardEnd) const override;

protected:
	virtual BarcodeFormat expectedFormat() const override;
	virtual ErrorStatus decodeMiddle(const BitArray& row, int &rowOffset, std::string& resultString) const override;

private:
	EAN13Reader _reader;
};

} // OneD
} // ZXing
