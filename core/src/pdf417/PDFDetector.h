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

#include "ResultPoint.h"
#include "ZXNullable.h"

#include <list>
#include <array>
#include <memory>

namespace ZXing {

class BitMatrix;
class BinaryBitmap;
enum class DecodeStatus;

namespace Pdf417 {

/**
* <p>Encapsulates logic that can detect a PDF417 Code in an image, even if the
* PDF417 Code is rotated or skewed, or partially obscured.< / p>
*
* @author SITA Lab(kevin.osullivan@sita.aero)
* @author dswitkin@google.com(Daniel Switkin)
* @author Guenther Grau
*/
class Detector
{
public:
	struct Result
	{
		std::shared_ptr<const BitMatrix> bits;
		std::list<std::array<Nullable<ResultPoint>, 8>> points;
	};

	static DecodeStatus Detect(const BinaryBitmap& image, bool multiple, Result& result);
};

} // Pdf417
} // ZXing
