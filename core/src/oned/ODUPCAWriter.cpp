/*
* Copyright 2016 Huy Cuong Nguyen
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

#include "oned/ODUPCAWriter.h"
#include "oned/ODEAN13Writer.h"

namespace ZXing {
namespace OneD {

void
UPCAWriter::encode(const std::wstring& contents, int width, int height, BitMatrix& output) const
{
	// Transform a UPC-A code into the equivalent EAN-13 code, and add a check digit if it is not already present.
	size_t length = contents.length();
	if (length == 11) {
		// No check digit present, calculate it and add it
		int sum = 0;
		for (int i = 0; i < 11; ++i) {
			sum += (contents[i] - '0') * (i % 2 == 0 ? 3 : 1);
		}
		return EAN13Writer().setMargin(_sidesMargin).encode(contents + static_cast<wchar_t>('0' + ((1000 - sum) % 10)), width, height, output);
	}
	else if (length == 12) {
		return EAN13Writer().setMargin(_sidesMargin).encode(L'0' + contents, width, height, output);
	}
	throw std::invalid_argument("Requested contents should be 11 or 12 digits long");
}

} // OneD
} // ZXing
