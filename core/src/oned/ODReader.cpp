/*
* Copyright 2016 Nu-book Inc.
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

#include <unordered_set>
#include <algorithm>

namespace ZXing {
namespace OneD {

Reader::Reader(const DecodeHints& hints) :
	_tryHarder(hints.shouldTryHarder()),
	_tryRotate(hints.shouldTryRotate())
{
	_readers.reserve(8);

	auto possibleFormats = hints.possibleFormats();
	std::unordered_set<BarcodeFormat, BarcodeFormatHasher> formats(possibleFormats.begin(), possibleFormats.end());

	if (formats.empty()) {
		_readers.emplace_back(new MultiUPCEANReader(hints));
		_readers.emplace_back(new Code39Reader(hints));
		_readers.emplace_back(new CodabarReader(hints));
		_readers.emplace_back(new Code93Reader());
		_readers.emplace_back(new Code128Reader(hints));
		_readers.emplace_back(new ITFReader(hints));
		_readers.emplace_back(new RSS14Reader());
		_readers.emplace_back(new RSSExpandedReader());
	}
	else {
		if (formats.find(BarcodeFormat::EAN_13) != formats.end() ||
			formats.find(BarcodeFormat::UPC_A) != formats.end() ||
			formats.find(BarcodeFormat::EAN_8) != formats.end() ||
			formats.find(BarcodeFormat::UPC_E) != formats.end()) {
			_readers.emplace_back(new MultiUPCEANReader(hints));
		}
		if (formats.find(BarcodeFormat::CODE_39) != formats.end()) {
			_readers.emplace_back(new Code39Reader(hints));
		}
		if (formats.find(BarcodeFormat::CODE_93) != formats.end()) {
			_readers.emplace_back(new Code93Reader());
		}
		if (formats.find(BarcodeFormat::CODE_128) != formats.end()) {
			_readers.emplace_back(new Code128Reader(hints));
		}
		if (formats.find(BarcodeFormat::ITF) != formats.end()) {
			_readers.emplace_back(new ITFReader(hints));
		}
		if (formats.find(BarcodeFormat::CODABAR) != formats.end()) {
			_readers.emplace_back(new CodabarReader(hints));
		}
		if (formats.find(BarcodeFormat::RSS_14) != formats.end()) {
			_readers.emplace_back(new RSS14Reader());
		}
		if (formats.find(BarcodeFormat::RSS_EXPANDED) != formats.end()) {
			_readers.emplace_back(new RSSExpandedReader());
		}
	}
}

Reader::~Reader()
{
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
DoDecode(const std::vector<std::unique_ptr<RowReader>>& readers, const BinaryBitmap& image, bool tryHarder)
{
	std::vector<std::unique_ptr<RowReader::DecodingState>> decodingState(readers.size());

	int width = image.width();
	int height = image.height();

	int middle = height >> 1;
	int rowStep = std::max(1, height >> (tryHarder ? 8 : 5));
	int maxLines = tryHarder ?
		height :	// Look at the whole image, not just the center
		15;			// 15 rows spaced 1/32 apart is roughly the middle half of the image

	BitArray row(width);
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
		if (!image.getBlackRow(rowNumber, row)) {
			continue;
		}

		// While we have the image data in a BitArray, it's fairly cheap to reverse it in place to
		// handle decoding upside down barcodes.
		for (bool upsideDown : {false, true}) {
			// trying again?
			if (upsideDown) {
				// reverse the row and continue
				row.reverse();
			}
			// Look for a barcode
			for (size_t r = 0; r < readers.size(); ++r) {
				Result result = readers[r]->decodeRow(rowNumber, row, decodingState[r]);
				if (result.isValid()) {
					// We found our barcode
					if (upsideDown) {
						// But it was upside down, so note that
						result.metadata().put(ResultMetadata::ORIENTATION, 180);
						// And remember to flip the result points horizontally.
						auto points = result.resultPoints();
						for (auto& p : points) {
							p.set(width - p.x() - 1, p.y());
						}
						result.setResultPoints(std::move(points));
					}
					return result;
				}
			}
		}
	}
	return Result(DecodeStatus::NotFound);
}

// Note that we don't try rotation without the try harder flag, even if rotation was supported.
Result
Reader::decode(const BinaryBitmap& image) const
{
	Result result = DoDecode(_readers, image, _tryHarder);
	if (result.isValid()) {
		return result;
	}

	if (_tryRotate && image.canRotate()) {
		auto rotatedImage = image.rotated(270);
		result = DoDecode(_readers, *rotatedImage, _tryHarder);
		if (result.isValid()) {
			// Record that we found it rotated 90 degrees CCW / 270 degrees CW
			auto& metadata = result.metadata();
			metadata.put(ResultMetadata::ORIENTATION, (270 + metadata.getInt(ResultMetadata::ORIENTATION)) % 360);
			// Update result points
			auto points = result.resultPoints();
			int height = rotatedImage->height();
			for (auto& p : points) {
				p.set(height - p.y() - 1, p.x());
			}
			result.setResultPoints(std::move(points));
		}
	}
	return result;
}


} // OneD
} // ZXing
