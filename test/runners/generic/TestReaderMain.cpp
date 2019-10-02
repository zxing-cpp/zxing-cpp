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

#include <iostream>
#include <fstream>
#include <set>

#if __has_include(<filesystem>)
#  include <filesystem>
#  ifdef __cpp_lib_filesystem
     namespace fs = std::filesystem;
#  endif
#endif
#if !defined(__cpp_lib_filesystem) && __has_include(<experimental/filesystem>)
#  include <experimental/filesystem>
#  ifdef __cpp_lib_experimental_filesystem
     namespace fs = std::experimental::filesystem;
#  endif
#endif

// compiling this with clang (e.g. version 6) might require linking against libc++experimental.a or libc++fs.a.
// E.g.: CMAKE_EXE_LINKER_FLAGS = -L/usr/local/Cellar/llvm/6.0.1/lib -lc++experimental

using namespace ZXing;
using namespace ZXing::Test;

int main(int argc, char** argv)
{
	if (argc <= 1) {
		std::cout << "Usage: " << argv[0] << " <test_path_prefix>" << std::endl;
		return 0;
	}

	fs::path pathPrefix = argv[1];

	if (Contains({".png", ".jpg", ".pgm", ".gif"}, pathPrefix.extension())) {
		DecodeHints hints;
		hints.setShouldTryHarder(true);
		hints.setShouldTryRotate(true);
//		hints.setPossibleFormats(BarcodeFormatFromString("QR_CODE"));
		MultiFormatReader reader(hints);
		bool isPure = getenv("IS_PURE");
		int rotation = getenv("ROTATION") ? atoi(getenv("ROTATION")) : 0;

		for (int i = 1; i < argc; ++i) {
			Result result = reader.read(*ImageLoader::load(argv[i], isPure).rotated(rotation));
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
