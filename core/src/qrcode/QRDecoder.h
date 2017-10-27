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

#include <string>

namespace ZXing {

class DecoderResult;
class BitMatrix;

namespace QRCode {


/**
* <p>The main class which implements QR Code decoding -- as opposed to locating and extracting
* the QR Code from an image.</p>
*
* @author Sean Owen
*/
class Decoder
{
public:
	/**
	* <p>Convenience method that can decode a QR Code represented as a 2D array of booleans.
	* "true" is taken to mean a black module.</p>
	*
	* @param image booleans representing white/black QR Code modules
	* @param hints decoding hints that should be used to influence decoding
	* @return text and bytes encoded within the QR Code
	* @throws FormatException if the QR Code cannot be decoded
	* @throws ChecksumException if error correction fails
	*/
	//DecoderResult decode(boolean[][] image, Map<DecodeHintType, ? > hints);

	/**
	* <p>Decodes a QR Code represented as a {@link BitMatrix}. A 1 or "true" is taken to mean a black module.</p>
	*
	* @param bits booleans representing white/black QR Code modules
	* @param hints decoding hints that should be used to influence decoding
	* @return text and bytes encoded within the QR Code
	* @throws FormatException if the QR Code cannot be decoded
	* @throws ChecksumException if error correction fails
	*/
	static DecoderResult Decode(const BitMatrix& bits, const std::string& hintedCharset);
};

} // QRCode
} // ZXing
