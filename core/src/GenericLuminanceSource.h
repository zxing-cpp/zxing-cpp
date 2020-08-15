#pragma once
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

#include "LuminanceSource.h"

#include <cstdint>
#include <memory>

namespace ZXing {

/**
* This decode images from an address in memory as RGB or ARGB data.
*/
class GenericLuminanceSource : public LuminanceSource
{
public:
	virtual ~GenericLuminanceSource() = default;

	// Don't use in client code. Used internally for now to prevent lots of deprecation warning noise until the GenericLuminanceSource is completely removed.
	GenericLuminanceSource(int left, int top, int width, int height, const void* bytes, int rowBytes, int pixelBytes, int redIndex, int greenIndex, int blueIndex, void* deprecation_tag);
	/**
	* Init with a RGB source.
	*/
	[[deprecated]] // please use interface from ReadBarcode.h
	GenericLuminanceSource(int width, int height, const void* bytes, int rowBytes, int pixelBytes, int redIndex, int greenIndex, int blueIndex);

	/**
	* Init with a RGB source, left, top, width, height specify the subregion area in orignal image; 'bytes' points to the begining of image buffer (i.e. pixel (0,0)).
	*/
	[[deprecated]] // please use interface from ReadBarcode.h
	GenericLuminanceSource(int left, int top, int width, int height, const void* bytes, int rowBytes, int pixelBytes, int redIndex, int greenIndex, int blueIndex);

	/**
	* Init with a grayscale source.
	*/
	[[deprecated]] // please use interface from ReadBarcode.h
	GenericLuminanceSource(int width, int height, const void* bytes, int rowBytes);

	/**
	* Init with a grayscale source, left, top, width, height specify the subregion area in orignal image; 'bytes' points to the begining of image buffer (i.e. pixel (0,0)).
	*/
	[[deprecated]] // please use interface from ReadBarcode.h
	GenericLuminanceSource(int left, int top, int width, int height, const void* bytes, int rowBytes);

	/**
	* Init with a grayscale source, left, top, width, height specify the subregion area in orignal image; 'bytes' points the begining of image buffer (i.e. pixel (0,0)).
	*/
	[[deprecated]] // please use interface from ReadBarcode.h
	GenericLuminanceSource(int left, int top, int width, int height, std::shared_ptr<const ByteArray> pixels, int rowBytes);

	virtual int width() const override;
	virtual int height() const override;
	virtual const uint8_t* getRow(int y, ByteArray& buffer, bool forceCopy = false) const override;
	virtual const uint8_t* getMatrix(ByteArray& buffer, int& outRowBytes, bool forceCopy = false) const override;
	virtual bool canCrop() const override;
	virtual std::shared_ptr<LuminanceSource> cropped(int left, int top, int width, int height) const override;
	virtual bool canRotate() const override;
	virtual std::shared_ptr<LuminanceSource> rotated(int degreeCW) const override;

private:
	std::shared_ptr<const ByteArray> _pixels;
	int _left;
	int _top;
	int _width;
	int _height;
	int _rowBytes;
};

} // ZXing
