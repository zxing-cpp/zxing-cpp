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

#include <vector>

namespace ZXing {

/**
 * Read barcode from a grayscale buffer
 *
 * <p>Use {@link #ReadBarcode(int width, int height, unsigned char* data, int rowStride,
				   std::vector<BarcodeFormat> formats = {}, bool tryRotate = true, bool tryHarder = true) to read a bar code from a buffer.
 *
 * @param width image width
 * @param height image height
 * @param data   image buffer
 * @param rowstride  row stride
 * @param formats   A list of format to search for ( faster)
 * @param tryRotate   try to rotate the buffer to find the barcode (slower)
 * @param tryHarder   try harder to find the barcode(slower). TODO needs to explain
 * @return            #Result structure
 * @since             0.x
 */
Result ReadBarcode(int width, int height, unsigned char* data, int rowStride,
				   std::vector<BarcodeFormat> formats = {}, bool tryRotate = true, bool tryHarder = true);

/**
 * Read barcode from a RGB buffer
 *
 * <p>Use {@link #ReadBarcode(int width, int height, unsigned char* data, int rowStride, int pixelStride, int rIndex, int gIndex, int bIndex,
				   std::vector<BarcodeFormat> formats = {}, bool tryRotate = true, bool tryHarder = true) to read a bar code from a buffer.
 *
 * @param width image width
 * @param height image height
 * @param data   image buffer
 * @param rowstride  row stride
 * @param pixelstride  pixel stride (ie 4 for 32 bits)
 * @param rIndex  red index
 * @param gIndex  green index
 * @param bIndex  blue index
 * @param formats   A list of format to search for ( faster)
 * @param tryRotate   try to rotate the buffer to find the barcode (slower)
 * @param tryHarder   try harder to find the barcode(slower). TODO needs to explain
 * @return            #Result structure
 * @since             0.x
 */
Result ReadBarcode(int width, int height, unsigned char* data, int rowStride, int pixelStride, int rIndex, int gIndex, int bIndex,
				   std::vector<BarcodeFormat> formats = {}, bool tryRotate = true, bool tryHarder = true);

} // ZXing

