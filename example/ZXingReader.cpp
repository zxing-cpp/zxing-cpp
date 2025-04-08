/*
* Copyright 2016 Nu-book Inc.
* Copyright 2019 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "GTIN.h"
#include "ReadBarcode.h"
#include "Version.h"

#ifdef ZXING_EXPERIMENTAL_API
#include "WriteBarcode.h"
#endif

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

struct CLI
{
	std::vector<std::string> filePaths;
	std::string outPath;
	int forceChannels = 0;
	int rotate = 0;
	bool oneLine = false;
	bool bytesOnly = false;
	bool showSymbol = false;
};

static void PrintUsage(const char* exePath)
{
	std::cout << "Usage: " << exePath << " [options] <image file>...\n"
			  << "    -fast      Skip some lines/pixels during detection (faster)\n"
			  << "    -norotate  Don't try rotated image during detection (faster)\n"
			  << "    -noinvert  Don't search for inverted codes during detection (faster)\n"
			  << "    -noscale   Don't try downscaled images during detection (faster)\n"
			  << "    -formats <FORMAT[,...]>\n"
			  << "               Only detect given format(s) (faster)\n"
			  << "    -single    Stop after the first barcode is detected (faster)\n"
			  << "    -ispure    Assume the image contains only a 'pure'/perfect code (faster)\n"
			  << "    -errors    Include barcodes with errors (like checksum error)\n"
			  << "    -binarizer <local|global|fixed>\n"
			  << "               Binarizer to be used for gray to binary conversion\n"
			  << "    -mode <plain|eci|hri|escaped>\n"
			  << "               Text mode used to render the raw byte content into text\n"
			  << "    -1         Print only file name, content/error on one line per file/barcode (implies '-mode Escaped')\n"
#ifdef ZXING_EXPERIMENTAL_API
			  << "    -symbol    Print the detected symbol (if available)\n"
#endif
			  << "    -bytes     Write (only) the bytes content of the symbol(s) to stdout\n"
			  << "    -pngout <file name>\n"
			  << "               Write a copy of the input image with barcodes outlined by a green line\n"
			  << "    -help      Print usage information\n"
			  << "    -version   Print version information\n"
			  << "\n"
			  << "Supported formats are:\n";
	for (auto f : BarcodeFormats::all()) {
		std::cout << "    " << ToString(f) << "\n";
	}
	std::cout << "Formats can be lowercase, with or without '-', separated by ',' and/or '|'\n";
}

static bool ParseOptions(int argc, char* argv[], ReaderOptions& options, CLI& cli)
{
#ifdef ZXING_EXPERIMENTAL_API
	options.setTryDenoise(true);
#endif

	for (int i = 1; i < argc; ++i) {
		auto is = [&](const char* str) { return strlen(argv[i]) > 1 && strncmp(argv[i], str, strlen(argv[i])) == 0; };
		if (is("-fast")) {
			options.setTryHarder(false);
#ifdef ZXING_EXPERIMENTAL_API
			options.setTryDenoise(false);
#endif
		} else if (is("-norotate")) {
			options.setTryRotate(false);
		} else if (is("-noinvert")) {
			options.setTryInvert(false);
		} else if (is("-noscale")) {
			options.setTryDownscale(false);
		} else if (is("-single")) {
			options.setMaxNumberOfSymbols(1);
		} else if (is("-ispure")) {
			options.setIsPure(true);
			options.setBinarizer(Binarizer::FixedThreshold);
		} else if (is("-errors")) {
			options.setReturnErrors(true);
		} else if (is("-formats")) {
			if (++i == argc)
				return false;
			try {
				options.setFormats(BarcodeFormatsFromString(argv[i]));
			} catch (const std::exception& e) {
				std::cerr << e.what() << "\n";
				return false;
			}
		} else if (is("-binarizer")) {
			if (++i == argc)
				return false;
			else if (is("local"))
				options.setBinarizer(Binarizer::LocalAverage);
			else if (is("global"))
				options.setBinarizer(Binarizer::GlobalHistogram);
			else if (is("fixed"))
				options.setBinarizer(Binarizer::FixedThreshold);
			else
				return false;
		} else if (is("-mode")) {
			if (++i == argc)
				return false;
			else if (is("plain"))
				options.setTextMode(TextMode::Plain);
			else if (is("eci"))
				options.setTextMode(TextMode::ECI);
			else if (is("hri"))
				options.setTextMode(TextMode::HRI);
			else if (is("escaped"))
				options.setTextMode(TextMode::Escaped);
			else
				return false;
		} else if (is("-1")) {
			cli.oneLine = true;
		} else if (is("-bytes")) {
			cli.bytesOnly = true;
		} else if (is("-symbol")) {
			cli.showSymbol = true;
		} else if (is("-pngout")) {
			if (++i == argc)
				return false;
			cli.outPath = argv[i];
		} else if (is("-channels")) {
			if (++i == argc)
				return false;
			cli.forceChannels = atoi(argv[i]);
		} else if (is("-rotate")) {
			if (++i == argc)
				return false;
			cli.rotate = atoi(argv[i]);
		} else if (is("-help") || is("--help")) {
			PrintUsage(argv[0]);
			exit(0);
		} else if (is("-version") || is("--version")) {
			std::cout << "ZXingReader " << ZXING_VERSION_STR << "\n";
			exit(0);
		} else {
			cli.filePaths.push_back(argv[i]);
		}
	}

	return !cli.filePaths.empty();
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
	ReaderOptions options;
	CLI cli;
	Barcodes allBarcodes;
	int ret = 0;

	options.setTextMode(TextMode::HRI);
	options.setEanAddOnSymbol(EanAddOnSymbol::Read);

	if (!ParseOptions(argc, argv, options, cli)) {
		PrintUsage(argv[0]);
		return -1;
	}

	std::cout.setf(std::ios::boolalpha);

	if (!cli.outPath.empty())
		cli.forceChannels = 3; // the drawing code only works for RGB data

	for (const auto& filePath : cli.filePaths) {
		int width, height, channels;
		std::unique_ptr<stbi_uc, void (*)(void*)> buffer(
			filePath == "-" ? stbi_load_from_file(stdin, &width, &height, &channels, cli.forceChannels)
							: stbi_load(filePath.c_str(), &width, &height, &channels, cli.forceChannels),
			stbi_image_free);
		if (buffer == nullptr) {
			std::cerr << "Failed to read image: " << filePath << " (" << stbi_failure_reason() << ")" << "\n";
			return -1;
		}
		channels = cli.forceChannels ? cli.forceChannels : channels;

		auto ImageFormatFromChannels = std::array{ImageFormat::None, ImageFormat::Lum, ImageFormat::LumA, ImageFormat::RGB, ImageFormat::RGBA};
		ImageView image{buffer.get(), width, height, ImageFormatFromChannels.at(channels)};
		auto barcodes = ReadBarcodes(image.rotated(cli.rotate), options);

		// if we did not find anything, insert a dummy to produce some output for each file
		if (barcodes.empty())
			barcodes.emplace_back();

		allBarcodes.insert(allBarcodes.end(), barcodes.begin(), barcodes.end());
		if (filePath == cli.filePaths.back()) {
			auto merged = MergeStructuredAppendSequences(allBarcodes);
			// report all merged sequences as part of the last file to make the logic not overly complicated here
			barcodes.insert(barcodes.end(), std::make_move_iterator(merged.begin()), std::make_move_iterator(merged.end()));
		}

		for (auto&& barcode : barcodes) {

			if (!cli.outPath.empty())
				drawRect(image, barcode.position(), bool(barcode.error()));

			ret |= static_cast<int>(barcode.error().type());

			if (cli.bytesOnly) {
				std::cout.write(reinterpret_cast<const char*>(barcode.bytes().data()), barcode.bytes().size());
				continue;
			}

			if (cli.oneLine) {
				std::cout << filePath << " " << ToString(barcode.format());
				if (barcode.isValid())
					std::cout << " \"" << barcode.text(TextMode::Escaped) << "\"";
				else if (barcode.error())
					std::cout << " " << ToString(barcode.error());
				std::cout << "\n";
				continue;
			}

			if (cli.filePaths.size() > 1 || barcodes.size() > 1) {
				static bool firstFile = true;
				if (!firstFile)
					std::cout << "\n";
				if (cli.filePaths.size() > 1)
					std::cout << "File:       " << filePath << "\n";
				firstFile = false;
			}

			if (barcode.format() == BarcodeFormat::None) {
				std::cout << "No barcode found\n";
				continue;
			}

			std::cout << "Text:       \"" << barcode.text() << "\"\n"
					  << "Bytes:      " << ToHex(options.textMode() == TextMode::ECI ? barcode.bytesECI() : barcode.bytes()) << "\n"
					  << "Format:     " << ToString(barcode.format()) << "\n"
					  << "Identifier: " << barcode.symbologyIdentifier() << "\n"
					  << "Content:    " << ToString(barcode.contentType()) << "\n"
					  << "HasECI:     " << barcode.hasECI() << "\n"
					  << "Position:   " << ToString(barcode.position()) << "\n"
					  << "Rotation:   " << barcode.orientation() << " deg\n"
					  << "IsMirrored: " << barcode.isMirrored() << "\n"
					  << "IsInverted: " << barcode.isInverted() << "\n";

			auto printOptional = [](const char* key, const std::string& v) {
				if (!v.empty())
					std::cout << key << v << "\n";
			};

			printOptional("EC Level:   ", barcode.ecLevel());
			printOptional("Version:    ", barcode.version());
			printOptional("Error:      ", ToString(barcode.error()));

			if (barcode.lineCount())
				std::cout << "Lines:      " << barcode.lineCount() << "\n";

			if ((BarcodeFormat::EAN13 | BarcodeFormat::EAN8 | BarcodeFormat::UPCA | BarcodeFormat::UPCE)
					.testFlag(barcode.format())) {
				printOptional("Country:    ", GTIN::LookupCountryIdentifier(barcode.text(), barcode.format()));
				printOptional("Add-On:     ", GTIN::EanAddOn(barcode));
				printOptional("Price:      ", GTIN::Price(GTIN::EanAddOn(barcode)));
				printOptional("Issue #:    ", GTIN::IssueNr(GTIN::EanAddOn(barcode)));
			} else if (barcode.format() == BarcodeFormat::ITF && Size(barcode.bytes()) == 14) {
				printOptional("Country:    ", GTIN::LookupCountryIdentifier(barcode.text(), barcode.format()));
			}

			if (barcode.isPartOfSequence())
				std::cout << "Structured Append: symbol " << barcode.sequenceIndex() + 1 << " of "
						  << barcode.sequenceSize() << " (parity/id: '" << barcode.sequenceId() << "')\n";
			else if (barcode.sequenceSize() > 0)
				std::cout << "Structured Append: merged result from " << barcode.sequenceSize() << " symbols (parity/id: '"
						  << barcode.sequenceId() << "')\n";

			if (barcode.readerInit())
				std::cout << "Reader Initialisation/Programming\n";

#ifdef ZXING_EXPERIMENTAL_API
			printOptional("Extra:      ", barcode.extra());
			if (cli.showSymbol && barcode.symbol().data())
				std::cout << "Symbol:\n" << WriteBarcodeToUtf8(barcode);
#endif
		}

		if (Size(cli.filePaths) == 1 && !cli.outPath.empty())
			stbi_write_png(cli.outPath.c_str(), image.width(), image.height(), 3, image.data(), image.rowStride());

#ifdef NDEBUG
		if (getenv("MEASURE_PERF")) {
			auto startTime = std::chrono::high_resolution_clock::now();
			auto duration = startTime - startTime;
			int N = 0;
			int blockSize = 1;
			do {
				for (int i = 0; i < blockSize; ++i)
					ReadBarcodes(image, options);
				N += blockSize;
				duration = std::chrono::high_resolution_clock::now() - startTime;
				if (blockSize < 1000 && duration < std::chrono::milliseconds(100))
					blockSize *= 10;
			} while (duration < std::chrono::seconds(1));
			printf("time: %5.2f ms per frame\n", double(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count()) / N);
		}
#endif
	}

	return ret;
}
