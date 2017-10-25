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

class DecoderResult;
class BitMatrix;

namespace DataMatrix {

/**
* <p>The main class which implements Data Matrix Code decoding -- as opposed to locating and extracting
* the Data Matrix Code from an image.</p>
*
* @author bbrown@google.com (Brian Brown)
*/
class Decoder
{
public:
	/**
	* <p>Convenience method that can decode a Data Matrix Code represented as a 2D array of booleans.
	* "true" is taken to mean a black module.</p>
	*
	* @param image booleans representing white/black Data Matrix Code modules
	* @return text and bytes encoded within the Data Matrix Code
	* @throws FormatException if the Data Matrix Code cannot be decoded
	* @throws ChecksumException if error correction fails
	*/
	//DecoderResult decode(boolean[][] image);

	/**
	* <p>Decodes a Data Matrix Code represented as a {@link BitMatrix}. A 1 or "true" is taken
	* to mean a black module.</p>
	*
	* @param bits booleans representing white/black Data Matrix Code modules
	* @return text and bytes encoded within the Data Matrix Code
	* @throws FormatException if the Data Matrix Code cannot be decoded
	* @throws ChecksumException if error correction fails
	*/
	static DecoderResult Decode(const BitMatrix& bits);
};

} // DataMatrix
} // ZXing
