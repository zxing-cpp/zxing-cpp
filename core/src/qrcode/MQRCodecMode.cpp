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

#include "MQRCodecMode.h"

#include "QRVersion.h"

#include <array>
#include <stdexcept>

namespace ZXing::MicroQRCode {

CodecMode CodecModeForBits(int bits, const QRCode::Version& version)
{
	int number = version.versionNumber();
	switch (number) {
	case 1: return CodecMode::NUMERIC; // M1 is always numeric.
	case 2:
		if (bits == 0)
			return CodecMode::NUMERIC;
		else if (bits == 1)
			return CodecMode::ALPHANUMERIC;
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
	}
	throw std::invalid_argument("Invalid mode");
}

int CharacterCountBits(CodecMode mode, const QRCode::Version& version)
{
	int number = version.versionNumber();

	switch (mode) {
	case CodecMode::NUMERIC: return std::array{3, 4, 5, 6}[number - 1];
	case CodecMode::ALPHANUMERIC: return std::array{3, 4, 5}[number - 2];
	case CodecMode::BYTE: return std::array{4, 5}[number - 3];
	case CodecMode::KANJI: [[fallthrough]];
	case CodecMode::HANZI: return std::array{3, 4}[number - 3];
	default: return 0;
	}
}

int CodecModeBitsLength(const QRCode::Version& version)
{
	return version.versionNumber() - 1;
}

int TerminatorBitsLength(const QRCode::Version& version)
{
	return version.versionNumber() * 2 + 1;
}

} // namespace ZXing::MicroQRCode
