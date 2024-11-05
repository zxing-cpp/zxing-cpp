/*
* Copyright 2016 Nu-book Inc.
* Copyright 2019 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "BlackboxTestRunner.h"

#include "ImageLoader.h"
#include "ReadBarcode.h"
#include "Utf.h"
#include "ZXAlgorithms.h"

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

		TestCase(int mntf, int mnts, int mmf, int mms, int r) : tc{{"fast", mntf, mmf}, {"slow", mnts, mms}}, rotation(r) {}
		TestCase(int mntf, int mnts, int r) : TestCase(mntf, mnts, 0, 0, r) {}
		TestCase(int mntp, int mmp, PureTag) : tc{{"pure", mntp, mmp}} {}
	};

	struct FalsePositiveTestCase
	{
		int maxAllowed; // Maximum number of images which can fail due to successfully reading the wrong contents
		int rotation;   // The rotation in degrees clockwise to use for this test.
	};
}

// Helper for `compareResult()` - map `key` to Barcode property, converting value to std::string
static std::string getBarcodeValue(const Barcode& barcode, const std::string& key)
{
	if (key == "contentType")
		return ToString(barcode.contentType());
	if (key == "ecLevel")
		return barcode.ecLevel();
	if (key == "orientation")
		return std::to_string(barcode.orientation());
	if (key == "symbologyIdentifier")
		return barcode.symbologyIdentifier();
	if (key == "sequenceSize")
		return std::to_string(barcode.sequenceSize());
	if (key == "sequenceIndex")
		return std::to_string(barcode.sequenceIndex());
	if (key == "sequenceId")
		return barcode.sequenceId();
	if (key == "isLastInSequence")
		return barcode.isLastInSequence() ? "true" : "false";
	if (key == "isPartOfSequence")
		return barcode.isPartOfSequence() ? "true" : "false";
	if (key == "isMirrored")
		return barcode.isMirrored() ? "true" : "false";
	if (key == "isInverted")
		return barcode.isInverted() ? "true" : "false";
	if (key == "readerInit")
		return barcode.readerInit() ? "true" : "false";

	return fmt::format("***Unknown key '{}'***", key);
}

// Read ".result.txt" file contents `expected` with lines "key=value" and compare to `actual`
static bool compareResult(const Barcode& barcode, const std::string& expected, std::string& actual)
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
		std::string actualValue = getBarcodeValue(barcode, key);
		if (actualValue != expectedValue) {
			ret = false;
			actualValue += " ***Mismatch***";
		}
		actual += key + '=' + actualValue + '\n';
	}
	return ret;
}

static std::string checkResult(const fs::path& imgPath, std::string_view expectedFormat, const Barcode& barcode)
{
	if (auto format = ToString(barcode.format()); expectedFormat != format)
		return fmt::format("Format mismatch: expected '{}' but got '{}'", expectedFormat, format);

	auto readFile = [imgPath](const char* ending) {
		std::ifstream ifs(fs::path(imgPath).replace_extension(ending), std::ios::binary);
		return ifs ? std::optional(std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>())) : std::nullopt;
	};

	if (auto expected = readFile(".result.txt")) {
		std::string actual;
		if (!compareResult(barcode, *expected, actual))
			return fmt::format("Result mismatch: expected\n{} but got\n{}", *expected, actual);
	}

	if (auto expected = readFile(".txt")) {
		expected = EscapeNonGraphical(*expected);
		auto utf8Result = barcode.text(TextMode::Escaped);
		return utf8Result != *expected ? fmt::format("Content mismatch: expected '{}' but got '{}'", *expected, utf8Result) : "";
	}

	if (auto expected = readFile(".bin")) {
		ByteArray binaryExpected(*expected);
		return barcode.bytes() != binaryExpected
				   ? fmt::format("Content mismatch: expected '{}' but got '{}'", ToHex(binaryExpected), ToHex(barcode.bytes()))
				   : "";
	}

	return "Error reading file";
}

static int failed = 0;
static int extra = 0;
static int totalImageLoadTime = 0;

int timeSince(std::chrono::steady_clock::time_point startTime)
{
	auto duration = std::chrono::steady_clock::now() - startTime;
	return narrow_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
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

static std::string printPositiveTestStats(int imageCount, const TestCase::TC& tc)
{
	int passCount = imageCount - Size(tc.misReadFiles) - Size(tc.notDetectedFiles);

	fmt::print(" | {}: {:3} of {:3}, misread {} of {}", tc.name, passCount, tc.minPassCount, Size(tc.misReadFiles), tc.maxMisreads);

	std::string failures;
	if (passCount < tc.minPassCount && !tc.notDetectedFiles.empty()) {
		failures += fmt::format("    Not detected ({}):", tc.name);
		for (const auto& f : tc.notDetectedFiles)
			failures += fmt::format(" {}", f.filename().string());
		failures += "\n";
		failed += tc.minPassCount - passCount;
	}

	extra += std::max(0, passCount - tc.minPassCount);
	if (passCount > tc.minPassCount)
		failures += fmt::format("    Unexpected detections ({}): {}\n", tc.name, passCount - tc.minPassCount);

	if (Size(tc.misReadFiles) > tc.maxMisreads) {
		failures += fmt::format("    Read error ({}):", tc.name);
		for (const auto& [path, error] : tc.misReadFiles)
			failures += fmt::format("      {}: {}\n", path.filename().string(), error);
		failed += Size(tc.misReadFiles) - tc.maxMisreads;
	}
	return failures;
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

static void doRunTests(const fs::path& directory, std::string_view format, int totalTests, const std::vector<TestCase>& tests,
					   ReaderOptions opts)
{
	auto imgPaths = getImagesInDirectory(directory);
	auto folderName = directory.stem();

	if (Size(imgPaths) != totalTests)
		fmt::print("TEST {} => Expected number of tests: {}, got: {} => FAILED\n", folderName.string(), totalTests, imgPaths.size());

	for (auto& test : tests) {
		fmt::print("{:20} @ {:3}, {:3}", folderName.string(), test.rotation, Size(imgPaths));
		std::vector<int> times;
		std::string failures;
		for (auto tc : test.tc) {
			if (tc.name.empty())
				break;
			auto startTime = std::chrono::steady_clock::now();
			opts.setTryDownscale(tc.name == "slow_");
			opts.setDownscaleFactor(2);
			opts.setDownscaleThreshold(180);
			opts.setTryHarder(tc.name == "slow");
			opts.setTryRotate(tc.name == "slow");
			opts.setTryInvert(tc.name == "slow");
			opts.setIsPure(tc.name == "pure");
			if (opts.isPure())
				opts.setBinarizer(Binarizer::FixedThreshold);
			for (const auto& imgPath : imgPaths) {
				auto barcode = ReadBarcode(ImageLoader::load(imgPath).rotated(test.rotation), opts);
				if (barcode.isValid()) {
					auto error = checkResult(imgPath, format, barcode);
					if (!error.empty())
						tc.misReadFiles[imgPath] = error;
				} else {
					tc.notDetectedFiles.insert(imgPath);
				}
			}

			times.push_back(timeSince(startTime));
			failures += printPositiveTestStats(Size(imgPaths), tc);
		}
		fmt::print(" | time: {:3} vs {:3} ms\n", times.front(), times.back());
		if (!failures.empty())
			fmt::print("\n{}\n", failures);
	}
}

static Barcode readMultiple(const std::vector<fs::path>& imgPaths, std::string_view format)
{
	Barcodes allBarcodes;
	for (const auto& imgPath : imgPaths) {
		auto barcodes = ReadBarcodes(ImageLoader::load(imgPath),
									 ReaderOptions().setFormats(BarcodeFormatFromString(format)).setTryDownscale(false));
		allBarcodes.insert(allBarcodes.end(), barcodes.begin(), barcodes.end());
	}

	return MergeStructuredAppendSequence(allBarcodes);
}

static void doRunStructuredAppendTest(const fs::path& directory, std::string_view format, int totalTests,
									  const std::vector<TestCase>& tests)
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
		fmt::print("TEST {} => Expected number of tests: {}, got: {} => FAILED\n", folderName.string(), totalTests,
				   imageGroups.size());

	for (auto& test : tests) {
		fmt::print("{:20} @ {:3}, {:3}", folderName.string(), test.rotation, Size(imgPaths));
		auto tc = test.tc[0];
		auto startTime = std::chrono::steady_clock::now();

		for (const auto& [testPath, testImgPaths] : imageGroups) {
			auto barcode = readMultiple(testImgPaths, format);
			if (barcode.isValid()) {
				auto error = checkResult(testPath, format, barcode);
				if (!error.empty())
					tc.misReadFiles[testPath] = error;
			} else {
				tc.notDetectedFiles.insert(testPath);
			}
		}

		auto failures = printPositiveTestStats(Size(imageGroups), tc);
		fmt::print(" | time: {:3} ms\n", timeSince(startTime));
		if (!failures.empty())
			fmt::print("\n{}\n", failures);
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
						const std::vector<TestCase>& tests, const ReaderOptions& opts = ReaderOptions()) {
		if (hasTest(directory))
			doRunTests(testPathPrefix / directory, format, total, tests, opts);
	};

	auto runStructuredAppendTest = [&](std::string_view directory, std::string_view format, int total,
									   const std::vector<TestCase>& tests) {
		if (hasTest(directory))
			doRunStructuredAppendTest(testPathPrefix / directory, format, total, tests);
	};

	try
	{
		auto startTime = std::chrono::steady_clock::now();

		// clang-format off

		// Expected failures:
		// abc-inverted.png (fast) - fast does not try inverted
		// az-thick.png (pure)
		runTests("aztec-1", "Aztec", 31, {
			{ 30, 31, 0   },
			{ 30, 31, 90  },
			{ 30, 31, 180 },
			{ 30, 31, 270 },
			{ 29, 0, pure },
		});

		runTests("aztec-2", "Aztec", 22, {
			{ 21, 21, 0   },
			{ 21, 21, 90  },
			{ 21, 21, 180 },
			{ 21, 21, 270 },
		});

		runTests("datamatrix-1", "DataMatrix", 29, {
			{ 29, 29, 0   },
			{  0, 27, 90  },
			{  0, 27, 180 },
			{  0, 27, 270 },
			{ 28, 0, pure },
		});

		runTests("datamatrix-2", "DataMatrix", 13, {
			{ 13, 13, 0   },
			{  0, 13, 90  },
			{  0, 13, 180 },
			{  0, 13, 270 },
		});

		runTests("datamatrix-3", "DataMatrix", 21, {
			{ 20, 21, 0   },
			{  0, 21, 90  },
			{  0, 21, 180 },
			{  0, 21, 270 },
		});

		runTests("datamatrix-4", "DataMatrix", 21, {
			{ 21, 21, 0   },
			{  0, 21, 90  },
			{  0, 21, 180 },
			{  0, 21, 270 },
			{ 19, 0, pure },
		});

		runTests("dxfilmedge-1", "DXFilmEdge", 3, {
			{ 1, 3, 0 },
			{ 0, 3, 180 },
		});

		runTests("codabar-1", "Codabar", 11, {
			{ 11, 11, 0   },
			{ 11, 11, 180 },
		});

		runTests("codabar-2", "Codabar", 4, {
			{ 2, 3, 0   },
			{ 2, 3, 180 },
		});

		runTests("code39-1", "Code39", 4, {
			{ 4, 4, 0   },
			{ 4, 4, 180 },
		});

		runTests("code39-2", "Code39", 2, {
			{ 2, 2, 0   },
			{ 2, 2, 180 },
		});

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

		runTests("code128-2", "Code128", 22, {
			{ 19, 22, 0   },
			{ 20, 22, 180 },
		});

		runTests("code128-3", "Code128", 2, {
			{ 2, 2, 0   },
			{ 2, 2, 180 },
		});

		runTests("ean8-1", "EAN-8", 9, {
			{ 9, 9, 0   },
			{ 9, 9, 180 },
			{ 8, 0, pure },
		});

		runTests("ean13-1", "EAN-13", 32, {
			{ 26, 30, 0   },
			{ 25, 30, 180 },
		});

		runTests("ean13-2", "EAN-13", 24, {
			{ 7, 13, 0   },
			{ 7, 13, 180 },
		});

		runTests("ean13-3", "EAN-13", 21, {
			{ 20, 21, 0   },
			{ 21, 21, 180 },
		});

		runTests("ean13-4", "EAN-13", 22, {
			{ 6, 13, 0   },
			{ 7, 13, 180 },
		});

		runTests("ean13-extension-1", "EAN-13", 5, {
			{ 3, 5, 0 },
			{ 3, 5, 180 },
		}, ReaderOptions().setEanAddOnSymbol(EanAddOnSymbol::Require));

		runTests("itf-1", "ITF", 14, {
			{ 13, 14, 0   },
			{ 13, 14, 180 },
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
			{ 10, 12, 0   },
			{ 11, 12, 180 },
		});

		runTests("upca-2", "UPC-A", 36, {
			{ 17, 22, 0   },
			{ 17, 22, 180 },
		});

		runTests("upca-3", "UPC-A", 21, {
			{ 7, 11, 0   },
			{ 8, 11, 180 },
		});

		runTests("upca-4", "UPC-A", 19, {
			{ 8, 12, 0, 1, 0 },
			{ 9, 12, 0, 1, 180 },
		});

		runTests("upca-5", "UPC-A", 32, {
			{ 18, 20, 0   },
			{ 18, 20, 180 },
		});

		runTests("upca-extension-1", "UPC-A", 6, {
			{ 4, 4, 0 },
			{ 3, 4, 180 },
		}, ReaderOptions().setEanAddOnSymbol(EanAddOnSymbol::Require));

		runTests("upce-1", "UPC-E", 3, {
			{ 3, 3, 0   },
			{ 3, 3, 180 },
			{ 3, 0, pure },
		});

		runTests("upce-2", "UPC-E", 28, {
			{ 18, 22, 0, 1, 0   },
			{ 19, 22, 1, 1, 180 },
		});

		runTests("upce-3", "UPC-E", 11, {
			{ 5, 7, 0   },
			{ 6, 7, 180 },
		});

		runTests("rss14-1", "DataBar", 6, {
			{ 6, 6, 0   },
			{ 6, 6, 180 },
		});

		runTests("rss14-2", "DataBar", 14, {
			{ 10, 11, 0   },
			{ 10, 11, 180 },
		});

		runTests("rssexpanded-1", "DataBarExpanded", 34, {
			{ 34, 34, 0   },
			{ 34, 34, 180 },
			{ 34, 0, pure },
		});

		runTests("rssexpanded-2", "DataBarExpanded", 15, {
			{ 13, 15, 0   },
			{ 13, 15, 180 },
		});

		runTests("rssexpanded-3", "DataBarExpanded", 118, {
			{ 118, 118, 0   },
			{ 118, 118, 180 },
			{ 118, 0, pure },
		});

		runTests("rssexpandedstacked-1", "DataBarExpanded", 65, {
			{ 55, 65, 0   },
			{ 55, 65, 180 },
			{ 60, 0, pure },
		});

		runTests("rssexpandedstacked-2", "DataBarExpanded", 2, {
			{ 2, 2, 0   },
			{ 2, 2, 180 },
		});

		runTests("databarltd-1", "DataBarLimited", 2, {
			{ 2, 2, 0   },
			{ 2, 2, 180 },
			{ 2, 0, pure },
		});

		runTests("qrcode-1", "QRCode", 16, {
			{ 16, 16, 0   },
			{ 16, 16, 90  },
			{ 16, 16, 180 },
			{ 16, 16, 270 },
		});

		runTests("qrcode-2", "QRCode", 51, {
			{ 45, 48, 0   },
			{ 45, 48, 90  },
			{ 45, 48, 180 },
			{ 45, 48, 270 },
			{ 22, 1, pure }, // the misread is the 'outer' symbol in 16.png
		});

		runTests("qrcode-3", "QRCode", 28, {
			{ 28, 28, 0   },
			{ 28, 28, 90  },
			{ 28, 28, 180 },
			{ 28, 28, 270 },
		});

		runTests("qrcode-4", "QRCode", 41, {
			{ 31, 31, 0   },
			{ 31, 31, 90  },
			{ 31, 31, 180 },
			{ 31, 31, 270 },
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

		runTests("microqrcode-1", "MicroQRCode", 16, {
			{ 15, 15, 0   },
			{ 14, 14, 90  },
			{ 14, 14, 180 }, // ughs: 1 result is platform/compiler dependent (e.g. -march=core2 vs. haswell)
			{ 15, 15, 270 },
			{ 9, 0, pure },
		});

		runTests("rmqrcode-1", "rMQRCode", 3, {
			{  2,  3, 0   },
			{  2,  3, 90  },
			{  2,  3, 180 },
			{  2,  3, 270 },
			{  2,  2, pure },
		});

		runTests("pdf417-1", "PDF417", 17, {
			{ 16, 17, 0   },
			{  1, 17, 90  },
			{ 16, 17, 180 },
			{  1, 17, 270 },
			{ 16, 0, pure },
		});

		runTests("pdf417-2", "PDF417", 25, {
			{ 25, 25, 0   },
			{  0, 25, 90   },
			{ 25, 25, 180 },
			{  0, 25, 270   },
		});

		runTests("pdf417-3", "PDF417", 16, {
			{ 16, 16, 0   },
			{  0, 16, 90  },
			{ 16, 16, 180 },
			{  0, 16, 270 },
			{ 7, 0, pure },
		});

		runStructuredAppendTest("pdf417-4", "PDF417", 3, {
			{ 3, 3, 0   },
		});

		runTests("falsepositives-1", "None", 27, {
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
		if (extra)
			fmt::print("INFO: {} tests succeeded unexpectedly.\n", extra);

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
