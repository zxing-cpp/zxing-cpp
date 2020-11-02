#pragma once
/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
* Copyright 2020 Axel Waggershauser
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
#include "DecodeHints.h"

namespace ZXing {
namespace OneD {

/**
* Decodes DataBarExpandedReader (formerly known as RSS) sybmols, including truncated and stacked variants. See ISO/IEC 24724:2006.
*/
class DataBarExpandedReader : public RowReader
{

public:
	explicit DataBarExpandedReader(const DecodeHints& hints);
	~DataBarExpandedReader() override;

	Result decodeRow(int, const BitArray&, std::unique_ptr<DecodingState>&) const override;
	Result decodePattern(int rowNumber, const PatternView& row, std::unique_ptr<DecodingState>& state) const override;
};

} // OneD
} // ZXing
