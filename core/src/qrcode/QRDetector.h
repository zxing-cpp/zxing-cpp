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

class DetectorResult;
class BitMatrix;

namespace QRCode {

/**
* <p>Encapsulates logic that can detect a QR Code in an image, even if the QR Code
* is rotated or skewed, or partially obscured.</p>
*
* @author Sean Owen
*/
class Detector
{
public:
	/**
	* <p>Detects a QR Code in an image.</p>
	*
	* @param hints optional hints to detector
	* @return {@link DetectorResult} encapsulating results of detecting a QR Code
	* @throws NotFoundException if QR Code cannot be found
	* @throws FormatException if a QR Code cannot be decoded
	*/
	static DetectorResult Detect(const BitMatrix& image, bool tryHarder);
};

} // QRCode
} // ZXing
