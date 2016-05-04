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
	_matrix(bits),
	_left(0),
	_top(0),
	_width(bits->width()),
	_height(bits->height()),
	_inverted(whitePixels)
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
	auto result = std::make_shared<BitWrapperBinarizer>();
	result->_bits
	return createBinarizer(_source->cropped(left, top, width, height));
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
