/*
* Copyright 2016 Nu-book Inc.
* Copyright 2017 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "BlackboxTestRunner.h"
#include "ImageLoader.h"
#include "ReadBarcode.h"
#include "StdPrint.h"
#include "ZXAlgorithms.h"

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <set>

using namespace ZXing;
using namespace ZXing::Test;

int getEnv(const char* name, int fallback = 0)
{
	auto var = getenv(name);
	return var ? std::stoi(var) : fallback;
}

int main(int argc, char** argv)
{
	if (argc <= 1) {
		std::println("Usage: {} <test_path_prefix>", argv[0]);
		return 0;
	}

	fs::path pathPrefix = argv[1];

	if (Contains({".png", ".jpg", ".pgm", ".gif"}, pathPrefix.extension())) {
		auto opts = ReaderOptions().tryHarder(!getEnv("FAST", false)).tryRotate(true).isPure(getEnv("IS_PURE"));
		if (getenv("FORMATS"))
			opts.formats(BarcodeFormatsFromString(getenv("FORMATS")));
		int rotation = getEnv("ROTATION");

		for (int i = 1; i < argc; ++i) {
			Barcode barcode = ReadBarcode(ImageLoader::load(argv[i]).rotated(rotation), opts);
			std::print("{}: ", argv[i]);
			if (barcode.isValid())
				std::println("{}: {}", ToString(barcode.format()), barcode.text());
			else
				std::println("FAILED");
			if (barcode.isValid() && getenv("WRITE_TEXT")) {
				std::ofstream f(fs::path(argv[i]).replace_extension(".txt"));
				f << barcode.text();
			}
		}
		return 0;
	} else {
		std::set<std::string> includedTests;
		for (int i = 2; i < argc; ++i) {
			if (std::strlen(argv[i]) > 2 && argv[i][0] == '-' && argv[i][1] == 't') {
				includedTests.insert(argv[i] + 2);
			}
		}

		return runBlackBoxTests(pathPrefix, includedTests);
	}
}
