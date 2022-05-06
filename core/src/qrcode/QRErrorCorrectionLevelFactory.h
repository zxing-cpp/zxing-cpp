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

#include <cassert>

namespace ZXing {
namespace QRCode {

static ErrorCorrectionLevel ECLevelFromBits(int bits)
{
	static constexpr ErrorCorrectionLevel LEVEL_FOR_BITS[] = {ErrorCorrectionLevel::Medium, ErrorCorrectionLevel::Low,
															  ErrorCorrectionLevel::High,
															  ErrorCorrectionLevel::Quality};
	return LEVEL_FOR_BITS[bits & 0x3];
}

static int BitsFromECLevel(ErrorCorrectionLevel l)
{
	assert(l != ErrorCorrectionLevel::Invalid);
	static constexpr int BITS[] = {1, 0, 3, 2, -1};
	return BITS[(int)l];
}

} // namespace QRCode
} // namespace ZXing
