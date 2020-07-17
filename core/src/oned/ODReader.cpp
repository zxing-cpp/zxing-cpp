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

#include "ODReader.h"
#include "ODMultiUPCEANReader.h"
#include "ODCode39Reader.h"
#include "ODCode93Reader.h"
#include "ODCode128Reader.h"
#include "ODITFReader.h"
#include "ODCodabarReader.h"
#include "ODRSS14Reader.h"
#include "ODRSSExpandedReader.h"
#include "Result.h"
#include "BitArray.h"
#include "BinaryBitmap.h"
#include "DecodeHints.h"

#include <algorithm>
#include <utility>

namespace ZXing {
namespace OneD {

Reader::Reader(const DecodeHints& hints) :
	_tryHarder(hints.tryHarder()),
	_tryRotate(hints.tryRotate())
{
	_readers.reserve(8);

	if (hints.hasNoFormat()) {
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
		if (hints.hasFormat(BarcodeFormat::EAN_13) ||
			hints.hasFormat(BarcodeFormat::UPC_A) ||
			hints.hasFormat(BarcodeFormat::EAN_8) ||
			hints.hasFormat(BarcodeFormat::UPC_E)) {
			_readers.emplace_back(new MultiUPCEANReader(hints));
		}
		if (hints.hasFormat(BarcodeFormat::CODE_39)) {
			_readers.emplace_back(new Code39Reader(hints));
		}
		if (hints.hasFormat(BarcodeFormat::CODE_93)) {
			_readers.emplace_back(new Code93Reader());
		}
		if (hints.hasFormat(BarcodeFormat::CODE_128)) {
			_readers.emplace_back(new Code128Reader(hints));
		}
		if (hints.hasFormat(BarcodeFormat::ITF)) {
			_readers.emplace_back(new ITFReader(hints));
		}
		if (hints.hasFormat(BarcodeFormat::CODABAR)) {
			_readers.emplace_back(new CodabarReader(hints));
		}
		if (hints.hasFormat(BarcodeFormat::RSS_14)) {
			_readers.emplace_back(new RSS14Reader());
		}
		if (hints.hasFormat(BarcodeFormat::RSS_EXPANDED)) {
			_readers.emplace_back(new RSSExpandedReader());
		}
	}
}

Reader::~Reader() = default;

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

	int middle = height / 2;
	int rowStep = std::max(1, height / (tryHarder ? 256 : 32));
	int maxLines = tryHarder ?
		height :	// Look at the whole image, not just the center
		15;			// 15 rows spaced 1/32 apart is roughly the middle half of the image

	BitArray row(width);
#ifdef ZX_USE_NEW_ROW_READERS
	PatternRow bars;
	bars.reserve(128); // e.g. EAN-13 has 96 bars
#endif
	for (int i = 0; i < maxLines; i++) {

		// Scanning from the middle out. Determine which row we're looking at next:
		int rowStepsAboveOrBelow = (i + 1) / 2;
		bool isAbove = (i & 0x01) == 0; // i.e. is x even?
		int rowNumber = middle + rowStep * (isAbove ? rowStepsAboveOrBelow : -rowStepsAboveOrBelow);
		if (rowNumber < 0 || rowNumber >= height) {
			// Oops, if we run off the top or bottom, stop
			break;
		}

#ifdef ZX_USE_NEW_ROW_READERS
		image.getPatternRow(rowNumber, bars);
		bool hasBitArray = false;
#else
		// Estimate black point for this row and load it:
		if (!image.getBlackRow(rowNumber, row)) {
			continue;
		}
#endif

		// While we have the image data in a BitArray, it's fairly cheap to reverse it in place to
		// handle decoding upside down barcodes.
		// Note: the RSSExpanded decoder depends on seeing each line from both directions. This
		// 'surprising' and inconsistent. It also requires the decoderState to be shared between
		// normal and reversed scans, which makes no sense in general because it would mix partial
		// detetection data from two codes of the same type next to each other. TODO..
		// See also https://github.com/nu-book/zxing-cpp/issues/87
		for (bool upsideDown : {false, true}) {
			// trying again?
			if (upsideDown) {
				// reverse the row and continue
				row.reverse();
#ifdef ZX_USE_NEW_ROW_READERS
				std::reverse(bars.begin(), bars.end());
#endif
			}
			// Look for a barcode
			for (size_t r = 0; r < readers.size(); ++r) {
#ifdef ZX_USE_NEW_ROW_READERS
				Result result = readers[r]->decodePattern(rowNumber, bars, decodingState[r]);
				if (result.status() == DecodeStatus::_internal) {
					if (!std::exchange(hasBitArray, true)) {
						row.clearBits();
						bool set = false;
						int pos = 0;
						for(int w : bars) {
							if (set)
								for (int i = 0; i < w; ++i)
									row.set(pos++);
							else
								pos += w;
							set = !set;
						}
					}
					result = readers[r]->decodeRow(rowNumber, row, decodingState[r]);
				}
#else
				Result result = readers[r]->decodeRow(rowNumber, row, decodingState[r]);
#endif
				if (result.isValid()) {
					if (upsideDown) {
						// update position (flip horizontally).
						auto points = result.position();
						for (auto& p : points) {
							p = {width - p.x - 1, p.y};
						}
						result.setPosition(std::move(points));
					}
					return result;
				}
			}
		}
	}
	return Result(DecodeStatus::NotFound);
}

Result
Reader::decode(const BinaryBitmap& image) const
{
	Result result = DoDecode(_readers, image, _tryHarder);

	if (!result.isValid() && _tryRotate && image.canRotate()) {
		auto rotatedImage = image.rotated(270);
		result = DoDecode(_readers, *rotatedImage, _tryHarder);
		if (result.isValid()) {
			// Update position
			auto points = result.position();
			int height = rotatedImage->height();
			for (auto& p : points) {
				p = {height - p.y - 1, p.x};
			}
			result.setPosition(std::move(points));
		}
	}

	result.metadata().put(ResultMetadata::ORIENTATION, result.orientation());

	return result;
}


} // OneD
} // ZXing
