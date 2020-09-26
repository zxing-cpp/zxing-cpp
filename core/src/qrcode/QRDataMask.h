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

#include "BitMatrix.h"

#include <stdexcept>

namespace ZXing {
namespace QRCode {

/**
* <p>Encapsulates data masks for the data bits in a QR code, per ISO 18004:2006 6.8.</p>
*
* <p>Note that the diagram in section 6.8.1 is misleading since it indicates that i is column position
* and j is row position. In fact, as the text says, i is row position and j is column position.</p>
*/

inline bool GetDataMaskBit(int maskIndex, int x, int y)
{
	switch (maskIndex) {
	case 0: return (y + x) % 2 == 0;
	case 1: return y % 2 == 0;
	case 2: return x % 3 == 0;
	case 3: return (y + x) % 3 == 0;
	case 4: return ((y / 2) + (x / 3)) % 2 == 0;
	case 5: return (y * x) % 6 == 0;
	case 6: return ((y * x) % 6) < 3;
	case 7: return (y + x + ((y * x) % 3)) % 2 == 0;
	}
	throw std::invalid_argument("QRCode maskIndex out of range");
}

inline bool GetMaskedBit(const BitMatrix& bits, int x, int y, int maskIndex)
{
	return GetDataMaskBit(maskIndex, x, y) != bits.get(x, y);
}

} // QRCode
} // ZXing
