#pragma once
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

#include "MultiFormatReader.h"
#include "DecodeHints.h"
#include "BarcodeFormat.h"
#include "Result.h"

#include "oned/ODMultiFormatReader.h"
#include "qrcode/QRReader.h"

#include <vector>
#include <memory>
#include <unordered_set>

namespace ZXing {

namespace {

std::vector<std::shared_ptr<Reader>>
BuildReaders(const DecodeHints* hints)
{
	std::vector<std::shared_ptr<Reader>> readers;
	bool tryHarder = false;
	if (hints != nullptr) {
		tryHarder = hints->getFlag(DecodeHint::TRY_HARDER);
		auto possibleFormats = hints->getFormatList(DecodeHint::POSSIBLE_FORMATS);
		if (!possibleFormats.empty()) {
			std::unordered_set<BarcodeFormat> formats(possibleFormats.begin(), possibleFormats.end());
			bool addOneDReader =
				formats.find(BarcodeFormat::UPC_A) != formats.end() ||
				formats.find(BarcodeFormat::UPC_E) != formats.end() ||
				formats.find(BarcodeFormat::EAN_13) != formats.end() ||
				formats.find(BarcodeFormat::EAN_8) != formats.end() ||
				formats.find(BarcodeFormat::CODABAR) != formats.end() ||
				formats.find(BarcodeFormat::CODE_39) != formats.end() ||
				formats.find(BarcodeFormat::CODE_93) != formats.end() ||
				formats.find(BarcodeFormat::CODE_128) != formats.end() ||
				formats.find(BarcodeFormat::ITF) != formats.end() ||
				formats.find(BarcodeFormat::RSS_14) != formats.end() ||
				formats.find(BarcodeFormat::RSS_EXPANDED) != formats.end();

			// Put 1D readers upfront in "normal" mode
			if (addOneDReader && !tryHarder) {
				readers.push_back(std::make_shared<OneD::MultiFormatReader>(hints));
			}
			if (formats.find(BarcodeFormat::QR_CODE) != formats.end()) {
				readers.push_back(std::make_shared<QRCode::Reader>());
			}
			if (formats.find(BarcodeFormat::DATA_MATRIX) != formats.end()) {
				readers.push_back(std::make_shared<DataMatrixReader>());
			}
			if (formats.find(BarcodeFormat::AZTEC) != formats.end()) {
				readers.push_back(std::make_shared<AztecReader>());
			}
			if (formats.find(BarcodeFormat::PDF_417) != formats.end()) {
				readers.push_back(std::make_shared<PDF417Reader>());
			}
			if (formats.find(BarcodeFormat::MAXICODE) != formats.end()) {
				readers.push_back(std::make_shared<MaxiCodeReader>());
			}
			// At end in "try harder" mode
			if (addOneDReader && tryHarder) {
				readers.push_back(std::make_shared<OneD::MultiFormatReader>(hints));
			}
		}
	}

	if (readers.empty()) {
		if (!tryHarder) {
			readers.push_back(std::make_shared<OneD::MultiFormatReader>(hints));
		}
		readers.push_back(std::make_shared<QRCodeReader>());
		readers.push_back(std::make_shared<DataMatrixReader>());
		readers.push_back(std::make_shared<AztecReader>());
		readers.push_back(std::make_shared<PDF417Reader>());
		readers.push_back(std::make_shared<MaxiCodeReader>());
		if (tryHarder) {
			readers.push_back(std::make_shared<OneD::MultiFormatReader>(hints));
		}
	}
	return readers;
}

} // anonymous

Result
MultiFormatReader::decode(const BinaryBitmap& image, const DecodeHints* hints) const
{
	auto readers = BuildReaders(hints);
	for (const auto& reader : readers) {
		Result r = reader->decode(image, hints);
		if (r.isValid())
			return r;
	}
	return Result(ErrorStatus::NotFound);
}

};

} // ZXing
