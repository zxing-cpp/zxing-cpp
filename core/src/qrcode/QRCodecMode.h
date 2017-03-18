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
namespace QRCode {

class Version;

/**
* <p>See ISO 18004:2006, 6.4.1, Tables 2 and 3. This enum encapsulates the various modes in which
* data can be encoded to bits in the QR code standard.</p>
*
* @author Sean Owen
*/
class CodecMode
{
public:
	enum Mode
	{
		TERMINATOR = 0x00, // Not really a mode...
		NUMERIC = 0x01,
		ALPHANUMERIC = 0x02,
		STRUCTURED_APPEND = 0x03, // Not supported
		BYTE = 0x04,
		FNC1_FIRST_POSITION = 0x05,
		ECI = 0x07, // character counts don't apply
		KANJI = 0x08,
		FNC1_SECOND_POSITION = 0x09,
		/** See GBT 18284-2000; "Hanzi" is a transliteration of this mode name. */
		HANZI = 0x0D,
	};

	/**
	* @param bits four bits encoding a QR Code data mode
	* @return Mode encoded by these bits
	* @throws IllegalArgumentException if bits do not correspond to a known mode
	*/
	static Mode ModeForBits(int bits);

	/**
	* @param version version in question
	* @return number of bits used, in this QR Code symbol {@link Version}, to encode the
	*         count of characters that will follow encoded in this Mode
	*/
	static int CharacterCountBits(Mode mode, const Version& version);
};

} // QRCode
} // ZXing
