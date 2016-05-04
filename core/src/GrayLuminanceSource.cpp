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

#include "GrayLuminanceSource.h"
#include "ByteArray.h"

namespace ZXing {

GrayLuminanceSource::GrayLuminanceSource(const void* bytes, int width, int height, int rowBytes, int xoffset, int yoffset) :
	_bytes(static_cast<const uint8_t*>(bytes)),
	_width(width),
	_height(height),
	_rowBytes(rowBytes),
	_xoffset(xoffset),
	_yoffset(yoffset)
{
	if (xoffset > width || yoffset > height) {
		throw std::out_of_range("Requested offset is outside the image");
	}
}

int
GrayLuminanceSource::width() const
{
	return _width;
}

int
GrayLuminanceSource::height() const
{
	return _height;
}

const uint8_t *
GrayLuminanceSource::getRow(int y, ByteArray& buffer, bool forceCopy) const
{
	if (y < 0 || y >= _height) {
		throw std::out_of_range("Requested row is outside the image");
	}

	buffer.resize(_width);
	const uint8_t *pixels = _bytes + (y + _yoffset) * _rowBytes + _xoffset;
	if (forceCopy)
	{
		std::copy(pixels, pixels + _width, buffer.begin());
		return buffer.data();
	}
	return pixels;
}

const uint8_t *
GrayLuminanceSource::getMatrix(ByteArray& buffer, int& outRowBytes, bool forceCopy) const
{
	if (forceCopy) {
		outRowBytes = _width;
		buffer.resize(_width * _height);
		auto dest = buffer.begin();
		const uint8_t *rowPixels = _bytes + _yoffset * _rowBytes;
		for (int y = 0; y < _height; ++y, rowPixels += _rowBytes, dest += _width) {
			const uint8_t *pixels = rowPixels + _xoffset;
			std::copy(pixels, pixels + _width, dest);
		}
		return buffer.data();
	}
	else {
		outRowBytes = _rowBytes;
		return _bytes + _yoffset * _rowBytes + _xoffset;
	}
}

bool
GrayLuminanceSource::canCrop() const
{
	return true;
}

std::shared_ptr<LuminanceSource>
GrayLuminanceSource::cropped(int left, int top, int width, int height) const
{
	if (left + width > _width || top + height > _height) {
		throw std::out_of_range("Crop rectangle does not fit within image data.");
	}
	return std::make_shared<GrayLuminanceSource>(_bytes, width, height, _rowBytes, _xoffset + left, _yoffset + top);
}

} // ZXing
