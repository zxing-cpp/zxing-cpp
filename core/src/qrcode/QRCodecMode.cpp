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

#include "qrcode/QRCodecMode.h"
#include "qrcode/QRVersion.h"

#include <stdexcept>

namespace ZXing {
namespace QRCode {

namespace {

static const int CHAR_COUNT_PER_MODE[] = {
	0, 0, 0,
	10, 12, 14,
	9, 11, 13,
	0, 0, 0,
	8, 16, 16,
	0, 0, 0,
	0, 0, 0,
	0, 0, 0,
	8, 10, 12,
	0, 0, 0,
	0, 0, 0,
	0, 0, 0,
	0, 0, 0,
	8, 10, 12,
};

} // anonymous

CodecMode::Mode
CodecMode::ModeForBits(int bits)
{
	if ((bits >= 0x00 && bits <= 0x05) || (bits >= 0x07 && bits <= 0x09) || bits == 0x0d)
	{
		return static_cast<Mode>(bits);
	}
	throw std::invalid_argument("Invalid mode");
}

int
CodecMode::CharacterCountBits(Mode mode, const Version& version)
{
	int number = version.versionNumber();
	int offset;
	if (number <= 9) {
		offset = 0;
	}
	else if (number <= 26) {
		offset = 1;
	}
	else {
		offset = 2;
	}
	return CHAR_COUNT_PER_MODE[static_cast<int>(mode) * 3 + offset];
}

} // QRCode
} // ZXing
