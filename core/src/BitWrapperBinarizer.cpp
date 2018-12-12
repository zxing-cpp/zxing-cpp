/*
* Copyright 2016 Nu-book Inc.
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

#include "BitWrapperBinarizer.h"
#include "DecodeStatus.h"
#include "BitMatrix.h"
#include "BitArray.h"

namespace ZXing {

BitWrapperBinarizer::BitWrapperBinarizer(const std::shared_ptr<const BitMatrix>& bits, bool pureBarcode) :
	BitWrapperBinarizer(bits, 0, 0, bits->width(), bits->height(), pureBarcode)
{
}

BitWrapperBinarizer::BitWrapperBinarizer(std::shared_ptr<const BitMatrix> bits, int left, int top, int width, int height, bool pureBarcode) :
	_matrix(std::move(bits)),
	_left(left),
	_top(top),
	_width(width),
	_height(height),
	_pureBarcode(pureBarcode)
{
}

bool
BitWrapperBinarizer::isPureBarcode() const
{
	return _pureBarcode;
}

int
BitWrapperBinarizer::width() const
{
	return _width;
}

int
BitWrapperBinarizer::height() const
{
	return _height;
}

// Applies simple sharpening to the row data to improve performance of the 1D Readers.
bool
BitWrapperBinarizer::getBlackRow(int y, BitArray& row) const
{
	if (y < 0 || y >= _height) {
		throw std::out_of_range("Requested row is outside the matrix");
	}

	if (_width == _matrix->width()) {
		_matrix->getRow(_top + y, row);
	}
	else {
		BitArray tmp;
		_matrix->getRow(_top + y, tmp);
		tmp.getSubArray(_left, _width, row);
	}
	return true;
}

// Does not sharpen the data, as this call is intended to only be used by 2D Readers.
std::shared_ptr<const BitMatrix>
BitWrapperBinarizer::getBlackMatrix() const
{
	if (_width == _matrix->width() && _height == _matrix->height()) {
		return _matrix;
	}
	else {
		auto matrix = std::make_shared<BitMatrix>(_width, _height);
		BitArray tmp;
		BitArray row;
		for (int y = 0; y < _height; ++y) {
			_matrix->getRow(_top + y, tmp);
			tmp.getSubArray(_left, _width, row);
			matrix->setRow(y, row);
		}
		return matrix;
	}
}

bool
BitWrapperBinarizer::canCrop() const
{
	return true;
}

std::shared_ptr<BinaryBitmap>
BitWrapperBinarizer::cropped(int left, int top, int width, int height) const
{
	return std::make_shared<BitWrapperBinarizer>(_matrix, left + _left, top + _top, width, height);
}

bool
BitWrapperBinarizer::canRotate() const
{
	return false;
}

std::shared_ptr<BinaryBitmap>
BitWrapperBinarizer::rotated(int) const
{
	throw std::runtime_error("This binarizer source does not support rotation by 90 degrees.");
}

} // ZXing
