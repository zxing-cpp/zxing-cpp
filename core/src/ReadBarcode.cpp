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

#include "ReadBarcode.h"
#include "DecodeHints.h"
#include "MultiFormatReader.h"
#include "GenericLuminanceSource.h"
#include "HybridBinarizer.h"

#include <memory>

namespace ZXing {

static Result ReadBarcode(GenericLuminanceSource&& source, const DecodeHints& hints)
{
	HybridBinarizer binImage(std::shared_ptr<LuminanceSource>(&source, [](void*) {}));
	MultiFormatReader reader(hints);
	return reader.read(binImage);
}

Result ReadBarcode(const ImageView& iv, const DecodeHints& hints)
{
	return ReadBarcode(
		{
			iv._width,
			iv._height,
			iv._data,
			iv._rowStride,
			iv._pixStride,
			RedIndex(iv._format),
			GreenIndex(iv._format),
			BlueIndex(iv._format),
		},
		hints);
}

Result ReadBarcode(int width, int height, const uint8_t* data, int rowStride, BarcodeFormats formats, bool tryRotate,
				   bool tryHarder)
{
	return ReadBarcode({width, height, data, rowStride},
					   DecodeHints().setTryHarder(tryHarder).setTryRotate(tryRotate).setFormats(formats));
}

Result ReadBarcode(int width, int height, const uint8_t* data, int rowStride, int pixelStride, int rIndex, int gIndex,
				   int bIndex, BarcodeFormats formats, bool tryRotate, bool tryHarder)
{
	return ReadBarcode({width, height, data, rowStride, pixelStride, rIndex, gIndex, bIndex},
					   DecodeHints().setTryHarder(tryHarder).setTryRotate(tryRotate).setFormats(formats));
}

} // ZXing
