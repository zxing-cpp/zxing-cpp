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

#include "ImageWriter.h"
#include "BitMatrix.h"

#include <windows.h>
#include <gdiplus.h>

namespace ZXing {

std::shared_ptr<Gdiplus::Bitmap>
ImageWriter::CreateImage(const BitMatrix& barcode)
{
	using namespace Gdiplus;

	auto bitmap = std::make_shared<Bitmap>(barcode.width(), barcode.height(), PixelFormat32bppARGB);
	auto black = Color::Black;
	auto white = Color::White;
	auto rect = Rect(0, 0, barcode.width(), barcode.height());
	BitmapData bitmapData;
	bitmap->LockBits(&rect, Gdiplus::ImageLockModeWrite, PixelFormat32bppARGB, &bitmapData);
	auto pixelBytes = reinterpret_cast<char*>(bitmapData.Scan0);
	for (int y = 0; y < barcode.height(); ++y) {
		auto pixels = reinterpret_cast<ARGB*>(pixelBytes);
		for (int x = 0; x < barcode.width(); ++x) {
			*pixels++ = barcode.get(x, y) ? black : white;
		}
		pixelBytes += bitmapData.Stride;
	}
	bitmap->UnlockBits(&bitmapData);
	return bitmap;
}


} // ZXing
