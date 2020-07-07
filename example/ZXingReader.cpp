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
#include "ZXNumeric.h"

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
			  << "    -ispure  Assume the image contains only a 'pure'/perfect code\n"
			  << "\n"
			  << "Supported formats are:\n";
	for (auto f : BarcodeFormats::all()) {
		std::cout << "    " << ToString(f) << "\n";
	}
	std::cout << "Formats can be lowercase, with or without underscore, separated by ',', '|' and/or ' '\n";
}

static bool ParseOptions(int argc, char* argv[], DecodeHints* hints, std::string* filePath)
{
	hints->setTryHarder(true);
	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "-fast") == 0) {
			hints->setTryHarder(false);
		}
		else if (strcmp(argv[i], "-rotate") == 0) {
			hints->setTryRotate(true);
		}
		else if (strcmp(argv[i], "-ispure") == 0) {
			hints->setIsPure(true);
			hints->setBinarizer(Binarizer::FixedThreshold);
		}
		else if (strcmp(argv[i], "-format") == 0) {
			if (++i == argc)
				return false;
			try {
				hints->setFormats(BarcodeFormatsFromString(argv[i]));
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

std::ostream& operator<<(std::ostream& os, const Position& points) {
	for (const auto& p : points)
		os << p.x << "x" << p.y << " ";
	return os;
}

int main(int argc, char* argv[])
{
	DecodeHints hints;
	std::string filePath;

	if (!ParseOptions(argc, argv, &hints, &filePath)) {
		PrintUsage(argv[0]);
		return -1;
	}

	int width, height, channels;
	std::unique_ptr<stbi_uc, void(*)(void*)> buffer(stbi_load(filePath.c_str(), &width, &height, &channels, 4), stbi_image_free);
	if (buffer == nullptr) {
		std::cerr << "Failed to read image: " << filePath << "\n";
		return -1;
	}

	auto result = ReadBarcode({buffer.get(), width, height, ImageFormat::RGBX}, hints);

	std::cout << "Text:     \"" << TextUtfEncoding::ToUtf8(result.text()) << "\"\n"
			  << "Format:   " << ToString(result.format()) << "\n"
			  << "Position: " << result.position() << "\n"
			  << "Rotation: " << std::lround(result.position().rotation() * kDegPerRad) << "\n"
			  << "Error:    " << ToString(result.status()) << "\n";
	auto errLevel = result.metadata().getString(ResultMetadata::Key::ERROR_CORRECTION_LEVEL);
	if (!errLevel.empty()) {
		std::cout << "EC Level: " << TextUtfEncoding::ToUtf8(errLevel) << "\n";
	}

	return static_cast<int>(result.status());
}
