#include <windows.h>
#include <gdiplus.h>
#include <iostream>
#include <string>
#include <memory>
#include <vector>

#include "RGBLuminanceSource.h"
#include "HybridBinarizer.h"
#include "BinaryBitmap.h"
#include "MultiFormatReader.h"
#include "Result.h"

using namespace Gdiplus;
using namespace ZXing;

std::shared_ptr<LuminanceSource> CreateLuminanceSource(const BitmapData& bitmap)
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

int main(int argc, char** argv)
{
	if (argc <= 1)
	{
		std::cout << "Usage: " << argv[0] << " <image_path>" << std::endl;
		return 0;
	}

	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	try
	{
		std::wstring filePath(argv[1], argv[1] + strlen(argv[1]));
		Bitmap bitmap(filePath.c_str());

		//CLSID pngClsid;
		//GetEncoderClsid(L"image/png", &pngClsid);

		Result result(ErrorStatus::NotFound);

		{
			BitmapData data;
			bitmap.LockBits(nullptr, ImageLockModeRead, bitmap.GetPixelFormat(), &data);
			try
			{
				auto lum = CreateLuminanceSource(data);
				auto binarizer = std::make_shared<HybridBinarizer>(lum);
				BinaryBitmap binImg(binarizer);
				MultiFormatReader reader;
				result = reader.decode(binImg);
				bitmap.UnlockBits(&data);
			}
			catch (...)
			{
				bitmap.UnlockBits(&data);
				throw;
			}
		}

		std::cout << result.text();

/* 		if (output.rows > 0)
		{
			Gdiplus::Bitmap outputBitmap(output.cols, output.rows, PixelFormatFromCVType(output.type()));
			BitmapData dataOut;
			outputBitmap.LockBits(nullptr, ImageLockModeWrite, outputBitmap.GetPixelFormat(), &dataOut);
			try
			{
				output.copyTo(cv::Mat(dataOut.Height, dataOut.Width, CVTypeFromPixelFormat(dataOut.PixelFormat), dataOut.Scan0, dataOut.Stride));
				outputBitmap.UnlockBits(&dataOut);
			}
			catch (...)
			{
				outputBitmap.UnlockBits(&dataOut);
				throw;
			}

			outputBitmap.Save((filePath + L".filtered.jpg").c_str(), &pngClsid, nullptr);
		}
 */
	}
	catch (...)
	{
		std::cerr << "Internal error";
	}

	GdiplusShutdown(gdiplusToken);
}
