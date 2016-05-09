#include <windows.h>
#include <gdiplus.h>
#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <vector>
#include <unordered_set>

#include "BarcodeScanner.h"

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

std::string BuildPath(const std::string& dir, const std::string& name)
{
	if (dir.empty()) {
		return name;
	}
	if (name.empty()) {
		return dir;
	}
	if (dir.back() == '/' || name.front() == '/') {
		return dir + name;
	}
	return dir + "/" + name;
}

static std::vector<std::string> GetImagesInDirectory(const std::string& dirPath)
{
	std::vector<std::string> result;
	WIN32_FIND_DATAA data;
	HANDLE hFind = FindFirstFileA(BuildPath(dirPath, "*.png").c_str(), &data);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
		//if (strcmp(data.cFileName, "06.png") == 0)
			result.push_back(data.cFileName);
		} while (FindNextFileA(hFind, &data));
		FindClose(hFind);
	}
	return result;
}

static void FixeBitmapFormat(Gdiplus::Bitmap& bitmap)
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
	int mustPassCount;			// The number of images which must decode for the test to pass.
	int tryHarderCount;			// The number of images which must pass using the try harder flag.
	int maxMisreads;			// Maximum number of images which can fail due to successfully reading the wrong contents
	int maxTryHarderMisreads;	// Maximum number of images which can fail due to successfully reading the wrong contents
	int rotation;				// The rotation in degrees clockwise to use for this test.

	TestCase(int mpc, int thc, int r) : TestCase(mpc, thc, 0, 0, r) {}
	TestCase(int mpc, int thc, int mm, int mt, int r) : mustPassCount(mpc), tryHarderCount(thc), maxMisreads(mm), maxTryHarderMisreads(mt), rotation(r) {}
};

bool CheckResult(std::string imgPath, const std::string& expectedFormat, const ZXing::BarcodeScanner::ScanResult& result, std::string& log)
{
	if (expectedFormat != result.format) {
		log += "Format mismatch: expected " + expectedFormat + " but got " + result.format + "\n";
		return false;
	}

	imgPath.replace(imgPath.size() - 4, 4, ".txt");
	std::string expected;
	if (std::getline(std::ifstream(imgPath), expected)) {
		if (result.text != expected) {
			log += "Content mismatch: expected " + expected + " but got " + result.text + "\n";
			return false;
		}
		return true;
	}

	log += "Error reading file\n";
	return false;
}

std::string GetFormatFromFolderName(const char* folderName)
{
	int len = std::strlen(folderName);
	while (--len > 0) {
		if (folderName[len] == '-') {
			break;
		}
	}
	std::string format(folderName, len);
	for (char& c : format) {
		c = static_cast<char>(toupper(c));
	}
	return format;
}

static std::string PathPrefix;

static void DoRunTests(std::ostream& output, const char* directory, const char* format, const std::vector<TestCase>& tests)
{
	auto dirPath = BuildPath(PathPrefix, directory);
	auto images = GetImagesInDirectory(dirPath);
	auto folderName = GetFileName(directory);
	//auto format = GetFormatFromFolderName(folderName);

	for (auto& test : tests)
	{
		int passCount[2] = { 0, 0 };
		std::vector<std::string> logTexts(images.size());
		std::unordered_set<std::string> notFound[2];
		std::unordered_set<std::string> misRead[2];

		for (size_t j = 0; j < images.size(); ++j) {
			auto imagePath = BuildPath(dirPath, images[j]);
			Gdiplus::Bitmap bitmap(std::wstring(imagePath.begin(), imagePath.end()).c_str());
			FixeBitmapFormat(bitmap);
			for (int i = 0; i < 2; ++i) {
				ZXing::BarcodeScanner scanner(i != 0, i != 0);
				auto result = scanner.scan(bitmap, GetRotationEnum(test.rotation));
				if (!result.format.empty()) {
					if (CheckResult(imagePath, format, result, logTexts[j])) {
						passCount[i]++;
					}
					else {
						misRead[i].insert(images[j]);
					}
				}
				else {
					notFound[i].insert(images[j]);
				}
			}
		}

		output << "Test: " << folderName << ", rotation: " << test.rotation << std::endl;
		output << "[Fast] Must pass: " << test.mustPassCount
			<< "; passed: " << passCount[0] << "/" << images.size()
			<< " => " << (passCount[0] >= test.mustPassCount ? "OK" : "Failed!!!")
			<< std::endl;
		output << "[Slow] Must pass: " << test.tryHarderCount
			<< "; passed: " << passCount[1] << "/" << images.size()
			<< " => " << (passCount[1] >= test.tryHarderCount ? "OK" : "Failed!!!")
			<< std::endl;

		for (int i = 0; i < 2; ++i) {
			if (!notFound[i].empty()) {
				output << "Not detected [" << (i == 0 ? "fast" : "slow") << "]:";
				for (const auto& f : notFound[i]) {
					output << ' ' << f;
				}
				output << std::endl;
			}
		}

		for (int i = 0; i < 2; ++i) {
			if (!misRead[i].empty()) {
				output << "Read error [" << (i == 0 ? "fast" : "slow") << "]:";
				for (const auto& f : misRead[i]) {
					output << ' ' << f;
				}
				output << std::endl;
			}
		}

		if (passCount[0] < test.mustPassCount || passCount[1] < test.tryHarderCount) {
			output << "Errors:\n";
			for (size_t j = 0; j < images.size(); ++j) {
				if (!logTexts[j].empty()) {
					output << images[j] << ": " << logTexts[j] << std::endl;
				}
			}
			output << std::endl;
		}
	}
}

int main(int argc, char** argv)
{
	if (argc <= 1)
	{
		std::cout << "Usage: " << argv[0] << " <test_path_prefix>" << std::endl;
		return 0;
	}

	PathPrefix = argv[1];

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	std::unordered_set<std::string> includedTests;
	for (int i = 2; i < argc; ++i) {
		if (std::strlen(argv[i]) > 2 && argv[i][0] == '-' && argv[i][1] == 't') {
			includedTests.insert(argv[i] + 2);
		}
	}

	auto runTests = [&](const char* directory, const char* format, const std::vector<TestCase>& tests) {
		if (includedTests.empty() || includedTests.find(GetFileName(directory)) != includedTests.end()) {
			DoRunTests(std::cout, directory, format, tests);
		}
	};

	try
	{
		runTests("blackbox/aztec-1", "AZTEC", {
			{ 12, 12, 0   },
			{ 12, 12, 90  },
			{ 12, 12, 180 },
			{ 12, 12, 270 },
		});

		runTests("blackbox/aztec-2", "AZTEC", {
			{ 5, 5, 0   },
			{ 4, 4, 90  },
			{ 6, 6, 180 },
			{ 3, 3, 270 },
		});

		runTests("blackbox/datamatrix-1", "DATA_MATRIX", {
			{ 18, 18, 0   },
			{ 18, 18, 90  },
			{ 18, 18, 180 },
			{ 18, 18, 270 },
		});

		runTests("blackbox/datamatrix-2", "DATA_MATRIX", {
			{ 8,  8,  0, 1, 0   },
			{ 14, 14, 0, 1, 90  },
			{ 14, 14, 0, 1, 180 },
			{ 13, 13, 0, 1, 270 },
		});

		runTests("blackbox/codabar-1", "CODABAR", {
			{ 11, 11, 0   },
			{ 11, 11, 180 },
		});

		runTests("blackbox/code39-1", "CODE_39", {
			{ 4, 4, 0 },
			{ 4, 4, 180 },
		});

		// need extended 
		//RunTests("blackbox/code39-2", "CODE_39", {
		//	{ 2, 2, 0 },
		//	{ 2, 2, 180 },
		//});

		runTests("blackbox/code39-3", "CODE_39", {
			{ 17, 17, 0 },
			{ 17, 17, 180 },
		});

		runTests("blackbox/code93-1", "CODE_93", {
			{ 3, 3, 0 },
			{ 3, 3, 180 },
		});

		runTests("blackbox/code128-1", "CODE_128", {
			{ 6, 6, 0 },
			{ 6, 6, 180 },
		});

		runTests("blackbox/code128-2", "CODE_128", {
			{ 36, 39, 0 },
			{ 36, 39, 180 },
		});

		runTests("blackbox/code128-3", "CODE_128", {
			{ 2, 2, 0 },
			{ 2, 2, 180 },
		});

		runTests("blackbox/ean8-1", "EAN_8", {
			{ 3, 3, 0 },
			{ 3, 3, 180 },
		});

		runTests("blackbox/ean13-1", "EAN_13", {
			{ 30, 32, 0 },
			{ 27, 32, 180 },
		});

		runTests("blackbox/ean13-2", "EAN_13", {
			{ 12, 17, 0, 1, 0 },
			{ 11, 17, 0, 1, 180 },
		});

		runTests("blackbox/ean13-3", "EAN_13", {
			{ 53, 55, 0 },
			{ 55, 55, 180 },
		});

		runTests("blackbox/ean13-4", "EAN_13", {
			{ 6, 13, 1, 1, 0 },
			{ 7, 13, 1, 1, 180 },
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

	Gdiplus::GdiplusShutdown(gdiplusToken);
}
