/*
* Copyright 2016 Nu-book Inc.
* Copyright 2017 Axel Waggershauser
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

#include "GenericLuminanceSource.h"
#include "HybridBinarizer.h"
#include "BinaryBitmap.h"
#include "MultiFormatReader.h"
#include "Result.h"
#include "DecodeHints.h"
#include "TextDecoder.h"
#include "TextUtfEncoding.h"
#include "ZXContainerAlgorithms.h"

#include <iostream>
#include <fstream>
#include <streambuf>
#include <string>
#include <memory>
#include <vector>
#include <unordered_set>
#include <chrono>
#include <stdexcept>

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

using namespace ZXing;

static std::vector<fs::path> getImagesInDirectory(const fs::path& dirPath)
{
	std::vector<fs::path> result;
	for (fs::directory_iterator i(dirPath); i != fs::directory_iterator(); ++i)
		if (is_regular_file(i->status()) && i->path().extension() == ".png")
			result.push_back(i->path());
	return result;
}

static std::shared_ptr<LuminanceSource> readPNM(FILE* f)
{
	int w, h;
	if (fscanf(f, "P5\n%d %d\n255\n", &w, &h) != 2)
		throw std::runtime_error("Failed to parse PNM file header.");
	auto ba = std::make_shared<ByteArray>(w * h);
	auto read = fread(ba->data(), sizeof(uint8_t), w*h, f);
//	if (read != w * h)
//		throw std::runtime_error("Failed to read PNM file data: " + std::to_string(read) + " != " + std::to_string(w * h) + " -> " + std::to_string(feof(f)));
	return std::make_shared<GenericLuminanceSource>(0, 0, w, h, ba, w);
}

static std::shared_ptr<LuminanceSource> readImage(const fs::path& filename)
{
	std::string cmd = "convert " + filename.native() + " -intensity Rec601Luma -colorspace gray pgm:-";
	bool pipe = filename.extension() != ".pgm";
	FILE* f = pipe ? popen(cmd.c_str(), "r") : fopen(filename.c_str(), "r");
	if (!f)
		throw std::runtime_error("Failed to open pipe '" + cmd + "': " + std::strerror(errno));
	try {
		auto res = readPNM(f);
		pipe ? pclose(f) : fclose(f);
		return res;
	} catch (std::runtime_error& e) {
		pipe ? pclose(f) : fclose(f);
		throw std::runtime_error("Failed to read pipe '" + cmd + "': " + e.what());
	}
}

class TestReader
{
	std::shared_ptr<MultiFormatReader> _reader;
	static std::map<fs::path, std::shared_ptr<HybridBinarizer>> _cache;
public:
	struct Result
	{
		std::string format, text;
	};

	TestReader(bool tryHarder, bool tryRotate, std::string format = "")
	{
		DecodeHints hints;
		hints.setShouldTryHarder(tryHarder);
		hints.setShouldTryRotate(tryRotate);
		auto f = FromString(format.c_str());
		if (f != BarcodeFormat::FORMAT_COUNT)
			hints.setPossibleFormats({f});

		_reader = std::make_shared<MultiFormatReader>(hints);
	}

	Result read(const fs::path& filename, int rotation = 0)
	{
		auto& binImg = _cache[filename];
		if (!binImg)
			binImg = std::make_shared<HybridBinarizer>(readImage(filename));
		auto result = _reader->read(*binImg->rotated(rotation));
		if (result.isValid()) {
			std::string text;
			TextUtfEncoding::ToUtf8(result.text(), text);
			return {ToString(result.format()), text};
		}
		return {};
	}

	static void clearCache() { _cache.clear(); }
};

std::map<fs::path, std::shared_ptr<HybridBinarizer>> TestReader::_cache;

struct TestCase
{
	struct TC
	{
		const char* name;
		int mustPassCount; // The number of images which must decode for the test to pass.
		int maxMisreads;   // Maximum number of images which can fail due to successfully reading the wrong contents
		std::unordered_set<std::string> notDetectedFiles;
		std::map<std::string, std::string> misReadFiles;
	};

	TC tc[2];
	int rotation; // The rotation in degrees clockwise to use for this test.

	TestCase(int mpc, int thc, int mm, int mt, int r) : tc{{"fast", mpc, mm}, {"slow", thc, mt}}, rotation(r) {}
	TestCase(int mpc, int thc, int r) : TestCase(mpc, thc, 0, 0, r) {}
};

static std::string checkResult(fs::path imgPath, const std::string& expectedFormat, const TestReader::Result& result)
{
	if (expectedFormat != result.format)
		return "Format mismatch: expected " + expectedFormat + " but got " + result.format;

	imgPath.replace_extension(".txt");
	std::ifstream utf8Stream(imgPath.native(), std::ios::binary);
	if (utf8Stream) {
		std::string expected((std::istreambuf_iterator<char>(utf8Stream)), std::istreambuf_iterator<char>());
		return result.text != expected ? "Content mismatch: expected " + expected + " but got " + result.text : "";
	}
	imgPath.replace_extension(".bin");
	std::ifstream latin1Stream(imgPath.native(), std::ios::binary);
	if (latin1Stream) {
		std::wstring rawStr = ZXing::TextDecoder::FromLatin1(
		    std::string((std::istreambuf_iterator<char>(latin1Stream)), std::istreambuf_iterator<char>()));
		std::string expected;
		ZXing::TextUtfEncoding::ToUtf8(rawStr, expected);
		return result.text != expected ? "Content mismatch: expected " + expected + " but got " + result.text : "";
	}
	return "Error reading file";
}

static fs::path pathPrefix;
static TestReader scanners[2] = {TestReader(false, false), TestReader(true, true)};

static const char* GOOD = "OK";
static const char* BAD = "!!!!!! FAILED !!!!!!";

static const char* goodOrBad(bool test)
{
	return test ? GOOD : BAD;
}

static void doRunTests(std::ostream& cout, const fs::path& directory, const char* format, int imageCount,
                       const std::vector<TestCase>& tests)
{
	TestReader::clearCache();

	auto images = getImagesInDirectory(pathPrefix / directory);
	auto folderName = directory.stem();

	if (images.size() != imageCount)
		cout << "TEST " << folderName << " => Expected number of tests: " << imageCount
		     << ", got: " << images.size() << " => " << BAD << std::endl;

	TestReader scanners[2] = {TestReader(false, false, format), TestReader(true, true, format)};

	for (auto& test : tests) {

		cout << "TEST " << folderName << ", rotation: " << test.rotation << ", total: " << images.size() << "\n";
		for (int i = 0; i < Length(scanners); ++i) {
			auto tc = test.tc[i];

			for (const fs::path& imagePath : images) {
				auto result = scanners[i].read(imagePath, test.rotation);
				if (!result.format.empty()) {
					auto error = checkResult(imagePath, format, result);
					if (!error.empty())
						tc.misReadFiles[imagePath.filename().string()] = error;
				} else {
					tc.notDetectedFiles.insert(imagePath.filename().string());
				}
			}

			auto passCount = images.size() - tc.misReadFiles.size() - tc.notDetectedFiles.size();

			cout << "    Must pass (" << tc.name << "): " << tc.mustPassCount << "; passed: " << passCount
			     << " => " << goodOrBad(passCount >= tc.mustPassCount) << "\n";
			if (tc.maxMisreads > 0) {
				cout << "    Max misread (" << tc.name << "): " << tc.maxMisreads
				     << "; misread: " << tc.misReadFiles.size()
				     << " => " << goodOrBad(tc.maxMisreads >= tc.misReadFiles.size()) << "\n";
			}

			if (passCount < tc.mustPassCount && !tc.notDetectedFiles.empty()) {
//			if (!tc.notDetectedFiles.empty()) {
				cout << "    Not detected (" << tc.name << "):";
				for (const auto& f : tc.notDetectedFiles)
					cout << ' ' << f;
				cout << "\n";
			}

			if (tc.misReadFiles.size() > tc.maxMisreads) {
				cout << "    Read error (" << tc.name << "):";
				for (const auto& f : tc.misReadFiles)
					cout << "      " << f.first << ": " << f.second << "\n";
				cout << "\n";
			}
		}

		cout << std::endl;
	}
}

struct FalsePositiveTestCase
{
	int maxAllowed; // Maximum number of images which can fail due to successfully reading the wrong contents
	int rotation;   // The rotation in degrees clockwise to use for this test.
};

static void doRunFalsePositiveTests(std::ostream& cout, const fs::path& directory, int totalTests,
                                    const std::vector<FalsePositiveTestCase>& tests)
{
	auto images = getImagesInDirectory(pathPrefix / directory);
	auto folderName = directory.filename();

	if (images.size() != totalTests) {
		cout << "TEST " << folderName << " => Expected number of tests: " << totalTests
		    << ", got: " << images.size() << " => " << BAD << std::endl;
	}

	for (auto& test : tests) {
		std::unordered_set<std::string> misReadFiles[2];

		for (const fs::path& imagePath : images) {
			for (int i = 0; i < Length(scanners); ++i) {
				auto result = scanners[i].read(imagePath, test.rotation);
				if (!result.format.empty())
					misReadFiles[i].insert(imagePath.string());
			}
		}

		cout << "TEST " << folderName << ", rotation: " << test.rotation << ", total: " << images.size() << "\n";
		cout << "    Max allowed (fast): " << test.maxAllowed << "; got: " << misReadFiles[0].size()
		     << " => " << goodOrBad(test.maxAllowed >= misReadFiles[0].size()) << "\n";
		cout << "    Max allowed (slow): " << test.maxAllowed << "; got: " << misReadFiles[1].size()
		     << " => " << goodOrBad(test.maxAllowed >= misReadFiles[1].size()) << "\n";
		if (test.maxAllowed < misReadFiles[0].size() || test.maxAllowed < misReadFiles[1].size()) {
			for (int i = 0; i < 2; ++i) {
				if (!misReadFiles[i].empty()) {
					cout << "    Misread files (" << (i == 0 ? "fast" : "slow") << "):";
					for (const auto& f : misReadFiles[i])
						cout << ' ' << f;
					cout << "\n";
				}
			}
		}
		cout << std::endl;
	}
}

int main(int argc, char** argv)
{
	if (argc <= 1) {
		std::cout << "Usage: " << argv[0] << " <test_path_prefix>" << std::endl;
		return 0;
	}

	pathPrefix = argv[1];

	if (pathPrefix.extension() == ".png" || pathPrefix.extension() == ".jpg" || pathPrefix.extension() == ".pgm") {
#if 0
		TestReader reader(false, false, "QR_CODE");
#else
		TestReader reader(true, true);
#endif
		auto result = reader.read(pathPrefix, argc >= 3 ? std::stoi(argv[2]) : 0);
		std::cout << result.format << ": " << result.text << std::endl;
		return 0;
	}

	std::unordered_set<std::string> includedTests;
	for (int i = 2; i < argc; ++i) {
		if (std::strlen(argv[i]) > 2 && argv[i][0] == '-' && argv[i][1] == 't')
			includedTests.insert(argv[i] + 2);
	}

	auto& out = std::cout;

	auto hasTest = [&includedTests](const fs::path& dir) {
		auto stem = dir.stem().string();
		return includedTests.empty() || includedTests.find(stem) != includedTests.end() ||
		       includedTests.find(stem.substr(0, stem.size() - 2)) != includedTests.end();
	};

	auto runTests = [&](const fs::path& directory, const char* format, int total, const std::vector<TestCase>& tests) {
		if (hasTest(directory))
			doRunTests(out, directory, format, total, tests);
	};

	auto runFalsePositiveTests = [&](const fs::path& directory, int total,
	                                 const std::vector<FalsePositiveTestCase>& tests) {
		if (hasTest(directory))
			doRunFalsePositiveTests(out, directory, total, tests);
	};

	try
	{
		auto startTime = std::chrono::steady_clock::now();
		// clang-format off
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
		// clang-format on

		auto duration = std::chrono::steady_clock::now() - startTime;

		std::cout << "Total time: " << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() << " ms." << std::endl;
	} catch (const std::exception& e) {
		std::cout << e.what() << std::endl;
	} catch (...) {
		std::cout << "Internal error" << std::endl;
	}
}
