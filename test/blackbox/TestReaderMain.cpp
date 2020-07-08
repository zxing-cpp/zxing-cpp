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

#include "ByteArray.h"
#include "BlackboxTestRunner.h"
#include "ImageLoader.h"
#include "MultiFormatReader.h"
#include "Result.h"
#include "BinaryBitmap.h"
#include "ImageLoader.h"
#include "DecodeHints.h"
#include "TextUtfEncoding.h"
#include "ZXContainerAlgorithms.h"
#include "ZXFilesystem.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
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
		MultiFormatReader reader(hints);
		int rotation = getEnv("ROTATION");

		for (int i = 1; i < argc; ++i) {
			Result result = reader.read(*ImageLoader::load(argv[i]).rotated(rotation));
			std::cout << argv[i] << ": ";
			if (result.isValid())
				std::cout << ToString(result.format()) << ": " << TextUtfEncoding::ToUtf8(result.text()) << " " << metadataToUtf8(result) << "\n";
			else
				std::cout << "FAILED\n";
			if (result.isValid() && getenv("WRITE_TEXT")) {
				std::ofstream f(fs::path(argv[i]).replace_extension(".txt"));
				f << TextUtfEncoding::ToUtf8(result.text());
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
