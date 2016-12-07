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

#include <windows.h>
#include <gdiplus.h>
#include <iostream>
#include <fstream>
#include <streambuf>
#include <string>
#include <memory>
#include <vector>
#include <unordered_set>
#include <chrono>
#include <sstream>

#include "BarcodeReader.h"
#include "../../core/src/TextDecoder.h"
#include "../../core/src/TextUtfEncoding.h"

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
//if (strcmp(data.cFileName, "24.png") == 0)
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
		return ZXing::BarcodeReader::RotationCW90;
	case 180:
		return ZXing::BarcodeReader::Rotation180;
	case 270:
		return ZXing::BarcodeReader::RotationCCW90;
	default:
		return ZXing::BarcodeReader::Rotation0;
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

bool CheckResult(std::string imgPath, const std::string& expectedFormat, const ZXing::BarcodeReader::ScanResult& result, std::string& log)
{
	if (expectedFormat != result.format) {
		log += "Format mismatch: expected " + expectedFormat + " but got " + result.format + "\n";
		return false;
	}

	imgPath.replace(imgPath.size() - 4, 4, ".txt");
	std::ifstream utf8Stream(imgPath, std::ios::binary);
	if (utf8Stream) {
		std::string expected((std::istreambuf_iterator<char>(utf8Stream)), std::istreambuf_iterator<char>());
		if (result.text != expected) {
			log += "Content mismatch: expected " + expected + " but got " + result.text + "\n";
			return false;
		}
		return true;
	}
	imgPath.replace(imgPath.size() - 4, 4, ".bin");
	std::ifstream latin1Stream(imgPath, std::ios::binary);
	if (latin1Stream) {
		std::wstring rawStr = ZXing::TextDecoder::FromLatin1(std::string((std::istreambuf_iterator<char>(latin1Stream)), std::istreambuf_iterator<char>()));
		std::string expected;
		ZXing::TextUtfEncoding::ToUtf8(rawStr, expected);
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
	int len = static_cast<int>(std::strlen(folderName));
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
static ZXing::BarcodeReader scanners[2] = { ZXing::BarcodeReader(false, false), ZXing::BarcodeReader(true, true) };

static const char* GOOD = "OK";
static const char* BAD = "!!!!!! FAILED !!!!!!";

static const char* GoodOrBad(bool test)
{
	return test ? GOOD : BAD;
}

static void DoRunTests(std::ostream& output, const char* directory, const char* format, int totalTests, const std::vector<TestCase>& tests)
{
	auto dirPath = BuildPath(PathPrefix, directory);
	auto images = GetImagesInDirectory(dirPath);
	auto folderName = GetFileName(directory);
	//auto format = GetFormatFromFolderName(folderName);

	if (images.size() != totalTests) {
		output << "TEST " << folderName << " => Expected number of tests: " << totalTests << ", got: " << images.size() << " => " << BAD << std::endl;
	}

	for (auto& test : tests)
	{
		int passCount[2] = { 0, 0 };
		int misReadCount[2] = { 0, 0 };
		std::vector<std::string> logTexts(images.size());
		std::unordered_set<std::string> notDetectedFiles[2];
		std::unordered_set<std::string> misReadFiles[2];

		for (size_t j = 0; j < images.size(); ++j) {
			auto imagePath = BuildPath(dirPath, images[j]);
			Gdiplus::Bitmap bitmap(std::wstring(imagePath.begin(), imagePath.end()).c_str());
			FixeBitmapFormat(bitmap);
			for (int i = 0; i < 2; ++i) {
				auto result = scanners[i].scan(bitmap, GetRotationEnum(test.rotation));
				if (!result.format.empty()) {
					if (CheckResult(imagePath, format, result, logTexts[j])) {
						passCount[i]++;
					}
					else {
						misReadCount[i]++;
						misReadFiles[i].insert(images[j]);
					}
				}
				else {
					notDetectedFiles[i].insert(images[j]);
				}
			}
		}

		output << "TEST " << folderName << ", rotation: " << test.rotation << ", total: " << images.size() << std::endl;
		output << "    Must pass (fast): " << test.mustPassCount
			<< "; passed: " << passCount[0]
			<< " => " << GoodOrBad(passCount[0] >= test.mustPassCount)
			<< std::endl;
		output << "    Must pass (slow): " << test.tryHarderCount
			<< "; passed: " << passCount[1]
			<< " => " << GoodOrBad(passCount[1] >= test.tryHarderCount)
			<< std::endl;
		if (test.maxMisreads > 0) {
			output << "    Max misread (fast): " << test.maxMisreads
				<< "; misread: " << misReadCount[0]
				<< " => " << GoodOrBad(test.maxMisreads >= misReadCount[0])
				<< std::endl;
		}
		if (test.maxTryHarderMisreads > 0) {
			output << "    Max misread (slow): " << test.maxTryHarderMisreads
				<< "; misread: " << misReadCount[1]
				<< " => " << GoodOrBad(test.maxTryHarderMisreads >= misReadCount[1])
				<< std::endl;
		}

		bool printError = false;
		if (passCount[0] < test.mustPassCount || passCount[1] < test.tryHarderCount) {
			printError = true;
			for (int i = 0; i < 2; ++i) {
				if (!notDetectedFiles[i].empty()) {
					output << "    Not detected (" << (i == 0 ? "fast" : "slow") << "):";
					for (const auto& f : notDetectedFiles[i]) {
						output << ' ' << f;
					}
					output << std::endl;
				}
			}
		}

		if (test.maxMisreads < misReadCount[0] || test.maxTryHarderMisreads < misReadCount[1]) {
			printError = true;
			for (int i = 0; i < 2; ++i) {
				if (!misReadFiles[i].empty()) {
					output << "    Read error (" << (i == 0 ? "fast" : "slow") << "):";
					for (const auto& f : misReadFiles[i]) {
						output << ' ' << f;
					}
					output << std::endl;
				}
			}
		}

		if (printError) {
			output << "Errors:\n";
			for (size_t j = 0; j < images.size(); ++j) {
				if (!logTexts[j].empty()) {
					output << images[j] << ": " << logTexts[j] << std::endl;
				}
			}
			output << std::endl;
		}

		output << std::endl;
	}
}

struct FalsePositiveTestCase
{
	int maxAllowed;			// Maximum number of images which can fail due to successfully reading the wrong contents
	int rotation;			// The rotation in degrees clockwise to use for this test.

	FalsePositiveTestCase(int m, int r) : maxAllowed(m), rotation(r) {};
};

static void DoRunFalsePositiveTests(std::ostream& output, const char* directory, int totalTests, const std::vector<FalsePositiveTestCase>& tests)
{
	auto dirPath = BuildPath(PathPrefix, directory);
	auto images = GetImagesInDirectory(dirPath);
	auto folderName = GetFileName(directory);
	//auto format = GetFormatFromFolderName(folderName);

	if (images.size() != totalTests) {
		output << "TEST " << folderName << " => Expected number of tests: " << totalTests << ", got: " << images.size() << " => " << BAD << std::endl;
	}

	for (auto& test : tests)
	{
		int positives[2] = { 0, 0 };
		std::unordered_set<std::string> misReadFiles[2];

		for (size_t j = 0; j < images.size(); ++j) {
			auto imagePath = BuildPath(dirPath, images[j]);
			Gdiplus::Bitmap bitmap(std::wstring(imagePath.begin(), imagePath.end()).c_str());
			FixeBitmapFormat(bitmap);
			for (int i = 0; i < 2; ++i) {
				auto result = scanners[i].scan(bitmap, GetRotationEnum(test.rotation));
				if (!result.format.empty()) {
					positives[i]++;
					misReadFiles[i].insert(images[j]);
				}
			}
		}

		output << "TEST " << folderName << ", rotation: " << test.rotation << ", total: " << images.size() << std::endl;
		output << "    Max allowed (fast): " << test.maxAllowed
			<< "; got: " << positives[0]
			<< " => " << GoodOrBad(test.maxAllowed >= positives[0])
			<< std::endl;
		output << "    Max allowed (slow): " << test.maxAllowed
			<< "; got: " << positives[1]
			<< " => " << GoodOrBad(test.maxAllowed >= positives[1])
			<< std::endl;
		if (test.maxAllowed < positives[0] || test.maxAllowed < positives[1]) {
			for (int i = 0; i < 2; ++i) {
				if (!misReadFiles[i].empty()) {
					output << "    Misread files (" << (i == 0 ? "fast" : "slow") << "):";
					for (const auto& f : misReadFiles[i]) {
						output << ' ' << f;
					}
					output << std::endl;
				}
			}
		}
		output << std::endl;
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

	auto removeExtension = [](const std::string& s) {
		auto pos = s.find_last_of('-');
		return pos != std::string::npos ? s.substr(0, pos) : s;
	};

	std::stringstream outbuf;

	std::ostream& out = std::cout;

	auto runTests = [&](const char* directory, const char* format, int total, const std::vector<TestCase>& tests) {
		std::string dirName = GetFileName(directory);
		if (includedTests.empty() || (includedTests.find(dirName) != includedTests.end() || includedTests.find(removeExtension(dirName)) != includedTests.end())) {
			DoRunTests(out, directory, format, total, tests);
		}
	};

	auto runFalsePositiveTests = [&](const char* directory, int total, const std::vector<FalsePositiveTestCase>& tests) {
		std::string dirName = GetFileName(directory);
		if (includedTests.empty() || (includedTests.find(dirName) != includedTests.end() || includedTests.find(removeExtension(dirName)) != includedTests.end())) {
			DoRunFalsePositiveTests(out, directory, total, tests);
		}
	};

	try
	{
		auto startTime = std::chrono::steady_clock::now();
		
		runTests("blackbox/aztec-1", "AZTEC", 13, {
			{ 13, 13, 0   },
			{ 13, 13, 90  },
			{ 13, 13, 180 },
			{ 13, 13, 270 },
		});

		runTests("blackbox/aztec-2", "AZTEC", 22, {
			{ 5, 5, 0   },
			{ 4, 4, 90  },
			{ 6, 6, 180 },
			{ 3, 3, 270 },
		});

		runTests("blackbox/datamatrix-1", "DATA_MATRIX", 21, {
			{ 21, 21, 0   },
			{ 21, 21, 90  },
			{ 21, 21, 180 },
			{ 21, 21, 270 },
		});

		runTests("blackbox/datamatrix-2", "DATA_MATRIX", 18, {
			{ 8,  8,  0, 1, 0   },
			{ 14, 14, 0, 1, 90  },
			{ 14, 14, 0, 1, 180 },
			{ 13, 13, 0, 1, 270 },
		});

		runTests("blackbox/codabar-1", "CODABAR", 11, {
			{ 11, 11, 0   },
			{ 11, 11, 180 },
		});

		runTests("blackbox/code39-1", "CODE_39", 4, {
			{ 4, 4, 0   },
			{ 4, 4, 180 },
		});

		// need extended mode
		//RunTests("blackbox/code39-2", "CODE_39", 2, {
		//	{ 2, 2, 0   },
		//	{ 2, 2, 180 },
		//});

		runTests("blackbox/code39-3", "CODE_39", 17, {
			{ 17, 17, 0   },
			{ 17, 17, 180 },
		});

		runTests("blackbox/code93-1", "CODE_93", 3, {
			{ 3, 3, 0   },
			{ 3, 3, 180 },
		});

		runTests("blackbox/code128-1", "CODE_128", 6, {
			{ 6, 6, 0   },
			{ 6, 6, 180 },
		});

		runTests("blackbox/code128-2", "CODE_128", 40, {
			{ 36, 39, 0   },
			{ 36, 39, 180 },
		});

		runTests("blackbox/code128-3", "CODE_128", 2, {
			{ 2, 2, 0   },
			{ 2, 2, 180 },
		});

		runTests("blackbox/ean8-1", "EAN_8", 8, {
			{ 3, 3, 0   },
			{ 3, 3, 180 },
		});

		runTests("blackbox/ean13-1", "EAN_13", 34, {
			{ 30, 32, 0   },
			{ 27, 32, 180 },
		});

		runTests("blackbox/ean13-2", "EAN_13", 28, {
			{ 12, 17, 0, 1, 0   },
			{ 11, 17, 0, 1, 180 },
		});

		runTests("blackbox/ean13-3", "EAN_13", 55, {
			{ 53, 55, 0   },
			{ 55, 55, 180 },
		});

		runTests("blackbox/ean13-4", "EAN_13", 22, {
			{ 6, 13, 1, 1, 0   },
			{ 7, 13, 1, 1, 180 },
		});

		runTests("blackbox/ean13-5", "EAN_13", 18, {
			{ 0, 0, 0   },
			{ 0, 0, 180 },
		});

		runTests("blackbox/itf-1", "ITF", 14, {
			{ 9,  13, 0   },
			{ 12, 13, 180 },
		});

		runTests("blackbox/itf-2", "ITF", 13, {
			{ 13, 13, 0   },
			{ 13, 13, 180 },
		});

		runTests("blackbox/upca-1", "UPC_A", 21, {
			{ 14, 18, 0, 1, 0   },
			{ 16, 18, 0, 1, 180 },
		});

		runTests("blackbox/upca-2", "UPC_A", 52, {
			{ 28, 36, 0, 2, 0   },
			{ 29, 36, 0, 2, 180 },
		});

		runTests("blackbox/upca-3", "UPC_A", 21, {
			{ 7, 9, 0, 2, 0   },
			{ 8, 9, 0, 2, 180 },
		});

		runTests("blackbox/upca-4", "UPC_A", 19, {
			{ 9, 11, 0, 1, 0   },
			{ 9, 11, 0, 1, 180 },
		});

		runTests("blackbox/upca-5", "UPC_A", 35, {
			{ 20, 23, 0, 0, 0   },
			{ 22, 23, 0, 0, 180 },
		});
		
		runTests("blackbox/upca-6", "UPC_A", 19, {
			{ 0, 0, 0   },
			{ 0, 0, 180 },
		});

		runTests("blackbox/upcean-extension-1", "EAN_13", 2, {
			{ 2, 2, 0 },
		});

		runTests("blackbox/upce-1", "UPC_E", 3, {
			{ 3, 3, 0   },
			{ 3, 3, 180 },
		});

		runTests("blackbox/upce-2", "UPC_E", 41, {
			{ 31, 35, 0, 1, 0   },
			{ 31, 35, 1, 1, 180 },
		});

		runTests("blackbox/upce-3", "UPC_E", 11, {
			{ 6, 8, 0   },
			{ 6, 8, 180 },
		});

		runTests("blackbox/rss14-1", "RSS_14", 6, {
			{ 6, 6, 0   },
			{ 6, 6, 180 },
		});

		runTests("blackbox/rss14-2", "RSS_14", 24, {
			{ 4, 8, 1, 2, 0   },
			{ 2, 8, 0, 2, 180 },
		});

		runTests("blackbox/rssexpanded-1", "RSS_EXPANDED", 32, {
			{ 32, 32, 0   },
			{ 32, 32, 180 },
		});

		runTests("blackbox/rssexpanded-2", "RSS_EXPANDED", 23, {
			{ 21, 23, 0   },
			{ 21, 23, 180 },
		});

		runTests("blackbox/rssexpanded-3", "RSS_EXPANDED", 117, {
			{ 117, 117, 0   },
			{ 117, 117, 180 },
		});

		runTests("blackbox/rssexpandedstacked-1", "RSS_EXPANDED", 64, {
			{ 59, 64, 0   },
			{ 59, 64, 180 },
		});

		runTests("blackbox/rssexpandedstacked-2", "RSS_EXPANDED", 7, {
			{ 2, 7, 0   },
			{ 2, 7, 180 },
		});

		runTests("blackbox/qrcode-1", "QR_CODE", 20, {
			{ 17, 17, 0   },
			{ 14, 14, 90  },
			{ 17, 17, 180 },
			{ 14, 14, 270 },
		});

		runTests("blackbox/qrcode-2", "QR_CODE", 34, {
			{ 30, 30, 0   },
			{ 29, 29, 90  },
			{ 30, 30, 180 },
			{ 29, 29, 270 },
		});

		runTests("blackbox/qrcode-3", "QR_CODE", 42, {
			{ 38, 38, 0   },
			{ 38, 38, 90  },
			{ 36, 36, 180 },
			{ 39, 39, 270 },
		});

		runTests("blackbox/qrcode-4", "QR_CODE", 48, {
			{ 36, 36, 0   },
			{ 35, 35, 90  },
			{ 35, 35, 180 },
			{ 35, 35, 270 },
		});

		runTests("blackbox/qrcode-5", "QR_CODE", 19, {
			{ 19, 19, 0   },
			{ 19, 19, 90  },
			{ 19, 19, 180 },
			{ 18, 18, 270 },
		});

		runTests("blackbox/qrcode-6", "QR_CODE", 15, {
			{ 15, 15, 0   },
			{ 14, 14, 90  },
			{ 12, 13, 180 },
			{ 14, 14, 270 },
		});

		runTests("blackbox/pdf417-1", "PDF_417", 10, {
			{ 10, 10, 0   },
			{ 10, 10, 180 },
		});

		runTests("blackbox/pdf417-2", "PDF_417", 25, {
			{ 25, 25, 0   },
			{ 25, 25, 180 },
		});

		runTests("blackbox/pdf417-3", "PDF_417", 18, {
			{ 18, 18, 0   },
			{ 18, 18, 180 },
		});

		runFalsePositiveTests("blackbox/falsepositives-1", 22, {
			{ 2, 0   },
			{ 2, 90  },
			{ 2, 180 },
			{ 2, 270 },
		});

		runFalsePositiveTests("blackbox/falsepositives-2", 25, {
			{ 4, 0   },
			{ 4, 90  },
			{ 4, 180 },
			{ 4, 270 },
		});

		auto duration = std::chrono::steady_clock::now() - startTime;

		std::cout << outbuf.str() << std::endl;
		std::cout << "Total time: " << std::chrono::duration_cast<std::chrono::seconds>(duration).count() << " seconds." << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cout << e.what();
	}
	catch (...)
	{
		std::cout << "Internal error";
	}

	Gdiplus::GdiplusShutdown(gdiplusToken);
}
