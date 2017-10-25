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

#include "ByteArray.h"

#include <vector>

namespace ZXing {
namespace QRCode {

class Version;
enum class ErrorCorrectionLevel;

/**
* <p>Encapsulates a block of data within a QR Code. QR Codes may split their data into
* multiple blocks, each of which is a unit of data and error-correction codewords. Each
* is represented by an instance of this class.</p>
*
* @author Sean Owen
*/
class DataBlock
{
public:
	int numDataCodewords() const {
		return _numDataCodewords;
	}

	const ByteArray& codewords() const {
		return _codewords;
	}

	ByteArray& codewords() {
		return _codewords;
	}

	/**
	* <p>When QR Codes use multiple data blocks, they are actually interleaved.
	* That is, the first byte of data block 1 to n is written, then the second bytes, and so on. This
	* method will separate the data into original blocks.</p>
	*
	* @param rawCodewords bytes as read directly from the QR Code
	* @param version version of the QR Code
	* @param ecLevel error-correction level of the QR Code
	* @return DataBlocks containing original bytes, "de-interleaved" from representation in the
	*         QR Code
	*/
	static std::vector<DataBlock> GetDataBlocks(const ByteArray& rawCodewords, const Version& version, ErrorCorrectionLevel ecLevel);

private:
	int _numDataCodewords = 0;
	ByteArray _codewords;
};

} // QRCode
} // ZXing
