/*
* Copyright 2024 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#ifdef ZXING_BUILD_EXPERIMENTAL_API

#include "WriteBarcode.h"

struct zint_symbol {};

namespace ZXing {

struct CreatorOptions::Data
{
	BarcodeFormat format;
	bool readerInit = false;
	bool forceSquareDataMatrix = false;
	std::string ecLevel;

	// symbol size (qrcode, datamatrix, etc), map from I, 'WxH'
	// structured_append (idx, cnt, ID)

	mutable std::unique_ptr<zint_symbol> zint;
};

#define ZX_PROPERTY(TYPE, NAME) \
	TYPE CreatorOptions::NAME() const noexcept { return d->NAME; } \
	CreatorOptions& CreatorOptions::NAME(TYPE v)& { return d->NAME = std::move(v), *this; } \
	CreatorOptions&& CreatorOptions::NAME(TYPE v)&& { return d->NAME = std::move(v), std::move(*this); }

	ZX_PROPERTY(BarcodeFormat, format)
	ZX_PROPERTY(bool, readerInit)
	ZX_PROPERTY(bool, forceSquareDataMatrix)
	ZX_PROPERTY(std::string, ecLevel)

#undef ZX_PROPERTY

CreatorOptions::CreatorOptions(BarcodeFormat format) : d(std::make_unique<Data>(format)) {}
CreatorOptions::~CreatorOptions() = default;
CreatorOptions::CreatorOptions(CreatorOptions&&) = default;
CreatorOptions& CreatorOptions::operator=(CreatorOptions&&) = default;


struct WriterOptions::Data
{
	int scale = 0;
	int sizeHint = 0;
	int rotate = 0;
	bool withHRT = false;
	bool withQuietZones = true;
};

#define ZX_PROPERTY(TYPE, NAME) \
	TYPE WriterOptions::NAME() const noexcept { return d->NAME; } \
	WriterOptions& WriterOptions::NAME(TYPE v)& { return d->NAME = std::move(v), *this; } \
	WriterOptions&& WriterOptions::NAME(TYPE v)&& { return d->NAME = std::move(v), std::move(*this); }

ZX_PROPERTY(int, scale)
ZX_PROPERTY(int, sizeHint)
ZX_PROPERTY(int, rotate)
ZX_PROPERTY(bool, withHRT)
ZX_PROPERTY(bool, withQuietZones)

#undef ZX_PROPERTY

WriterOptions::WriterOptions() : d(std::make_unique<Data>()) {}
WriterOptions::~WriterOptions() = default;
WriterOptions::WriterOptions(WriterOptions&&) = default;
WriterOptions& WriterOptions::operator=(WriterOptions&&) = default;

} // namespace ZXing


#include "BitMatrix.h"
#include "MultiFormatWriter.h"
#include "ReadBarcode.h"

namespace ZXing {

static Barcode CreateBarcode(BitMatrix&& bits, const CreatorOptions& opts)
{
	auto img = ToMatrix<uint8_t>(bits);

	auto res = ReadBarcode({img.data(), img.width(), img.height(), ImageFormat::Lum},
						   ReaderOptions().setFormats(opts.format()).setIsPure(true).setBinarizer(Binarizer::BoolCast));
	res.symbol(std::move(bits));
	return res;
}

static bool IsLinearCode(BarcodeFormat format)
{
	return BarcodeFormats(BarcodeFormat::LinearCodes).testFlag(format);
}

Barcode CreateBarcodeFromText(std::string_view contents, const CreatorOptions& opts)
{
	auto writer = MultiFormatWriter(opts.format()).setMargin(0);
	if (!opts.ecLevel().empty())
		writer.setEccLevel(std::stoi(opts.ecLevel()));

	return CreateBarcode(writer.encode(std::string(contents), 0, IsLinearCode(opts.format()) ? 50 : 0), opts);
}

Barcode CreateBarcodeFromText(std::u8string_view contents, const CreatorOptions& opts)
{
	return CreateBarcodeFromText({reinterpret_cast<const char*>(contents.data()), contents.size()}, opts);
}

Barcode CreateBarcodeFromBytes(const void* data, int size, const CreatorOptions& opts)
{
	std::wstring bytes;
	for (uint8_t c : std::basic_string_view<uint8_t>((uint8_t*)data, size))
		bytes.push_back(c);

	auto writer = MultiFormatWriter(opts.format()).setMargin(0);
	if (!opts.ecLevel().empty())
		writer.setEccLevel(std::stoi(opts.ecLevel()));
	writer.setEncoding(CharacterSet::BINARY);

	return CreateBarcode(writer.encode(bytes, 0, IsLinearCode(opts.format()) ? 50 : 0), opts);
}

std::string WriteBarcodeToSVG(const Barcode& barcode, [[maybe_unused]] const WriterOptions& opts)
{
	return ToSVG(barcode.symbol());
}

Image WriteBarcodeToImage(const Barcode& barcode, [[maybe_unused]] const WriterOptions& opts)
{
	auto symbol = Inflate(barcode.symbol().copy(), opts.sizeHint(),
						  IsLinearCode(barcode.format()) ? std::clamp(opts.sizeHint() / 2, 50, 300) : opts.sizeHint(),
						  opts.withQuietZones() ? 10 : 0);
	auto bitmap = ToMatrix<uint8_t>(symbol);
	auto iv = Image(symbol.width(), symbol.height());
	std::memcpy(const_cast<uint8_t*>(iv.data()), bitmap.data(), iv.width() * iv.height());
	return iv;
}

} // namespace ZXing

#endif // ZXING_BUILD_EXPERIMENTAL_API
