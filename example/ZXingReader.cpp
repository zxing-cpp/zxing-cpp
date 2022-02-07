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
#include "GTIN.h"

#include <cctype>
#include <chrono>
#include <clocale>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

using namespace ZXing;
using namespace TextUtfEncoding;

static void PrintUsage(const char* exePath)
{
	std::cout << "Usage: " << exePath << " [-fast] [-norotate] [-format <FORMAT[,...]>] [-pngout <png out path>] [-ispure] [-1] <png image path>...\n"
			  << "    -fast      Skip some lines/pixels during detection (faster)\n"
			  << "    -norotate  Don't try rotated image during detection (faster)\n"
			  << "    -format    Only detect given format(s) (faster)\n"
			  << "    -ispure    Assume the image contains only a 'pure'/perfect code (faster)\n"
			  << "    -1         Print only file name, text and status on one line per file/barcode\n"
			  << "    -escape    Escape non-graphical characters in angle brackets (ignored for -1 option, which always escapes)\n"
			  << "    -pngout    Write a copy of the input image with barcodes outlined by a red line\n"
			  << "\n"
			  << "Supported formats are:\n";
	for (auto f : BarcodeFormats::all()) {
		std::cout << "    " << ToString(f) << "\n";
	}
	std::cout << "Formats can be lowercase, with or without '-', separated by ',' and/or '|'\n";
}

static bool ParseOptions(int argc, char* argv[], DecodeHints& hints, bool& oneLine, bool& angleEscape,
						 std::vector<std::string>& filePaths, std::string& outPath)
{
	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "-fast") == 0) {
			hints.setTryHarder(false);
		} else if (strcmp(argv[i], "-norotate") == 0) {
			hints.setTryRotate(false);
		} else if (strcmp(argv[i], "-ispure") == 0) {
			hints.setIsPure(true);
			hints.setBinarizer(Binarizer::FixedThreshold);
		} else if (strcmp(argv[i], "-format") == 0) {
			if (++i == argc)
				return false;
			try {
				hints.setFormats(BarcodeFormatsFromString(argv[i]));
			} catch (const std::exception& e) {
				std::cerr << e.what() << "\n";
				return false;
			}
		} else if (strcmp(argv[i], "-1") == 0) {
			oneLine = true;
		} else if (strcmp(argv[i], "-escape") == 0) {
			angleEscape = true;
		} else if (strcmp(argv[i], "-pngout") == 0) {
			if (++i == argc)
				return false;
			outPath = argv[i];
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

void drawLine(const ImageView& image, PointI a, PointI b)
{
	int steps = maxAbsComponent(b - a);
	PointF dir = bresenhamDirection(PointF(b - a));
	for (int i = 0; i < steps; ++i) {
		auto p = centered(a + i * dir);
		*((uint32_t*)image.data(p.x, p.y)) = 0xff0000ff;
	}
}

void drawRect(const ImageView& image, const Position& pos)
{
	for (int i = 0; i < 4; ++i)
		drawLine(image, pos[i], pos[(i + 1) % 4]);
}

int main(int argc, char* argv[])
{
	DecodeHints hints;
	std::vector<std::string> filePaths;
	std::string outPath;
	bool oneLine = false;
	bool angleEscape = false;
	int ret = 0;

	if (!ParseOptions(argc, argv, hints, oneLine, angleEscape, filePaths, outPath)) {
		PrintUsage(argv[0]);
		return -1;
	}

	hints.setEanAddOnSymbol(EanAddOnSymbol::Read);

	if (oneLine)
		angleEscape = true;

	if (angleEscape)
		std::setlocale(LC_CTYPE, "en_US.UTF-8"); // Needed so `std::iswgraph()` in `ToUtf8(angleEscape)` does not 'swallow' all printable non-ascii utf8 chars

	for (const auto& filePath : filePaths) {
		int width, height, channels;
		std::unique_ptr<stbi_uc, void(*)(void*)> buffer(stbi_load(filePath.c_str(), &width, &height, &channels, 4), stbi_image_free);
		if (buffer == nullptr) {
			std::cerr << "Failed to read image: " << filePath << "\n";
			return -1;
		}

		ImageView image{buffer.get(), width, height, ImageFormat::RGBX};
		auto results = ReadBarcodes(image, hints);

		// if we did not find anything, insert a dummy to produce some output for each file
		if (results.empty())
			results.emplace_back(DecodeStatus::NotFound);

		for (auto&& result : results) {

			if (!outPath.empty())
				drawRect(image, result.position());

			ret |= static_cast<int>(result.status());

			if (oneLine) {
				std::cout << filePath << " " << ToString(result.format());
				if (result.isValid())
					std::cout << " \"" << ToUtf8(result.text(), angleEscape) << "\"";
				else if (result.format() != BarcodeFormat::None)
					std::cout << " " << ToString(result.status());
				std::cout << "\n";
				continue;
			}

			if (filePaths.size() > 1 || results.size() > 1) {
				static bool firstFile = true;
				if (!firstFile)
					std::cout << "\n";
				if (filePaths.size() > 1)
					std::cout << "File:     " << filePath << "\n";
				firstFile = false;
			}
			std::cout << "Text:     \"" << ToUtf8(result.text(), angleEscape) << "\"\n"
					  << "Format:   " << ToString(result.format()) << "\n"
					  << "Position: " << result.position() << "\n"
					  << "Rotation: " << result.orientation() << " deg\n"
					  << "Error:    " << ToString(result.status()) << "\n";

			auto printOptional = [](const char* key, const std::string& v) {
				if (!v.empty())
					std::cout << key << v << "\n";
			};

			printOptional("EC Level: ", ToUtf8(result.ecLevel()));

			if (result.lineCount())
				std::cout << "Lines:    " << result.lineCount() << "\n";

			if ((BarcodeFormat::EAN13 | BarcodeFormat::EAN8 | BarcodeFormat::UPCA | BarcodeFormat::UPCE)
					.testFlag(result.format())) {
				printOptional("Country:  ", GTIN::LookupCountryIdentifier(ToUtf8(result.text()), result.format()));
				printOptional("Add-On:   ", GTIN::EanAddOn(result));
				printOptional("Price:    ", GTIN::Price(GTIN::EanAddOn(result)));
				printOptional("Issue #:  ", GTIN::IssueNr(GTIN::EanAddOn(result)));
			} else if (result.format() == BarcodeFormat::ITF && result.text().length() == 14) {
				printOptional("Country:  ", GTIN::LookupCountryIdentifier(ToUtf8(result.text()), result.format()));
			}

			if (result.isPartOfSequence())
				std::cout << "Structured Append: symbol " << result.sequenceIndex() + 1 << " of "
						  << result.sequenceSize() << " (parity/id: '" << result.sequenceId() << "')\n";

			if (result.readerInit())
				std::cout << "Reader Initialisation/Programming\n";
		}

		if (Size(filePaths) == 1 && !outPath.empty())
			stbi_write_png(outPath.c_str(), image.width(), image.height(), 4, image.data(0, 0), image.rowStride());

	}

	return ret;
}
