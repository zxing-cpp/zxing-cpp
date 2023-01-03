/*
* Copyright 2016 Nu-book Inc.
* Copyright 2019 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "ReadBarcode.h"
#include "GTIN.h"

#include <cctype>
#include <chrono>
#include <cstring>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

using namespace ZXing;

static void PrintUsage(const char* exePath)
{
	std::cout << "Usage: " << exePath << " [options] <image file>...\n"
			  << "    -fast      Skip some lines/pixels during detection (faster)\n"
			  << "    -norotate  Don't try rotated image during detection (faster)\n"
			  << "    -noinvert  Don't search for inverted codes during detection (faster)\n"
			  << "    -noscale   Don't try downscaled images during detection (faster)\n"
			  << "    -format <FORMAT[,...]>\n"
			  << "               Only detect given format(s) (faster)\n"
			  << "    -ispure    Assume the image contains only a 'pure'/perfect code (faster)\n"
			  << "    -errors    Include results with errors (like checksum error)\n"
			  << "    -mode <plain|eci|hri|escaped>\n"
			  << "               Text mode used to render the raw byte content into text\n"
			  << "    -1         Print only file name, content/error on one line per file/barcode (implies '-mode Escaped')\n"
			  << "    -bytes     Write (only) the bytes content of the symbol(s) to stdout\n"
			  << "    -pngout <file name>\n"
			  << "               Write a copy of the input image with barcodes outlined by a green line\n"
			  << "    -help      Print usage information and exit\n"
			  << "\n"
			  << "Supported formats are:\n";
	for (auto f : BarcodeFormats::all()) {
		std::cout << "    " << ToString(f) << "\n";
	}
	std::cout << "Formats can be lowercase, with or without '-', separated by ',' and/or '|'\n";
}

static bool ParseOptions(int argc, char* argv[], DecodeHints& hints, bool& oneLine, bool& bytesOnly,
						 std::vector<std::string>& filePaths, std::string& outPath)
{
	for (int i = 1; i < argc; ++i) {
		auto is = [&](const char* str) { return strncmp(argv[i], str, strlen(argv[i])) == 0; };
		if (is("-fast")) {
			hints.setTryHarder(false);
		} else if (is("-norotate")) {
			hints.setTryRotate(false);
		} else if (is("-noinvert")) {
			hints.setTryInvert(false);
		} else if (is("-noscale")) {
			hints.setTryDownscale(false);
		} else if (is("-ispure")) {
			hints.setIsPure(true);
			hints.setBinarizer(Binarizer::FixedThreshold);
		} else if (is("-errors")) {
			hints.setReturnErrors(true);
		} else if (is("-format")) {
			if (++i == argc)
				return false;
			try {
				hints.setFormats(BarcodeFormatsFromString(argv[i]));
			} catch (const std::exception& e) {
				std::cerr << e.what() << "\n";
				return false;
			}
		} else if (is("-mode")) {
			if (++i == argc)
				return false;
			else if (is("plain"))
				hints.setTextMode(TextMode::Plain);
			else if (is("eci"))
				hints.setTextMode(TextMode::ECI);
			else if (is("hri"))
				hints.setTextMode(TextMode::HRI);
			else if (is("escaped"))
				hints.setTextMode(TextMode::Escaped);
			else
				return false;
		} else if (is("-1")) {
			oneLine = true;
		} else if (is("-bytes")) {
			bytesOnly = true;
		} else if (is("-pngout")) {
			if (++i == argc)
				return false;
			outPath = argv[i];
		} else if (is("-help") || is("--help")) {
			PrintUsage(argv[0]);
			exit(0);
		} else {
			filePaths.push_back(argv[i]);
		}
	}

	return !filePaths.empty();
}

std::ostream& operator<<(std::ostream& os, const Position& points)
{
	for (const auto& p : points)
		os << p.x << "x" << p.y << " ";
	return os;
}

void drawLine(const ImageView& iv, PointI a, PointI b, bool error)
{
	int steps = maxAbsComponent(b - a);
	PointF dir = bresenhamDirection(PointF(b - a));
	int R = RedIndex(iv.format()), G = GreenIndex(iv.format()), B = BlueIndex(iv.format());
	for (int i = 0; i < steps; ++i) {
		auto p = PointI(centered(a + i * dir));
		auto* dst = const_cast<uint8_t*>(iv.data(p.x, p.y));
		if (dst < iv.data(0, 0) || dst > iv.data(iv.width() - 1, iv.height() - 1))
			continue;
		dst[R] = error ? 0xff : 0;
		dst[G] = error ? 0 : 0xff;
		dst[B] = 0;
	}
}

void drawRect(const ImageView& image, const Position& pos, bool error)
{
	for (int i = 0; i < 4; ++i)
		drawLine(image, pos[i], pos[(i + 1) % 4], error);
}

int main(int argc, char* argv[])
{
	DecodeHints hints;
	std::vector<std::string> filePaths;
	Results allResults;
	std::string outPath;
	bool oneLine = false;
	bool bytesOnly = false;
	int ret = 0;

	hints.setTextMode(TextMode::HRI);
	hints.setEanAddOnSymbol(EanAddOnSymbol::Read);

	if (!ParseOptions(argc, argv, hints, oneLine, bytesOnly, filePaths, outPath)) {
		PrintUsage(argv[0]);
		return -1;
	}


	std::cout.setf(std::ios::boolalpha);

	for (const auto& filePath : filePaths) {
		int width, height, channels;
		std::unique_ptr<stbi_uc, void(*)(void*)> buffer(stbi_load(filePath.c_str(), &width, &height, &channels, 3), stbi_image_free);
		if (buffer == nullptr) {
			std::cerr << "Failed to read image: " << filePath << "\n";
			return -1;
		}

		ImageView image{buffer.get(), width, height, ImageFormat::RGB};
		auto results = ReadBarcodes(image, hints);

		// if we did not find anything, insert a dummy to produce some output for each file
		if (results.empty())
			results.emplace_back();

		allResults.insert(allResults.end(), results.begin(), results.end());
		if (filePath == filePaths.back()) {
			auto merged = MergeStructuredAppendSequences(allResults);
			// report all merged sequences as part of the last file to make the logic not overly complicated here
			results.insert(results.end(), std::make_move_iterator(merged.begin()), std::make_move_iterator(merged.end()));
		}

		for (auto&& result : results) {

			if (!outPath.empty())
				drawRect(image, result.position(), bool(result.error()));

			ret |= static_cast<int>(result.error().type());

			if (bytesOnly) {
				std::cout.write(reinterpret_cast<const char*>(result.bytes().data()), result.bytes().size());
				continue;
			}

			if (oneLine) {
				std::cout << filePath << " " << ToString(result.format());
				if (result.isValid())
					std::cout << " \"" << result.text(TextMode::Escaped) << "\"";
				else if (result.error())
					std::cout << " " << ToString(result.error());
				std::cout << "\n";
				continue;
			}

			if (filePaths.size() > 1 || results.size() > 1) {
				static bool firstFile = true;
				if (!firstFile)
					std::cout << "\n";
				if (filePaths.size() > 1)
					std::cout << "File:       " << filePath << "\n";
				firstFile = false;
			}

			if (result.format() == BarcodeFormat::None) {
				std::cout << "No barcode found\n";
				continue;
			}

			std::cout << "Text:       \"" << result.text() << "\"\n"
					  << "Bytes:      " << ToHex(hints.textMode() == TextMode::ECI ? result.bytesECI() : result.bytes()) << "\n"
					  << "Format:     " << ToString(result.format()) << "\n"
					  << "Identifier: " << result.symbologyIdentifier() << "\n"
					  << "Content:    " << ToString(result.contentType()) << "\n"
					  << "HasECI:     " << result.hasECI() << "\n"
					  << "Position:   " << result.position() << "\n"
					  << "Rotation:   " << result.orientation() << " deg\n"
					  << "IsMirrored: " << result.isMirrored() << "\n"
					  << "IsInverted: " << result.isInverted() << "\n";

			auto printOptional = [](const char* key, const std::string& v) {
				if (!v.empty())
					std::cout << key << v << "\n";
			};

			printOptional("EC Level:   ", result.ecLevel());
			printOptional("Version:    ", result.version());
			printOptional("Error:      ", ToString(result.error()));

			if (result.lineCount())
				std::cout << "Lines:      " << result.lineCount() << "\n";

			if ((BarcodeFormat::EAN13 | BarcodeFormat::EAN8 | BarcodeFormat::UPCA | BarcodeFormat::UPCE)
					.testFlag(result.format())) {
				printOptional("Country:    ", GTIN::LookupCountryIdentifier(result.text(), result.format()));
				printOptional("Add-On:     ", GTIN::EanAddOn(result));
				printOptional("Price:      ", GTIN::Price(GTIN::EanAddOn(result)));
				printOptional("Issue #:    ", GTIN::IssueNr(GTIN::EanAddOn(result)));
			} else if (result.format() == BarcodeFormat::ITF && Size(result.bytes()) == 14) {
				printOptional("Country:    ", GTIN::LookupCountryIdentifier(result.text(), result.format()));
			}

			if (result.isPartOfSequence())
				std::cout << "Structured Append: symbol " << result.sequenceIndex() + 1 << " of "
						  << result.sequenceSize() << " (parity/id: '" << result.sequenceId() << "')\n";
			else if (result.sequenceSize() > 0)
				std::cout << "Structured Append: merged result from " << result.sequenceSize() << " symbols (parity/id: '"
						  << result.sequenceId() << "')\n";

			if (result.readerInit())
				std::cout << "Reader Initialisation/Programming\n";
		}

		if (Size(filePaths) == 1 && !outPath.empty())
			stbi_write_png(outPath.c_str(), image.width(), image.height(), 3, image.data(0, 0), image.rowStride());

#ifdef NDEBUG
		if (getenv("MEASURE_PERF")) {
			auto startTime = std::chrono::high_resolution_clock::now();
			auto duration = startTime - startTime;
			int N = 0;
			int blockSize = 1;
			do {
				for (int i = 0; i < blockSize; ++i)
					ReadBarcodes(image, hints);
				N += blockSize;
				duration = std::chrono::high_resolution_clock::now() - startTime;
				if (blockSize < 1000 && duration < std::chrono::milliseconds(100))
					blockSize *= 10;
			} while (duration < std::chrono::seconds(1));
			printf("time: %5.1f ms per frame\n", double(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count()) / N);
		}
#endif
	}

	return ret;
}
