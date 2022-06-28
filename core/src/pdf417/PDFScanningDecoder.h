/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <string>

namespace ZXing {

class BitMatrix;
class ResultPoint;
class DecoderResult;
template <typename T> class Nullable;

namespace Pdf417 {

/**
* @author Guenther Grau
*/
class ScanningDecoder
{
public:
	static DecoderResult Decode(const BitMatrix& image,
		const Nullable<ResultPoint>& imageTopLeft, const Nullable<ResultPoint>& imageBottomLeft,
		const Nullable<ResultPoint>& imageTopRight, const Nullable<ResultPoint>& imageBottomRight,
		int minCodewordWidth, int maxCodewordWidth);
};

} // Pdf417
} // ZXing
