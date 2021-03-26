/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
* Copyright 2020 Axel Waggershauser
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

#include "BinaryBitmap.h"
#include "BitArray.h"
#include "DecodeHints.h"
#include "ODCodabarReader.h"
#include "ODCode128Reader.h"
#include "ODCode39Reader.h"
#include "ODCode93Reader.h"
#include "ODDataBarExpandedReader.h"
#include "ODDataBarReader.h"
#include "ODITFReader.h"
#include "ODMultiUPCEANReader.h"
#include "Result.h"

#include <algorithm>
#include <utility>

namespace ZXing::OneD {

Reader::Reader(const DecodeHints& hints) :
	_tryHarder(hints.tryHarder()),
	_tryRotate(hints.tryRotate()),
	_isPure(hints.isPure())
{
	_readers.reserve(8);

	auto formats = hints.formats().empty() ? BarcodeFormat::Any : hints.formats();

	if (formats.testFlags(BarcodeFormat::EAN13 | BarcodeFormat::UPCA | BarcodeFormat::EAN8 | BarcodeFormat::UPCE))
		_readers.emplace_back(new MultiUPCEANReader(hints));

	if (formats.testFlag(BarcodeFormat::Code39))
		_readers.emplace_back(new Code39Reader(hints));
	if (formats.testFlag(BarcodeFormat::Code93))
		_readers.emplace_back(new Code93Reader());
	if (formats.testFlag(BarcodeFormat::Code128))
		_readers.emplace_back(new Code128Reader(hints));
	if (formats.testFlag(BarcodeFormat::ITF))
		_readers.emplace_back(new ITFReader(hints));
	if (formats.testFlag(BarcodeFormat::Codabar))
		_readers.emplace_back(new CodabarReader(hints));
	if (formats.testFlags(BarcodeFormat::DataBar))
		_readers.emplace_back(new DataBarReader(hints));
	if (formats.testFlags(BarcodeFormat::DataBarExpanded))
		_readers.emplace_back(new DataBarExpandedReader(hints));
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
DoDecode(const std::vector<std::unique_ptr<RowReader>>& readers, const BinaryBitmap& image, bool tryHarder, bool isPure)
{
	std::vector<std::unique_ptr<RowReader::DecodingState>> decodingState(readers.size());

	int width = image.width();
	int height = image.height();

	int middle = height / 2;
	int rowStep = std::max(1, height / (tryHarder ? 256 : 32));
	int maxLines = tryHarder ?
		height :	// Look at the whole image, not just the center
		15;			// 15 rows spaced 1/32 apart is roughly the middle half of the image

	PatternRow bars;
	bars.reserve(128); // e.g. EAN-13 has 96 bars

	for (int i = 0; i < maxLines; i++) {

		// Scanning from the middle out. Determine which row we're looking at next:
		int rowStepsAboveOrBelow = (i + 1) / 2;
		bool isAbove = (i & 0x01) == 0; // i.e. is x even?
		int rowNumber = middle + rowStep * (isAbove ? rowStepsAboveOrBelow : -rowStepsAboveOrBelow);
		if (rowNumber < 0 || rowNumber >= height) {
			// Oops, if we run off the top or bottom, stop
			break;
		}

		if (!image.getPatternRow(rowNumber, bars))
			continue;

		// While we have the image data in a PatternRow, it's fairly cheap to reverse it in place to
		// handle decoding upside down barcodes.
		// Note: the DataBarExpanded decoder depends on seeing each line from both directions. This
		// 'surprising' and inconsistent. It also requires the decoderState to be shared between
		// normal and reversed scans, which makes no sense in general because it would mix partial
		// detection data from two codes of the same type next to each other. TODO..
		// See also https://github.com/nu-book/zxing-cpp/issues/87
		for (bool upsideDown : {false, true}) {
			// trying again?
			if (upsideDown) {
				// reverse the row and continue
				std::reverse(bars.begin(), bars.end());
			}
			// Look for a barcode
			for (size_t r = 0; r < readers.size(); ++r) {
				Result result = readers[r]->decodePattern(rowNumber, bars, decodingState[r]);
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

		// If this is a pure symbol, then checking a single non-empty line is sufficient
		if (isPure)
			break;
	}
	return Result(DecodeStatus::NotFound);
}

Result
Reader::decode(const BinaryBitmap& image) const
{
	Result result = DoDecode(_readers, image, _tryHarder, _isPure);

	if (!result.isValid() && _tryRotate && image.canRotate()) {
		auto rotatedImage = image.rotated(270);
		result = DoDecode(_readers, *rotatedImage, _tryHarder, _isPure);
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

	return result;
}

} // namespace ZXing::OneD
