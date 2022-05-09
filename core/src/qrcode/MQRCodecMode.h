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

namespace ZXing {
namespace MicroQRCode {

class Version;

/**
 * <p>See ISO 18004:2006, 6.4.1, Tables 2 and 3. This enum encapsulates the various modes in which
 * data can be encoded to bits in the micro QR code standard.</p>
 */
enum class CodecMode
{
	TERMINATOR = 0xFF, // Not really a mode...
	NUMERIC = 0x00,
	ALPHANUMERIC = 0x01,
	BYTE = 0x02,
	KANJI = 0x03,
	HANZI = 0x04, // See GBT 18284-2000; "Hanzi" is a transliteration of this mode name.
};

/**
 * @param bits one, two or three bits encoding a micro QR Code data mode
 * @return Mode encoded by these bits
 * @throws IllegalArgumentException if bits do not correspond to a known mode
 */
CodecMode CodecModeForBits(int bits, const Version& version);

/**
 * @param version version in question
 * @return number of bits used, in this QR Code symbol {@link Version}, to encode the
 *         count of characters that will follow encoded in this Mode
 */
int CharacterCountBits(CodecMode mode, const Version& version);

/**
 * @param version version in question
 * @return number of bits used to encode a codec mode.
 */
int CodecModeBitsLength(const Version& version);

/**
 * @param version version in question
 * @return number of bits in the Terminator code.
 */
int TerminatorBitsLength(const Version& version);

} // namespace MicroQRCode

} // namespace ZXing
