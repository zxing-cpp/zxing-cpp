/*
* Copyright 2016 Nu-book Inc.
* Copyright 2017 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "BlackboxTestRunner.h"
#include "ImageLoader.h"
#include "ReadBarcode.h"
#include "ZXAlgorithms.h"
#include "ZXFilesystem.h"

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <set>

using namespace ZXing;
using namespace ZXing::Test;

int getEnv(const char* name, int fallback = 0)
{
	auto var = getenv(name);
	return var ? atoi(var) : fallback;
}

int main(int argc, char** argv)
{
	if (argc <= 1) {
		std::cout << "Usage: " << argv[0] << " <test_path_prefix>" << std::endl;
		return 0;
	}

	fs::path pathPrefix = argv[1];

	if (Contains({".png", ".jpg", ".pgm", ".gif"}, pathPrefix.extension())) {
		auto hints = DecodeHints().setTryHarder(!getEnv("FAST", false)).setTryRotate(true).setIsPure(getEnv("IS_PURE"));
		if (getenv("FORMATS"))
			hints.setFormats(BarcodeFormatsFromString(getenv("FORMATS")));
		int rotation = getEnv("ROTATION");

		for (int i = 1; i < argc; ++i) {
			Result result = ReadBarcode(ImageLoader::load(argv[i]).rotated(rotation), hints);
			std::cout << argv[i] << ": ";
			if (result.isValid())
				std::cout << ToString(result.format()) << ": " << result.text() << "\n";
			else
				std::cout << "FAILED\n";
			if (result.isValid() && getenv("WRITE_TEXT")) {
				std::ofstream f(fs::path(argv[i]).replace_extension(".txt"));
				f << result.text();
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
