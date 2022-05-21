/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

namespace ZXing {

class BitMatrix;

namespace Aztec {

class DetectorResult;

/**
 * Detects an Aztec Code in an image.
 *
 * @param isMirror if true, image is a mirror-image of original
 * @return {@link AztecDetectorResult} encapsulating results of detecting an Aztec Code
 * @throws NotFoundException if no Aztec Code can be found
 */
DetectorResult Detect(const BitMatrix& image, bool isMirror, bool isPure);

} // Aztec
} // ZXing
