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

#include "ReadBarcode.h"
#include "TextUtfEncoding.h"

#include <iostream>
#include <cstring>
#include <string>
#include <algorithm>
#include <cctype>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace ZXing;

static void PrintUsage(const char* exePath)
{
	std::cout << "Usage: " << exePath << " [-fast] [-rotate] [-format <FORMAT[,...]>] <png image path>\n"
			  << "    -fast    Skip some lines/pixels during detection\n"
			  << "    -rotate  Also try rotated image during detection\n"
			  << "    -format  Only detect given format(s)\n"
			  << "\n"
			  << "Supported formats are:\n";
	for (int i = 0; i < (int)BarcodeFormat::FORMAT_COUNT; ++i) {
		std::cout << "    " << ToString((BarcodeFormat)i) << "\n";
	}
	std::cout << "Formats can be lowercase, with or without underscore, separated by ',', '|' and/or ' '\n";
}

static bool ParseOptions(int argc, char* argv[], bool* fastMode, bool* tryRotate, BarcodeFormats* formats, std::string* filePath)
{
	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "-fast") == 0) {
			*fastMode = true;
		}
		else if (strcmp(argv[i], "-rotate") == 0) {
			*tryRotate = true;
		}
		else if (strcmp(argv[i], "-format") == 0) {
			if (++i == argc)
				return false;
			try {
				*formats = BarcodeFormatsFromString(argv[i]);
			} catch (const std::exception& e) {
				std::cerr << e.what() << "\n";
				return false;
			}
		}
		else {
			*filePath = argv[i];
		}
	}

	return !filePath->empty();
}

std::ostream& operator<<(std::ostream& os, const std::vector<ResultPoint>& points) {
	for (const auto& p : points)
		os << int(p.x() + .5f) << "x" << int(p.y() + .5f) << " ";
	return os;
}

int main(int argc, char* argv[])
{
	if (argc <= 1) {
		PrintUsage(argv[0]);
		return 0;
	}

	bool fastMode = false;
	bool tryRotate = false;
	std::string filePath;
	BarcodeFormats formats;

	if (!ParseOptions(argc, argv, &fastMode, &tryRotate, &formats, &filePath)) {
		PrintUsage(argv[0]);
		return -1;
	}

	int width, height, channels;
	std::unique_ptr<stbi_uc, void(*)(void*)> buffer(stbi_load(filePath.c_str(), &width, &height, &channels, 4), stbi_image_free);
	if (buffer == nullptr) {
		std::cerr << "Failed to read image: " << filePath << "\n";
		return -1;
	}

	auto result = ReadBarcode(width, height, buffer.get(), width * 4, 4, 0, 1, 2, formats, tryRotate, !fastMode);

	if (result.isValid()) {
		std::cout << "Text:     " << TextUtfEncoding::ToUtf8(result.text()) << "\n"
		          << "Format:   " << ToString(result.format()) << "\n"
		          << "Position: " << result.resultPoints() << "\n";
		auto errLevel = result.metadata().getString(ResultMetadata::Key::ERROR_CORRECTION_LEVEL);
		if (!errLevel.empty()) {
			std::cout << "EC Level: " << TextUtfEncoding::ToUtf8(errLevel) << "\n";
		}
		return 0;
	}

	return 1;
}
