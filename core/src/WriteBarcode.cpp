/*
* Copyright 2024 Axel Waggershauser
* Copyright 2025 gitlost
*/
// SPDX-License-Identifier: Apache-2.0

#include "WriteBarcode.h"

#include "BarcodeData.h"
#include "BitMatrix.h"
#include "BitMatrixIO.h"
#include "CreateBarcode.h"
#include "Log.h"
#include "Version.h"

#ifdef ZXING_USE_ZINT
#include <zint.h>
#endif // ZXING_USE_ZINT

namespace ZXing {

struct WriterOptions::Data
{
	int scale = 1;
	int rotate = 0;
	bool invert = false;
	bool addHRT = false;
	bool addQuietZones = true;
};

#define ZX_PROPERTY(TYPE, NAME) \
	TYPE WriterOptions::NAME() const noexcept { return d->NAME; } \
	WriterOptions& WriterOptions::NAME(TYPE v)& { return d->NAME = std::move(v), *this; } \
	WriterOptions&& WriterOptions::NAME(TYPE v)&& { return d->NAME = std::move(v), std::move(*this); }

ZX_PROPERTY(int, scale)
ZX_PROPERTY(int, rotate)
ZX_PROPERTY(bool, invert)
ZX_PROPERTY(bool, addHRT)
ZX_PROPERTY(bool, addQuietZones)

#undef ZX_PROPERTY

WriterOptions::WriterOptions() : d(std::make_unique<Data>()) {}
WriterOptions::~WriterOptions() = default;
WriterOptions::WriterOptions(WriterOptions&&) noexcept = default;
WriterOptions& WriterOptions::operator=(WriterOptions&&) noexcept = default;

#ifdef ZXING_USE_ZINT

struct ShallowCloneWithOptions
{
	zint_symbol* zint;

	ShallowCloneWithOptions(const zint_symbol* original, const WriterOptions& opts)
	{
		zint = static_cast<zint_symbol*>(malloc(sizeof(zint_symbol)));
		*zint = *original;
		zint->bitmap = NULL;
		zint->alphamap = NULL;
		zint->vector = NULL;
		zint->memfile = NULL;
		// don't set zint->content_segs = NULL, so it keeps on pointing to the original symbol's memory

		zint->show_hrt = opts.addHRT();

		zint->output_options &= ~(OUT_BUFFER_INTERMEDIATE | BARCODE_NO_QUIET_ZONES);
		zint->output_options |= opts.addQuietZones() ? BARCODE_QUIET_ZONES : BARCODE_NO_QUIET_ZONES;

		if (opts.scale() > 0)
			zint->scale = opts.scale() / 2.f;
		else if (opts.scale() < 0) {
			int size = std::max(zint->width, zint->rows);
			zint->scale = std::max(1, int(float(-opts.scale()) / size)) / 2.f;
		}

		if (opts.invert()) {
			strcpy(zint->bgcolour, "000000");
			strcpy(zint->fgcolour, "ffffff");
		}
	}

	zint_symbol* get() const { return zint; }

	~ShallowCloneWithOptions()
	{
		// prevent ZBarcode_Delete from freeing content_segs, which point to the original symbol's memory
		zint->content_segs = NULL;
		zint->content_seg_count = 0;
		ZBarcode_Delete(zint);
	}
};

#define CHECK(ZINT_CALL) \
	if (int err = (ZINT_CALL); err >= ZINT_ERROR) \
		throw std::invalid_argument(StrCat(zint->errtxt, " (retval: ", std::to_string(err), ")"));

#endif // ZXING_USE_ZINT

static Image ToImage(BitMatrix bits, bool isLinearCode, const WriterOptions& opts)
{
	if (bits.empty())
		return {};
	bits.flipAll();
	int width = opts.scale() > 0 ? bits.width() * opts.scale() : -opts.scale();
	int height = isLinearCode ? std::clamp(width / 2, 50, 300) : opts.scale() > 0 ? bits.height() * opts.scale() : -opts.scale();
	auto symbol = Inflate(std::move(bits), width, height, opts.addQuietZones() ? 10 : 0);
	if (opts.rotate() && opts.rotate() <= 180)
		symbol.rotate180();
	if (opts.rotate() % 180 == 90)
		symbol.rotate90();
	auto bitmap = ToMatrix<uint8_t>(symbol);
	auto iv = Image(symbol.width(), symbol.height());
	std::memcpy(const_cast<uint8_t*>(iv.data()), bitmap.data(), iv.width() * iv.height());
	return iv;
}

std::string WriteBarcodeToSVG(const Barcode& barcode, const WriterOptions& options)
{
#if defined(ZXING_WRITERS) && defined(ZXING_USE_ZINT)
	auto* zint = barcode.d->zint.get();

	if (!zint)
#endif
		return ToSVG(barcode.symbol().rotated(options.rotate()), options.invert());

#if defined(ZXING_WRITERS) && defined(ZXING_USE_ZINT)
	auto tmp_zint = ShallowCloneWithOptions(zint, options);
	zint = tmp_zint.get();

	zint->output_options |= BARCODE_MEMORY_FILE;// | EMBED_VECTOR_FONT;
	strcpy(zint->outfile, "null.svg");

	CHECK(ZBarcode_Print(zint, options.rotate()));

	return std::string(reinterpret_cast<const char*>(zint->memfile), zint->memfile_size);
#endif
}

Image WriteBarcodeToImage(const Barcode& barcode, const WriterOptions& options)
{
#if defined(ZXING_WRITERS) && defined(ZXING_USE_ZINT)
	auto* zint = barcode.d->zint.get();

	if (!zint)
#endif
		return ToImage(barcode.d->symbol.copy(), barcode.format() & BarcodeFormat::AllLinear, options);

#if defined(ZXING_WRITERS) && defined(ZXING_USE_ZINT)
	auto tmp_zint = ShallowCloneWithOptions(zint, options);
	zint = tmp_zint.get();

	CHECK(ZBarcode_Buffer(zint, options.rotate()));

	log_l("write symbol with size: %dx%d", zint->bitmap_width, zint->bitmap_height);
	auto iv = Image(zint->bitmap_width, zint->bitmap_height);
	auto* src = zint->bitmap;
	auto* dst = const_cast<uint8_t*>(iv.data());
	for(int y = 0; y < iv.height(); ++y)
		for(int x = 0, w = iv.width(); x < w; ++x, src += 3)
			*dst++ = RGBToLum(src[0], src[1], src[2]);

	return iv;
#endif
}

std::string WriteBarcodeToUtf8(const Barcode& barcode, const WriterOptions& options)
{
	bool invert = !options.invert();

	Image buffer = WriteBarcodeToImage(barcode, WriterOptions().addQuietZones(options.addQuietZones()).rotate(options.rotate()));
	ImageView iv = buffer;
	if (barcode.symbology() == BarcodeFormat::EANUPC && options.rotate() % 180 == 0)
		iv = iv.cropped(0, iv.height() / 2 - 1, iv.width(), 2); // don't print the extending guard bars

	constexpr auto map = std::array{" ", "▀", "▄", "█"};
	std::string res;
	res.reserve(iv.width() * iv.height() / 2 + 1);
	for (int y = 0; y < iv.height(); y += 2) {
		// for linear barcodes in horizontal orientation, only print line pairs that are distinct from the previous one
		if (barcode.format() & BarcodeFormat::AllLinear && options.rotate() % 180 == 0 && y > 1 && y < iv.height() - 1
			&& memcmp(iv.data(0, y), iv.data(0, y - 2), 2 * iv.rowStride()) == 0)
			continue;

		for (int x = 0; x < iv.width(); ++x) {
			int tp = bool(*iv.data(x, y)) ^ invert;
			int bt = (iv.height() == 1 && tp) || (y + 1 < iv.height() && (bool(*iv.data(x, y + 1)) ^ invert));
			res += map[tp | (bt << 1)];
		}
		res += '\n';
	}

	return res;
}

} // namespace ZXing
