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

#include <algorithm>
#include <cctype>
#include <chrono>
#include <clocale>
#include <cstring>
#include <cwctype>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace ZXing;

static void PrintUsage(const char* exePath)
{
	std::cout << "Usage: " << exePath << " [-fast] [-norotate] [-format <FORMAT[,...]>] [-ispure] [-1] <png image path>...\n"
			  << "    -fast      Skip some lines/pixels during detection (faster)\n"
			  << "    -norotate  Don't try rotated image during detection (faster)\n"
			  << "    -format    Only detect given format(s) (faster)\n"
			  << "    -ispure    Assume the image contains only a 'pure'/perfect code (faster)\n"
			  << "    -1         Print only file name, text and status on one line per file\n"
			  << "\n"
			  << "Supported formats are:\n";
	for (auto f : BarcodeFormats::all()) {
		std::cout << "    " << ToString(f) << "\n";
	}
	std::cout << "Formats can be lowercase, with or without '-', separated by ',' and/or '|'\n";
}

static bool ParseOptions(int argc, char* argv[], DecodeHints* hints, bool& oneLine, std::list<std::string>& filePaths)
{
	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "-fast") == 0) {
			hints->setTryHarder(false);
		}
		else if (strcmp(argv[i], "-norotate") == 0) {
			hints->setTryRotate(false);
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
		else if (strcmp(argv[i], "-1") == 0) {
			oneLine = true;
		}
		else {
			filePaths.push_back(argv[i]);
		}
	}

	return !filePaths.empty();
}

// Convert to UTF-8, placing non-graphical characters in angle brackets with text name
static std::string FormatText(const std::wstring& text) {
	static const char* const ascii_nongraphicals[33] = {
		"NUL", "SOH", "STX", "ETX", "EOT", "ENQ", "ACK", "BEL",
		 "BS",  "HT",  "LF",  "VT",  "FF",  "CR",  "SO",  "SI",
		"DLE", "DC1", "DC2", "DC3", "DC4", "NAK", "SYN", "ETB",
		"CAN",  "EM", "SUB", "ESC",  "FS",  "GS",  "RS",  "US",
		"DEL",
	};
	std::wostringstream ws;

	ws.fill(L'0');

	for (wchar_t wc : text) {
		if (wc < 128) { // ASCII
			if (wc < 32 || wc == 127) { // Non-graphical ASCII, excluding space
				ws << "<" << ascii_nongraphicals[wc == 127 ? 33 : wc] << ">";
			}
			else {
				ws << wc;
			}
		}
		else {
			if ((wc >= 0xd800 && wc < 0xe000) || std::iswgraph(wc)) { // Include surrogates
				ws << wc;
			}
			else { // Non-graphical Unicode
				int width = wc < 256 ? 2 : 4;
				ws << "<U+" << std::setw(width) << std::uppercase << std::hex << static_cast<unsigned int>(wc) << ">";
			}
		}
	}

	return TextUtfEncoding::ToUtf8(ws.str());
}

std::ostream& operator<<(std::ostream& os, const Position& points) {
	for (const auto& p : points)
		os << p.x << "x" << p.y << " ";
	return os;
}

int main(int argc, char* argv[])
{
	DecodeHints hints;
	std::list<std::string> filePaths;
	bool oneLine = false;
	int fileCount = 0;
	int ret = 0;

	std::setlocale(LC_ALL, "en_US.UTF-8"); // Needed for `std::iswgraph()`

	if (!ParseOptions(argc, argv, &hints, oneLine, filePaths)) {
		PrintUsage(argv[0]);
		return -1;
	}

	for (std::string filePath : filePaths) {
		int width, height, channels;
		std::unique_ptr<stbi_uc, void(*)(void*)> buffer(stbi_load(filePath.c_str(), &width, &height, &channels, 4), stbi_image_free);
		if (buffer == nullptr) {
			std::cerr << "Failed to read image: " << filePath << "\n";
			return -1;
		}
		fileCount++;

		const auto& result = ReadBarcode({buffer.get(), width, height, ImageFormat::RGBX}, hints);
		const auto& meta = result.metadata();

		ret |= static_cast<int>(result.status());

		if (oneLine) {
			if (!meta.getString(ResultMetadata::UPC_EAN_EXTENSION).empty()) {
				std::cout << filePath << " \"" << FormatText(result.text())
						  << "+" << FormatText(meta.getString(ResultMetadata::UPC_EAN_EXTENSION)) << "\" "
						  << ToString(result.status()) << "\n";
			}
			else {
				std::cout << filePath << " \"" << FormatText(result.text()) << "\" " << ToString(result.status()) << "\n";
			}
			continue;
		}

		if (filePaths.size() > 1) {
			if (fileCount > 1) {
				std::cout << "\n";
			}
			std::cout << "File:     " << filePath << "\n";
		}
		std::cout << "Text:     \"" << FormatText(result.text()) << "\"\n"
				  << "Format:   " << ToString(result.format()) << "\n"
				  << "Position: " << result.position() << "\n"
				  << "Rotation: " << result.orientation() << " deg\n"
				  << "Error:    " << ToString(result.status()) << "\n";

		std::map<ResultMetadata::Key, const char*> keys = {{ResultMetadata::ERROR_CORRECTION_LEVEL, "EC Level: "},
														   {ResultMetadata::POSSIBLE_COUNTRY, "Country:  "}};

		for (auto key : keys) {
			auto value = TextUtfEncoding::ToUtf8(meta.getString(key.first));
			if (value.size())
				std::cout << key.second << value << "\n";
		}

		if (!meta.getString(ResultMetadata::UPC_EAN_EXTENSION).empty()) {
			std::cout << "Add-on:   \"" << FormatText(meta.getString(ResultMetadata::UPC_EAN_EXTENSION)) << "\"";
			if (!meta.getString(ResultMetadata::ISSUE_NUMBER).empty()) {
				std::cout << " (Issue #" << FormatText(meta.getString(ResultMetadata::ISSUE_NUMBER)) << ")";
			}
			else if (!meta.getString(ResultMetadata::SUGGESTED_PRICE).empty()) {
				std::cout << " (Price " << FormatText(meta.getString(ResultMetadata::SUGGESTED_PRICE)) << ")";
			}
			std::cout << "\n";
		}
	}

	return ret;
}
