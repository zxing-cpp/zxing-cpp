#pragma once
/*
* Copyright 2016 Huy Cuong Nguyen
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
#include "ByteMatrix.h"
#include "qrcode/QRCodecMode.h"
#include "qrcode/QRVersion.h"

namespace ZXing {
namespace QRCode {

/**
* @author satorux@google.com (Satoru Takabayashi) - creator
* @author dswitkin@google.com (Daniel Switkin) - ported from C++
*
* Original class name in Java was QRCode, as this name is taken already for the namespace,
* so it's renamed here EncodeResult.
*/
class EncodeResult
{
public:
	ErrorCorrectionLevel ecLevel = ErrorCorrectionLevel::Invalid;
	CodecMode::Mode mode = CodecMode::TERMINATOR;
	const Version* version = nullptr;
	int maskPattern = -1;
	ByteMatrix matrix;
};

} // QRCode
} // ZXing
