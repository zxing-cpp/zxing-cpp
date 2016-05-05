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
#include "RGBLuminanceSource.h"
#include "HybridBinarizer.h"
#include "BinaryBitmap.h"
#include "MultiFormatReader.h"
#include "Result.h"
#include "DecodeHints.h"

#include <windows.h>
#include <gdiplus.h>
#include <mutex>

namespace ZXing {


static void InitStringCodecs()
{

}


static std::shared_ptr<LuminanceSource>
CreateLuminanceSource(Gdiplus::Bitmap& bitmap, const Gdiplus::BitmapData& data)
{
	switch (bitmap.GetPixelFormat())
	{
	case PixelFormat24bppRGB:
		return std::make_shared<RGBLuminanceSource>(data.Scan0, data.Width, data.Height, data.Stride, 3, 2, 1, 0);
	case PixelFormat32bppARGB:
	case PixelFormat32bppRGB:
		return std::make_shared<RGBLuminanceSource>(data.Scan0, data.Width, data.Height, data.Stride, 4, 2, 1, 0);
	}
	throw std::invalid_argument("Unsupported format");
}

static std::shared_ptr<Binarizer>
CreateBinarizer(Gdiplus::Bitmap& bitmap, const Gdiplus::BitmapData& data)
{
	return std::make_shared<HybridBinarizer>(CreateLuminanceSource(bitmap, data));
}


BarcodeScanner::ScanResult
BarcodeScanner::Scan(Gdiplus::Bitmap& bitmap)
{
	static std::once_flag s_once;
	std::call_once(s_once, InitStringCodecs);

	Result result(ErrorStatus::NotFound);
	{
		Gdiplus::BitmapData data;
		bitmap.LockBits(nullptr, Gdiplus::ImageLockModeRead, bitmap.GetPixelFormat(), &data);
		try
		{
			auto binarizer = CreateBinarizer(bitmap, data);
			BinaryBitmap binImg(binarizer);
			MultiFormatReader reader;
			result = reader.decode(binImg);
			if (!result.isValid()) {
				DecodeHints hints;
				hints.put(DecodeHint::TRY_HARDER, true);
				result = reader.decode(binImg, &hints);
			}
			bitmap.UnlockBits(&data);
		}
		catch (...)
		{
			bitmap.UnlockBits(&data);
			throw;
		}
	}

	if (result.isValid()) {
		return{ ToString(result.format()), result.text().toStdString() };
	}
	return ScanResult();
}

} // ZXing
