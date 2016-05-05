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

#include "BitWrapperBinarizer.h"
#include "ErrorStatus.h"
#include "BitMatrix.h"
#include "BitArray.h"

namespace ZXing {

BitWrapperBinarizer::BitWrapperBinarizer(const BitMatrix& bits, bool whitePixels) :
	BitWrapperBinarizer(std::make_shared<BitMatrix>(bits), whitePixels)
{
}

BitWrapperBinarizer::BitWrapperBinarizer(const std::shared_ptr<const BitMatrix>& bits, bool whitePixels) :
	BitWrapperBinarizer(bits, 0, 0, bits->width(), bits->height(), whitePixels)
{
}

BitWrapperBinarizer::BitWrapperBinarizer(const std::shared_ptr<const BitMatrix>& bits, int left, int top, int width, int height, bool inverted) :
	_matrix(bits),
	_left(left),
	_top(top),
	_width(width),
	_height(height),
	_inverted(inverted)
{
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
ErrorStatus
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
	if (_inverted) {
		row.flipAll();
	}
	return ErrorStatus::NoError;
}

// Does not sharpen the data, as this call is intended to only be used by 2D Readers.
ErrorStatus
BitWrapperBinarizer::getBlackMatrix(BitMatrix& matrix) const
{
	if (_width == _matrix->width() && _height == _matrix->height()) {
		matrix = *_matrix;
	}
	else {
		matrix.init(_width, _height);
		BitArray tmp;
		BitArray row;
		for (int y = 0; y < _height; ++y) {
			_matrix->getRow(_top + y, tmp);
			tmp.getSubArray(_left, _width, row);
			matrix.setRow(y, row);
		}
	}
	if (_inverted) {
		matrix.flipAll();
	}
	return ErrorStatus::NoError;
}

bool
BitWrapperBinarizer::canCrop() const
{
	return true;
}

std::shared_ptr<Binarizer>
BitWrapperBinarizer::cropped(int left, int top, int width, int height) const
{
	return std::make_shared<BitWrapperBinarizer>(_matrix, left + _left, top + _top, width, height, _inverted);
}

bool
BitWrapperBinarizer::canRotate() const
{
	return false;
}

std::shared_ptr<Binarizer>
BitWrapperBinarizer::rotatedCCW90() const
{
	throw std::runtime_error("This binarizer source does not support rotation by 90 degrees.");
}

std::shared_ptr<Binarizer>
BitWrapperBinarizer::rotatedCCW45() const
{
	throw std::runtime_error("This binarizer source does not support rotation by 45 degrees.");
}

} // ZXing
