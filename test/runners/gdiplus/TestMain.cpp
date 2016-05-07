#include <windows.h>
#include <gdiplus.h>
#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <vector>

#include "BarcodeScanner.h"

using namespace Gdiplus;

const char* GetFileName(const char* filePath)
{
	const char* result = filePath;
	while (*filePath) {
		if (*filePath == '\\' || *filePath == '/') {
			result = filePath + 1;
		}
		++filePath;
	}
	return result;
}

const char* CheckResult(const char* filePath, const std::string& result)
{
	std::string path(filePath);
	path.replace(path.size() - 4, 4, ".txt");
	std::string expected;
	if (std::getline(std::ifstream(path), expected)) {
		return result == expected ? "OK" : "Error";
	}
	return "Error reading file";
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

	for (int i = 1; i < argc; ++i)
	{
		std::cout << "Reading " << GetFileName(argv[i]) << " : " << std::flush;
		try
		{
			std::wstring filePath(argv[i], argv[i] + strlen(argv[i]));
			Bitmap bitmap(filePath.c_str());

			switch (bitmap.GetPixelFormat()) {
			case PixelFormat24bppRGB:
			case PixelFormat32bppARGB:
			case PixelFormat32bppRGB:
				break;
			default:
				if (bitmap.ConvertFormat(PixelFormat24bppRGB, Gdiplus::DitherTypeNone, Gdiplus::PaletteTypeCustom, nullptr, 0) != Gdiplus::Ok) {
					throw std::runtime_error("Cannot convert bitmap");
				}
			}

			auto result = ZXing::BarcodeScanner(false, true).scan(bitmap);
			if (result.format.empty()) {
				result = ZXing::BarcodeScanner(true, true).scan(bitmap);
			}
			if (result.format.empty()) {

			}

			if (result.format.empty()) {
				std::cout << "Not found";
			}
			else {
				std::cout << result.format << ": " << result.text << " => " << CheckResult(argv[i], result.text);
			}
		}
		catch (const std::exception& e)
		{
			std::cout << e.what();
		}
		catch (...)
		{
			std::cout << "Internal error";
		}
		std::cout << std::endl;
	}

	GdiplusShutdown(gdiplusToken);
}
