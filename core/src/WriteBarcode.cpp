/*
* Copyright 2024 Axel Waggershauser
* Copyright 2025 gitlost
*/
// SPDX-License-Identifier: Apache-2.0

#ifdef ZXING_EXPERIMENTAL_API

#include "WriteBarcode.h"
#include "BitMatrix.h"
#include "JSON.h"

#if !defined(ZXING_READERS) && !defined(ZXING_WRITERS)
#include "Version.h"
#endif

#include <sstream>

#ifdef ZXING_USE_ZINT

#include "oned/ODUPCEANCommon.h"
#include "DecoderResult.h"
#include "DetectorResult.h"
#include <zint.h>

#else

struct zint_symbol {};

#endif // ZXING_USE_ZINT

namespace ZXing {

struct CreatorOptions::Data
{
	BarcodeFormat format;
	std::string options;
	bool readerInit = false;
	bool forceSquareDataMatrix = false;
	std::string ecLevel;

	// symbol size (qrcode, datamatrix, etc), map from I, 'WxH'
	// structured_append (idx, cnt, ID)

	mutable unique_zint_symbol zint;

#ifndef __cpp_aggregate_paren_init
	Data(BarcodeFormat f, std::string o) : format(f), options(std::move(o)) {}
#endif
};

#define ZX_PROPERTY(TYPE, NAME) \
	const TYPE& CreatorOptions::NAME() const noexcept { return d->NAME; } \
	CreatorOptions& CreatorOptions::NAME(TYPE v)& { return d->NAME = std::move(v), *this; } \
	CreatorOptions&& CreatorOptions::NAME(TYPE v)&& { return d->NAME = std::move(v), std::move(*this); }

	ZX_PROPERTY(BarcodeFormat, format)
	ZX_PROPERTY(bool, readerInit)
	ZX_PROPERTY(bool, forceSquareDataMatrix)
	ZX_PROPERTY(std::string, ecLevel)
	ZX_PROPERTY(std::string, options)

#undef ZX_PROPERTY

#define ZX_RO_PROPERTY(TYPE, NAME) \
	TYPE CreatorOptions::NAME() const noexcept { return JsonGet<TYPE>(d->options, #NAME); }

	ZX_RO_PROPERTY(bool, gs1);
	ZX_RO_PROPERTY(bool, stacked);
	ZX_RO_PROPERTY(std::string_view, version);
	ZX_RO_PROPERTY(std::string_view, dataMask);

#undef ZX_PROPERTY

	CreatorOptions::CreatorOptions(BarcodeFormat format, std::string options) : d(std::make_unique<Data>(format, std::move(options))) {}
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

static bool SupportsGS1(BarcodeFormat format)
{
	return (BarcodeFormat::Aztec | BarcodeFormat::Code128 | BarcodeFormat::DataMatrix | BarcodeFormat::QRCode
			| BarcodeFormat::RMQRCode)
		.testFlag(format);
}

static std::string ToSVG(ImageView iv)
{
	if (!iv.data())
		return {};

	// see https://stackoverflow.com/questions/10789059/create-qr-code-in-vector-image/60638350#60638350

	std::ostringstream res;

	res << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		<< "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" viewBox=\"0 0 " << iv.width() << " " << iv.height()
		<< "\" stroke=\"none\">\n"
		<< "<path d=\"";

	for (int y = 0; y < iv.height(); ++y)
		for (int x = 0; x < iv.width(); ++x)
			if (*iv.data(x, y) == 0)
				res << "M" << x << "," << y << "h1v1h-1z";

	res << "\"/>\n</svg>";

	return res.str();
}

static Image ToImage(BitMatrix bits, bool isLinearCode, const WriterOptions& opts)
{
	bits.flipAll();
	auto symbol = Inflate(std::move(bits), opts.sizeHint(),
						  isLinearCode ? std::clamp(opts.sizeHint() / 2, 50, 300) : opts.sizeHint(),
						  opts.withQuietZones() ? 10 : 0);
	auto bitmap = ToMatrix<uint8_t>(symbol);
	auto iv = Image(symbol.width(), symbol.height());
	std::memcpy(const_cast<uint8_t*>(iv.data()), bitmap.data(), iv.width() * iv.height());
	return iv;
}

} // namespace ZXing


#ifdef ZXING_WRITERS

#ifdef ZXING_USE_ZINT
#include "ECI.h"

#ifdef ZXING_READERS
#include "ReadBarcode.h"
#endif

#include <charconv>
#include <zint.h>

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
	{BarcodeFormat::DataBarLimited, BARCODE_DBAR_LTD},
	{BarcodeFormat::DataMatrix, BARCODE_DATAMATRIX},
	{BarcodeFormat::DXFilmEdge, BARCODE_DXFILMEDGE},
	{BarcodeFormat::EAN8, BARCODE_EANX},
	{BarcodeFormat::EAN13, BARCODE_EANX},
	{BarcodeFormat::ITF, BARCODE_C25INTER},
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

	// Convert L/M/Q/H to Zint 1-4
	if (Contains({BARCODE_QRCODE, BARCODE_MICROQR, BARCODE_RMQR}, symbology))
		if ((res = IndexOf(EC_LABELS_QR, s)) != -1)
			return res + 1;

	if (std::from_chars(s.data(), s.data() + s.size() - (s.back() == '%'), res).ec != std::errc{})
		throw std::invalid_argument("Invalid ecLevel: '" + std::string(s) + "'");

	auto findClosestECLevel = [](const std::vector<int>& list, int val) {
		int mIdx = -2, mAbs = 100;
		for (int i = 0; i < Size(list); ++i)
			if (int abs = std::abs(val - list[i]); abs < mAbs) {
				mIdx = i;
				mAbs = abs;
			}
		return mIdx + 1;
	};

	// Convert percentage to Zint
	if (s.back() == '%') {
		switch (symbology) {
		case BARCODE_QRCODE: return findClosestECLevel({20, 37, 55, 65}, res);
		case BARCODE_MICROQR: return findClosestECLevel({20, 37, 55}, res);
		case BARCODE_RMQR: return res <= 46 ? 2 : 4;
		case BARCODE_AZTEC: return findClosestECLevel({10, 23, 36, 50}, res);
		case BARCODE_PDF417:
			// TODO: do something sensible with PDF417?
		default:
			return -1;
		}
	}

	return res;
};

static constexpr struct { BarcodeFormat format; SymbologyIdentifier si; } barcodeFormat2SymbologyIdentifier[] = {
	{BarcodeFormat::Aztec, {'z', '0', 3}}, // '1' GS1, '2' AIM
	{BarcodeFormat::Codabar, {'F', '0'}}, // if checksum processing were implemented and checksum present and stripped then modifier would be 4
	// {BarcodeFormat::CodablockF, {'O', '4'}}, // '5' GS1
	{BarcodeFormat::Code128, {'C', '0'}}, // '1' GS1, '2' AIM
	// {BarcodeFormat::Code16K, {'K', '0'}}, // '1' GS1, '2' AIM, '4' D1 PAD
	{BarcodeFormat::Code39, {'A', '0'}}, // '3' checksum, '4' extended, '7' checksum,extended
	{BarcodeFormat::Code93, {'G', '0'}}, // no modifiers
	{BarcodeFormat::DataBar, {'e', '0', 0, AIFlag::GS1}},
	{BarcodeFormat::DataBarExpanded, {'e', '0', 0, AIFlag::GS1}},
	{BarcodeFormat::DataBarLimited, {'e', '0', 0, AIFlag::GS1}},
	{BarcodeFormat::DataMatrix, {'d', '1', 3}}, // '2' GS1, '3' AIM
	// {BarcodeFormat::DotCode, {'J', '0', 3}}, // '1' GS1, '2' AIM
	{BarcodeFormat::DXFilmEdge, {}},
	{BarcodeFormat::EAN8, {'E', '4'}},
	{BarcodeFormat::EAN13, {'E', '0'}},
	// {BarcodeFormat::HanXin, {'h', '0', 1}}, // '2' GS1
	{BarcodeFormat::ITF, {'I', '0'}}, // '1' check digit
	{BarcodeFormat::MaxiCode, {'U', '0', 2}}, // '1' mode 2 or 3
	// {BarcodeFormat::MicroPDF417, {'L', '2', char(-1)}},
	{BarcodeFormat::MicroQRCode, {'Q', '1', 1}},
	{BarcodeFormat::PDF417, {'L', '2', char(-1)}},
	{BarcodeFormat::QRCode, {'Q', '1', 1}}, // '3' GS1, '5' AIM
	{BarcodeFormat::RMQRCode, {'Q', '1', 1}}, // '3' GS1, '5' AIM
	{BarcodeFormat::UPCA, {'E', '0'}},
	{BarcodeFormat::UPCE, {'E', '0'}},
};

static SymbologyIdentifier SymbologyIdentifierZint2ZXing(const CreatorOptions& opts, const ByteArray& ba)
{
	const BarcodeFormat format = opts.format();

	auto i = FindIf(barcodeFormat2SymbologyIdentifier, [format](auto& v) { return v.format == format; });
	assert(i != std::end(barcodeFormat2SymbologyIdentifier));
	SymbologyIdentifier ret = i->si;

	if ((BarcodeFormat::EAN13 | BarcodeFormat::UPCA | BarcodeFormat::UPCE).testFlag(format)) {
		if (Contains(ba.asString().data(), ' ')) // Have EAN-2/5 add-on?
			ret.modifier = '3'; // Combined packet, EAN-13, UPC-A, UPC-E, with add-on
	} else if (format == BarcodeFormat::Code39) {
		if (FindIf(ba, iscntrl) != ba.end()) // Extended Code 39?
			ret.modifier = static_cast<char>(ret.modifier + 4);
	} else if (opts.gs1() && SupportsGS1(format)) {
		if ((BarcodeFormat::Aztec | BarcodeFormat::Code128).testFlag(format))
			ret.modifier = '1';
		else if (format == BarcodeFormat::DataMatrix)
			ret.modifier = '2';
		else if ((BarcodeFormat::QRCode | BarcodeFormat::RMQRCode).testFlag(format))
			ret.modifier = '3';
		ret.aiFlag = AIFlag::GS1;
	}

	return ret;
}

static std::string ECLevelZint2ZXing(const zint_symbol* zint)
{
	constexpr char EC_LABELS_QR[4] = {'L', 'M', 'Q', 'H'};

	const int symbology = zint->symbology;
	const int option_1 = zint->option_1;

	switch (symbology) {
	case BARCODE_AZTEC:
		if ((option_1 >> 8) >= 0 && (option_1 >> 8) <= 99)
			return std::to_string(option_1 >> 8) + "%";
		break;
	case BARCODE_MAXICODE:
		// Mode
		if (option_1 >= 2 && option_1 <= 6)
			return std::to_string(option_1);
		break;
	case BARCODE_PDF417:
	case BARCODE_PDF417COMP:
		// Convert to percentage
		if (option_1 >= 0 && option_1 <= 8) {
			int overhead = symbology == BARCODE_PDF417COMP ? 35 : 69;
			int cols = (zint->width - overhead) / 17;
			int tot_cws = zint->rows * cols;
			assert(tot_cws);
			return std::to_string((2 << option_1) * 100 / tot_cws) + "%";
		}
		break;
	// case BARCODE_MICROPDF417:
	// 	if ((option_1 >> 8) >= 0 && (option_1 >> 8) <= 99)
	// 		return std::to_string(option_1 >> 8) + "%";
	// 	break;
	case BARCODE_QRCODE:
	case BARCODE_MICROQR:
	case BARCODE_RMQR:
		// Convert to L/M/Q/H
		if (option_1 >= 1 && option_1 <= 4)
			return {EC_LABELS_QR[option_1 - 1]};
		break;
	// case BARCODE_HANXIN:
	// 	if (option_1 >= 1 && option_1 <= 4)
	// 		return "L" + std::to_string(option_1);
	// 	break;
	default:
		break;
	}

	return {};
}

zint_symbol* CreatorOptions::zint() const
{
	auto& zint = d->zint;

	if (!zint) {
#ifdef PRINT_DEBUG
		printf("zint version: %d, sizeof(zint_symbol): %ld\n", ZBarcode_Version(), sizeof(zint_symbol));
#endif
		zint.reset(ZBarcode_Create());

#ifdef PRINT_DEBUG
		printf("options: %s\n", options().c_str());
#endif

		auto i = FindIf(barcodeFormatZXing2Zint, [zxing = format()](auto& v) { return v.zxing == zxing; });
		if (i == std::end(barcodeFormatZXing2Zint))
			throw std::invalid_argument("unsupported barcode format: " + ToString(format()));

		if (format() == BarcodeFormat::Code128 && gs1())
			zint->symbology = BARCODE_GS1_128;
		else if (format() == BarcodeFormat::DataBar && stacked())
			zint->symbology = BARCODE_DBAR_OMNSTK;
		else if (format() == BarcodeFormat::DataBarExpanded && stacked())
			zint->symbology = BARCODE_DBAR_EXPSTK;
		else
			zint->symbology = i->zint;

		zint->scale = 0.5f;

		if (!ecLevel().empty())
			zint->option_1 = ParseECLevel(zint->symbology, ecLevel());

		if (auto str = version(); str.size() && !IsLinearBarcode(format()))
			zint->option_2 = svtoi(str);

		if (auto str = dataMask(); str.size() && (BarcodeFormat::QRCode | BarcodeFormat::MicroQRCode).testFlag(format()))
			zint->option_3 = (zint->option_3 & 0xFF) | (svtoi(str) + 1) << 8;
	}

	return zint.get();
}

#define CHECK(ZINT_CALL) \
	if (int err = (ZINT_CALL); err >= ZINT_ERROR) \
		throw std::invalid_argument(std::string(zint->errtxt) + " (retval: " + std::to_string(err) + ")");

Barcode CreateBarcode(const void* data, int size, int mode, const CreatorOptions& opts)
{
	auto zint = opts.zint();

	zint->input_mode = mode == UNICODE_MODE && opts.gs1() && SupportsGS1(opts.format()) ? GS1_MODE | GS1PARENS_MODE : mode;
	zint->output_options |= OUT_BUFFER_INTERMEDIATE | BARCODE_QUIET_ZONES | BARCODE_RAW_TEXT;

	if (mode == DATA_MODE && ZBarcode_Cap(zint->symbology, ZINT_CAP_ECI))
		zint->eci = static_cast<int>(ECI::Binary);

	CHECK(ZBarcode_Encode_and_Buffer(zint, (uint8_t*)data, size, 0));

#ifdef PRINT_DEBUG
	printf("create symbol with size: %dx%d\n", zint->width, zint->rows);
#endif

#if 0 // use ReadBarcode to create Barcode object
	auto buffer = std::vector<uint8_t>(zint->bitmap_width * zint->bitmap_height);
	std::transform(zint->bitmap, zint->bitmap + zint->bitmap_width * zint->bitmap_height, buffer.data(),
				   [](unsigned char v) { return (v == '0') * 0xff; });

	auto res = ReadBarcode({buffer.data(), zint->bitmap_width, zint->bitmap_height, ImageFormat::Lum},
						   ReaderOptions().setFormats(opts.format()).setIsPure(true).setBinarizer(Binarizer::BoolCast));
#else
	Content content;

#ifdef ZXING_READERS
	for (int i = 0; i < zint->raw_seg_count; ++i) {
		const auto& raw_seg = zint->raw_segs[i];
#ifdef PRINT_DEBUG
		printf("  seg %d of %d with eci %d: %s\n", i, zint->raw_seg_count, raw_seg.eci, (char*)raw_seg.source);
#endif
		if (ECI(raw_seg.eci) != ECI::ISO8859_1)
			content.switchEncoding(ECI(raw_seg.eci));
		else
			content.switchEncoding(CharacterSet::ISO8859_1); // set this as default to prevent guessing without setting "hasECI"
		content.append({raw_seg.source, static_cast<size_t>(raw_seg.length - (opts.format() == BarcodeFormat::Code93 ? 2 : 0))});
	}
	if (opts.format() == BarcodeFormat::UPCE)
		content.bytes = ByteArray("0" + OneD::UPCEANCommon::ConvertUPCEtoUPCA(std::string(content.bytes.asString())));
	else if (opts.format() == BarcodeFormat::UPCA)
		content.bytes = ByteArray("0" + std::string(content.bytes.asString()));
#else
	if (zint->text_length) {
		content.switchEncoding(ECI::UTF8);
		content.append({zint->text, static_cast<size_t>(zint->text_length)});
	} else {
		content.switchEncoding(mode == DATA_MODE ? ECI::Binary : ECI::UTF8);
		content.append({static_cast<const uint8_t*>(data), static_cast<size_t>(size)});
	}
#endif

	content.symbology = SymbologyIdentifierZint2ZXing(opts, content.bytes);

	DecoderResult decRes(std::move(content));
	decRes.setEcLevel(ECLevelZint2ZXing(zint));
	DetectorResult detRes;

	auto res = Barcode(std::move(decRes), std::move(detRes), opts.format());
#endif

	auto bits = BitMatrix(zint->bitmap_width, zint->bitmap_height);
	std::transform(zint->bitmap, zint->bitmap + zint->bitmap_width * zint->bitmap_height, bits.row(0).begin(),
				   [](unsigned char v) { return (v == '1') * BitMatrix::SET_V; });
	res.symbol(std::move(bits));
	res.zint(std::move(opts.d->zint));

	return res;
}

Barcode CreateBarcodeFromText(std::string_view contents, const CreatorOptions& opts)
{
	return CreateBarcode(contents.data(), contents.size(), UNICODE_MODE, opts);
}

#if __cplusplus > 201703L
Barcode CreateBarcodeFromText(std::u8string_view contents, const CreatorOptions& opts)
{
	return CreateBarcode(contents.data(), contents.size(), UNICODE_MODE, opts);
}
#endif

Barcode CreateBarcodeFromBytes(const void* data, int size, const CreatorOptions& opts)
{
	return CreateBarcode(data, size, DATA_MODE, opts);
}

// Writer ========================================================================

struct SetCommonWriterOptions
{
	zint_symbol* zint;

	SetCommonWriterOptions(zint_symbol* zint, const WriterOptions& opts) : zint(zint)
	{
		zint->show_hrt = opts.withHRT();

		zint->output_options &= ~OUT_BUFFER_INTERMEDIATE;
		zint->output_options |= opts.withQuietZones() ? BARCODE_QUIET_ZONES : BARCODE_NO_QUIET_ZONES;

		if (opts.scale())
			zint->scale = opts.scale() / 2.f;
		else if (opts.sizeHint()) {
			int size = std::max(zint->width, zint->rows);
			zint->scale = std::max(1, int(float(opts.sizeHint()) / size)) / 2.f;
		}
	}

	// reset the defaults such that consecutive write calls don't influence each other
	~SetCommonWriterOptions() { zint->scale = 0.5f; }
};

} // ZXing

#else // ZXING_USE_ZINT

#include "MultiFormatWriter.h"
#include "ReadBarcode.h"

namespace ZXing {

zint_symbol* CreatorOptions::zint() const { return nullptr; }

static Barcode CreateBarcode(BitMatrix&& bits, const CreatorOptions& opts)
{
	auto img = ToMatrix<uint8_t>(bits);

	auto res = ReadBarcode({img.data(), img.width(), img.height(), ImageFormat::Lum},
						   ReaderOptions().setFormats(opts.format()).setIsPure(true).setBinarizer(Binarizer::BoolCast));
	res.symbol(std::move(bits));
	return res;
}

Barcode CreateBarcodeFromText(std::string_view contents, const CreatorOptions& opts)
{
	auto writer = MultiFormatWriter(opts.format()).setMargin(0);
	if (!opts.ecLevel().empty())
		writer.setEccLevel(std::stoi(opts.ecLevel()));
	writer.setEncoding(CharacterSet::UTF8); // write UTF8 (ECI value 26) for maximum compatibility

	return CreateBarcode(writer.encode(std::string(contents), 0, IsLinearCode(opts.format()) ? 50 : 0), opts);
}

#if __cplusplus > 201703L
Barcode CreateBarcodeFromText(std::u8string_view contents, const CreatorOptions& opts)
{
	return CreateBarcodeFromText({reinterpret_cast<const char*>(contents.data()), contents.size()}, opts);
}
#endif

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

} // namespace ZXing

#endif // ZXING_USE_ZINT

#else // ZXING_WRITERS

namespace ZXing {

zint_symbol* CreatorOptions::zint() const { return nullptr; }

Barcode CreateBarcodeFromText(std::string_view, const CreatorOptions&)
{
	throw std::runtime_error("This build of zxing-cpp does not support creating barcodes.");
}

#if __cplusplus > 201703L
Barcode CreateBarcodeFromText(std::u8string_view, const CreatorOptions&)
{
	throw std::runtime_error("This build of zxing-cpp does not support creating barcodes.");
}
#endif

Barcode CreateBarcodeFromBytes(const void*, int, const CreatorOptions&)
{
	throw std::runtime_error("This build of zxing-cpp does not support creating barcodes.");
}

} // namespace ZXing

#endif // ZXING_WRITERS

namespace ZXing {

std::string WriteBarcodeToSVG(const Barcode& barcode, [[maybe_unused]] const WriterOptions& opts)
{
	auto zint = barcode.zint();

	if (!zint)
		return ToSVG(barcode.symbol());

#if defined(ZXING_WRITERS) && defined(ZXING_USE_ZINT)
	auto resetOnExit = SetCommonWriterOptions(zint, opts);

	zint->output_options |= BARCODE_MEMORY_FILE;// | EMBED_VECTOR_FONT;
	strcpy(zint->outfile, "null.svg");

	CHECK(ZBarcode_Print(zint, opts.rotate()));

	return std::string(reinterpret_cast<const char*>(zint->memfile), zint->memfile_size);
#else
	return {}; // unreachable code
#endif
}

Image WriteBarcodeToImage(const Barcode& barcode, [[maybe_unused]] const WriterOptions& opts)
{
	auto zint = barcode.zint();

	if (!zint)
		return ToImage(barcode._symbol->copy(), IsLinearBarcode(barcode.format()), opts);

#if defined(ZXING_WRITERS) && defined(ZXING_USE_ZINT)
	auto resetOnExit = SetCommonWriterOptions(zint, opts);

	CHECK(ZBarcode_Buffer(zint, opts.rotate()));

#ifdef PRINT_DEBUG
	printf("write symbol with size: %dx%d\n", zint->bitmap_width, zint->bitmap_height);
#endif
	auto iv = Image(zint->bitmap_width, zint->bitmap_height);
	auto* src = zint->bitmap;
	auto* dst = const_cast<uint8_t*>(iv.data());
	for(int y = 0; y < iv.height(); ++y)
		for(int x = 0, w = iv.width(); x < w; ++x, src += 3)
			*dst++ = RGBToLum(src[0], src[1], src[2]);

	return iv;
#else
	return {}; // unreachable code
#endif
}

std::string WriteBarcodeToUtf8(const Barcode& barcode, [[maybe_unused]] const WriterOptions& options)
{
	auto iv = barcode.symbol();
	if (!iv.data())
		return {};

	constexpr auto map = std::array{" ", "▀", "▄", "█"};
	std::ostringstream res;
	bool inverted = false; // TODO: take from WriterOptions

	for (int y = 0; y < iv.height(); y += 2) {
		// for linear barcodes, only print line pairs that are distinct from the previous one
		if (IsLinearBarcode(barcode.format()) && y > 1 && y < iv.height() - 1
			&& memcmp(iv.data(0, y), iv.data(0, y - 2), 2 * iv.rowStride()) == 0)
			continue;

		for (int x = 0; x < iv.width(); ++x) {
			int tp = bool(*iv.data(x, y)) ^ inverted;
			int bt = (iv.height() == 1 && tp) || (y + 1 < iv.height() && (bool(*iv.data(x, y + 1)) ^ inverted));
			res << map[tp | (bt << 1)];
		}
		res << '\n';
	}

	return res.str();
}

} // namespace ZXing

#endif // ZXING_EXPERIMENTAL_API
