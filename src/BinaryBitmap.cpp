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

#include "BinaryBitmap.h"
#include "Binarizer.h"
#include "LuminanceSource.h"

#include <stdexcept>
#include <mutex>

namespace ZXing {

class BinaryBitmap::Private
{
public:
	std::shared_ptr<Binarizer> binarizer;
	BitMatrix matrix;
	std::once_flag matrixOnce;
	bool matrixOK;

	void initMatrix()
	{
		matrixOK = binarizer->getBlackMatrix(matrix);
	}
};


BinaryBitmap::BinaryBitmap(const std::shared_ptr<Binarizer>& binarizer)
{
	if (binarizer == nullptr) {
		throw std::invalid_argument("Binarizer must be non-null.");
	}

	_impl = std::make_shared<BinaryBitmap::Private>();
	_impl->binarizer = binarizer;
}

BinaryBitmap::~BinaryBitmap()
{
}

int
BinaryBitmap::width() const
{
	return _impl->binarizer->width();
}

int
BinaryBitmap::height() const
{
	return _impl->binarizer->height();
}

bool
BinaryBitmap::getBlackRow(int y, BitArray& outRow) const
{
	return _impl->binarizer->getBlackRow(y, outRow);
}

bool
BinaryBitmap::getBlackMatrix(BitMatrix& outMatrix) const
{
	// The matrix is created on demand the first time it is requested, then cached. There are two
	// reasons for this:
	// 1. This work will never be done if the caller only installs 1D Reader objects, or if a
	//    1D Reader finds a barcode before the 2D Readers run.
	// 2. This work will only be done once even if the caller installs multiple 2D Readers.
	std::call_once(_impl->matrixOnce, &BinaryBitmap::Private::initMatrix, _impl);

	if (_impl->matrixOK) {
		outMatrix = _impl->matrix;
		return true;
	}
	return false;
}

bool
BinaryBitmap::canCrop() const
{
	return _impl->binarizer->luminanceSource()->canCrop();
}

BinaryBitmap
BinaryBitmap::cropped(int left, int top, int width, int height) const
{
	auto newSource = _impl->binarizer->luminanceSource()->cropped(left, top, width, height);
	return BinaryBitmap(_impl->binarizer->createBinarizer(newSource));
}

bool
BinaryBitmap::canRotate() const
{
	return _impl->binarizer->luminanceSource()->canRotate();
}

BinaryBitmap
BinaryBitmap::rotatedCCW90() const
{
	auto newSource = _impl->binarizer->luminanceSource()->rotatedCCW90();
	return BinaryBitmap(_impl->binarizer->createBinarizer(newSource));
}

BinaryBitmap
BinaryBitmap::rotatedCCW45() const
{
	auto newSource = _impl->binarizer->luminanceSource()->rotatedCCW45();
	return BinaryBitmap(_impl->binarizer->createBinarizer(newSource));
}

} // ZXing
