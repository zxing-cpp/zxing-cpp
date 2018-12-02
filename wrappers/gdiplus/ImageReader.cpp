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
#include "ImageReader.h"

#include <windows.h>
#include <gdiplus.h>
#include <type_traits>
#include <stdexcept>

#include "GenericLuminanceSource.h"

namespace ZXing {

static std::shared_ptr<LuminanceSource>
CreateLuminanceSource(Gdiplus::Bitmap& bitmap, const Gdiplus::BitmapData& data)
{
	switch (bitmap.GetPixelFormat())
	{
	case PixelFormat24bppRGB:
		return std::make_shared<GenericLuminanceSource>(data.Width, data.Height, data.Scan0, data.Stride, 3, 2, 1, 0);
	case PixelFormat32bppARGB:
	case PixelFormat32bppRGB:
		return std::make_shared<GenericLuminanceSource>(data.Width, data.Height, data.Scan0, data.Stride, 4, 2, 1, 0);
	}
	throw std::invalid_argument("Unsupported format");
}

std::shared_ptr<LuminanceSource> ImageReader::Read(Gdiplus::Bitmap& bitmap)
{
	Gdiplus::BitmapData data;
	bitmap.LockBits(nullptr, Gdiplus::ImageLockModeRead, bitmap.GetPixelFormat(), &data);
	try
	{
		auto result = CreateLuminanceSource(bitmap, data);
		bitmap.UnlockBits(&data);
		return result;
	}
	catch (...)
	{
		bitmap.UnlockBits(&data);
		throw;
	}
}

} // ZXing
