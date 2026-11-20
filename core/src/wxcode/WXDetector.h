// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Quadrilateral.h"
#include "WXGrid.h"

#include <vector>

namespace ZXing {
class BitMatrix;
class ImageView;

namespace WXCode {

struct Detection
{
	Grid bits{};
	QuadrilateralI position;
};

// Sample the angular/radial grid and full symbol boundary of each candidate.
// Use Otsu on the luminance image unless the caller supplies a binarized matrix.
std::vector<Detection> Detect(const ImageView& image, const BitMatrix* matrix = nullptr);

} // namespace WXCode
} // namespace ZXing
