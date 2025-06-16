/*
* Copyright 2016 Nu-book Inc.
*/
// SPDX-License-Identifier: Apache-2.0

#ifdef ZXING_EXPERIMENTAL_API
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
#include <string>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

using namespace ZXing;

static void PrintUsage(const char* exePath)
{
	std::cout << "Usage: " << exePath
			  << " [-size <width/height>] [-eclevel <level>] [-noqz] [-hrt] <format> <text> <output>\n"
			  << "    -size      Size of generated image\n"
//			  << "    -margin    Margin around barcode\n"
//			  << "    -encoding  Encoding used to encode input text\n"
			  << "    -eclevel   Error correction level, [0-8]\n"
			  << "    -binary    Interpret <text> as a file name containing binary data\n"
			  << "    -noqz      Print barcode witout quiet zone\n"
			  << "    -hrt       Print human readable text below the barcode (if supported)\n"
			  << "    -options   Comma separated list of symbology specific options and flags\n"
			  << "    -help      Print usage information\n"
			  << "    -version   Print version information\n"
			  << "\n"
			  << "Supported formats are:\n";
#ifdef ZXING_EXPERIMENTAL_API
	for (auto f : BarcodeFormats::all())
#else
	for (auto f : BarcodeFormatsFromString("Aztec Codabar Code39 Code93 Code128 DataMatrix EAN8 EAN13 ITF PDF417 QRCode UPCA UPCE"))
#endif
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

struct CLI
{
	BarcodeFormat format;
	int sizeHint = 0;
	std::string input;
	std::string outPath;
	std::string ecLevel;
	std::string options;
	bool inputIsFile = false;
	bool withHRT = false;
	bool withQZ = true;
	bool verbose = false;
//	CharacterSet encoding = CharacterSet::Unknown;
};

static bool ParseOptions(int argc, char* argv[], CLI& cli)
{
	int nonOptArgCount = 0;
	for (int i = 1; i < argc; ++i) {
		auto is = [&](const char* str) { return strncmp(argv[i], str, strlen(argv[i])) == 0; };
		if (is("-size")) {
			if (++i == argc)
				return false;
			cli.sizeHint = std::stoi(argv[i]);
		} else if (is("-eclevel")) {
			if (++i == argc)
				return false;
			cli.ecLevel = argv[i];
		// } else if (is("-margin")) {
		// 	if (++i == argc)
		// 		return false;
		// 	cli.margin = std::stoi(argv[i]);
		// } else if (is("-encoding")) {
		// 	if (++i == argc)
		// 		return false;
		// 	cli.encoding = CharacterSetFromString(argv[i]);
		} else if (is("-binary")) {
			cli.inputIsFile = true;
		} else if (is("-hrt")) {
			cli.withHRT = true;
		} else if (is("-noqz")) {
			cli.withQZ = false;
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
			cli.format = BarcodeFormatFromString(argv[i]);
			if (cli.format == BarcodeFormat::None) {
				std::cerr << "Unrecognized format: " << argv[i] << std::endl;
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
#ifdef ZXING_EXPERIMENTAL_API
		auto cOpts = CreatorOptions(cli.format).ecLevel(cli.ecLevel).options(cli.options);
		auto barcode = cli.inputIsFile ? CreateBarcodeFromBytes(ReadFile(cli.input), cOpts) : CreateBarcodeFromText(cli.input, cOpts);

		auto wOpts = WriterOptions().sizeHint(cli.sizeHint).withQuietZones(cli.withQZ).withHRT(cli.withHRT).rotate(0);
		auto bitmap = WriteBarcodeToImage(barcode, wOpts);

		if (cli.verbose) {
			std::cout << "Text:       \"" << barcode.text() << "\"\n"
					  << "Bytes:      " << ToHex(barcode.bytes()) << "\n"
					  << "Format:     " << ToString(barcode.format()) << "\n"
					  << "Identifier: " << barcode.symbologyIdentifier() << "\n"
					  << "Content:    " << ToString(barcode.contentType()) << "\n"
					  << "HasECI:     " << barcode.hasECI() << "\n"
					  << "Position:   " << ToString(barcode.position()) << "\n"
					  << "Rotation:   " << barcode.orientation() << " deg\n"
					  << "IsMirrored: " << barcode.isMirrored() << "\n"
					  << "IsInverted: " << barcode.isInverted() << "\n"
					  << "ecLevel:    " << barcode.ecLevel() << "\n";
			std::cout << WriteBarcodeToUtf8(barcode);
		}
#else
		auto writer = MultiFormatWriter(cli.format).setMargin(cli.withQZ ? 10 : 0);
		if (!cli.ecLevel.empty())
			writer.setEccLevel(std::stoi(cli.ecLevel));

		BitMatrix matrix;
		if (cli.inputIsFile) {
			auto file = ReadFile(cli.input);
			std::wstring bytes;
			for (uint8_t c : file)
				bytes.push_back(c);
			writer.setEncoding(CharacterSet::BINARY);
			matrix = writer.encode(bytes, cli.sizeHint, std::clamp(cli.sizeHint / 2, 50, 300));
		} else {
			writer.setEncoding(CharacterSet::UTF8);
			matrix = writer.encode(cli.input, cli.sizeHint, std::clamp(cli.sizeHint / 2, 50, 300));
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
#ifdef ZXING_EXPERIMENTAL_API
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
