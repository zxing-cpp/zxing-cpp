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

#include "BarcodeFormat.h"
#include "DecodeHints.h"
#include "aztec/AZReader.h"
#include "datamatrix/DMReader.h"
#include "maxicode/MCReader.h"
#include "oned/ODReader.h"
#include "pdf417/PDFReader.h"
#include "qrcode/QRReader.h"

#include <memory>

namespace ZXing {

MultiFormatReader::MultiFormatReader(const DecodeHints& hints)
{
	bool tryHarder = hints.tryHarder();
	auto formats = hints.formats().empty() ? BarcodeFormat::Any : hints.formats();

	// Put 1D readers upfront in "normal" mode
	if (formats.testFlags(BarcodeFormat::OneDCodes) && !tryHarder)
		_readers.emplace_back(new OneD::Reader(hints));

	if (formats.testFlag(BarcodeFormat::QRCode))
		_readers.emplace_back(new QRCode::Reader(hints));
	if (formats.testFlag(BarcodeFormat::DataMatrix))
		_readers.emplace_back(new DataMatrix::Reader(hints));
	if (formats.testFlag(BarcodeFormat::Aztec))
		_readers.emplace_back(new Aztec::Reader(hints));
	if (formats.testFlag(BarcodeFormat::PDF417))
		_readers.emplace_back(new Pdf417::Reader(hints));
	if (formats.testFlag(BarcodeFormat::MaxiCode))
		_readers.emplace_back(new MaxiCode::Reader(hints));

	// At end in "try harder" mode
	if (formats.testFlags(BarcodeFormat::OneDCodes) && tryHarder) {
		_readers.emplace_back(new OneD::Reader(hints));
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

Results MultiFormatReader::readMultiple(const BinaryBitmap& image) const
{
	std::vector<Result> res;

	for (const auto& reader : _readers) {
		auto r = reader->decode(image, 0);
		res.insert(res.end(), r.begin(), r.end());
	}

	// sort results based on their position on the image
	std::sort(res.begin(), res.end(), [](const Result& l, const Result& r) {
		auto lp = l.position().topLeft();
		auto rp = r.position().topLeft();
		return lp.y < rp.y || (lp.y == rp.y && lp.x <= rp.x);
	});

	return res;
}

} // ZXing
