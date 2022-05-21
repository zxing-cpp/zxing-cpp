/*
* Copyright 2019 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <algorithm>
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

constexpr inline uint8_t RGBToLum(unsigned r, unsigned g, unsigned b)
{
	// .299R + 0.587G + 0.114B (YUV/YIQ for PAL and NTSC),
	// (306*R) >> 10 is approximately equal to R*0.299, and so on.
	// 0x200 >> 10 is 0.5, it implements rounding.
	return static_cast<uint8_t>((306 * r + 601 * g + 117 * b + 0x200) >> 10);
}

/**
 * Simple class that stores a non-owning const pointer to image data plus layout and format information.
 */
class ImageView
{
protected:
	const uint8_t* _data = nullptr;
	ImageFormat _format;
	int _width = 0, _height = 0, _pixStride = 0, _rowStride = 0;

public:
	/**
	 * ImageView constructor
	 *
	 * @param data  pointer to image buffer
	 * @param width  image width in pixels
	 * @param height  image height in pixels
	 * @param format  image/pixel format
	 * @param rowStride  optional row stride in bytes, default is width * pixStride
	 * @param pixStride  optional pixel stride in bytes, default is calculated from format
	 */
	ImageView(const uint8_t* data, int width, int height, ImageFormat format, int rowStride = 0, int pixStride = 0)
		: _data(data),
		  _format(format),
		  _width(width),
		  _height(height),
		  _pixStride(pixStride ? pixStride : PixStride(format)),
		  _rowStride(rowStride ? rowStride : width * _pixStride)
	{}

	int width() const { return _width; }
	int height() const { return _height; }
	int pixStride() const { return _pixStride; }
	int rowStride() const { return _rowStride; }
	ImageFormat format() const { return _format; }

	const uint8_t* data(int x, int y) const { return _data + y * _rowStride + x * _pixStride; }

	ImageView cropped(int left, int top, int width, int height) const
	{
		left   = std::max(0, left);
		top    = std::max(0, top);
		width  = width <= 0 ? (_width - left) : std::min(_width - left, width);
		height = height <= 0 ? (_height - top) : std::min(_height - top, height);
		return {data(left, top), width, height, _format, _rowStride, _pixStride};
	}

	ImageView rotated(int degree) const
	{
		switch ((degree + 360) % 360) {
		case 90:  return {data(0, _height - 1), _height, _width, _format, _pixStride, -_rowStride};
		case 180: return {data(_width - 1, _height - 1), _width, _height, _format, -_rowStride, -_pixStride};
		case 270: return {data(_width - 1, 0), _height, _width, _format, -_pixStride, _rowStride};
		}
		return *this;
	}

	ImageView subsampled(int scale) const
	{
		return {_data, _width / scale, _height / scale, _format, _rowStride * scale, _pixStride * scale};
	}

};

} // ZXing

