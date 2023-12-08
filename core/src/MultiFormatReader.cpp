/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "MultiFormatReader.h"

#include "BarcodeFormat.h"
#include "BinaryBitmap.h"
#include "ReaderOptions.h"
#include "aztec/AZReader.h"
#include "datamatrix/DMReader.h"
#include "maxicode/MCReader.h"
#include "oned/ODReader.h"
#include "pdf417/PDFReader.h"
#include "qrcode/QRReader.h"

#include <memory>

namespace ZXing {

MultiFormatReader::MultiFormatReader(const ReaderOptions& opts) : _opts(opts)
{
	auto formats = opts.formats().empty() ? BarcodeFormat::Any : opts.formats();

	// Put linear readers upfront in "normal" mode
	if (formats.testFlags(BarcodeFormat::LinearCodes) && !opts.tryHarder())
		_readers.emplace_back(new OneD::Reader(opts));

	if (formats.testFlags(BarcodeFormat::QRCode | BarcodeFormat::MicroQRCode | BarcodeFormat::RMQRCode))
		_readers.emplace_back(new QRCode::Reader(opts, true));
	if (formats.testFlag(BarcodeFormat::DataMatrix))
		_readers.emplace_back(new DataMatrix::Reader(opts, true));
	if (formats.testFlag(BarcodeFormat::Aztec))
		_readers.emplace_back(new Aztec::Reader(opts, true));
	if (formats.testFlag(BarcodeFormat::PDF417))
		_readers.emplace_back(new Pdf417::Reader(opts));
	if (formats.testFlag(BarcodeFormat::MaxiCode))
		_readers.emplace_back(new MaxiCode::Reader(opts));

	// At end in "try harder" mode
	if (formats.testFlags(BarcodeFormat::LinearCodes) && opts.tryHarder())
		_readers.emplace_back(new OneD::Reader(opts));
}

MultiFormatReader::~MultiFormatReader() = default;

Result
MultiFormatReader::read(const BinaryBitmap& image) const
{
	Result r;
	for (const auto& reader : _readers) {
		r = reader->decode(image);
  		if (r.isValid())
			return r;
	}
	return _opts.returnErrors() ? r : Result();
}

Results MultiFormatReader::readMultiple(const BinaryBitmap& image, int maxSymbols) const
{
	std::vector<Result> res;

	for (const auto& reader : _readers) {
		if (image.inverted() && !reader->supportsInversion)
			continue;
		auto r = reader->decode(image, maxSymbols);
		if (!_opts.returnErrors()) {
			//TODO: C++20 res.erase_if()
			auto it = std::remove_if(res.begin(), res.end(), [](auto&& r) { return !r.isValid(); });
			res.erase(it, res.end());
		}
		maxSymbols -= Size(r);
		res.insert(res.end(), std::move_iterator(r.begin()), std::move_iterator(r.end()));
		if (maxSymbols <= 0)
			break;
	}

	// sort results based on their position on the image
	std::sort(res.begin(), res.end(), [](const Result& l, const Result& r) {
		auto lp = l.position().topLeft();
		auto rp = r.position().topLeft();
		return lp.y < rp.y || (lp.y == rp.y && lp.x < rp.x);
	});

	return res;
}

} // ZXing
