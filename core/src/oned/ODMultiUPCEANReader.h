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

#include "BarcodeFormat.h"
#include "DecodeHints.h"
#include "ODRowReader.h"

namespace ZXing {
namespace OneD {

/**
* <p>A reader that can read all available UPC/EAN formats. If a caller wants to try to
* read all such formats, it is most efficient to use this implementation rather than invoke
* individual readers.</p>
*
* @author Sean Owen
*/
class MultiUPCEANReader : public RowReader
{
public:
	explicit MultiUPCEANReader(const DecodeHints& hints);
	~MultiUPCEANReader() override;

	Result decodePattern(int rowNumber, PatternView& next, std::unique_ptr<DecodingState>&) const override;

private:
	DecodeHints _hints;
};

} // OneD
} // ZXing
