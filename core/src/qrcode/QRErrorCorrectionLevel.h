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
namespace QRCode {

/**
* <p>See ISO 18004:2006, 6.5.1. This enum encapsulates the four error correction levels
* defined by the QR code standard.</p>
*
* @author Sean Owen
*/
enum class ErrorCorrectionLevel
{
	Low,			// L = ~7 % correction
	Medium,			// M = ~15% correction
	Quality,		// Q = ~25% correction
	High,			// H = ~30% correction
	Invalid,		// denotes in invalid/unknown value
};

const wchar_t* ToString(ErrorCorrectionLevel l);
ErrorCorrectionLevel ECLevelFromString(const char* str);
ErrorCorrectionLevel ECLevelFromBits(int bits);
int BitsFromECLevel(ErrorCorrectionLevel l);

} // QRCode
} // ZXing
