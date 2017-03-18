#pragma once
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

#include "BinaryBitmap.h"
#include <memory>

namespace ZXing {

/**
* This class provides a binarizer that wraps around a BitMatrix
*/
class BitWrapperBinarizer : public BinaryBitmap
{
public:
	explicit BitWrapperBinarizer(const std::shared_ptr<const BitMatrix>& bits, bool pureBarcode = false);
	BitWrapperBinarizer(const std::shared_ptr<const BitMatrix>& bits, int left, int top, int width, int height, bool pureBarcode = false);

	virtual bool isPureBarcode() const override;
	virtual int width() const override;
	virtual int height() const override;
	virtual DecodeStatus getBlackRow(int y, BitArray& outArray) const override;
	virtual std::shared_ptr<const BitMatrix> getBlackMatrix() const override;
	virtual bool canCrop() const override;
	virtual std::shared_ptr<BinaryBitmap> cropped(int left, int top, int width, int height) const override;
	virtual bool canRotate() const override;
	virtual std::shared_ptr<BinaryBitmap> rotated(int degreeCW) const override;

private:
	std::shared_ptr<const BitMatrix> _matrix;
	int _left;
	int _top;
	int _width;
	int _height;
	bool _pureBarcode;
};

} // ZXing
