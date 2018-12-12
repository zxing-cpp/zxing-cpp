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
#include "ByteArray.h"

#include <stdexcept>
#include <algorithm>

namespace ZXing {

namespace {

/**
* A wrapper implementation of {@link LuminanceSource} which inverts the luminances it returns -- black becomes
* white and vice versa, and each value becomes (255-value).
*
* @author Sean Owen
*/
class InvertedLuminanceSource : public LuminanceSource
{
	std::shared_ptr<LuminanceSource> _src;

public:
	explicit InvertedLuminanceSource(std::shared_ptr<LuminanceSource> src) : _src(std::move(src)) {}

	const uint8_t* getRow(int y, ByteArray& outBytes, bool) const override
	{
		_src->getRow(y, outBytes, true);
		std::transform(outBytes.begin(), outBytes.end(), outBytes.begin(), [](uint8_t b) { return 255 - b; });
		return outBytes.data();
	}

	const uint8_t* getMatrix(ByteArray& outBytes, int& outRowBytes, bool) const override
	{
		_src->getMatrix(outBytes, outRowBytes, true);
		std::transform(outBytes.begin(), outBytes.end(), outBytes.begin(), [](uint8_t b) { return 255 - b; });
		return outBytes.data();
	}

	int width() const override
	{
		return _src->width();
	}

	/**
	* @return The height of the bitmap.
	*/
	int height() const override
	{
		return _src->height();
	}

	bool canCrop() const override
	{
		return _src->canCrop();
	}

	std::shared_ptr<LuminanceSource> cropped(int left, int top, int width, int height) const override
	{
		return CreateInverted(_src->cropped(left, top, width, height));
	}

	bool canRotate() const override
	{
		return _src->canRotate();
	}

	std::shared_ptr<LuminanceSource> rotated(int degreeCW) const override
	{
		return CreateInverted(_src->rotated(degreeCW));
	}

protected:
	std::shared_ptr<LuminanceSource> getInverted() const override
	{
		return _src;
	}

}; // InvertedLuminanceSource

} // anonymous


bool
LuminanceSource::canCrop() const
{
	return false;
}

std::shared_ptr<LuminanceSource>
LuminanceSource::cropped(int, int, int, int) const
{
	throw std::runtime_error("This luminance source does not support cropping.");
}

bool
LuminanceSource::canRotate() const
{
	return false;
}

/**
* Returns a new object with rotated image data by 90 degrees counterclockwise.
* Only callable if {@link #isRotateSupported()} is true.
*
* @return A rotated version of this object.
*/
std::shared_ptr<LuminanceSource>
LuminanceSource::rotated(int) const
{
	throw std::runtime_error("This luminance source does not support rotation by 90 degrees.");
}

std::shared_ptr<LuminanceSource>
LuminanceSource::getInverted() const
{
	return nullptr;
}

std::shared_ptr<LuminanceSource>
LuminanceSource::CreateInverted(const std::shared_ptr<LuminanceSource>& src)
{
	auto result = src->getInverted();
	if (result == nullptr)
		result = std::make_shared<InvertedLuminanceSource>(src);
	return result;
}

} // ZXing
