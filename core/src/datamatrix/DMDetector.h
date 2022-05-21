/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

namespace ZXing {

class BitMatrix;
class DetectorResult;

namespace DataMatrix {

/**
 * @brief Detects a Data Matrix symbol in an image.
 */
DetectorResult Detect(const BitMatrix& image, bool tryHarder, bool tryRotate, bool isPure);

} // DataMatrix
} // ZXing
