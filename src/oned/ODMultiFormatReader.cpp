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

#include "oned/ODMultiFormatReader.h"
#include "DecodeHints.h"
#include "BarcodeFormat.h"
#include "Result.h"

#include <unordered_set>

namespace ZXing {

namespace OneD {

MultiFormatReader::MultiFormatReader(const DecodeHints* hints)
{
	if (hints != nullptr)
	{
		auto possibleFormats = hints->getFormatList(DecodeHint::POSSIBLE_FORMATS);
		if (!possibleFormats.empty())
		{
			std::unordered_set<BarcodeFormat> formats(possibleFormats.begin(), possibleFormats.end());
			bool useCode39CheckDigit = hints->getFlag(DecodeHint::ASSUME_CODE_39_CHECK_DIGIT);

			if (formats.find(BarcodeFormat::EAN_13) != formats.end() ||
				formats.find(BarcodeFormat::UPC_A) != formats.end() ||
				formats.find(BarcodeFormat::EAN_8) != formats.end() ||
				formats.find(BarcodeFormat::UPC_E) != formats.end()) {
				_readers.push_back(std::make_shared<MultiFormatUPCEANReader>(hints));
			}
			if (formats.find(BarcodeFormat::CODE_39) != formats.end()) {
				_readers.push_back(std::make_shared<Code39Reader>(useCode39CheckDigit));
			}
			if (formats.find(BarcodeFormat::CODE_93) != formats.end()) {
				_readers.push_back(std::make_shared<Code93Reader>());
			}
			if (formats.find(BarcodeFormat::CODE_128) != formats.end()) {
				_readers.push_back(std::make_shared<Code128Reader>());
			}
			if (formats.find(BarcodeFormat::ITF) != formats.end()) {
				_readers.push_back(std::make_shared<ITFReader>());
			}
			if (formats.find(BarcodeFormat::CODABAR) != formats.end()) {
				_readers.push_back(std::make_shared<CodaBarReader>());
			}
			if (formats.find(BarcodeFormat::RSS_14) != formats.end()) {
				_readers.push_back(std::make_shared<RSS14Reader>());
			}
			if (formats.find(BarcodeFormat::RSS_EXPANDED) != formats.end()) {
				_readers.push_back(std::make_shared<RSSExpandedReader>());
			}
		}
	}

	if (_readers.empty()) {
		_readers.push_back(std::make_shared<MultiFormatUPCEANReader>(hints));
		_readers.push_back(std::make_shared<Code39Reader>());
		_readers.push_back(std::make_shared<CodaBarReader>());
		_readers.push_back(std::make_shared<Code93Reader>());
		_readers.push_back(std::make_shared<Code128Reader>());
		_readers.push_back(std::make_shared<ITFReader>());
		_readers.push_back(std::make_shared<RSS14Reader>());
		_readers.push_back(std::make_shared<RSSExpandedReader>());
	}
}

Result
MultiFormatReader::decodeRow(int rowNumber, const BitArray& row, const DecodeHints* hints) const
{
	for (auto reader : _readers) {
		Result r = reader->decodeRow(rowNumber, row, hints);
		if (r.isValid()) {
			return r;
		}
	}
	return Result();
}


} // OneD
} // ZXing
