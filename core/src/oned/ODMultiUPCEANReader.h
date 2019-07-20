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

#include "oned/ODRowReader.h"
#include "BarcodeFormat.h"

#include <vector>
#include <memory>

namespace ZXing {

class DecodeHints;
enum class BarcodeFormat;

namespace OneD {

class UPCEANReader;

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

	Result decodeRow(int rowNumber, const BitArray& row, std::unique_ptr<DecodingState>& state) const override;

private:
	std::vector<std::unique_ptr<const UPCEANReader>> _readers;
	bool _canReturnUPCA = false;
};

} // OneD
} // ZXing
