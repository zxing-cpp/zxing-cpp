/*
* Copyright 2016 Huy Cuong Nguyen
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "TritMatrix.h"

namespace ZXing {
namespace QRCode {
namespace MaskUtil {

int CalculateMaskPenalty(const TritMatrix& matrix);

} // namespace MaskUtil
} // namespace QRCode
} // namespace ZXing
