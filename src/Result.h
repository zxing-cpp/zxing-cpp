#pragma once
/*
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

#include "ZXString.h"
#include "ByteArray.h"
#include "BarcodeFormat.h"
#include "ResultPoint.h"
#include "ResultMetadata.h"

#include <chrono>

namespace ZXing {

/**
* <p>Encapsulates the result of decoding a barcode within an image.</p>
*/
class Result
{
	String _text;
	ByteArray _rawBytes;
	std::vector<ResultPoint> _resultPoints;
	BarcodeFormat _format;
	ResultMetadata _metadata;
	std::chrono::steady_clock::time_point _timestamp;
};

} // ZXing
