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

#include "QRErrorCorrectionLevel.h"

namespace ZXing {
namespace MicroQRCode {

static QRCode::ErrorCorrectionLevel ECLevelFromBits(int bits)
{
	using namespace ZXing::QRCode;

	static constexpr ErrorCorrectionLevel LEVEL_FOR_BITS[] = {
		ErrorCorrectionLevel::Low,    ErrorCorrectionLevel::Low,    ErrorCorrectionLevel::Medium,
		ErrorCorrectionLevel::Low,    ErrorCorrectionLevel::Medium, ErrorCorrectionLevel::Low,
		ErrorCorrectionLevel::Medium, ErrorCorrectionLevel::Quality};
	return LEVEL_FOR_BITS[bits & 0x07];
}

} // namespace MicroQRCode

} // namespace ZXing
