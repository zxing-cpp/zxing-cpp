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

#include "qrcode/QRErrorCorrectionLevel.h"

namespace ZXing {
namespace QRCode {

const wchar_t* ToString(ErrorCorrectionLevel l)
{
	static const wchar_t* const LEVEL_STR[] = { L"M", L"L", L"H", L"Q" };
	return LEVEL_STR[static_cast<int>(l)];
}

ErrorCorrectionLevel ECLevelFromBits(int bits)
{
	static const ErrorCorrectionLevel LEVEL_FOR_BITS[] = { ErrorCorrectionLevel::Medium, ErrorCorrectionLevel::Low, ErrorCorrectionLevel::High, ErrorCorrectionLevel::Quality };
	return LEVEL_FOR_BITS[bits & 0x3];
}


} // QRCode
} // ZXing
