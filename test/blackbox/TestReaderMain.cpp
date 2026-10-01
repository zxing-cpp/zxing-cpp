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
		std::println("Usage: {} <test_path_prefix> [test_name_prefix,...]", argv[0]);
		return 0;
	}

	if (fs::is_directory(argv[1])) {
		std::set<std::string> includedTests;
		for (int i = 2; i < argc; ++i)
			includedTests.insert(argv[i]);

		try {
			return runBlackBoxTests(argv[1], includedTests);
		} catch (const std::exception& e) {
			std::println("{}", e.what());
		}
		return -1;
	} else {
		auto opts = ReaderOptions().tryHarder(!getEnv("FAST", false)).tryDownscale(false).isPure(getEnv("IS_PURE"));
		if (getenv("FORMATS"))
			opts.formats(BarcodeFormatsFromString(getenv("FORMATS")));
		int rotation = getEnv("ROTATION");

		for (int i = 1; i < argc; ++i) {
			if (!Contains({".png", ".jpg", ".jpeg", ".pgm", ".gif", ".webp", ".jxl"}, fs::path(argv[i]).extension()))
				continue;

			Barcode barcode = ReadBarcode(ImageLoader::load(argv[i]).rotated(rotation), opts);
			std::print("{}: ", argv[i]);
			if (barcode.isValid())
				std::println("{}: {}", EnumName(barcode.format()), barcode.text());
			else
				std::println("FAILED");
			if (barcode.isValid() && getenv("WRITE_TOML")) {
				std::ofstream f(fs::path(argv[i]).replace_extension(".toml"));
				f << std::format("Format = \"{}\"\n", ToString(barcode.format()));
				if (barcode.contentType() == ContentType::Binary)
					f << std::format("TextHex = \"{}\"\n", barcode.text(TextMode::Hex));
				else
					f << std::format("TextPlain = \"{}\"\n", barcode.text(TextMode::Plain));
				f << std::format("UEC = \"{}\"\n", barcode.extra(BarcodeExtra::UEC));
				f << std::format("\nfind = \"sa fa\"\n");
			}
		}
		return 0;
	}
}
