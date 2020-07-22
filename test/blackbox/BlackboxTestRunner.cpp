/*
* Copyright 2016 Nu-book Inc.
* Copyright 2019 Axel Waggershauser
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

#include "BlackboxTestRunner.h"
#include "TextDecoder.h"
#include "TextUtfEncoding.h"
#include "DecodeHints.h"
#include "Result.h"
#include "MultiFormatReader.h"
#include "ImageLoader.h"
#include "BinaryBitmap.h"
#include "Pdf417MultipleCodeReader.h"
#include "QRCodeStructuredAppendReader.h"
#include "ZXContainerAlgorithms.h"

#include <chrono>
#include <exception>
#include <iostream>
#include <fstream>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace ZXing::Test {

namespace {

	struct PureTag {} pure;

	struct TestCase
	{
		struct TC
		{
			std::string name = {};
			size_t minPassCount = 0; // The number of images which must decode for the test to pass.
			size_t maxMisreads = 0; // Maximum number of successfully read images with the wrong contents.
			std::set<fs::path> notDetectedFiles = {};
			std::map<fs::path, std::string> misReadFiles = {};
		};

		TC tc[2] = {};
		int rotation = 0; // The rotation in degrees clockwise to use for this test.

		TestCase(size_t mntf, size_t mnts, size_t mmf, size_t mms, int r)
			: tc{{"fast", mntf, mmf}, {"slow", mnts, mms}}, rotation(r)
		{}
		TestCase(size_t mntf, size_t mnts, int r) : TestCase(mntf, mnts, 0, 0, r) {}
		TestCase(size_t mntp, size_t mmp, PureTag) : tc{{"pure", mntp, mmp}} {}
	};

	struct FalsePositiveTestCase
	{
		int maxAllowed; // Maximum number of images which can fail due to successfully reading the wrong contents
		int rotation;   // The rotation in degrees clockwise to use for this test.
	};
}

std::string metadataToUtf8(const Result& result)
{
	constexpr ResultMetadata::Key keys[] = {ResultMetadata::SUGGESTED_PRICE, ResultMetadata::ISSUE_NUMBER, ResultMetadata::UPC_EAN_EXTENSION};
	constexpr char const * prefixs[] = {"SUGGESTED_PRICE", "ISSUE_NUMBER", "UPC_EAN_EXTENSION"};
	static_assert(Size(keys) == Size(prefixs), "lut size mismatch");

	for (int i = 0; i < Size(keys); ++i) {
		auto res = TextUtfEncoding::ToUtf8(result.metadata().getString(keys[i]));
		if (res.size()) {
			res.insert(0, std::string(prefixs[i]) + "=");
			return res;
		}
	}

	return {};
}

static std::string checkResult(const fs::path& imgPath, const char* expectedFormat, const Result& result)
{
	using namespace std::literals;

	if (std::string format = ToString(result.format()); expectedFormat != format)
		return "Format mismatch: expected '"s + expectedFormat + "' but got '" + format + "'";

	auto readFile = [imgPath](const char* ending) {
		std::ifstream ifs(fs::path(imgPath).replace_extension(ending), std::ios::binary);
		return ifs ? std::optional(std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>())) : std::nullopt;
	};

	if (auto expected = readFile(".metadata.txt")) {
		auto metadata = metadataToUtf8(result);
		if (metadata != *expected)
			return "Metadata mismatch: expected '" + *expected + "' but got '" + metadata + "'";
	}

	if (auto expected = readFile(".txt")) {
		auto utf8Result = TextUtfEncoding::ToUtf8(result.text());
		return utf8Result != *expected ? "Content mismatch: expected '" + *expected + "' but got '" + utf8Result + "'" : "";
	}

	if (auto expected = readFile(".bin")) {
		std::string latin1Result(result.text().begin(), result.text().end());
		return latin1Result != *expected ? "Content mismatch: expected '" + *expected + "' but got '" + latin1Result + "'" : "";
	}

	return "Error reading file";
}

static int failed = 0;

static void printPositiveTestStats(size_t imageCount, const TestCase::TC& tc)
{
	size_t passCount = imageCount - tc.misReadFiles.size() - tc.notDetectedFiles.size();

	printf(", %s: %3d of %3d, misread: %d of %d", tc.name.c_str(), (int)passCount, (int)tc.minPassCount,
		(int)tc.misReadFiles.size(), (int)tc.maxMisreads);

	if (passCount < tc.minPassCount && !tc.notDetectedFiles.empty()) {
		std::cout << "\nFAILED: Not detected (" << tc.name << "):";
		for (const auto& f : tc.notDetectedFiles)
			std::cout << ' ' << f.filename();
		std::cout << "\n";
		++failed;
	}

	if (tc.misReadFiles.size() > tc.maxMisreads) {
		std::cout << "FAILED: Read error (" << tc.name << "):";
		for (const auto& [path, error] : tc.misReadFiles)
			std::cout << "      " << path.filename() << ": " << error << "\n";
		std::cout << "\n";
		++failed;
	}
}

static std::vector<fs::path> getImagesInDirectory(const fs::path& directory)
{
	std::vector<fs::path> result;
	for (const auto& entry : fs::directory_iterator(directory))
		if (fs::is_regular_file(entry.status()) &&
				Contains({".png", ".jpg", ".pgm", ".gif"}, entry.path().extension()))
			result.push_back(entry.path());
	return result;
}

static void doRunTests(
	const fs::path& directory, const char* format, size_t totalTests, const std::vector<TestCase>& tests, DecodeHints hints)
{
	ImageLoader::cache.clear();

	auto images = getImagesInDirectory(directory);
	auto folderName = directory.stem();

	if (images.size() != totalTests)
		std::cout << "TEST " << folderName << " => Expected number of tests: " << totalTests
			 << ", got: " << images.size() << " => FAILED!" << std::endl;

	for (auto& test : tests) {
		auto startTime = std::chrono::steady_clock::now();
		printf("%-20s @ %3d, total: %3d", folderName.string().c_str(), test.rotation, (int)images.size());
		for (auto tc : test.tc) {
			if (tc.name.empty())
				break;
			hints.setTryHarder(tc.name == "slow");
			hints.setTryRotate(tc.name == "slow");
			hints.setIsPure(tc.name == "pure");
			MultiFormatReader reader(hints);
			for (const auto& imgPath : images) {
				auto result = reader.read(*ImageLoader::load(imgPath).rotated(test.rotation));
				if (result.isValid()) {
					auto error = checkResult(imgPath, format, result);
					if (!error.empty())
						tc.misReadFiles[imgPath] = error;
				} else {
					tc.notDetectedFiles.insert(imgPath);
				}
			}

			printPositiveTestStats(images.size(), tc);
		}

		auto duration = std::chrono::steady_clock::now() - startTime;
		printf(", time: %4d ms", (int)std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
		std::cout << std::endl;
	}
}

template <typename ReaderT>
static void doRunStructuredAppendTest(
	const fs::path& directory, [[maybe_unused]]const char* format, size_t totalTests, const std::vector<TestCase>& tests)
{
	auto imgPaths = getImagesInDirectory(directory);
	auto folderName = directory.stem();

	std::map<fs::path, std::vector<fs::path>> imageGroups;
	for (const auto& imgPath : imgPaths) {
		std::string fn = imgPath.filename().string();
		auto p = fn.find_last_of('-');
		imageGroups[imgPath.parent_path() / fn.substr(0, p)].push_back(imgPath);
	}

	if (imageGroups.size() != (size_t)totalTests) {
		std::cout << "TEST " << folderName << " => Expected number of tests: " << totalTests
			<< ", got: " << imageGroups.size() << " => FAILED!" << std::endl;
	}

	for (auto& test : tests) {
		printf("%-20s @ %3d, total: %3d", folderName.string().c_str(), test.rotation, (int)imgPaths.size());
		auto tc = test.tc[0];

		for (const auto& [testPath, testImgPaths] : imageGroups) {
			auto result = ReaderT::readMultiple(testImgPaths, test.rotation);
			if (result.isValid()) {
				auto error = checkResult(testPath, format, result);
				if (!error.empty())
					tc.misReadFiles[testPath] = error;
			}
			else {
				tc.notDetectedFiles.insert(testPath);
			}
		}

		printPositiveTestStats(imageGroups.size(), tc);

		std::cout << std::endl;
	}
}

int runBlackBoxTests(const fs::path& testPathPrefix, const std::set<std::string>& includedTests)
{
	auto hasTest = [&includedTests](const fs::path& dir) {
		auto stem = dir.stem().string();
		return includedTests.empty() || Contains(includedTests, stem) ||
				Contains(includedTests, stem.substr(0, stem.size() - 2));
	};

	auto runTests = [&](std::string_view directory, const char* format, size_t total,
					    const std::vector<TestCase>& tests, const DecodeHints& hints = DecodeHints()) {
		if (hasTest(directory))
			doRunTests(testPathPrefix / directory, format, total, tests, hints);
	};

	auto runPdf417StructuredAppendTest = [&](std::string_view directory, const char* format, size_t total, const std::vector<TestCase>& tests) {
		if (hasTest(directory))
			doRunStructuredAppendTest<Pdf417MultipleCodeReader>(testPathPrefix / directory, format, total, tests);
	};

	auto runQRCodeStructuredAppendTest = [&](std::string_view directory, const char* format, size_t total, const std::vector<TestCase>& tests) {
		if (hasTest(directory))
			doRunStructuredAppendTest<QRCodeStructuredAppendReader>(testPathPrefix / directory, format, total, tests);
	};

	try
	{
		auto startTime = std::chrono::steady_clock::now();

		// clang-format off
		runTests("aztec-1", "AZTEC", 13, {
			{ 13, 13, 0   },
			{ 13, 13, 90  },
			{ 13, 13, 180 },
			{ 13, 13, 270 },
		});

		runTests("aztec-2", "AZTEC", 22, {
			{ 5, 5, 0   },
			{ 4, 4, 90  },
			{ 6, 6, 180 },
			{ 3, 3, 270 },
		});

		runTests("datamatrix-1", "DATA_MATRIX", 21, {
			{ 21, 21, 0   },
			{  0, 21, 90  },
			{  0, 21, 180 },
			{  0, 21, 270 },
			{ 19, 0, pure },
		});

		runTests("datamatrix-2", "DATA_MATRIX", 18, {
			{ 18, 18, 0   },
			{  0, 18, 90  },
			{  0, 18, 180 },
			{  0, 18, 270 },
		});

		runTests("datamatrix-3", "DATA_MATRIX", 19, {
			{ 18, 19, 0   },
			{  0, 19, 90  },
			{  0, 18, 180 }, // 1 fail because of a different binarizer output
			{  0, 19, 270 },
		});

		runTests("codabar-1", "CODABAR", 11, {
#ifdef ZX_USE_NEW_ROW_READERS
			{ 11, 11, 0   },
			{ 11, 11, 180 },
#else
			{ 10, 10, 0   },
			{ 10, 10, 180 },
#endif
		});

		runTests("codabar-2", "CODABAR", 4, {
#ifdef ZX_USE_NEW_ROW_READERS
			{ 3, 3, 0   },
			{ 3, 3, 180 },
#else
			{ 2, 2, 0   },
			{ 2, 2, 180 },
#endif
		});

		runTests("code39-1", "CODE_39", 4, {
			{ 4, 4, 0   },
			{ 4, 4, 180 },
		});

		runTests("code39-2", "CODE_39", 2, {
			{ 2, 2, 0   },
			{ 2, 2, 180 },
		}, DecodeHints().setTryCode39ExtendedMode(true).setFormats(BarcodeFormat::CODE_39));

		runTests("code39-3", "CODE_39", 17, {
			{ 17, 17, 0   },
			{ 17, 17, 180 },
		});

		runTests("code93-1", "CODE_93", 3, {
			{ 3, 3, 0   },
			{ 3, 3, 180 },
		});

		runTests("code128-1", "CODE_128", 6, {
			{ 6, 6, 0   },
			{ 6, 6, 180 },
		});

		runTests("code128-2", "CODE_128", 40, {
			{ 36, 39, 0   },
			{ 36, 39, 180 },
		});

		runTests("code128-3", "CODE_128", 2, {
			{ 2, 2, 0   },
			{ 2, 2, 180 },
		});

		runTests("ean8-1", "EAN_8", 8, {
			{ 3, 3, 0   },
			{ 3, 3, 180 },
		});

		runTests("ean13-1", "EAN_13", 31, {
			{ 26, 29, 0   },
			{ 23, 29, 180 },
		});

		runTests("ean13-2", "EAN_13", 28, {
			{ 11, 17, 0   },
			{ 11, 17, 180 },
		});

		runTests("ean13-3", "EAN_13", 55, {
			{ 53, 55, 0   },
			{ 55, 55, 180 },
		});

		runTests("ean13-4", "EAN_13", 22, {
			{ 7, 14, 0   },
			{ 8, 14, 180 },
		});

		runTests("ean13-5", "EAN_13", 18, {
			{ 0, 0, 0   },
			{ 0, 0, 180 },
		});

		runTests("ean13-extension-1", "EAN_13", 5, {
			{ 4, 5, 0 },
			{ 3, 5, 180 },
		}, DecodeHints().setAllowedEanExtensions({2,5}));

		runTests("itf-1", "ITF", 14, {
			{ 14, 14, 0   },
			{ 14, 14, 180 },
		});

		runTests("itf-2", "ITF", 13, {
			{ 13, 13, 0   },
			{ 13, 13, 180 },
		});

		runTests("maxicode-1", "MAXICODE", 6, {
			{ 1, 1, 5, 5, 0 },
		});

		runTests("upca-1", "UPC_A", 15, {
			{ 10, 12, 0, 1, 0   },
			{ 12, 12, 0, 1, 180 },
		});

		runTests("upca-2", "UPC_A", 52, {
			{ 27, 35, 0   },
			{ 29, 35, 180 },
		});

		runTests("upca-3", "UPC_A", 21, {
			{ 7, 10, 0, 1, 0   },
			{ 8, 10, 0, 1, 180 },
		});

		runTests("upca-4", "UPC_A", 19, {
			{ 9, 11, 0, 1, 0   },
			{ 9, 11, 0, 1, 180 },
		});

		runTests("upca-5", "UPC_A", 35, {
			{ 20, 23, 0, 0, 0   },
			{ 22, 23, 0, 0, 180 },
		});
		
		runTests("upca-6", "UPC_A", 19, {
			{ 0, 0, 0   },
			{ 0, 0, 180 },
		});

		runTests("upca-extension-1", "UPC_A", 6, {
			{ 3, 6, 0 },
			{ 4, 6, 180 },
		}, DecodeHints().setAllowedEanExtensions({2,5}));

		runTests("upce-1", "UPC_E", 3, {
			{ 3, 3, 0   },
			{ 3, 3, 180 },
		});

		runTests("upce-2", "UPC_E", 41, {
			{ 30, 34, 0, 1, 0   },
			{ 30, 34, 1, 1, 180 },
		});

		runTests("upce-3", "UPC_E", 11, {
			{ 6, 8, 0   },
			{ 6, 8, 180 },
		});

		runTests("rss14-1", "RSS_14", 6, {
			{ 6, 6, 0   },
			{ 6, 6, 180 },
		});

		runTests("rss14-2", "RSS_14", 16, {
			{ 7, 10, 1, 1, 0   },
			{ 8, 10, 0, 1, 180 },
		});

		runTests("rssexpanded-1", "RSS_EXPANDED", 32, {
			{ 32, 32, 0   },
			{ 32, 32, 180 },
		});

		runTests("rssexpanded-2", "RSS_EXPANDED", 23, {
			{ 21, 23, 0   },
			{ 21, 23, 180 },
		});

		runTests("rssexpanded-3", "RSS_EXPANDED", 117, {
			{ 117, 117, 0   },
			{ 117, 117, 180 },
		});

		runTests("rssexpandedstacked-1", "RSS_EXPANDED", 64, {
			{ 59, 64, 0   },
			{ 59, 64, 180 },
		});

		runTests("rssexpandedstacked-2", "RSS_EXPANDED", 7, {
			{ 2, 7, 0   },
			{ 2, 7, 180 },
		});

		runTests("qrcode-1", "QR_CODE", 20, {
			{ 17, 17, 0   },
			{ 14, 14, 90  },
			{ 17, 17, 180 },
			{ 14, 14, 270 },
		});

		runTests("qrcode-2", "QR_CODE", 34, {
			{ 32, 32, 0   },
			{ 31, 31, 90  },
			{ 31, 31, 180 },
			{ 31, 31, 270 },
			{ 9, 0, pure },
		});

		runTests("qrcode-3", "QR_CODE", 42, {
			{ 38, 38, 0   },
			{ 39, 39, 90  },
			{ 38, 38, 180 },
			{ 38, 38, 270 },
		});

		runTests("qrcode-4", "QR_CODE", 48, {
			{ 36, 36, 0   },
			{ 36, 36, 90  },
			{ 36, 36, 180 },
			{ 36, 36, 270 },
		});

		runTests("qrcode-5", "QR_CODE", 19, {
			{ 19, 19, 0   },
			{ 19, 19, 90  },
			{ 19, 19, 180 },
			{ 19, 19, 270 },
			{ 4, 0, pure },
		});

		runTests("qrcode-6", "QR_CODE", 15, {
			{ 15, 15, 0   },
			{ 15, 15, 90  },
			{ 15, 15, 180 },
			{ 15, 15, 270 },
		});

		runQRCodeStructuredAppendTest("qrcode-7", "QR_CODE", 1, {
			{ 1, 1, 0   },
		});

		runTests("pdf417-1", "PDF_417", 10, {
			{ 10, 10, 0   },
			{ 10, 10, 180 },
		});

		runTests("pdf417-2", "PDF_417", 25, {
			{ 25, 25, 0   },
			{ 25, 25, 180 },
		});

		runTests("pdf417-3", "PDF_417", 16, {
			{ 16, 16, 0   },
			{ 16, 16, 180 },
		});

		runPdf417StructuredAppendTest("pdf417-4", "PDF_417", 2, {
			{ 2, 2, 0   },
		});

		runTests("falsepositives-1", "NONE", 24, {
			{ 0, 0, 0, 0, 0   },
			{ 0, 0, 0, 0, 90  },
			{ 0, 0, 0, 0, 180 },
			{ 0, 0, 0, 0, 270 },
		});

		runTests("falsepositives-2", "NONE", 25, {
			{ 0, 0, 0, 2, 0   },
			{ 0, 0, 0, 2, 90  },
			{ 0, 0, 0, 2, 180 },
			{ 0, 0, 0, 2, 270 },
		});
		// clang-format on

		auto duration = std::chrono::steady_clock::now() - startTime;
		std::cout << "Total time: " << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() << " ms." << std::endl;
		if (failed)
			std::cout << "WARNING: " << failed << " tests failed." << std::endl;

		return failed;
	}
	catch (const std::exception& e) {
		std::cout << e.what() << std::endl;
	}
	catch (...) {
		std::cout << "Internal error" << std::endl;
	}
	return -1;
}

} // ZXing::Test
