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

#include "DecodeHints.h"
#include "BarcodeFormat.h"
#include "BitHacks.h"

namespace ZXing {

std::vector<BarcodeFormat>
DecodeHints::possibleFormats() const
{
	std::vector<BarcodeFormat> result;
	int formatCount = (int)BarcodeFormat::FORMAT_COUNT;
	result.reserve(BitHacks::CountBitsSet(_flags & ~(0xffffffff << formatCount)));

	for (int i = 0; i < formatCount; ++i) {
		if (_flags & (1 << i)) {
			result.push_back((BarcodeFormat)i);
		}
	}
	return result;
}

void
DecodeHints::setPossibleFormats(const std::vector<BarcodeFormat>& formats)
{
	_flags &= (0xffffffff << (int)BarcodeFormat::FORMAT_COUNT);
	for (BarcodeFormat format : formats) {
		_flags |= (1 << (int)format);
	}
}

} // ZXing
