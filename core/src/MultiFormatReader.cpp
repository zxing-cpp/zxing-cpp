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

#include "oned/ODReader.h"
#include "qrcode/QRReader.h"
#include "datamatrix/DMReader.h"
#include "aztec/AZReader.h"
#include "maxicode/MCReader.h"
#include "pdf417/PDFReader.h"

#include <vector>
#include <memory>
#include <unordered_set>

namespace ZXing {

MultiFormatReader::MultiFormatReader(const DecodeHints& hints) :
	_hints(hints)
{
	_readers.reserve(6);
	bool tryHarder = hints.shouldTryHarder();
	auto possibleFormats = hints.possibleFormats();
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
			_readers.push_back(std::make_shared<OneD::Reader>(hints));
		}
		if (formats.find(BarcodeFormat::QR_CODE) != formats.end()) {
			_readers.push_back(std::make_shared<QRCode::Reader>(hints));
		}
		if (formats.find(BarcodeFormat::DATA_MATRIX) != formats.end()) {
			_readers.push_back(std::make_shared<DataMatrix::Reader>());
		}
		if (formats.find(BarcodeFormat::AZTEC) != formats.end()) {
			_readers.push_back(std::make_shared<Aztec::Reader>());
		}
		if (formats.find(BarcodeFormat::PDF_417) != formats.end()) {
			_readers.push_back(std::make_shared<Pdf417::Reader>());
		}
		if (formats.find(BarcodeFormat::MAXICODE) != formats.end()) {
			_readers.push_back(std::make_shared<MaxiCode::Reader>());
		}
		// At end in "try harder" mode
		if (addOneDReader && tryHarder) {
			_readers.push_back(std::make_shared<OneD::Reader>(hints));
		}
	}

	if (_readers.empty()) {
		if (!tryHarder) {
			_readers.push_back(std::make_shared<OneD::Reader>(hints));
		}
		_readers.push_back(std::make_shared<QRCode::Reader>(hints));
		_readers.push_back(std::make_shared<DataMatrix::Reader>());
		_readers.push_back(std::make_shared<Aztec::Reader>());
		_readers.push_back(std::make_shared<Pdf417::Reader>());
		_readers.push_back(std::make_shared<MaxiCode::Reader>());
		if (tryHarder) {
			_readers.push_back(std::make_shared<OneD::Reader>(hints));
		}
	}
}

Result
MultiFormatReader::read(const BinaryBitmap& image) const
{
	for (const auto& reader : _readers) {
		Result r = reader->decode(image);
		if (r.isValid())
			return r;
	}
	return Result(ErrorStatus::NotFound);
}

} // ZXing
