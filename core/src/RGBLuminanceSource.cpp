/*
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

#include "RGBLuminanceSource.h"
#include "ByteArray.h"

namespace ZXing {

RGBLuminanceSource::RGBLuminanceSource(const void* bytes, int width, int height, int rowBytes, int pixelBytes, int redIndex, int greenIndex, int blueIndex, int xoffset, int yoffset) :
	_bytes(static_cast<const uint8_t*>(bytes)),
	_width(width),
	_height(height),
	_rowBytes(rowBytes),
	_pixelBytes(pixelBytes),
	_redIndex(redIndex),
	_greenIndex(greenIndex),
	_blueIndex(blueIndex),
	_xoffset(xoffset),
	_yoffset(yoffset)
{
	if (xoffset > width || yoffset > height) {
		throw std::out_of_range("Requested offset is outside the image");
	}
}

int
RGBLuminanceSource::width() const
{
	return _width;
}

int
RGBLuminanceSource::height() const
{
	return _height;
}

inline static uint8_t
ConvertToGray(int r, int g, int b)
{
	if (r == g && g == b) {
		// Image is already greyscale, so pick any channel.
		return static_cast<uint8_t>(r);
	}
	// Calculate luminance cheaply, favoring green.
	//return static_cast<uint8_t>((r + 2 * g + b) / 4);

	// .299R + 0.587G + 0.114B (YUV/YIQ for PAL and NTSC), 
    // (306*R) >> 10 is approximately equal to R*0.299, and so on.
    // 0x200 >> 10 is 0.5, it implements rounding.
	return static_cast<uint8_t>((306 * r + 601 * g + 117 * b + 0x200) >> 10);
}

const uint8_t *
RGBLuminanceSource::getRow(int y, ByteArray& buffer, bool) const
{
	if (y < 0 || y >= _height) {
		throw std::out_of_range("Requested row is outside the image");
	}

	buffer.resize(_width);
	const uint8_t *pixels = _bytes + (y + _yoffset) * _rowBytes + _xoffset * _pixelBytes;
	for (int x = 0; x < _width; ++x, pixels += _pixelBytes) {
		buffer[x] = ConvertToGray(pixels[_redIndex], pixels[_greenIndex], pixels[_blueIndex]);
	}
	return buffer.data();
}

const uint8_t *
RGBLuminanceSource::getMatrix(ByteArray& buffer, int& outRowBytes, bool) const
{
	outRowBytes = _width;
	buffer.resize(_width * _height);
	const uint8_t *rowPixels = _bytes + _yoffset * _rowBytes;
	for (int y = 0; y < _height; ++y, rowPixels += _rowBytes) {
		const uint8_t *pixels = rowPixels + _xoffset * _pixelBytes;
		for (int x = 0; x < _width; ++x, pixels += _pixelBytes) {
			buffer[y * _width + x] = ConvertToGray(pixels[_redIndex], pixels[_greenIndex], pixels[_blueIndex]);
		}
	}
	return buffer.data();
}

bool
RGBLuminanceSource::canCrop() const
{
	return true;
}

std::shared_ptr<LuminanceSource>
RGBLuminanceSource::cropped(int left, int top, int width, int height) const
{
	if (left + width > _width || top + height > _height) {
		throw std::out_of_range("Crop rectangle does not fit within image data.");
	}
	return std::make_shared<RGBLuminanceSource>(_bytes, width, height, _rowBytes, _pixelBytes, _redIndex, _greenIndex, _blueIndex, _xoffset + left, _yoffset + top);
}

} // ZXing
