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

#include "BarcodeScanner.h"
#include "StringCodecs.h"

#include <mutex>

namespace ZXing {


static void InitStringCodecs()
{

}


static std::shared_ptr<Binarizer> CreateLuminanceSource(const BitmapData& bitmap)
{
	switch (bitmap.PixelFormat)
	{
	case PixelFormat24bppRGB:
		return std::make_shared<RGBLuminanceSource>(bitmap.Scan0, bitmap.Width, bitmap.Height, bitmap.Stride, 3, 2, 1, 0);
	case PixelFormat32bppARGB:
	case PixelFormat32bppRGB:
		return std::make_shared<RGBLuminanceSource>(bitmap.Scan0, bitmap.Width, bitmap.Height, bitmap.Stride, 4, 2, 1, 0);
	}
	throw std::invalid_argument("Unsupported format");
}


BarcodeScanner::ScanResult
BarcodeScanner::Scan(const Gdiplus::Bitmap& bitmap)
{
	static std::once_flag s_once;
	std::call_once(s_once, InitStringCodecs);


}

} // ZXing
