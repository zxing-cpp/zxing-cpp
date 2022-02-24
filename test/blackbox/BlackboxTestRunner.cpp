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

#include "ImageLoader.h"
#include "ReadBarcode.h"
#include "TextUtfEncoding.h"
#include "ThresholdBinarizer.h"
#include "ZXContainerAlgorithms.h"
#include "pdf417/PDFReader.h"
#include "qrcode/QRReader.h"

#include <fmt/core.h>
#include <fmt/ostream.h>

#include <chrono>
#include <exception>
#include <fstream>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace ZXing::Test {

namespace {

	struct PureTag {} pure;

	struct TestCase
	{
		struct TC
		{
			std::string name = {};
			int minPassCount = 0; // The number of images which must decode for the test to pass.
			int maxMisreads = 0; // Maximum number of successfully read images with the wrong contents.
			std::set<fs::path> notDetectedFiles = {};
			std::map<fs::path, std::string> misReadFiles = {};
		};

		TC tc[2] = {};
		int rotation = 0; // The rotation in degrees clockwise to use for this test.

		TestCase(int mntf, int mnts, int mmf, int mms, int r)
			: tc{{"fast", mntf, mmf}, {"slow", mnts, mms}}, rotation(r)
		{}
		TestCase(int mntf, int mnts, int r) : TestCase(mntf, mnts, 0, 0, r) {}
		TestCase(int mntp, int mmp, PureTag) : tc{{"pure", mntp, mmp}} {}
	};

	struct FalsePositiveTestCase
	{
		int maxAllowed; // Maximum number of images which can fail due to successfully reading the wrong contents
		int rotation;   // The rotation in degrees clockwise to use for this test.
	};
}

// Helper for `compareResult()` - map `key` to Result property, converting value to std::string
static std::string getResultValue(const Result& result, const std::string& key)
{
	if (key == "ecLevel")
		return TextUtfEncoding::ToUtf8(result.ecLevel());
	if (key == "orientation")
		return std::to_string(result.orientation());
	if (key == "numBits")
		return std::to_string(result.numBits());
	if (key == "symbologyIdentifier")
		return result.symbologyIdentifier();
	if (key == "sequenceSize")
		return std::to_string(result.sequenceSize());
	if (key == "sequenceIndex")
		return std::to_string(result.sequenceIndex());
	if (key == "sequenceId")
		return result.sequenceId();
	if (key == "isLastInSequence")
		return result.isLastInSequence() ? "true" : "false";
	if (key == "isPartOfSequence")
		return result.isPartOfSequence() ? "true" : "false";
	if (key == "readerInit")
		return result.readerInit() ? "true" : "false";

	return fmt::format("***Unknown key '{}'***", key);
}

// Read ".result.txt" file contents `expected` with lines "key=value" and compare to `actual`
static bool compareResult(const Result& result, const std::string& expected, std::string& actual)
{
	bool ret = true;

	actual.clear();
	actual.reserve(expected.size());

	std::stringstream expectedLines(expected);
	std::string expectedLine;
	while (std::getline(expectedLines, expectedLine)) {
		if (expectedLine.empty() || expectedLine[0] == '#')
			continue;
		auto equals = expectedLine.find('=');
		if (equals == std::string::npos) {
			actual += "***Bad format, missing equals***\n";
			return false;
		}
		std::string key = expectedLine.substr(0, equals);
		std::string expectedValue = expectedLine.substr(equals + 1);
		std::string actualValue = getResultValue(result, key);
		if (actualValue != expectedValue) {
			ret = false;
			actualValue += " ***Mismatch***";
		}
		actual += key + '=' + actualValue + '\n';
	}
	return ret;
}

static std::string checkResult(const fs::path& imgPath, std::string_view expectedFormat, const Result& result)
{
	if (auto format = ToString(result.format()); expectedFormat != format)
		return fmt::format("Format mismatch: expected '{}' but got '{}'", expectedFormat, format);

	auto readFile = [imgPath](const char* ending) {
		std::ifstream ifs(fs::path(imgPath).replace_extension(ending), std::ios::binary);
		return ifs ? std::optional(std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>())) : std::nullopt;
	};

	if (auto expected = readFile(".result.txt")) {
		std::string actual;
		if (!compareResult(result, *expected, actual))
			return fmt::format("Result mismatch: expected\n{} but got\n{}", *expected, actual);
	}

	if (auto expected = readFile(".txt")) {
		auto utf8Result = TextUtfEncoding::ToUtf8(result.text());
		return utf8Result != *expected ? fmt::format("Content mismatch: expected '{}' but got '{}'", *expected, utf8Result) : "";
	}

	if (auto expected = readFile(".bin")) {
		std::string latin1Result(result.text().length(), '\0');
		std::transform(result.text().begin(), result.text().end(), latin1Result.begin(), [](wchar_t c) { return static_cast<char>(c); });
		return latin1Result != *expected ? fmt::format("Content mismatch: expected '{}' but got '{}'", *expected, latin1Result) : "";
	}

	return "Error reading file";
}

static int failed = 0;
static int totalImageLoadTime = 0;

int timeSince(std::chrono::steady_clock::time_point startTime)
{
	auto duration = std::chrono::steady_clock::now() - startTime;
	return static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
}

// pre-load images into cache, so the disc io time does not end up in the timing measurement
void preloadImageCache(const std::vector<fs::path>& imgPaths)
{
	auto startTime = std::chrono::steady_clock::now();
	ImageLoader::clearCache();
	for (const auto& imgPath : imgPaths)
		ImageLoader::load(imgPath);
	totalImageLoadTime += timeSince(startTime);
}

static void printPositiveTestStats(int imageCount, const TestCase::TC& tc)
{
	int passCount = imageCount - Size(tc.misReadFiles) - Size(tc.notDetectedFiles);

	fmt::print(" | {}: {:3} of {:3}, misread {} of {}", tc.name, passCount, tc.minPassCount, Size(tc.misReadFiles),
			   tc.maxMisreads);

	if (passCount < tc.minPassCount && !tc.notDetectedFiles.empty()) {
		fmt::print("\nFAILED: Not detected ({}):", tc.name);
		for (const auto& f : tc.notDetectedFiles)
			fmt::print(" {}", f.filename().string());
		fmt::print("\n");
		++failed;
	}

	if (Size(tc.misReadFiles) > tc.maxMisreads) {
		fmt::print("\nFAILED: Read error ({}):", tc.name);
		for (const auto& [path, error] : tc.misReadFiles)
			fmt::print("      {}: {}\n", path.filename().string(), error);
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

	preloadImageCache(result);

	return result;
}

static void doRunTests(
	const fs::path& directory, std::string_view format, int totalTests, const std::vector<TestCase>& tests, DecodeHints hints)
{
	auto imgPaths = getImagesInDirectory(directory);
	auto folderName = directory.stem();

	if (Size(imgPaths) != totalTests)
		fmt::print("TEST {} => Expected number of tests: {}, got: {} => FAILED\n", folderName, totalTests,
				   imgPaths.size());

	for (auto& test : tests) {
		fmt::print("{:20} @ {:3}, {:3}", folderName.string(), test.rotation, Size(imgPaths));
		std::vector<int> times;
		for (auto tc : test.tc) {
			if (tc.name.empty())
				break;
			auto startTime = std::chrono::steady_clock::now();
			hints.setTryHarder(tc.name == "slow");
			hints.setTryRotate(tc.name == "slow");
			hints.setIsPure(tc.name == "pure");
			if (hints.isPure())
				hints.setBinarizer(Binarizer::FixedThreshold);
			for (const auto& imgPath : imgPaths) {
				auto result = ReadBarcode(ImageLoader::load(imgPath).rotated(test.rotation), hints);
				if (result.isValid()) {
					auto error = checkResult(imgPath, format, result);
					if (!error.empty())
						tc.misReadFiles[imgPath] = error;
				} else {
					tc.notDetectedFiles.insert(imgPath);
				}
			}

			times.push_back(timeSince(startTime));
			printPositiveTestStats(Size(imgPaths), tc);
		}
		fmt::print(" | time: {:3} vs {:3} ms\n", times.front(), times.back());
	}
}

static Result readMultiple(const std::vector<fs::path>& imgPaths, std::string_view format)
{
	std::list<Result> allResults;
	for (const auto& imgPath : imgPaths) {
		auto results =
			ReadBarcodes(ImageLoader::load(imgPath), DecodeHints().setFormats(BarcodeFormatFromString(format.data())));
		allResults.insert(allResults.end(), results.begin(), results.end());
	}

	if (allResults.empty())
		return Result(DecodeStatus::NotFound);

	allResults.sort([](const Result& r1, const Result& r2) { return r1.sequenceIndex() < r2.sequenceIndex(); });

	if (allResults.back().sequenceSize() != Size(allResults) ||
		!std::all_of(allResults.begin(), allResults.end(),
					 [&](Result& it) { return it.sequenceId() == allResults.front().sequenceId(); }))
		return Result(DecodeStatus::FormatError);

	std::wstring text;
	for (const auto& r : allResults)
		text.append(r.text());

	const auto& first = allResults.front();
	StructuredAppendInfo sai{ first.sequenceIndex(), first.sequenceSize(), first.sequenceId() };
	return Result(std::move(text), {}, first.format(), {}, first.symbologyIdentifier(), sai, first.readerInit());
}

static void doRunStructuredAppendTest(
	const fs::path& directory, std::string_view format, int totalTests, const std::vector<TestCase>& tests)
{
	auto imgPaths = getImagesInDirectory(directory);
	auto folderName = directory.stem();

	std::map<fs::path, std::vector<fs::path>> imageGroups;
	for (const auto& imgPath : imgPaths) {
		std::string fn = imgPath.filename().string();
		auto p = fn.find_last_of('-');
		imageGroups[imgPath.parent_path() / fn.substr(0, p)].push_back(imgPath);
	}

	if (Size(imageGroups) != totalTests)
		fmt::print("TEST {} => Expected number of tests: {}, got: {} => FAILED\n", folderName, totalTests,
				   imageGroups.size());

	for (auto& test : tests) {
		fmt::print("{:20} @ {:3}, {:3}", folderName.string(), test.rotation, Size(imgPaths));
		auto tc = test.tc[0];
		auto startTime = std::chrono::steady_clock::now();

		for (const auto& [testPath, testImgPaths] : imageGroups) {
			auto result = readMultiple(testImgPaths, format);
			if (result.isValid()) {
				auto error = checkResult(testPath, format, result);
				if (!error.empty())
					tc.misReadFiles[testPath] = error;
			} else {
				tc.notDetectedFiles.insert(testPath);
			}
		}

		printPositiveTestStats(Size(imageGroups), tc);
		fmt::print(" | time: {:3} ms\n", timeSince(startTime));
	}
}

int runBlackBoxTests(const fs::path& testPathPrefix, const std::set<std::string>& includedTests)
{
	auto hasTest = [&includedTests](const fs::path& dir) {
		auto stem = dir.stem().string();
		return includedTests.empty() || Contains(includedTests, stem) ||
				Contains(includedTests, stem.substr(0, stem.size() - 2));
	};

	auto runTests = [&](std::string_view directory, std::string_view format, int total,
						const std::vector<TestCase>& tests, const DecodeHints& hints = DecodeHints()) {
		if (hasTest(directory))
			doRunTests(testPathPrefix / directory, format, total, tests, hints);
	};

	auto runStructuredAppendTest = [&](std::string_view directory, std::string_view format, int total, const std::vector<TestCase>& tests) {
		if (hasTest(directory))
			doRunStructuredAppendTest(testPathPrefix / directory, format, total, tests);
	};

	try
	{
		auto startTime = std::chrono::steady_clock::now();

		// clang-format off
		runTests("aztec-1", "Aztec", 22, {
			{ 21, 21, 0   },
			{ 21, 21, 90  },
			{ 21, 21, 180 },
			{ 21, 21, 270 },
			{ 22, 0, pure },
		});

		runTests("aztec-2", "Aztec", 22, {
			{ 5, 5, 0   },
			{ 4, 4, 90  },
			{ 6, 6, 180 },
			{ 3, 3, 270 },
		});

		runTests("datamatrix-1", "DataMatrix", 26, {
			{ 26, 26, 0   },
			{  0, 26, 90  },
			{  0, 26, 180 },
			{  0, 26, 270 },
			{ 25, 0, pure },
		});

		runTests("datamatrix-2", "DataMatrix", 13, {
			{ 13, 13, 0   },
			{  0, 13, 90  },
			{  0, 13, 180 },
			{  0, 13, 270 },
		});

		runTests("datamatrix-3", "DataMatrix", 19, {
			{ 18, 19, 0   },
			{  0, 19, 90  },
			{  0, 19, 180 },
			{  0, 19, 270 },
		});

		runTests("datamatrix-4", "DataMatrix", 21, {
			{ 21, 21, 0   },
			{  0, 21, 90  },
			{  0, 21, 180 },
			{  0, 21, 270 },
			{ 19, 0, pure },
		});

		runTests("codabar-1", "Codabar", 11, {
			{ 11, 11, 0   },
			{ 11, 11, 180 },
		});

		runTests("codabar-2", "Codabar", 4, {
			{ 3, 3, 0   },
			{ 3, 3, 180 },
		});

		runTests("code39-1", "Code39", 4, {
			{ 4, 4, 0   },
			{ 4, 4, 180 },
		});

		runTests("code39-2", "Code39", 2, {
			{ 2, 2, 0   },
			{ 2, 2, 180 },
		}, DecodeHints().setTryCode39ExtendedMode(true));

		runTests("code39-3", "Code39", 12, {
			{ 12, 12, 0   },
			{ 12, 12, 180 },
		});

		runTests("code93-1", "Code93", 3, {
			{ 3, 3, 0   },
			{ 3, 3, 180 },
		});

		runTests("code128-1", "Code128", 6, {
			{ 6, 6, 0   },
			{ 6, 6, 180 },
		});

		runTests("code128-2", "Code128", 21, {
			{ 19, 21, 0   },
			{ 19, 21, 180 },
		});

		runTests("code128-3", "Code128", 2, {
			{ 2, 2, 0   },
			{ 2, 2, 180 },
		});

		runTests("ean8-1", "EAN-8", 8, {
			{ 8, 8, 0   },
			{ 8, 8, 180 },
		});

		runTests("ean13-1", "EAN-13", 31, {
			{ 26, 29, 0   },
			{ 23, 29, 180 },
		});

		runTests("ean13-2", "EAN-13", 24, {
			{ 7, 14, 0   },
			{ 7, 14, 180 },
		});

		runTests("ean13-3", "EAN-13", 21, {
			{ 20, 21, 0   },
			{ 21, 21, 180 },
		});

		runTests("ean13-4", "EAN-13", 22, {
			{ 7, 14, 0   },
			{ 8, 14, 180 },
		});

		runTests("ean13-extension-1", "EAN-13", 5, {
			{ 4, 5, 0 },
			{ 3, 5, 180 },
		}, DecodeHints().setEanAddOnSymbol(EanAddOnSymbol::Require));

		runTests("itf-1", "ITF", 11, {
			{ 10, 11, 0   },
			{ 10, 11, 180 },
		});

		runTests("itf-2", "ITF", 6, {
			{ 6, 6, 0   },
			{ 6, 6, 180 },
		});

		runTests("maxicode-1", "MaxiCode", 9, {
			{ 9, 9, 0 },
		});

		runTests("maxicode-2", "MaxiCode", 4, {
			{ 0, 0, 0 },
		});

		runTests("upca-1", "UPC-A", 12, {
			{  9, 12, 0, 1, 0   },
			{ 11, 12, 0, 1, 180 },
		});

		runTests("upca-2", "UPC-A", 36, {
			{ 17, 23, 0   },
			{ 18, 23, 180 },
		});

		runTests("upca-3", "UPC-A", 21, {
			{ 7, 10, 0, 1, 0   },
			{ 8, 10, 0, 1, 180 },
		});

		runTests("upca-4", "UPC-A", 19, {
			{ 9, 11, 0, 1, 0   },
			{ 9, 11, 0, 1, 180 },
		});

		runTests("upca-5", "UPC-A", 32, {
			{ 17, 20, 0   },
			{ 19, 20, 180 },
		});
		
		runTests("upca-extension-1", "UPC-A", 6, {
			{ 4, 6, 0 },
			{ 4, 6, 180 },
		}, DecodeHints().setEanAddOnSymbol(EanAddOnSymbol::Require));

		runTests("upce-1", "UPC-E", 3, {
			{ 3, 3, 0   },
			{ 3, 3, 180 },
		});

		runTests("upce-2", "UPC-E", 28, {
			{ 19, 22, 0, 1, 0   },
			{ 20, 22, 1, 1, 180 },
		});

		runTests("upce-3", "UPC-E", 11, {
			{ 6, 8, 0   },
			{ 6, 8, 180 },
		});

		runTests("rss14-1", "DataBar", 6, {
			{ 6, 6, 0   },
			{ 6, 6, 180 },
		});

		runTests("rss14-2", "DataBar", 16, {
			{ 8 , 10, 0   },
			{ 10, 10, 180 },
		});

		runTests("rssexpanded-1", "DataBarExpanded", 33, {
			{ 33, 33, 0   },
			{ 33, 33, 180 },
		});

		runTests("rssexpanded-2", "DataBarExpanded", 15, {
			{ 13, 15, 0   },
			{ 13, 15, 180 },
		});

		runTests("rssexpanded-3", "DataBarExpanded", 118, {
			{ 118, 118, 0   },
			{ 118, 118, 180 },
		});

		runTests("rssexpandedstacked-1", "DataBarExpanded", 65, {
			{ 60, 65, 0   },
			{ 60, 65, 180 },
		});

		runTests("rssexpandedstacked-2", "DataBarExpanded", 7, {
			{ 2, 7, 0   },
			{ 2, 7, 180 },
		});

		runTests("qrcode-1", "QRCode", 16, {
			{ 16, 16, 0   },
			{ 16, 16, 90  },
			{ 16, 16, 180 },
			{ 16, 16, 270 },
		});

		runTests("qrcode-2", "QRCode", 43, {
			{ 41, 41, 0   },
			{ 41, 41, 90  },
			{ 41, 41, 180 },
			{ 41, 41, 270 },
			{ 21, 1, pure }, // the misread is the 'outer' symbol in 16.png
		});

		runTests("qrcode-3", "QRCode", 28, {
			{ 25, 25, 0   },
			{ 25, 25, 90  },
			{ 25, 25, 180 },
			{ 24, 24, 270 },
		});

		runTests("qrcode-4", "QRCode", 41, {
			{ 29, 29, 0   },
			{ 29, 29, 90  },
			{ 29, 29, 180 },
			{ 29, 29, 270 },
		});

		runTests("qrcode-5", "QRCode", 16, {
			{ 16, 16, 0   },
			{ 16, 16, 90  },
			{ 16, 16, 180 },
			{ 16, 16, 270 },
			{ 4, 0, pure },
		});

		runTests("qrcode-6", "QRCode", 15, {
			{ 15, 15, 0   },
			{ 15, 15, 90  },
			{ 15, 15, 180 },
			{ 15, 15, 270 },
		});

		runStructuredAppendTest("qrcode-7", "QRCode", 1, {
			{ 1, 1, 0   },
		});

		runTests("pdf417-1", "PDF417", 17, {
			{ 16, 16, 0   },
			{  1,  1, 90  },
			{ 16, 16, 180 },
			{  1,  1, 270 },
			{ 17, 0, pure },
		});

		runTests("pdf417-2", "PDF417", 25, {
			{ 25, 25, 0   },
			{ 25, 25, 180 },
		});

		runTests("pdf417-3", "PDF417", 16, {
			{ 16, 16, 0   },
			{ 16, 16, 180 },
			{ 7, 0, pure },
		});

		runStructuredAppendTest("pdf417-4", "PDF417", 2, {
			{ 2, 2, 0   },
		});

		runTests("falsepositives-1", "None", 25, {
			{ 0, 0, 0, 0, 0   },
			{ 0, 0, 0, 0, 90  },
			{ 0, 0, 0, 0, 180 },
			{ 0, 0, 0, 0, 270 },
			{ 0, 0, pure },
		});

		runTests("falsepositives-2", "None", 25, {
			{ 0, 0, 0, 0, 0   },
			{ 0, 0, 0, 0, 90  },
			{ 0, 0, 0, 0, 180 },
			{ 0, 0, 0, 0, 270 },
			{ 0, 0, pure },
		});
		// clang-format on

		int totalTime = timeSince(startTime);
		int decodeTime = totalTime - totalImageLoadTime;
		fmt::print("load time:   {} ms.\n", totalImageLoadTime);
		fmt::print("decode time: {} ms.\n", decodeTime);
		fmt::print("total time:  {} ms.\n", totalTime);
		if (failed)
			fmt::print("WARNING: {} tests failed.\n", failed);

		return failed;
	}
	catch (const std::exception& e) {
		fmt::print("{}\n", e.what());
	}
	catch (...) {
		fmt::print("Internal error\n");
	}
	return -1;
}

} // ZXing::Test
