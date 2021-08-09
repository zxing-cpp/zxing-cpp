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

static bool IsIntersecting(QuadrilateralI a, QuadrilateralI b)
{
	bool x = b.topRight().x < a.topLeft().x || b.topLeft().x > a.topRight().x;
	bool y = b.bottomLeft().y < a.topLeft().y || b.topLeft().y > a.bottomLeft().y;
	return !(x || y);
}

/**
* We're going to examine rows from the middle outward, searching alternately above and below the
* middle, and farther out each time. rowStep is the number of rows between each successive
* attempt above and below the middle. So we'd scan row middle, then middle - rowStep, then
* middle + rowStep, then middle - (2 * rowStep), etc.
* rowStep is bigger as the image is taller, but is always at least 1. We've somewhat arbitrarily
* decided that moving up and down by about 1/16 of the image is pretty good; we try more of the
* image if "trying harder".
*/
static Results DoDecode(const std::vector<std::unique_ptr<RowReader>>& readers, const BinaryBitmap& image,
						bool tryHarder, bool rotate, bool isPure, int maxSymbols)
{
	Results res;

	std::vector<std::unique_ptr<RowReader::DecodingState>> decodingState(readers.size());

	int width = image.width();
	int height = image.height();

	if (rotate)
		std::swap(width, height);

	int middle = height / 2;
	// TODO: find a better heuristic/parameterization if maxSymbols != 1
	int rowStep = std::max(1, height / (tryHarder ? (maxSymbols == 1 ? 256 : 512) : 32));
	int maxLines = tryHarder ?
		height :	// Look at the whole image, not just the center
		15;			// 15 rows spaced 1/32 apart is roughly the middle half of the image

	PatternRow bars;
	bars.reserve(128); // e.g. EAN-13 has 59 bars/spaces

	for (int i = 0; i < maxLines; i++) {

		// Scanning from the middle out. Determine which row we're looking at next:
		int rowStepsAboveOrBelow = (i + 1) / 2;
		bool isAbove = (i & 0x01) == 0; // i.e. is x even?
		int rowNumber = middle + rowStep * (isAbove ? rowStepsAboveOrBelow : -rowStepsAboveOrBelow);
		if (rowNumber < 0 || rowNumber >= height) {
			// Oops, if we run off the top or bottom, stop
			break;
		}

		if (!image.getPatternRow(rowNumber, rotate ? 270 : 0, bars))
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
				PatternView next(bars);
				do {
					Result result = readers[r]->decodePattern(rowNumber, next, decodingState[r]);
					result.incrementLineCount();
					if (result.isValid()) {
						if (upsideDown) {
							// update position (flip horizontally).
							auto points = result.position();
							for (auto& p : points) {
								p = {width - p.x - 1, p.y};
							}
							result.setPosition(std::move(points));
						}
						if (rotate) {
							auto points = result.position();
							for (auto& p : points) {
								p = {height - p.y - 1, p.x};
							}
							result.setPosition(std::move(points));
						}

						// check if we know this code already
						for (auto& other : res) {
							if (other == result) {
								auto dTop = maxAbsComponent(other.position().topLeft() - result.position().topLeft());
								auto dBot = maxAbsComponent(other.position().bottomLeft() - result.position().topLeft());
								auto length = maxAbsComponent(other.position().topLeft() - other.position().bottomRight());
								// if the new line is less than half the length of the existing result away from the
								// latter, we consider it to belong to the same symbol
								if (std::min(dTop, dBot) < length / 2) {
									// if so, merge the position information
									auto points = other.position();
									if (dTop < dBot ||
										(dTop == dBot && rotate ^ (sumAbsComponent(points[0]) >
																   sumAbsComponent(result.position()[0])))) {
										points[0] = result.position()[0];
										points[1] = result.position()[1];
									} else {
										points[2] = result.position()[2];
										points[3] = result.position()[3];
									}
									other.setPosition(points);
									other.incrementLineCount();
									// clear the result below, so we don't insert it again
									result = Result(DecodeStatus::NotFound);
								}
							}
						}

						if (result.isValid()) {
							res.push_back(std::move(result));
							if (maxSymbols && Size(res) == maxSymbols)
								goto out;
						}
					}
					// make sure we make progress and we start the next try on a bar
					next.shift(2 - (next.index() % 2));
					next.extend();
				} while (tryHarder && next.isValid());
			}
		}

		// If this is a pure symbol, then checking a single non-empty line is sufficient
		if (isPure)
			break;
	}

out:
	// if symbols overlap, remove the one with a lower line count
	for (auto a = res.begin(); a != res.end(); ++a)
		for (auto b = std::next(a); b != res.end(); ++b)
			if (IsIntersecting(a->position(), b->position()))
				*(a->lineCount() < b->lineCount() ? a : b) = Result(DecodeStatus::NotFound);

	//TODO: C++20 res.erase_if()
	auto it = std::remove_if(res.begin(), res.end(), [](auto&& r) { return r.status() == DecodeStatus::NotFound; });
	res.erase(it, res.end());

	return res;
}

Result
Reader::decode(const BinaryBitmap& image) const
{
	auto result = DoDecode(_readers, image, _tryHarder, false, _isPure, 1);

	if (result.empty() && _tryRotate)
		result = DoDecode(_readers, image, _tryHarder, true, _isPure, 1);

	return result.empty() ? Result(DecodeStatus::NotFound) : result.front();
}

Results Reader::decode(const BinaryBitmap& image, int maxSymbols) const
{
	auto resH = DoDecode(_readers, image, _tryHarder, false, _isPure, maxSymbols);
	if ((!maxSymbols || Size(resH) < maxSymbols) && _tryRotate) {
		auto resV = DoDecode(_readers, image, _tryHarder, true, _isPure, maxSymbols);
		resH.insert(resH.end(), resV.begin(), resV.end());
	}
	return resH;
}

} // namespace ZXing::OneD
