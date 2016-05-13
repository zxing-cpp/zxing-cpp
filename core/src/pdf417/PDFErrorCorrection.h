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

#include <vector>

namespace ZXing {
namespace Pdf417 {

/**
* <p>PDF417 error correction implementation.</p>
*
* <p>This <a href="http://en.wikipedia.org/wiki/Reed%E2%80%93Solomon_error_correction#Example">example</a>
* is quite useful in understanding the algorithm.</p>
*
* @author Sean Owen
* @see com.google.zxing.common.reedsolomon.ReedSolomonDecoder
*/
class ErrorCorrection
{
public:
	/**
	* @param received received codewords
	* @param numECCodewords number of those codewords used for EC
	* @param erasures location of erasures
	* @return number of errors
	* @throws ChecksumException if errors cannot be corrected, maybe because of too many errors
	*/
	static bool Decode(std::vector<int>& received, int numECCodewords, const std::vector<int>& erasures, int& nbErrors);
};

} // Pdf417
} // ZXing
