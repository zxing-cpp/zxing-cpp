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

#include "BarcodeFormat.h"
#include "DecodeHints.h"
#include "MultiFormatReader.h"
#include "GenericLuminanceSource.h"
#include "HybridBinarizer.h"
#include "Result.h"
#include "ResultMetadata.h"
#include "TextUtfEncoding.h"
#include "lodepng.h"

#include <iostream>
#include <cstring>
#include <string>
#include <algorithm>
#include <cctype>

using namespace ZXing;

static void PrintUsage(const char* exePath)
{
    std::cout << "Usage: " << exePath << " [-fast] [-rotate] [-format <FORMAT>] <png image path>" << std::endl
		<< "    -fast    Do not try harder to detect, thus faster" << std::endl
		<< "    -rotate  Try to rotate image of 90 degrees if it fails to detect barcode" << std::endl
		<< "    -format  Try to read given format only. Supported formats are:" << std::endl;
	for (int i = 0; i < (int)BarcodeFormat::FORMAT_COUNT; ++i) {
		std::cout << "        " << ToString((BarcodeFormat)i) << std::endl;
	}
	std::cout << "    Formats can be lowercase letters, with or without underscore." << std::endl;
}

static std::string FormatClean(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(), [](char c) { return (char)std::tolower(c); });
	str.erase(std::remove(str.begin(), str.end(), '_'), str.end());
	return str;
}

static std::string ParseFormat(std::string str)
{
	str = FormatClean(str);
	for (int i = 0; i < (int)BarcodeFormat::FORMAT_COUNT; ++i) {
		auto standardForm = ToString((BarcodeFormat)i);
		if (str == FormatClean(standardForm))
			return standardForm;
	}
	return std::string();
}

static bool ParseOptions(int argc, char* argv[], bool* fastMode, bool* tryRotate, std::string* format, std::string* filePath)
{
	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "-fast") == 0) {
			*fastMode = true;
		}
		else if (strcmp(argv[i], "-rotate") == 0) {
			*tryRotate = true;
		}
		else if (strcmp(argv[i], "-format") == 0) {
			if (i + 1 < argc) {
				++i;
				*format = ParseFormat(argv[i]);
				if (format->empty()) {
					std::cerr << "Unreconigned format: " << argv[i] << std::endl;
					return false;
				}
			}
			else {
				return false;
			}
		}
		else {
			*filePath = argv[i];
		}
	}

	return !filePath->empty();
}

#if 0
using Binarizer = ZXing::GlobalHistogramBinarizer;
#else
using Binarizer = ZXing::HybridBinarizer;
#endif


int main(int argc, char* argv[])
{
	if (argc <= 1) {
		PrintUsage(argv[0]);
		return 0;
	}

	bool fastMode = false;
	bool tryRotate = false;
	std::string singleFormat, filePath;

	if (!ParseOptions(argc, argv, &fastMode, &tryRotate, &singleFormat, &filePath)) {
		PrintUsage(argv[0]);
		return -1;
	}

	DecodeHints hints;
	hints.setShouldTryHarder(!fastMode);
	hints.setShouldTryRotate(tryRotate);
	auto format = BarcodeFormatFromString(singleFormat);
	if (format != BarcodeFormat::FORMAT_COUNT)
		hints.setPossibleFormats({ format });
	MultiFormatReader reader(hints);

	std::vector<unsigned char> buffer;
	unsigned width, height;
	unsigned error = lodepng::decode(buffer, width, height, filePath);
	if (error) {
		std::cerr << "Error: " << lodepng_error_text(error) << std::endl
				<< "Failed to read image: " << filePath << std::endl;
		return -1;
	}
	GenericLuminanceSource source((int)width, (int)height, buffer.data(), width * 4, 4, 0, 1, 2);
	Binarizer binImage(std::shared_ptr<LuminanceSource>(&source, [](void*) {}));

	auto result = reader.read(binImage);
	if (result.isValid()) {
		std::cout << "Text:     " << TextUtfEncoding::ToUtf8(result.text()) << std::endl
			<< "Format:   " << ToString(result.format()) << std::endl;
		auto errLevel = result.metadata().getString(ResultMetadata::Key::ERROR_CORRECTION_LEVEL);
		if (!errLevel.empty()) {
			std::cout << "EC Level: " << TextUtfEncoding::ToUtf8(errLevel) << std::endl;
		}
		return 0;
	}

	return 1;
}
