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

#include "GenericLuminanceSource.h"
#include "ByteArray.h"

#include <algorithm>
#include <stdexcept>

namespace ZXing {

inline static uint8_t RGBToGray(unsigned r, unsigned g, unsigned b)
{
	// This optimization is not necessary as the computation below is cheap enough.
	//if (r == g && g == b) {
	//	// Image is already greyscale, so pick any channel.
	//	return static_cast<uint8_t>(r);
	//}

	// .299R + 0.587G + 0.114B (YUV/YIQ for PAL and NTSC), 
	// (306*R) >> 10 is approximately equal to R*0.299, and so on.
	// 0x200 >> 10 is 0.5, it implements rounding.
	return static_cast<uint8_t>((306 * r + 601 * g + 117 * b + 0x200) >> 10);
}

static std::shared_ptr<ByteArray> MakeCopy(const void* src, int rowBytes, int left, int top, int width, int height)
{
	auto result = std::make_shared<ByteArray>();
	result->resize(width * height);
	const uint8_t* srcRow = static_cast<const uint8_t*>(src) + top * rowBytes + left;
	uint8_t* destRow = result->data();
	for (int y = 0; y < height; ++y, srcRow += rowBytes, destRow += width) {
		std::copy_n(srcRow, width, destRow);
	}
	return result;
}

static std::shared_ptr<ByteArray> MakeCopy(const ByteArray& pixels, int rowBytes, int left, int top, int width, int height)
{
	if (top == 0 && left == 0 && width * height == (int)pixels.size()) {
		return std::make_shared<ByteArray>(pixels);
	}
	return MakeCopy(pixels.data(), rowBytes, left, top, width, height);
}

GenericLuminanceSource::GenericLuminanceSource(int left, int top, int width, int height, const void* bytes, int rowBytes, int pixelBytes, int redIndex, int greenIndex, int blueIndex) :
	_left(0),	// since we copy the pixels
	_top(0),
	_width(width),
	_height(height),
	_rowBytes(width)
{
	if (left < 0 || top < 0 || width < 0 || height < 0) {
		throw std::out_of_range("Requested offset is outside the image");
	}

	auto pixels = std::make_shared<ByteArray>();
	pixels->resize(width * height);
	const uint8_t *rgbSource = static_cast<const uint8_t*>(bytes) + top * rowBytes;
	uint8_t *destRow = pixels->data();
	for (int y = 0; y < height; ++y, rgbSource += rowBytes, destRow += width) {
		const uint8_t *src = rgbSource + left * pixelBytes;
		for (int x = 0; x < width; ++x, src += pixelBytes) {
			destRow[x] = RGBToGray(src[redIndex], src[greenIndex], src[blueIndex]);
		}
	}
	_pixels = pixels;
}

GenericLuminanceSource::GenericLuminanceSource(int left, int top, int width, int height, const void* bytes, int rowBytes) :
	_left(0),	// since we copy the pixels
	_top(0),
	_width(width),
	_height(height),
	_rowBytes(width)
{
	if (left < 0 || top < 0 || width < 0 || height < 0) {
		throw std::out_of_range("Requested offset is outside the image");
	}

	_pixels = MakeCopy(bytes, rowBytes, left, top, width, height);
}

GenericLuminanceSource::GenericLuminanceSource(int left, int top, int width, int height, std::shared_ptr<const ByteArray> pixels, int rowBytes) :
	_pixels(std::move(pixels)),
	_left(left),
	_top(top),
	_width(width),
	_height(height),
	_rowBytes(rowBytes)
{
	if (left < 0 || top < 0 || width < 0 || height < 0) {
		throw std::out_of_range("Requested offset is outside the image");
	}
}


int
GenericLuminanceSource::width() const
{
	return _width;
}

int
GenericLuminanceSource::height() const
{
	return _height;
}



const uint8_t *
GenericLuminanceSource::getRow(int y, ByteArray& buffer, bool forceCopy) const
{
	if (y < 0 || y >= _height) {
		throw std::out_of_range("Requested row is outside the image");
	}

	const uint8_t* row = _pixels->data() + (y + _top)*_rowBytes + _left;
	if (!forceCopy) {
		return row;
	}

	buffer.resize(_width);
	std::copy_n(row, _width, buffer.begin());
	return buffer.data();
}

const uint8_t *
GenericLuminanceSource::getMatrix(ByteArray& buffer, int& outRowBytes, bool forceCopy) const
{
	const uint8_t* row = _pixels->data() + _top*_rowBytes + _left;
	if (!forceCopy) {
		outRowBytes = _rowBytes;
		return row;
	}

	outRowBytes = _width;
	buffer.resize(_width * _height);
	uint8_t* dest = buffer.data();
	for (int y = 0; y < _height; ++y, row += _rowBytes, dest += _width) {
		std::copy_n(row, _width, dest);
	}
	return buffer.data();
}

bool
GenericLuminanceSource::canCrop() const
{
	return true;
}

std::shared_ptr<LuminanceSource>
GenericLuminanceSource::cropped(int left, int top, int width, int height) const
{
	if (left < 0 || top < 0 || width < 0 || height < 0 || left + width > _width || top + height > _height) {
		throw std::out_of_range("Crop rectangle does not fit within image data.");
	}
	return std::make_shared<GenericLuminanceSource>(_left + left, _top + top, width, height, _pixels, _rowBytes);
}

bool
GenericLuminanceSource::canRotate() const
{
	return true;
}

std::shared_ptr<LuminanceSource>
GenericLuminanceSource::rotated(int degreeCW) const
{
	degreeCW = (degreeCW + 360) % 360;
	if (degreeCW == 90)
	{
		auto pixels = std::make_shared<ByteArray>(_width * _height);
		const uint8_t* srcRow = _pixels->data() + _top * _rowBytes + _left;
		uint8_t* dest = pixels->data();
		for (int y = 0; y < _height; ++y, srcRow += _rowBytes) {
			for (int x = 0; x < _width; ++x) {
				dest[x * _height + (_height - y - 1)] = srcRow[x];
			}
		}
		return std::make_shared<GenericLuminanceSource>(0, 0, _height, _width, pixels, _height);
	}
	else if (degreeCW == 180) {
		// same as a vertical flip followed a horizonal flip
		auto pixels = MakeCopy(*_pixels, _rowBytes, _left, _top, _width, _height);
		std::reverse(pixels->begin(), pixels->end());
		return std::make_shared<GenericLuminanceSource>(0, 0, _width, _height, pixels, _width);
	}
	else if (degreeCW == 270) {
		auto pixels = std::make_shared<ByteArray>(_width * _height);
		const uint8_t* srcRow = _pixels->data() + _top * _rowBytes + _left;
		uint8_t* dest = pixels->data();
		for (int y = 0; y < _height; ++y, srcRow += _rowBytes) {
			for (int x = 0; x < _width; ++x) {
				dest[(_width - x - 1) * _height + y] = srcRow[x];
			}
		}
		return std::make_shared<GenericLuminanceSource>(0, 0, _height, _width, pixels, _height);
	}
	else if (degreeCW == 0) {
		return std::make_shared<GenericLuminanceSource>(0, 0, _width, _height, _pixels, _width);
	}
	throw std::invalid_argument("Unsupported rotation");
}

} // ZXing
