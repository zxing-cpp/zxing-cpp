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

#include "BitMatrix.h"
#include "BitArray.h"
#include "ByteMatrix.h"
#include "BitHacks.h"

#include <stdexcept>

namespace ZXing {

//void
//BitMatrix::xor(const BitMatrix& mask)
//{
//	if (_width != mask._width || _height != mask._height || _rowSize != mask._rowSize)
//	{
//		throw std::invalid_argument("BitMatrix::xor(): input matrix dimensions do not match");
//	}
//	
//	for (size_t i = 0; i < _bits.size(); ++i) {
//		_bits[i] ^= mask._bits[i];
//	}
//}

void
BitMatrix::getRow(int y, BitArray& row) const
{
	if (y < 0 || y >= _height) {
		throw std::out_of_range("Requested row is outside the matrix");
	}
	if (row.size() != _width)
		row = BitArray(_width);
	std::copy_n(_bits.begin() + y * _rowSize, _rowSize, row._bits.begin());
}

/**
* @param y row to set
* @param row {@link BitArray} to copy from
*/
void
BitMatrix::setRow(int y, const BitArray& row)
{
	if (int(row._bits.size()) != _rowSize) {
		throw std::invalid_argument("BitMatrix::setRegion(): row sizes do not match");
	}
	std::copy(row._bits.begin(), row._bits.end(), _bits.begin() + y *_rowSize);
}

BitMatrix::BitMatrix(const ByteMatrix& other, int blackValue) : BitMatrix(other.width(), other.height())
{
	for (int y = 0; y < height(); ++y)
		for (int x = 0; x < width(); ++x)
			if (other.get(x, y) == blackValue)
				set(x, y);
}

void
BitMatrix::setRegion(int left, int top, int width, int height)
{
	if (top < 0 || left < 0) {
		throw std::invalid_argument("BitMatrix::setRegion(): Left and top must be nonnegative");
	}
	if (height < 1 || width < 1) {
		throw std::invalid_argument("BitMatrix::setRegion(): Height and width must be at least 1");
	}
	int right = left + width;
	int bottom = top + height;
	if (bottom > _height || right > _width) {
		throw std::invalid_argument("BitMatrix::setRegion(): The region must fit inside the matrix");
	}
	for (int y = top; y < bottom; y++) {
		int offset = y * _rowSize;
		for (int x = left; x < right; x++) {
#ifdef ZX_FAST_BIT_STORAGE
			_bits[offset + x] = 1;
#else
			_bits[offset + (x / 32)] |= 1 << (x & 0x1f);
#endif
		}
	}
}

void
BitMatrix::rotate90()
{
	BitMatrix result(height(), width());
	for (int x = 0; x < width(); ++x) {
		for (int y = 0; y < height(); ++y) {
			if (get(x, y)) {
				result.set(y, width() - x - 1);
			}
		}
	}
	*this = std::move(result);
}

void
BitMatrix::rotate180()
{
#ifdef ZX_FAST_BIT_STORAGE
	std::reverse(_bits.begin(), _bits.end());
#else
	BitHacks::Reverse(_bits, _rowSize * 32 - _width);
#endif
}

void
BitMatrix::mirror()
{
	for (int x = 0; x < _width; x++) {
		for (int y = x + 1; y < _height; y++) {
			if (get(x, y) != get(y, x)) {
				flip(y, x);
				flip(x, y);
			}
		}
	}
}

/**
* This is useful in detecting the enclosing rectangle of a 'pure' barcode.
*
* @return {@code left,top,width,height} enclosing rectangle of all 1 bits, or null if it is all white
*/
bool
BitMatrix::getEnclosingRectangle(int &left, int& top, int& width, int& height) const
{
	left = _width;
	top = _height;
	int right = -1;
	int bottom = -1;

	for (int y = 0; y < _height; y++)
	{
		for (int x32 = 0; x32 < _rowSize; x32++)
		{
			uint32_t theBits = _bits[y * _rowSize + x32];
			if (theBits != 0)
			{
				if (y < top) {
					top = y;
				}
				if (y > bottom) {
					bottom = y;
				}
				if (x32 * 32 < left) {
					int bit = 0;
					while ((theBits << (31 - bit)) == 0) {
						bit++;
					}
					if ((x32 * 32 + bit) < left) {
						left = x32 * 32 + bit;
					}
				}
				if (x32 * 32 + 31 > right) {
					int bit = 31;
					while ((theBits >> bit) == 0) {
						bit--;
					}
					if ((x32 * 32 + bit) > right) {
						right = x32 * 32 + bit;
					}
				}
			}
		}
	}
	if (right < left || bottom < top) {
		return false;
	}
	width = right - left + 1;
	height = bottom - top + 1;
	return true;
}

/**
* This is useful in detecting a corner of a 'pure' barcode.
*
* @return {@code x,y} coordinate of top-left-most 1 bit, or null if it is all white
*/
bool
BitMatrix::getTopLeftOnBit(int& left, int& top) const
{
	int bitsOffset = 0;
	while (bitsOffset < (int)_bits.size() && _bits[bitsOffset] == 0) {
		bitsOffset++;
	}
	if (bitsOffset == (int)_bits.size()) {
		return false;
	}
	top = bitsOffset / _rowSize;
	left = (bitsOffset % _rowSize) * 32;

	uint32_t theBits = _bits[bitsOffset];
	int bit = 0;
	while ((theBits << (31 - bit)) == 0) {
		bit++;
	}
	left += bit;
	return true;
}

bool
BitMatrix::getBottomRightOnBit(int& right, int& bottom) const
{
	int bitsOffset = int(_bits.size()) - 1;
	while (bitsOffset >= 0 && _bits[bitsOffset] == 0) {
		bitsOffset--;
	}
	if (bitsOffset < 0) {
		return false;
	}

	bottom = bitsOffset / _rowSize;
	right = (bitsOffset % _rowSize) * 32;

	uint32_t theBits = _bits[bitsOffset];
	int bit = 31;
	while ((theBits >> bit) == 0) {
		bit--;
	}
	right += bit;
	return true;
}

BitMatrix Inflate(BitMatrix&& input, int width, int height, int quietZone)
{
	const int codeWidth = input.width();
	const int codeHeight = input.height();
	const int outputWidth = std::max(width, codeWidth + 2 * quietZone);
	const int outputHeight = std::max(height, codeHeight + 2 * quietZone);

	if (input.width() == outputWidth && input.height() == outputHeight)
		return std::move(input);

	const int scale = std::min((outputWidth - 2*quietZone) / codeWidth, (outputHeight - 2*quietZone) / codeHeight);
	// Padding includes both the quiet zone and the extra white pixels to
	// accommodate the requested dimensions.
	const int leftPadding = (outputWidth - (codeWidth * scale)) / 2;
	const int topPadding = (outputHeight - (codeHeight * scale)) / 2;

	BitMatrix result(outputWidth, outputHeight);

	for (int inputY = 0, outputY = topPadding; inputY < input.height(); ++inputY, outputY += scale) {
		for (int inputX = 0, outputX = leftPadding; inputX < input.width(); ++inputX, outputX += scale) {
			if (input.get(inputX, inputY))
				result.setRegion(outputX, outputY, scale, scale);
		}
	}

	return result;
}

BitMatrix Deflate(const BitMatrix& input, int width, int height, int top, int left, int subSampling)
{
	BitMatrix result(width, height);
	for (int y = 0; y < result.height(); y++) {
		int yOffset = top + y * subSampling;
		for (int x = 0; x < result.width(); x++) {
			if (input.get(left + x * subSampling, yOffset))
				result.set(x, y);
		}
	}
	return result;

}

} // ZXing

