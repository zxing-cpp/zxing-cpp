#include <windows.h>
#include <gdiplus.h>
#include <iostream>
#include <string>
#include <memory>
#include <vector>

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
		Gdiplus::Bitmap bitmap(filePath.c_str());

		//CLSID pngClsid;
		//GetEncoderClsid(L"image/png", &pngClsid);

		{
			BitmapData data;
			bitmap.LockBits(nullptr, ImageLockModeRead, bitmap.GetPixelFormat(), &data);
			try
			{
				cv::Mat image(data.Height, data.Width, CVTypeFromPixelFormat(data.PixelFormat), data.Scan0, data.Stride);
				cv::Mat gray;
				
				
				
				bitmap.UnlockBits(&data);
			}
			catch (...)
			{
				bitmap.UnlockBits(&data);
				throw;
			}
		}

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