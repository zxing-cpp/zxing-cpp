/*
* Copyright 2019 Axel Waggershauser
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

#pragma once

#include "Result.h"
#include "BarcodeFormat.h"

#include <cstdint>

namespace ZXing {

/**
 * Read barcode from a grayscale buffer
 *
 * @param width  image width in pixels
 * @param height  image height in pixels
 * @param data  image buffer
 * @param rowStride  row stride in bytes
 * @param formats  list of formats to search for (faster)
 * @param tryRotate  rotate the buffer if necessary to find the barcode (slower)
 * @param tryHarder  dense scan of the image to detect the barcode (slower). Setting this to false, will skip some lines.
 * @return #Result structure
 */
Result ReadBarcode(int width, int height, const uint8_t* data, int rowStride,
				   BarcodeFormats formats = {}, bool tryRotate = true, bool tryHarder = true);

/**
 * Read barcode from a RGB buffer
 *
 * @param width  image width in pixels
 * @param height  image height in pixels
 * @param data  image buffer
 * @param rowStride  row stride in bytes
 * @param pixelStride  pixel stride in bytes (e.g. 4 for 32 bits)
 * @param rIndex  index of red channel
 * @param gIndex  index of green channel
 * @param bIndex  index of blue channel
 * @param formats  list of formats to search for (faster)
 * @param tryRotate  rotate the buffer if necessary to find the barcode (slower)
 * @param tryHarder  dense scan of the image to detect the barcode (slower). Setting this to false, will skip some lines.
 * @return #Result structure
 */
Result ReadBarcode(int width, int height, const uint8_t* data, int rowStride, int pixelStride, int rIndex, int gIndex, int bIndex,
				   BarcodeFormats formats = {}, bool tryRotate = true, bool tryHarder = true);

} // ZXing

