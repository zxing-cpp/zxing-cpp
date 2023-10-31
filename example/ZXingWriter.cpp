/*
* Copyright 2016 Nu-book Inc.
*/
// SPDX-License-Identifier: Apache-2.0

#include "BarcodeFormat.h"
#include "BitMatrix.h"
#include "BitMatrixIO.h"
#include "CharacterSet.h"
#include "MultiFormatWriter.h"
#include "ZXVersion.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

using namespace ZXing;

static void PrintUsage(const char* exePath)
{
	std::cout << "Usage: " << exePath
			  << " [-size <width>x<height>] [-margin <margin>] [-encoding <encoding>] [-ecc <level>] <format> <text> <output>\n"
			  << "    -size      Size of generated image\n"
			  << "    -margin    Margin around barcode\n"
			  << "    -encoding  Encoding used to encode input text\n"
			  << "    -ecc       Error correction level, [0-8]\n"
			  << "    -binary    Interpret <text> as a file name containing binary data\n"
			  << "    -help      Print usage information\n"
			  << "    -version   Print version information\n"
			  << "\n"
			  << "Supported formats are:\n";
	for (auto f : BarcodeFormatsFromString("Aztec Codabar Code39 Code93 Code128 DataMatrix EAN8 EAN13 ITF PDF417 QRCode UPCA UPCE"))
		std::cout << "    " << ToString(f) << "\n";

	std::cout << "Format can be lowercase letters, with or without '-'.\n"
			  << "Output format is determined by file name, supported are png, jpg and svg.\n";
}

static bool ParseSize(std::string str, int* width, int* height)
{
	std::transform(str.begin(), str.end(), str.begin(), [](char c) { return (char)std::tolower(c); });
	auto xPos = str.find('x');
	if (xPos != std::string::npos) {
		*width  = std::stoi(str.substr(0, xPos));
		*height = std::stoi(str.substr(xPos + 1));
		return true;
	}
	return false;
}

static bool ParseOptions(int argc, char* argv[], int* width, int* height, int* margin, CharacterSet* encoding,
						 int* eccLevel, BarcodeFormat* format, std::string* text, std::string* filePath, bool* inputIsFile)
{
	int nonOptArgCount = 0;
	for (int i = 1; i < argc; ++i) {
		auto is = [&](const char* str) { return strncmp(argv[i], str, strlen(argv[i])) == 0; };
		if (is("-size")) {
			if (++i == argc)
				return false;
			if (!ParseSize(argv[i], width, height)) {
				std::cerr << "Invalid size specification: " << argv[i] << std::endl;
				return false;
			}
		} else if (is("-margin")) {
			if (++i == argc)
				return false;
			*margin = std::stoi(argv[i]);
		} else if (is("-ecc")) {
			if (++i == argc)
				return false;
			*eccLevel = std::stoi(argv[i]);
		} else if (is("-encoding")) {
			if (++i == argc)
				return false;
			*encoding = CharacterSetFromString(argv[i]);
		} else if (is("-binary")) {
			*inputIsFile = true;
		} else if (is("-help") || is("--help")) {
			PrintUsage(argv[0]);
			exit(0);
		} else if (is("-version") || is("--version")) {
			std::cout << "ZXingWriter " << ZXING_VERSION_STR << "\n";
			exit(0);
		} else if (nonOptArgCount == 0) {
			*format = BarcodeFormatFromString(argv[i]);
			if (*format == BarcodeFormat::None) {
				std::cerr << "Unrecognized format: " << argv[i] << std::endl;
				return false;
			}
			++nonOptArgCount;
		} else if (nonOptArgCount == 1) {
			*text = argv[i];
			++nonOptArgCount;
		} else if (nonOptArgCount == 2) {
			*filePath = argv[i];
			++nonOptArgCount;
		} else {
			return false;
		}
	}

	return nonOptArgCount == 3;
}

static std::string GetExtension(const std::string& path)
{
	auto fileNameStart = path.find_last_of("/\\");
	auto fileName = fileNameStart == std::string::npos ? path : path.substr(fileNameStart + 1);
	auto extStart = fileName.find_last_of('.');
	auto ext = extStart == std::string::npos ? "" : fileName.substr(extStart + 1);
	std::transform(ext.begin(), ext.end(), ext.begin(), [](char c) { return std::tolower(c); });
	return ext;
}

static std::string ReadFile(const std::string& fn)
{
	std::ifstream ifs(fn, std::ios::binary);
	return ifs ? std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>()) : std::string();
};

int main(int argc, char* argv[])
{
	int width = 100, height = 100;
	int margin = 10;
	int eccLevel = -1;
	bool inputIsFile = false;
	CharacterSet encoding = CharacterSet::Unknown;
	std::string input, filePath;
	BarcodeFormat format;

	if (!ParseOptions(argc, argv, &width, &height, &margin, &encoding, &eccLevel, &format, &input, &filePath, &inputIsFile)) {
		PrintUsage(argv[0]);
		return -1;
	}

	try {
		auto writer = MultiFormatWriter(format).setMargin(margin).setEncoding(encoding).setEccLevel(eccLevel);

		BitMatrix matrix;
		if (inputIsFile) {
			auto file = ReadFile(input);
			std::wstring bytes;
			for (uint8_t c : file)
				bytes.push_back(c);
			writer.setEncoding(CharacterSet::BINARY);
			matrix = writer.encode(bytes, width, height);
		} else {
			matrix = writer.encode(input, width, height);
		}
		auto bitmap = ToMatrix<uint8_t>(matrix);

		auto ext = GetExtension(filePath);
		int success = 0;
		if (ext == "" || ext == "png") {
			success = stbi_write_png(filePath.c_str(), bitmap.width(), bitmap.height(), 1, bitmap.data(), 0);
		} else if (ext == "jpg" || ext == "jpeg") {
			success = stbi_write_jpg(filePath.c_str(), bitmap.width(), bitmap.height(), 1, bitmap.data(), 0);
		} else if (ext == "svg") {
			success = (std::ofstream(filePath) << ToSVG(matrix)).good();
		}

		if (!success) {
			std::cerr << "Failed to write image: " << filePath << std::endl;
			return -1;
		}
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return -1;
	}

	return 0;
}
