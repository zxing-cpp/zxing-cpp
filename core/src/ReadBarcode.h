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

Result ReadBarcode(int width, int height, unsigned char* data, int rowStride,
				   std::vector<BarcodeFormat> formats = {}, bool tryRotate = true, bool tryHarder = true);

Result ReadBarcode(int width, int height, unsigned char* data, int rowStride, int pixelStride, int rIndex, int gIndex, int bIndex,
				   std::vector<BarcodeFormat> formats = {}, bool tryRotate = true, bool tryHarder = true);

} // ZXing

