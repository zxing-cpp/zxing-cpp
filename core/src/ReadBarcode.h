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
#include "DecodeHints.h"

#include <cstdint>

namespace ZXing {

enum class ImageFormat : uint32_t
{
	None = 0,
	Lum  = 0x01000000,
	RGB  = 0x03000102,
	BGR  = 0x03020100,
	RGBX = 0x04000102,
	XRGB = 0x04010203,
	BGRX = 0x04020100,
	XBGR = 0x04030201,
};

constexpr inline int PixStride(ImageFormat format) { return (static_cast<uint32_t>(format) >> 3*8) & 0xFF; }
constexpr inline int RedIndex(ImageFormat format) { return (static_cast<uint32_t>(format) >> 2*8) & 0xFF; }
constexpr inline int GreenIndex(ImageFormat format) { return (static_cast<uint32_t>(format) >> 1*8) & 0xFF; }
constexpr inline int BlueIndex(ImageFormat format) { return (static_cast<uint32_t>(format) >> 0*8) & 0xFF; }

/**
 * Simple class that stores a non-owning const pointer to image data plus layout and format information.
 */
class ImageView
{
	const uint8_t* _data = nullptr;
	ImageFormat _format;
	int _width = 0, _height = 0, _pixStride = 0, _rowStride = 0;

	friend Result ReadBarcode(const ImageView&, const DecodeHints&);
	friend class ThresholdBinarizer;

public:
	/**
	 * ImageView contructor
	 *
	 * @param data  pointer to image buffer
	 * @param width  image width in pixels
	 * @param height  image height in pixels
	 * @param format  image/pixel format
	 * @param rowStride  optional row stride in bytes, default is width * pixStride
	 * @param pixStride  optional pixel stride in bytes, default is calculated from format
	 */
	ImageView(const uint8_t* data, int width, int height, ImageFormat format, int rowStride = 0, int pixStride = 0)
		: _data(data), _format(format), _width(width), _height(height),
		  _pixStride(pixStride ? pixStride : PixStride(format)), _rowStride(rowStride ? rowStride : width * _pixStride)
	{}

	const uint8_t* data(int x, int y) const { return _data + y * _rowStride + x * _pixStride; }
};

/**
 * Read barcode from an ImageView
 *
 * @param buffer  view of the image data including layout and format
 * @param hints  optional DecodeHints to parameterize / speed up decoding
 * @return #Result structure
 */
Result ReadBarcode(const ImageView& buffer, const DecodeHints& hints = {});


[[deprecated]]
Result ReadBarcode(int width, int height, const uint8_t* data, int rowStride,
			BarcodeFormats formats = {}, bool tryRotate = true, bool tryHarder = true);

[[deprecated]]
Result ReadBarcode(int width, int height, const uint8_t* data, int rowStride, int pixelStride, int rIndex, int gIndex, int bIndex,
			BarcodeFormats formats = {}, bool tryRotate = true, bool tryHarder = true);

} // ZXing

