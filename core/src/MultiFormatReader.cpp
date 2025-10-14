/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "MultiFormatReader.h"

#include "BarcodeFormat.h"
#include "BinaryBitmap.h"
#include "Reader.h"
#include "ReaderOptions.h"
#ifdef ZXING_WITH_AZTEC
#include "aztec/AZReader.h"
#endif
#ifdef ZXING_WITH_DATAMATRIX
#include "datamatrix/DMReader.h"
#endif
#ifdef ZXING_WITH_MAXICODE
#include "maxicode/MCReader.h"
#endif
#ifdef ZXING_WITH_1D
#include "oned/ODReader.h"
#endif
#ifdef ZXING_WITH_PDF417
#include "pdf417/PDFReader.h"
#endif
#ifdef ZXING_WITH_QRCODE
#include "qrcode/QRReader.h"
#endif

#include <memory>

namespace ZXing {

MultiFormatReader::MultiFormatReader(const ReaderOptions& opts) : _opts(opts)
{
	auto formats = opts.formats().empty() ? BarcodeFormat::Any : opts.formats();

	// Put linear readers upfront in "normal" mode
#ifdef ZXING_WITH_1D
	if (formats.testFlags(BarcodeFormat::LinearCodes) && !opts.tryHarder())
		_readers.emplace_back(new OneD::Reader(opts));
#endif

#ifdef ZXING_WITH_QRCODE
	if (formats.testFlags(BarcodeFormat::QRCode | BarcodeFormat::MicroQRCode | BarcodeFormat::RMQRCode))
		_readers.emplace_back(new QRCode::Reader(opts, true));
#endif
#ifdef ZXING_WITH_DATAMATRIX
	if (formats.testFlag(BarcodeFormat::DataMatrix))
		_readers.emplace_back(new DataMatrix::Reader(opts, true));
#endif
#ifdef ZXING_WITH_AZTEC
	if (formats.testFlag(BarcodeFormat::Aztec))
		_readers.emplace_back(new Aztec::Reader(opts, true));
#endif
#ifdef ZXING_WITH_PDF417
	if (formats.testFlag(BarcodeFormat::PDF417))
		_readers.emplace_back(new Pdf417::Reader(opts));
#endif
#ifdef ZXING_WITH_MAXICODE
	if (formats.testFlag(BarcodeFormat::MaxiCode))
		_readers.emplace_back(new MaxiCode::Reader(opts));
#endif

	// At end in "try harder" mode
#ifdef ZXING_WITH_1D
	if (formats.testFlags(BarcodeFormat::LinearCodes) && opts.tryHarder())
		_readers.emplace_back(new OneD::Reader(opts));
#endif
}

MultiFormatReader::~MultiFormatReader() = default;

Barcode MultiFormatReader::read(const BinaryBitmap& image) const
{
	Barcode r;
	for (const auto& reader : _readers) {
		r = reader->decode(image);
  		if (r.isValid())
			return r;
	}
	return _opts.returnErrors() ? r : Barcode();
}

Barcodes MultiFormatReader::readMultiple(const BinaryBitmap& image, int maxSymbols) const
{
	Barcodes res;

	for (const auto& reader : _readers) {
		if (image.inverted() && !reader->supportsInversion)
			continue;
		auto r = reader->decode(image, maxSymbols);
		if (!_opts.returnErrors()) {
#ifdef __cpp_lib_erase_if
			std::erase_if(r, [](auto&& s) { return !s.isValid(); });
#else
			auto it = std::remove_if(r.begin(), r.end(), [](auto&& s) { return !s.isValid(); });
			r.erase(it, r.end());
#endif
		}
		maxSymbols -= Size(r);
		res.insert(res.end(), std::move_iterator(r.begin()), std::move_iterator(r.end()));
		if (maxSymbols <= 0)
			break;
	}

	// sort barcodes based on their position on the image
	std::sort(res.begin(), res.end(), [](const Barcode& l, const Barcode& r) {
		auto lp = l.position().topLeft();
		auto rp = r.position().topLeft();
		return lp.y < rp.y || (lp.y == rp.y && lp.x < rp.x);
	});

	return res;
}

} // ZXing
