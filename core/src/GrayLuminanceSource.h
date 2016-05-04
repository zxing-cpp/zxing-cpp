#pragma once
/*
* Copyright 2016 Huy Cuong Nguyen
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

#include "LuminanceSource.h"

namespace ZXing {

/**
* If you have already a buffer of grayscale pixels, you can use this class to get a luminance source.
* This class does not support rotation.
*/
class GrayLuminanceSource : public LuminanceSource
{
public:
	GrayLuminanceSource(const void* bytes, int width, int height, int rowBytes) :
		GrayLuminanceSource(bytes, width, height, rowBytes, 0, 0) {}
	GrayLuminanceSource(const void* bytes, int width, int height, int rowBytes, int xoffset, int yoffset);

	virtual int width() const override;
	virtual int height() const override;
	virtual const uint8_t* getRow(int y, ByteArray& buffer, bool forceCopy = false) const override;
	virtual const uint8_t* getMatrix(ByteArray& buffer, int& outRowBytes, bool forceCopy = false) const override;
	virtual bool canCrop() const override;
	virtual std::shared_ptr<LuminanceSource> cropped(int left, int top, int width, int height) const override;

private:
	const uint8_t* _bytes;
	int _width;
	int _height;
	int _rowBytes;
	int _xoffset;
	int _yoffset;
};

} // ZXing
