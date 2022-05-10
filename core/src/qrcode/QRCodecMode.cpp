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

#include "QRCodecMode.h"

#include "QRVersion.h"

#include <array>
#include <stdexcept>

namespace ZXing::QRCode {

CodecMode CodecModeForBits(int bits, const Version& version)
{
	if (!version.isMicroQRCode() && ((bits >= 0x00 && bits <= 0x05) || (bits >= 0x07 && bits <= 0x09) || bits == 0x0d))
		return static_cast<CodecMode>(bits);

	// It's a micro QR code.
	switch (version.versionNumber()) {
	case 1: return CodecMode::NUMERIC; // M1 is always numeric.
	case 2:
		if (bits == 0)
			return CodecMode::NUMERIC;
		else if (bits == 1)
			return CodecMode::ALPHANUMERIC;
		break;
	case 3: [[fallthrough]];
	case 4:
		if (bits == 0)
			return CodecMode::NUMERIC;
		else if (bits == 1)
			return CodecMode::ALPHANUMERIC;
		else if (bits == 2)
			return CodecMode::BYTE;
		else if (bits == 3)
			return CodecMode::KANJI;
		break;
	}
	throw std::invalid_argument("Invalid mode");
}

int CharacterCountBits(CodecMode mode, const Version& version)
{
	int number = version.versionNumber();
	if (!version.isMicroQRCode()) {
		int i;
		if (number <= 9)
			i = 0;
		else if (number <= 26)
			i = 1;
		else
			i = 2;

		switch (mode) {
		case CodecMode::NUMERIC: return std::array{10, 12, 14}[i];
		case CodecMode::ALPHANUMERIC: return std::array{9, 11, 13}[i];
		case CodecMode::BYTE: return std::array{8, 16, 16}[i];
		case CodecMode::KANJI: [[fallthrough]];
		case CodecMode::HANZI: return std::array{8, 10, 12}[i];
		default: return 0;
		}
	}
	switch (mode) {
	case CodecMode::NUMERIC: return std::array{3, 4, 5, 6}[number - 1];
	case CodecMode::ALPHANUMERIC: return std::array{3, 4, 5}[number - 2];
	case CodecMode::BYTE: return std::array{4, 5}[number - 3];
	case CodecMode::KANJI: [[fallthrough]];
	case CodecMode::HANZI: return std::array{3, 4}[number - 3];
	default: return 0;
	}
}

int CodecModeBitsLength(const Version& version)
{
	if (!version.isMicroQRCode())
		return 4;
	return version.versionNumber() - 1;
}

int TerminatorBitsLength(const Version& version)
{
	if (!version.isMicroQRCode())
		return 4;
	return version.versionNumber() * 2 + 1;
}

} // namespace ZXing::QRCode
