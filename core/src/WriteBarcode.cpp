/*
* Copyright 2024 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#ifdef ZXING_BUILD_EXPERIMENTAL_API

#include "WriteBarcode.h"

#ifdef ZXING_USE_ZINT
#include <zint.h>

template <>
struct std::default_delete<zint_symbol>
{
	void operator()(zint_symbol* p) const noexcept { ZBarcode_Delete(p); }
};

#else
struct zint_symbol {};
#endif

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

#ifdef ZXING_USE_ZINT
#include "ECI.h"
#include "ReadBarcode.h"

#include <zint.h>
#include <charconv>

namespace ZXing {

struct BarcodeFormatZXing2Zint
{
	BarcodeFormat zxing;
	int zint;
};

static constexpr BarcodeFormatZXing2Zint barcodeFormatZXing2Zint[] = {
	{BarcodeFormat::Aztec, BARCODE_AZTEC},
	{BarcodeFormat::Codabar, BARCODE_CODABAR},
	{BarcodeFormat::Code39, BARCODE_CODE39},
	{BarcodeFormat::Code93, BARCODE_CODE93},
	{BarcodeFormat::Code128, BARCODE_CODE128},
	{BarcodeFormat::DataBar, BARCODE_DBAR_OMN},
	{BarcodeFormat::DataBarExpanded, BARCODE_DBAR_EXP},
	{BarcodeFormat::DataMatrix, BARCODE_DATAMATRIX},
	{BarcodeFormat::DXFilmEdge, -1},
	{BarcodeFormat::EAN8, BARCODE_EANX},
	{BarcodeFormat::EAN13, BARCODE_EANX},
	{BarcodeFormat::ITF, BARCODE_ITF14},
	{BarcodeFormat::MaxiCode, BARCODE_MAXICODE},
	{BarcodeFormat::MicroQRCode, BARCODE_MICROQR},
	{BarcodeFormat::PDF417, BARCODE_PDF417},
	{BarcodeFormat::QRCode, BARCODE_QRCODE},
	{BarcodeFormat::RMQRCode, BARCODE_RMQR},
	{BarcodeFormat::UPCA, BARCODE_UPCA},
	{BarcodeFormat::UPCE, BARCODE_UPCE},
};

struct String2Int
{
	const char* str;
	int val;
};

static int ParseECLevel(int symbology, std::string_view s)
{
	constexpr std::string_view EC_LABELS_QR[4] = {"L", "M", "Q", "H"};

	int res = 0;
	if (Contains({BARCODE_QRCODE, BARCODE_MICROQR, BARCODE_RMQR}, symbology))
		if ((res = IndexOf(EC_LABELS_QR, s) != -1))
			return res + 1;

	if (std::from_chars(s.data(), s.data() + s.size() - (s.back() == '%'), res).ec != std::errc{})
		throw std::invalid_argument("Invalid ecLevel: '" + std::string(s) + "'");

	auto findClosestECLevel = [](const std::vector<int>& list, int val) {
		int mIdx, mAbs = 100;
		for (int i = 0; i < Size(list); ++i)
		if (int abs = std::abs(val - list[i]); abs < mAbs) {
				mIdx = i;
				mAbs = abs;
		}
		return mIdx + 1;
	};

	if (s.back()=='%'){
		switch (symbology) {
		case BARCODE_QRCODE:
		case BARCODE_MICROQR:
		case BARCODE_RMQR:
			return findClosestECLevel({20, 37, 55, 65}, res);
		case BARCODE_AZTEC:
			return findClosestECLevel({10, 23, 26, 50}, res);
		case BARCODE_PDF417:
			// TODO: do something sensible with PDF417?
		default:
			return -1;
		}
	}

	return res;
};

zint_symbol* CreatorOptions::zint() const
{
	auto& zint = d->zint;

	if (!zint) {
		printf("zint version: %d, sizeof(zint_symbol): %ld\n", ZBarcode_Version(), sizeof(zint_symbol));

		zint.reset(ZBarcode_Create());

		auto i = FindIf(barcodeFormatZXing2Zint, [zxing = format()](auto& v) { return v.zxing == zxing; });
		if (i == std::end(barcodeFormatZXing2Zint))
			throw std::invalid_argument("unsupported barcode format: " + ToString(format()));
		zint->symbology = i->zint;

		zint->scale = 0.5f;

		if (!ecLevel().empty())
			zint->option_1 = ParseECLevel(zint->symbology, ecLevel());
	}

	return zint.get();
}

Barcode CreateBarcode(const void* data, int size, int mode, const CreatorOptions& opts)
{
	auto zint = opts.zint();

	zint->input_mode = mode;
	zint->output_options |= OUT_BUFFER_INTERMEDIATE | BARCODE_QUIET_ZONES;

	if (mode == DATA_MODE && ZBarcode_Cap(zint->symbology, ZINT_CAP_ECI))
		zint->eci = static_cast<int>(ECI::Binary);

	int err = ZBarcode_Encode_and_Buffer(zint, (uint8_t*)data, size, 0);

	if (err || *zint->errtxt)
		printf("Error %d: %s\n", err, zint->errtxt);

	printf("symbol size: %dx%d\n", zint->width, zint->rows);

	auto buffer = std::vector<uint8_t>(zint->bitmap_width * zint->bitmap_height);
	std::transform(zint->bitmap, zint->bitmap + zint->bitmap_width * zint->bitmap_height, buffer.data(),
				   [](unsigned char v) { return (v == '0') * 0xff; });
	auto bits = BitMatrix(zint->bitmap_width, zint->bitmap_height);
	std::transform(zint->bitmap, zint->bitmap + zint->bitmap_width * zint->bitmap_height, bits.row(0).begin(),
				   [](unsigned char v) { return (v == '0') * BitMatrix::SET_V; });

	auto res = ReadBarcode({buffer.data(), zint->bitmap_width, zint->bitmap_height, ImageFormat::Lum},
						   ReaderOptions().setFormats(opts.format()).setIsPure(true).setBinarizer(Binarizer::BoolCast));
	res.symbol(std::move(bits));
	res.zint(std::move(opts.d->zint));

	return res;
}

Barcode CreateBarcodeFromText(std::string_view contents, const CreatorOptions& opts)
{
	return CreateBarcode(contents.data(), contents.size(), UNICODE_MODE, opts);
}

Barcode CreateBarcodeFromText(std::u8string_view contents, const CreatorOptions& opts)
{
	return CreateBarcode(contents.data(), contents.size(), UNICODE_MODE, opts);
}

Barcode CreateBarcodeFromBytes(const void* data, int size, const CreatorOptions& opts)
{
	return CreateBarcode(data, size, DATA_MODE, opts);
}

// Writer ========================================================================

static void SetCommonWriterOptions(zint_symbol* zint, const WriterOptions& opts)
{
	zint->show_hrt = opts.withHRT();

	zint->output_options &= ~OUT_BUFFER_INTERMEDIATE;
	zint->output_options |= opts.withQuietZones() ? BARCODE_QUIET_ZONES: BARCODE_NO_QUIET_ZONES;

	if (opts.scale())
		zint->scale = opts.scale();
	else if (opts.sizeHint()) {
		int size = std::max(zint->width, zint->rows);
		zint->scale = std::max(1, int(float(opts.sizeHint()) / size)) / 2.f;
	}
}

std::string WriteBarcodeToSVG(const Barcode& barcode, const WriterOptions& opts)
{
	auto zs = barcode.zint();

	if (!zs)
		return ToSVG(barcode.symbol());

	SetCommonWriterOptions(zs, opts);

	zs->output_options |= BARCODE_MEMORY_FILE;// | EMBED_VECTOR_FONT;
	strcpy(zs->outfile, "null.svg");

	int err = ZBarcode_Print(zs, opts.rotate());
	if (err || *zs->errtxt)
		printf("Error %d: %s\n", err, zs->errtxt);

	return std::string(reinterpret_cast<const char*>(zs->memfile), zs->memfile_size);
}

Image WriteBarcodeToImage(const Barcode& barcode, const WriterOptions& opts)
{
	auto zs = barcode.zint();

	SetCommonWriterOptions(zs, opts);

	int err = ZBarcode_Buffer(zs, opts.rotate());
	if (err || *zs->errtxt)
		printf("Error %d: %s\n", err, zs->errtxt);

	printf("symbol size: %dx%d\n", zs->bitmap_width, zs->bitmap_height);
	auto iv = Image(zs->bitmap_width, zs->bitmap_height);
	auto* src = zs->bitmap;
	auto* dst = const_cast<uint8_t*>(iv.data());
	for(int y = 0; y < iv.height(); ++y)
		for(int x = 0, w = iv.width(); x < w; ++x, src += 3)
			*dst++ = RGBToLum(src[0], src[1], src[2]);

	return iv;
}

} // ZXing


#else

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

#endif

#endif // ZXING_BUILD_EXPERIMENTAL_API
