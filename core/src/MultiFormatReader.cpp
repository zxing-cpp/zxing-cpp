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

#include <memory>

namespace ZXing {

MultiFormatReader::MultiFormatReader(const DecodeHints& hints)
{
	bool tryHarder = hints.tryHarder();
	if (!hints.hasNoFormat()) {
		bool addOneDReader =
			hints.hasFormat(BarcodeFormat::UPC_A) ||
			hints.hasFormat(BarcodeFormat::UPC_E) ||
			hints.hasFormat(BarcodeFormat::EAN_13) ||
			hints.hasFormat(BarcodeFormat::EAN_8) ||
			hints.hasFormat(BarcodeFormat::CODABAR) ||
			hints.hasFormat(BarcodeFormat::CODE_39) ||
			hints.hasFormat(BarcodeFormat::CODE_93) ||
			hints.hasFormat(BarcodeFormat::CODE_128) ||
			hints.hasFormat(BarcodeFormat::ITF) ||
			hints.hasFormat(BarcodeFormat::RSS_14) ||
			hints.hasFormat(BarcodeFormat::RSS_EXPANDED);

		// Put 1D readers upfront in "normal" mode
		if (addOneDReader && !tryHarder) {
			_readers.emplace_back(new OneD::Reader(hints));
		}
		if (hints.hasFormat(BarcodeFormat::QR_CODE)) {
			_readers.emplace_back(new QRCode::Reader(hints));
		}
		if (hints.hasFormat(BarcodeFormat::DATA_MATRIX)) {
			_readers.emplace_back(new DataMatrix::Reader(hints));
		}
		if (hints.hasFormat(BarcodeFormat::AZTEC)) {
			_readers.emplace_back(new Aztec::Reader());
		}
		if (hints.hasFormat(BarcodeFormat::PDF_417)) {
			_readers.emplace_back(new Pdf417::Reader());
		}
		if (hints.hasFormat(BarcodeFormat::MAXICODE)) {
			_readers.emplace_back(new MaxiCode::Reader());
		}
		// At end in "try harder" mode
		if (addOneDReader && tryHarder) {
			_readers.emplace_back(new OneD::Reader(hints));
		}
	}

	if (_readers.empty()) {
		if (!tryHarder) {
			_readers.emplace_back(new OneD::Reader(hints));
		}
		_readers.emplace_back(new QRCode::Reader(hints));
		_readers.emplace_back(new DataMatrix::Reader(hints));
		_readers.emplace_back(new Aztec::Reader());
		_readers.emplace_back(new Pdf417::Reader());
		_readers.emplace_back(new MaxiCode::Reader());
		if (tryHarder) {
			_readers.emplace_back(new OneD::Reader(hints));
		}
	}
}

MultiFormatReader::~MultiFormatReader() = default;

Result
MultiFormatReader::read(const BinaryBitmap& image) const
{
	// If we have only one reader in our list, just return whatever that decoded.
	// This preserves information (e.g. ChecksumError) instead of just returning 'NotFound'.
	if (_readers.size() == 1)
		return _readers.front()->decode(image);

	for (const auto& reader : _readers) {
		Result r = reader->decode(image);
  		if (r.isValid())
			return r;
	}
	return Result(DecodeStatus::NotFound);
}

} // ZXing
