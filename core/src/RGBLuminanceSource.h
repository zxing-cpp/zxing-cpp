#pragma once
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

#include "LuminanceSource.h"

namespace ZXing {

/**
* This decode images from an address in memory as RGB or ARGB data.
* It doesn't support rotation.
*/
class RGBLuminanceSource : public LuminanceSource
{
public:
	RGBLuminanceSource(const void* bytes, int width, int height, int rowBytes, int pixelBytes, int redIndex, int greenIndex, int blueIndex) :
		RGBLuminanceSource(bytes, width, height, rowBytes, pixelBytes, redIndex, greenIndex, blueIndex, 0, 0) {}
	RGBLuminanceSource(const void* bytes, int width, int height, int rowBytes, int pixelBytes, int redIndex, int greenIndex, int blueIndex, int xoffset, int yoffset);

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
	int _pixelBytes;
	int _redIndex;
	int _greenIndex;
	int _blueIndex;
	int _xoffset;
	int _yoffset;
};

} // ZXing
