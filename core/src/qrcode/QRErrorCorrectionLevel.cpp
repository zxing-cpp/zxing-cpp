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

#include "qrcode/QRErrorCorrectionLevel.h"

namespace ZXing {
namespace QRCode {

const wchar_t* ToString(ErrorCorrectionLevel l)
{
	static const wchar_t* const LEVEL_STR[] = { L"L", L"M", L"Q", L"H" };
	return LEVEL_STR[static_cast<int>(l)];
}

ErrorCorrectionLevel FromString(const char* str)
{
	switch (str[0])
	{
		case 'L': return ErrorCorrectionLevel::Low;
		case 'M': return ErrorCorrectionLevel::Medium;
		case 'Q': return ErrorCorrectionLevel::Quality;
		case 'H': return ErrorCorrectionLevel::High;
		default: return ErrorCorrectionLevel::Medium;
	}
}

ErrorCorrectionLevel ECLevelFromBits(int bits)
{
	static const ErrorCorrectionLevel LEVEL_FOR_BITS[] = { ErrorCorrectionLevel::Medium, ErrorCorrectionLevel::Low, ErrorCorrectionLevel::High, ErrorCorrectionLevel::Quality };
	return LEVEL_FOR_BITS[bits & 0x3];
}

int BitsFromECLevel(ErrorCorrectionLevel l)
{
	static const int BITS[] = { 1, 0, 3, 2 };
	return BITS[(int)l];
}

} // QRCode
} // ZXing
