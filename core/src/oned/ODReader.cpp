/*
* Copyright 2016 ZXing authors
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

#include "oned/ODReader.h"
#include "oned/ODMultiUPCEANReader.h"
#include "oned/ODCode39Reader.h"
#include "oned/ODCode93Reader.h"
#include "oned/ODCode128Reader.h"
#include "oned/ODITFReader.h"
#include "oned/ODCodabarReader.h"
#include "oned/ODRSS14Reader.h"
#include "oned/ODRSSExpandedReader.h"
#include "Result.h"
#include "BitArray.h"
#include "BinaryBitmap.h"
#include "DecodeHints.h"

namespace ZXing {
namespace OneD {


Reader::Reader(const DecodeHints* hints)
{
	if (hints != nullptr)
	{
		auto possibleFormats = hints->getFormatList(DecodeHint::POSSIBLE_FORMATS);
		if (!possibleFormats.empty())
		{
			_formats.insert(possibleFormats.begin(), possibleFormats.end());
		}
	}
	if (_formats.empty()) {
		_formats.insert({
			BarcodeFormat::EAN_13,
			BarcodeFormat::CODE_39,
			BarcodeFormat::CODE_93,
			BarcodeFormat::CODE_128,
			BarcodeFormat::ITF,
			BarcodeFormat::CODABAR,
			BarcodeFormat::RSS_14,
			BarcodeFormat::RSS_EXPANDED,
		});
	}
}

static void
BuildReaders(const std::unordered_set<BarcodeFormat>& formats, std::vector<std::shared_ptr<RowReader>>& readers)
{
	readers.reserve(8);
	if (formats.empty()) {
		readers.insert(readers.end(), {
			std::make_shared<MultiUPCEANReader>(formats),
			std::make_shared<Code39Reader>(),
			std::make_shared<CodabarReader>(),
			std::make_shared<Code93Reader>(),
			std::make_shared<Code128Reader>(),
			std::make_shared<ITFReader>(),
			std::make_shared<RSS14Reader>(),
			std::make_shared<RSSExpandedReader>(),
		});
	}
	else {
		if (formats.find(BarcodeFormat::EAN_13) != formats.end() ||
			formats.find(BarcodeFormat::UPC_A) != formats.end() ||
			formats.find(BarcodeFormat::EAN_8) != formats.end() ||
			formats.find(BarcodeFormat::UPC_E) != formats.end()) {
			readers.push_back(std::make_shared<MultiUPCEANReader>(formats));
		}
		if (formats.find(BarcodeFormat::CODE_39) != formats.end()) {
			readers.push_back(std::make_shared<Code39Reader>());
		}
		if (formats.find(BarcodeFormat::CODE_93) != formats.end()) {
			readers.push_back(std::make_shared<Code93Reader>());
		}
		if (formats.find(BarcodeFormat::CODE_128) != formats.end()) {
			readers.push_back(std::make_shared<Code128Reader>());
		}
		if (formats.find(BarcodeFormat::ITF) != formats.end()) {
			readers.push_back(std::make_shared<ITFReader>());
		}
		if (formats.find(BarcodeFormat::CODABAR) != formats.end()) {
			readers.push_back(std::make_shared<CodabarReader>());
		}
		if (formats.find(BarcodeFormat::RSS_14) != formats.end()) {
			readers.push_back(std::make_shared<RSS14Reader>());
		}
		if (formats.find(BarcodeFormat::RSS_EXPANDED) != formats.end()) {
			readers.push_back(std::make_shared<RSSExpandedReader>());
		}
	}
}

/**
* We're going to examine rows from the middle outward, searching alternately above and below the
* middle, and farther out each time. rowStep is the number of rows between each successive
* attempt above and below the middle. So we'd scan row middle, then middle - rowStep, then
* middle + rowStep, then middle - (2 * rowStep), etc.
* rowStep is bigger as the image is taller, but is always at least 1. We've somewhat arbitrarily
* decided that moving up and down by about 1/16 of the image is pretty good; we try more of the
* image if "trying harder".
*
* @param image The image to decode
* @param hints Any hints that were requested
* @return The contents of the decoded barcode
* @throws NotFoundException Any spontaneous errors which occur
*/
static Result
DoDecode(const std::vector<std::shared_ptr<RowReader>>& readers, const BinaryBitmap& image, const DecodeHints* hints)
{
	int width = image.width();
	int height = image.height();

	int middle = height >> 1;
	bool tryHarder = hints != nullptr && hints->getFlag(DecodeHint::TRY_HARDER);
	int rowStep = std::max(1, height >> (tryHarder ? 8 : 5));
	int maxLines = tryHarder ?
		height :	// Look at the whole image, not just the center
		15;			// 15 rows spaced 1/32 apart is roughly the middle half of the image

	BitArray row(width);
	DecodeHints copyHints;
	const DecodeHints* currentHints = hints;
	for (int x = 0; x < maxLines; x++) {

		// Scanning from the middle out. Determine which row we're looking at next:
		int rowStepsAboveOrBelow = (x + 1) / 2;
		bool isAbove = (x & 0x01) == 0; // i.e. is x even?
		int rowNumber = middle + rowStep * (isAbove ? rowStepsAboveOrBelow : -rowStepsAboveOrBelow);
		if (rowNumber < 0 || rowNumber >= height) {
			// Oops, if we run off the top or bottom, stop
			break;
		}

		// Estimate black point for this row and load it:
		auto status = image.getBlackRow(rowNumber, row);
		if (StatusIsError(status))
			continue;

		// While we have the image data in a BitArray, it's fairly cheap to reverse it in place to
		// handle decoding upside down barcodes.
		for (int attempt = 0; attempt < 2; attempt++) {
			if (attempt == 1) { // trying again?
				row.reverse(); // reverse the row and continue
							   // This means we will only ever draw result points *once* in the life of this method
							   // since we want to avoid drawing the wrong points after flipping the row, and,
							   // don't want to clutter with noise from every single row scan -- just the scans
							   // that start on the center line.
				if (currentHints != nullptr && currentHints->getPointCallback(DecodeHint::NEED_RESULT_POINT_CALLBACK) != nullptr) {
					copyHints = *currentHints;
					copyHints.remove(DecodeHint::NEED_RESULT_POINT_CALLBACK);
					currentHints = &copyHints;
				}
			}
			// Look for a barcode
			for (auto& reader : readers) {
				Result result = reader->decodeRow(rowNumber, row, currentHints);
				if (result.isValid()) {
					// We found our barcode
					if (attempt == 1) {
						// But it was upside down, so note that
						result.metadata().put(ResultMetadata::ORIENTATION, 180);
						// And remember to flip the result points horizontally.
						auto points = result.resultPoints();
						if (!points.empty()) {
							for (auto& p : points) {
								p = ResultPoint(width - p.x() - 1, p.y());
							}
							result.setResultPoints(points);
						}
					}
					return result;
				}
			}
		}
	}
	return Result(ErrorStatus::NotFound);
}

// Note that we don't try rotation without the try harder flag, even if rotation was supported.
Result
Reader::decode(const BinaryBitmap& image, const DecodeHints* hints) const
{
	std::vector<std::shared_ptr<RowReader>> readers;
	BuildReaders(_formats, readers);

	Result result = DoDecode(readers, image, hints);
	if (result.isValid()) {
		return result;
	}

	bool tryHarder = hints != nullptr && hints->getFlag(DecodeHint::TRY_HARDER);
	if (tryHarder && image.canRotate()) {
		BinaryBitmap rotatedImage = image.rotatedCCW90();
		result = DoDecode(readers, rotatedImage, hints);
		if (result.isValid()) {
			// Record that we found it rotated 90 degrees CCW / 270 degrees CW
			auto& metadata = result.metadata();
			metadata.put(ResultMetadata::ORIENTATION, (270 + metadata.getInt(ResultMetadata::ORIENTATION)) % 360);
			// Update result points
			auto points = result.resultPoints();
			if (!points.empty()) {
				int height = rotatedImage.height();
				for (auto& p : points) {
					p = ResultPoint(height - p.y() - 1, p.x());
				}
				result.setResultPoints(points);
			}
		}
	}
	return result;
}


} // OneD
} // ZXing
