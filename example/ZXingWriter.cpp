/*
* Copyright 2016 Nu-book Inc.
*/
// SPDX-License-Identifier: Apache-2.0

// #define USE_OLD_WRITER_API

#ifndef USE_OLD_WRITER_API
#include "CreateBarcode.h"
#include "WriteBarcode.h"
#else
#include "BitMatrix.h"
#include "BitMatrixIO.h"
#include "CharacterSet.h"
#include "MultiFormatWriter.h"
#endif
#include "Version.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

using namespace ZXing;

static void PrintUsage(const char* exePath)
{
	std::cout << "Usage: " << exePath
			  << " [-options <creator-options>] [-scale <factor>] [-binary] [-noqz] [-hrt] [-invert] <format> <text> <output>\n"
			  << "    -options   Comma separated list of format specific options and flags\n"
			  << "    -scale     module size of generated image / negative numbers mean 'target size in pixels'\n"
//			  << "    -encoding  Encoding used to encode input text\n"
			  << "    -binary    Interpret <text> as a file name containing binary data\n"
			  << "    -noqz      Print barcode witout quiet zone\n"
			  << "    -hrt       Print human readable text below the barcode (if supported)\n"
			  << "    -invert    Invert colors (switch black and white)\n"
			  << "    -help      Print usage information\n"
			  << "    -version   Print version information\n"
			  << "\n"
			  << "Supported formats are (Symbology : Variants):";
	for (auto f : BarcodeFormats::list(BarcodeFormat::AllCreatable)) {
		if (Symbology(f) == f || f == BarcodeFormat::DXFilmEdge)
			std::cout << "\n " << std::setw(13) << ToString(f) << " : ";
		else
			std::cout << ToString(f) << ", ";
	}
	std::cout << "\n\n";

	std::cout << "Format can be lowercase letters, with or without any of ' -_/'.\n"
			  << "Output format is determined by file name, supported are png, jpg and svg.\n";
}

struct CLI
{
	BarcodeFormat format = BarcodeFormat::None;
	int scale = 0;
	std::string input;
	std::string outPath;
	std::string options;
	bool inputIsFile = false;
	bool invert = false;
	bool addHRT = false;
	bool addQZs = true;
	bool verbose = false;
//	CharacterSet encoding = CharacterSet::Unknown;
};

static bool ParseOptions(int argc, char* argv[], CLI& cli)
{
	int nonOptArgCount = 0;
	for (int i = 1; i < argc; ++i) {
		auto is = [&](const char* str) { return strncmp(argv[i], str, strlen(argv[i])) == 0; };
		if (is("-scale")) {
			if (++i == argc)
				return false;
			cli.scale = std::stoi(argv[i]);
		// } else if (is("-encoding")) {
		// 	if (++i == argc)
		// 		return false;
		// 	cli.encoding = CharacterSetFromString(argv[i]);
		} else if (is("-binary")) {
			cli.inputIsFile = true;
		} else if (is("-hrt")) {
			cli.addHRT = true;
		} else if (is("-noqz")) {
			cli.addQZs = false;
		} else if (is("-invert")) {
			cli.invert = true;
		} else if (is("-options")) {
			if (++i == argc)
				return false;
			cli.options = argv[i];
		} else if (is("-verbose")) {
			cli.verbose = true;
		} else if (is("-help") || is("--help")) {
			PrintUsage(argv[0]);
			exit(0);
		} else if (is("-version") || is("--version")) {
			std::cout << "ZXingWriter " << ZXING_VERSION_STR << "\n";
			exit(0);
		} else if (nonOptArgCount == 0) {
			try {
				cli.format = BarcodeFormatFromString(argv[i]);
			} catch (const std::exception& e) {
				std::cerr << "Error: " << e.what() << "\n\n";
				return false;
			}
			++nonOptArgCount;
		} else if (nonOptArgCount == 1) {
			cli.input = argv[i];
			++nonOptArgCount;
		} else if (nonOptArgCount == 2) {
			cli.outPath = argv[i];
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

template <typename T = char>
std::vector<T> ReadFile(const std::string& fn)
{
	std::basic_ifstream<T> ifs(fn, std::ios::binary);
	if (!ifs.good())
		throw std::runtime_error("failed to open/read file " + fn);
	return ifs ? std::vector(std::istreambuf_iterator<T>(ifs), std::istreambuf_iterator<T>()) : std::vector<T>();
};

int main(int argc, char* argv[])
{
	CLI cli;

	if (!ParseOptions(argc, argv, cli)) {
		PrintUsage(argv[0]);
		return -1;
	}

	try {
#if 1
		auto cOpts = CreatorOptions(cli.format, cli.options);
		auto barcode = cli.inputIsFile ? CreateBarcodeFromBytes(ReadFile(cli.input), cOpts) : CreateBarcodeFromText(cli.input, cOpts);

		auto wOpts = WriterOptions().scale(cli.scale).addQuietZones(cli.addQZs).addHRT(cli.addHRT).invert(cli.invert).rotate(0);
		auto bitmap = WriteBarcodeToImage(barcode, wOpts);

		if (cli.verbose) {
			std::cout.setf(std::ios::boolalpha);
			std::cout << "Text:       \"" << barcode.text() << "\"\n"
					  << "Bytes:      " << barcode.text(TextMode::Hex) << "\n"
					  << "Format:     " << ToString(barcode.format()) << "\n"
					  << "Symbology:  " << ToString(barcode.symbology()) << "\n"
					  << "Identifier: " << barcode.symbologyIdentifier() << "\n"
					  << "Content:    " << ToString(barcode.contentType()) << "\n"
					  << "HasECI:     " << barcode.hasECI() << "\n"
					  << "Position:   " << ToString(barcode.position()) << "\n"
					  << "Rotation:   " << barcode.orientation() << " deg\n"
					  << "IsMirrored: " << barcode.isMirrored() << "\n"
					  << "IsInverted: " << barcode.isInverted() << "\n"
					  << "ECLevel:    " << barcode.ecLevel() << "\n";
			std::cout << WriteBarcodeToUtf8(barcode, wOpts);
		}
#else // 'old' writer API (non zint based)
		auto writer = MultiFormatWriter(cli.format).setMargin(cli.addQZs ? 10 : 0);

		BitMatrix matrix;
		if (cli.inputIsFile) {
			auto file = ReadFile(cli.input);
			std::wstring bytes;
			for (uint8_t c : file)
				bytes.push_back(c);
			writer.setEncoding(CharacterSet::BINARY);
			matrix = writer.encode(bytes, cli.scale, std::clamp(cli.scale / 2, 50, 300));
		} else {
			writer.setEncoding(CharacterSet::UTF8);
			matrix = writer.encode(cli.input, cli.scale, std::clamp(cli.scale / 2, 50, 300));
		}
		auto bitmap = ToMatrix<uint8_t>(matrix);
#endif

		auto ext = GetExtension(cli.outPath);
		int success = 0;
		if (ext == "" || ext == "png") {
			success = stbi_write_png(cli.outPath.c_str(), bitmap.width(), bitmap.height(), 1, bitmap.data(), 0);
		} else if (ext == "jpg" || ext == "jpeg") {
			success = stbi_write_jpg(cli.outPath.c_str(), bitmap.width(), bitmap.height(), 1, bitmap.data(), 0);
		} else if (ext == "svg") {
#ifndef USE_OLD_WRITER_API
			success = (std::ofstream(cli.outPath) << WriteBarcodeToSVG(barcode, wOpts)).good();
#else
			success = (std::ofstream(cli.outPath) << ToSVG(matrix)).good();
#endif
		}

		if (!success) {
			std::cerr << "Failed to write image: " << cli.outPath << std::endl;
			return -1;
		}
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return -1;
	}

	return 0;
}
