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

std::wstring BuildPath(const std::wstring& dir, const std::wstring& name)
{
	if (dir.empty()) {
		return name;
	}
	if (name.empty()) {
		return dir;
	}
	if (dir.back() == L'/' || name.front() == L'/') {
		return dir + name;
	}
	return dir + L"/" + name;
}

static std::vector<std::wstring> GetImagesInDirectory(const std::wstring& dirPath)
{
	std::vector<std::wstring> result;
	WIN32_FIND_DATA data;
	HANDLE hFind = FindFirstFile(BuildPath(dirPath, L"*.png").c_str(), &data);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			result.push_back(data.cFileName);
		} while (FindNextFile(hFind, &data));
		FindClose(hFind);
	}
	return result;
}

static void FixeBitmapFormat(Bitmap& bitmap)
{
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
}

static int GetRotationEnum(int degree)
{
	switch (degree) {
	case 90:
		return ZXing::BarcodeScanner::RotationCW90;
	case 180:
		return ZXing::BarcodeScanner::Rotation180;
	case 270:
		return ZXing::BarcodeScanner::RotationCCW90;
	default:
		return ZXing::BarcodeScanner::Rotation0;
	}
}

struct TestCase
{
	std::string testName;
	int mustPassCount;			// The number of images which must decode for the test to pass.
	int tryHarderCount;			// The number of images which must pass using the try harder flag.
	int maxMisreads;			// Maximum number of images which can fail due to successfully reading the wrong contents
	int maxTryHarderMisreads;	// Maximum number of images which can fail due to successfully reading the wrong contents
	int rotation;				// The rotation in degrees clockwise to use for this test.
};

bool CheckResult(std::wstring imgPath, const std::string& expectedFormat, const ZXing::BarcodeScanner::ScanResult& result)
{
	if (expectedFormat != result.format) {
		std::cout << "Format mismatch: expected " << expectedFormat << " but got " << result.format << std::endl;
		return false;
	}

	imgPath.replace(imgPath.size() - 4, 4, L".txt");
	std::string expected;
	if (std::getline(std::ifstream(imgPath), expected)) {
		if (result.text != expected) {
			std::cout << "Content mismatch: expected " << expected << " but got " << result.text << std::endl;
			return false;
		}
		return true;
	}

	std::cout << "Error reading file " << std::flush;
	std::wcout << imgPath << std::endl;
	return false;
}


static void RunTests(std::wstring& dirPath, const std::string& format, const std::vector<TestCase>& tests)
{
	auto images = GetImagesInDirectory(dirPath);

	for (auto& test : tests)
	{
		int passCount[2] = { 0, 0 };
		int misreadCount[2] = { 0, 0 };

		for (int i = 0; i < 2; ++i) {
			ZXing::BarcodeScanner scanner(i != 0, false, format);
			for (auto& fileName : images) {
				auto imagePath = BuildPath(dirPath, fileName);
				Bitmap bitmap(imagePath.c_str());
				FixeBitmapFormat(bitmap);
				auto result = scanner.scan(bitmap, GetRotationEnum(test.rotation));
				if (!result.format.empty() && CheckResult(imagePath, format, result)) {
					passCount[i]++;
				}
				else {
					misreadCount[i]++;
				}
			}
		}

		std::cout << "Test: " << test.testName << std::endl;
		std::cout << "[0] Must pass: " << test.mustPassCount
			<< "; passed: " << passCount[0] << "/" << images.size()
			<< " => " << (passCount[0] >= test.mustPassCount ? "OK" : "Failed!!!")
			<< std::endl;
		std::cout << "[1] Must pass: " << test.tryHarderCount
			<< "; passed: " << passCount[1] << "/" << images.size()
			<< " => " << (passCount[1] >= test.tryHarderCount ? "OK" : "Failed!!!")
			<< std::endl;
	}


}

int main(int argc, char** argv)
{
	if (argc <= 1)
	{
		std::cout << "Usage: " << argv[0] << " <test_path_prefix>" << std::endl;
		return 0;
	}

	std::wstring prefix(argv[1], argv[1] + strlen(argv[1]));

	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);


	try
	{
		RunTests(BuildPath(prefix, L"blackbox/aztec-1"), "AZTEC", {
			{ "aztec-1 0  ", 12, 12, 0   },
			{ "aztec-1 90 ", 12, 12, 90  },
			{ "aztec-1 180", 12, 12, 180 },
			{ "aztec-1 270", 12, 12, 270 },
		});

		RunTests(BuildPath(prefix, L"blackbox/aztec-2"), "AZTEC", {
			{ "aztec-2 0  ", 5, 5, 0   },
			{ "aztec-2 90 ", 4, 4, 90  },
			{ "aztec-2 180", 6, 6, 180 },
			{ "aztec-2 270", 3, 3, 270 },
		});
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

	GdiplusShutdown(gdiplusToken);
}
