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

#include "TritMatrix.h"

namespace ZXing {

class BitArray;

namespace QRCode {

enum class ErrorCorrectionLevel;
class Version;

constexpr int NUM_MASK_PATTERNS = 8;

void BuildMatrix(const BitArray& dataBits, ErrorCorrectionLevel ecLevel, const Version& version, int maskPattern, TritMatrix& matrix);

} // QRCode
} // ZXing
