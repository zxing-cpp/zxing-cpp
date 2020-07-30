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

#include "PerspectiveTransform.h"
#include "DetectorResult.h"

namespace ZXing {

/**
* Samples an image for a rectangular matrix of bits of the given dimension. The sampling
* transformation is determined by the coordinates of 4 points, in the original and transformed
* image space.
*
* The following figure is showing the layout a 'pixel'. The point (0,0) is the upper left corner
* of the first pixel. (1,1) is its lower right corner.
*
*   0    1   ...   w
* 0 #----#-- ... --#
*   |    |   ...   |
*   |    |   ...   |
* 1 #----#   ... --#
*   |    |   ...   |
*
*   |    |   ...   |
* h #----#-- ... --#
*
* @param image image to sample
* @param width width of {@link BitMatrix} to sample from image
* @param height height of {@link BitMatrix} to sample from image
* @param mod2Pix transforming a module (grid) coordinate into an image (pixel) coordinate
* @return {@link DetectorResult} representing a grid of points sampled from the image within a region
*   defined by the "src" parameters. Result is empty if transformation is invalid (out of bound access).
*/
DetectorResult SampleGrid(const BitMatrix& image, int width, int height, const PerspectiveTransform& mod2Pix);

} // ZXing
