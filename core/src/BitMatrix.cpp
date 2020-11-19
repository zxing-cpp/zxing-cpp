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
#include "Pattern.h"

#ifndef ZX_FAST_BIT_STORAGE
#include "BitHacks.h"
#endif

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace ZXing {

void
BitMatrix::getRow(int y, BitArray& row) const
{
	if (y < 0 || y >= _height) {
		throw std::out_of_range("Requested row is outside the matrix");
	}
	if (row.size() != _width)
		row = BitArray(_width);
#ifdef ZX_FAST_BIT_STORAGE
	std::transform(_bits.begin() + y * _rowSize, _bits.begin() + (y + 1) * _rowSize, row._bits.begin(), isSet);
#else
	std::copy_n(_bits.begin() + y * _rowSize, _rowSize, row._bits.begin());
#endif
}

ByteMatrix BitMatrix::toByteMatrix(int black, int white) const
{
	ByteMatrix res(width(), height());
	for (int y = 0; y < height(); ++y)
		for (int x = 0; x < width(); ++x)
			res.set(x, y, get(x, y) ? black : white);
	return res;
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
		size_t offset = y * _rowSize;
		for (int x = left; x < right; x++) {
#ifdef ZX_FAST_BIT_STORAGE
			_bits[offset + x] = SET_V;
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

bool
BitMatrix::findBoundingBox(int &left, int& top, int& width, int& height, int minSize) const
{
	int right, bottom;
	if (!getTopLeftOnBit(left, top) || !getBottomRightOnBit(right, bottom) || bottom - top + 1 < minSize)
		return false;

#ifdef ZX_FAST_BIT_STORAGE
	for (int y = top; y <= bottom; y++ ) {
		for (int x = 0; x < left; ++x)
			if (get(x, y)) {
				left = x;
				break;
			}
		for (int x = _width-1; x > right; x--)
			if (get(x, y)) {
				right = x;
				break;
			}
	}
#else
	for (int y = top; y <= bottom; y++)
	{
		for (int x32 = 0; x32 < _rowSize; x32++)
		{
			uint32_t theBits = _bits[y * _rowSize + x32];
			if (theBits != 0)
			{
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
#endif

	width = right - left + 1;
	height = bottom - top + 1;
	return width >= minSize && height >= minSize;
}

static auto isSet_ = [](auto v) {
#ifdef ZX_FAST_BIT_STORAGE
	return BitMatrix::isSet(v);
#else
	return v;
#endif
};

bool
BitMatrix::getTopLeftOnBit(int& left, int& top) const
{
	int bitsOffset = (int)std::distance(_bits.begin(), std::find_if(_bits.begin(), _bits.end(), isSet_));
	if (bitsOffset == Size(_bits)) {
		return false;
	}
	top = bitsOffset / _rowSize;
	left = (bitsOffset % _rowSize);
#ifndef ZX_FAST_BIT_STORAGE
	left = left * 32 + BitHacks::NumberOfTrailingZeros(_bits[bitsOffset]);
#endif
	return true;
}

bool
BitMatrix::getBottomRightOnBit(int& right, int& bottom) const
{
	int bitsOffset = Size(_bits) - 1 - (int)std::distance(_bits.rbegin(), std::find_if(_bits.rbegin(), _bits.rend(), isSet_));
	if (bitsOffset < 0) {
		return false;
	}

	bottom = bitsOffset / _rowSize;
	right = (bitsOffset % _rowSize);
#ifndef ZX_FAST_BIT_STORAGE
	right = right * 32 + 31 - BitHacks::NumberOfLeadingZeros(_bits[bitsOffset]);
#endif
	return true;
}

#ifdef ZX_FAST_BIT_STORAGE
constexpr BitMatrix::data_t BitMatrix::SET_V;
constexpr BitMatrix::data_t BitMatrix::UNSET_V;

void BitMatrix::getPatternRow(int r, PatternRow& p_row) const
{
	auto b_row = row(r);
#if 0
	p_row.reserve(64);
	p_row.clear();

	auto* lastPos = b_row.begin();
	if (BitMatrix::isSet(*lastPos))
		p_row.push_back(0); // first value is number of white pixels, here 0

	for (auto* p = b_row.begin() + 1; p < b_row.end(); ++p)
		if (bool(*p) != bool(*lastPos))
			p_row.push_back(p - std::exchange(lastPos, p));

	p_row.push_back(b_row.end() - lastPos);

	if (BitMatrix::isSet(*lastPos))
		p_row.push_back(0); // last value is number of white pixels, here 0
#else
	p_row.resize(width() + 2);
	std::fill(p_row.begin(), p_row.end(), 0);

	auto* bitPos = b_row.begin();
	auto* intPos = p_row.data();

	intPos += BitMatrix::isSet(*bitPos); // first value is number of white pixels, here 0

	while (++bitPos < b_row.end()) {
		++(*intPos);
		intPos += bitPos[0] != bitPos[-1];
	}
	++(*intPos);

	if (BitMatrix::isSet(bitPos[-1]))
		intPos++;

	p_row.resize(intPos - p_row.data() + 1);
#endif
}
#endif

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

BitMatrix Deflate(const BitMatrix& input, int width, int height, float top, float left, float subSampling)
{
	BitMatrix result(width, height);

	for (int y = 0; y < result.height(); y++) {
		auto yOffset = top + y * subSampling;
		for (int x = 0; x < result.width(); x++) {
			if (input.get(PointF(left + x * subSampling, yOffset)))
				result.set(x, y);
		}
	}

	return result;
}

} // ZXing

