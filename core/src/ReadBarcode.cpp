/*
* Copyright 2019 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "ReadBarcode.h"
#include "ReaderOptions.h"
#include "BarcodeData.h"

#include <utility>

#if !defined(ZXING_READERS) && !defined(ZXING_WRITERS)
#include "Version.h"
#endif

#ifdef ZXING_READERS
#include "GlobalHistogramBinarizer.h"
#include "HybridBinarizer.h"
#include "MultiFormatReader.h"
#include "Pattern.h"
#include "ThresholdBinarizer.h"
#endif

#include <climits>
#include <memory>
#include <stdexcept>

namespace ZXing {

// ==============================================================================
// ReaderOptions implementation
// ==============================================================================

struct ReaderOptions::Data
{
	bool tryHarder                : 1;
	bool tryRotate                : 1;
	bool tryInvert                : 1;
	bool tryDownscale             : 1;
#ifdef ZXING_EXPERIMENTAL_API
	bool tryDenoise               : 1;
#endif
	bool isPure                   : 1;
	bool tryCode39ExtendedMode    : 1;
	bool validateCode39CheckSum   : 1;
	bool validateITFCheckSum      : 1;
	bool returnErrors             : 1;
	uint8_t downscaleFactor       : 3;
	EanAddOnSymbol eanAddOnSymbol : 2;
	Binarizer binarizer           : 2;
	TextMode textMode             : 3;
	CharacterSet characterSet     : 6;

	uint8_t minLineCount          = 2;
	uint8_t maxNumberOfSymbols    = 0xff;
	uint16_t downscaleThreshold   = 500;
	BarcodeFormats formats        = BarcodeFormat::None;

	Data()
		: tryHarder(1),
		  tryRotate(1),
		  tryInvert(1),
		  tryDownscale(1),
#ifdef ZXING_EXPERIMENTAL_API
		  tryDenoise(0),
#endif
		  isPure(0),
		  tryCode39ExtendedMode(1),
		  validateCode39CheckSum(0),
		  validateITFCheckSum(0),
		  returnErrors(0),
		  downscaleFactor(3),
		  eanAddOnSymbol(EanAddOnSymbol::Ignore),
		  binarizer(Binarizer::LocalAverage),
		  textMode(TextMode::HRI),
		  characterSet(CharacterSet::Unknown)
	{}
};

ReaderOptions::ReaderOptions() : d(std::make_unique<Data>()) {}
ReaderOptions::~ReaderOptions() = default;

// copy
ReaderOptions::ReaderOptions(const ReaderOptions& other) : d(std::make_unique<Data>(*other.d)) {}
ReaderOptions& ReaderOptions::operator=(const ReaderOptions& other)
{
	if (this != &other)
		d = std::make_unique<Data>(*other.d);
	return *this;
}

// move
ReaderOptions::ReaderOptions(ReaderOptions&&) = default;
ReaderOptions& ReaderOptions::operator=(ReaderOptions&&) = default;

#define ZX_PROPERTY(TYPE, NAME, SETTER) \
	TYPE ReaderOptions::NAME() const noexcept { return d->NAME; } \
	ReaderOptions& ReaderOptions::NAME(TYPE v) & { return (void)(d->NAME = std::move(v)), *this; } \
	ReaderOptions&& ReaderOptions::NAME(TYPE v) && { return (void)(d->NAME = std::move(v)), std::move(*this); }

ZX_PROPERTY(BarcodeFormats, formats, setFormats)
ZX_PROPERTY(bool, tryHarder, setTryHarder)
ZX_PROPERTY(bool, tryRotate, setTryRotate)
ZX_PROPERTY(bool, tryInvert, setTryInvert)
ZX_PROPERTY(bool, tryDownscale, setTryDownscale)
#ifdef ZXING_EXPERIMENTAL_API
ZX_PROPERTY(bool, tryDenoise, setTryDenoise)
#endif
ZX_PROPERTY(Binarizer, binarizer, setBinarizer)
ZX_PROPERTY(bool, isPure, setIsPure)
ZX_PROPERTY(uint16_t, downscaleThreshold, setDownscaleThreshold)
ZX_PROPERTY(uint8_t, downscaleFactor, setDownscaleFactor)
ZX_PROPERTY(uint8_t, minLineCount, setMinLineCount)
ZX_PROPERTY(uint8_t, maxNumberOfSymbols, setMaxNumberOfSymbols)
ZX_PROPERTY(bool, tryCode39ExtendedMode, setTryCode39ExtendedMode)
ZX_PROPERTY(bool, validateCode39CheckSum, setValidateCode39CheckSum)
ZX_PROPERTY(bool, validateITFCheckSum, setValidateITFCheckSum)
ZX_PROPERTY(bool, returnErrors, setReturnErrors)
ZX_PROPERTY(EanAddOnSymbol, eanAddOnSymbol, setEanAddOnSymbol)
ZX_PROPERTY(TextMode, textMode, setTextMode)
ZX_PROPERTY(CharacterSet, characterSet, setCharacterSet)

#undef ZX_PROPERTY

ReaderOptions& ReaderOptions::characterSet(std::string_view v) &
{
	d->characterSet = CharacterSetFromString(v);
	return *this;
}
ReaderOptions&& ReaderOptions::characterSet(std::string_view v) &&
{
	d->characterSet = CharacterSetFromString(v);
	return std::move(*this);
}

bool ReaderOptions::hasFormat(BarcodeFormats f) const noexcept
{
	return d->formats.testFlags(f) || d->formats.empty();
}


// ==============================================================================
// ReadBarcode implementation
// ==============================================================================

#ifdef ZXING_READERS

class LumImage : public Image
{
public:
	using Image::Image;

	uint8_t* data() { return const_cast<uint8_t*>(Image::data()); }
};

template<typename P>
static LumImage ExtractLum(const ImageView& iv, P projection)
{
	LumImage res(iv.width(), iv.height());

	auto* dst = res.data();
	for(int y = 0; y < iv.height(); ++y)
		for(int x = 0, w = iv.width(); x < w; ++x)
			*dst++ = projection(iv.data(x, y));

	return res;
}

class LumImagePyramid
{
	std::vector<LumImage> buffers;

	template<int N>
	void addLayer()
	{
		auto siv = layers.back();
		buffers.emplace_back(siv.width() / N, siv.height() / N);
		layers.push_back(buffers.back());
		auto& div = buffers.back();
		auto* d   = div.data();

		for (int dy = 0; dy < div.height(); ++dy)
			for (int dx = 0; dx < div.width(); ++dx) {
				int sum = (N * N) / 2;
				for (int ty = 0; ty < N; ++ty)
					for (int tx = 0; tx < N; ++tx)
						sum += *siv.data(dx * N + tx, dy * N + ty);
				*d++ = sum / (N * N);
			}
	}

	void addLayer(int factor)
	{
		// help the compiler's auto-vectorizer by hard-coding the scale factor
		switch (factor) {
		case 2: addLayer<2>(); break;
		case 3: addLayer<3>(); break;
		case 4: addLayer<4>(); break;
		default: throw std::invalid_argument("Invalid ReaderOptions::downscaleFactor"); break;
		}
	}

public:
	std::vector<ImageView> layers;

	LumImagePyramid(const ImageView& iv, int threshold, int factor)
	{
		if (factor < 2)
			throw std::invalid_argument("Invalid ReaderOptions::downscaleFactor");

		layers.push_back(iv);
		// TODO: if only matrix codes were considered, then using std::min would be sufficient (see #425)
		while (threshold > 0 && std::max(layers.back().width(), layers.back().height()) > threshold &&
			   std::min(layers.back().width(), layers.back().height()) >= factor)
			addLayer(factor);
#if 0
		// Reversing the layers means we'd start with the smallest. that can make sense if we are only looking for a
		// single symbol. If we start with the higher resolution, we get better (high res) position information.
		// TODO: see if masking out higher res layers based on found symbols in lower res helps overall performance.
		std::reverse(layers.begin(), layers.end());
#endif
	}
};

ImageView SetupLumImageView(ImageView iv, LumImage& lum, const ReaderOptions& opts)
{
	if (iv.format() == ImageFormat::None)
		throw std::invalid_argument("Invalid image format");

	if (opts.binarizer() == Binarizer::GlobalHistogram || opts.binarizer() == Binarizer::LocalAverage) {
		// manually spell out the 3 most common pixel formats to get at least gcc to vectorize the code
		if (iv.format() == ImageFormat::RGB && iv.pixStride() == 3) {
			lum = ExtractLum(iv, [](const uint8_t* src) { return RGBToLum(src[0], src[1], src[2]); });
		} else if (iv.format() == ImageFormat::RGBA && iv.pixStride() == 4) {
			lum = ExtractLum(iv, [](const uint8_t* src) { return RGBToLum(src[0], src[1], src[2]); });
		} else if (iv.format() == ImageFormat::BGR && iv.pixStride() == 3) {
			lum = ExtractLum(iv, [](const uint8_t* src) { return RGBToLum(src[2], src[1], src[0]); });
		} else if (iv.format() != ImageFormat::Lum) {
			lum = ExtractLum(iv, [r = RedIndex(iv.format()), g = GreenIndex(iv.format()), b = BlueIndex(iv.format())](
									 const uint8_t* src) { return RGBToLum(src[r], src[g], src[b]); });
		} else if (iv.pixStride() != 1) {
			// GlobalHistogram and LocalAverage need dense line memory layout
			lum = ExtractLum(iv, [](const uint8_t* src) { return *src; });
		}
		if (lum.data())
			return lum;
	}
	return iv;
}

std::unique_ptr<BinaryBitmap> CreateBitmap(ZXing::Binarizer binarizer, const ImageView& iv)
{
	switch (binarizer) {
	case Binarizer::BoolCast: return std::make_unique<ThresholdBinarizer>(iv, 0);
	case Binarizer::FixedThreshold: return std::make_unique<ThresholdBinarizer>(iv, 127);
	case Binarizer::GlobalHistogram: return std::make_unique<GlobalHistogramBinarizer>(iv);
	case Binarizer::LocalAverage: return std::make_unique<HybridBinarizer>(iv);
	}
	return {}; // silence gcc warning
}

Barcode ReadBarcode(const ImageView& _iv, const ReaderOptions& opts)
{
	return FirstOrDefault(ReadBarcodes(_iv, ReaderOptions(opts).maxNumberOfSymbols(1)));
}

Barcodes ReadBarcodes(const ImageView& _iv, const ReaderOptions& opts)
{
	if (sizeof(PatternType) < 4 && (_iv.width() > 0xffff || _iv.height() > 0xffff))
		throw std::invalid_argument("Maximum image width/height is 65535");

	if (!_iv.data() || _iv.width() * _iv.height() == 0)
		throw std::invalid_argument("ImageView is null/empty");

	LumImage lum;
	ImageView iv = SetupLumImageView(_iv, lum, opts);
	MultiFormatReader reader(opts);

	if (opts.isPure())
		return {FirstOrDefault(reader.read(*CreateBitmap(opts.binarizer(), iv), 1)).setReaderOptions(opts)};

	std::unique_ptr<MultiFormatReader> closedReader;
#ifdef ZXING_EXPERIMENTAL_API
	auto formatsBenefittingFromClosing = BarcodeFormat::Aztec | BarcodeFormat::DataMatrix | BarcodeFormat::QRCode | BarcodeFormat::MicroQRCode;
	ReaderOptions closedOptions = opts;
	if (opts.tryDenoise() && opts.hasFormat(formatsBenefittingFromClosing) && _iv.height() >= 3) {
		closedOptions.formats((opts.formats().empty() ? BarcodeFormat::Any : opts.formats()) & formatsBenefittingFromClosing);
		closedReader = std::make_unique<MultiFormatReader>(closedOptions);
	}
#endif
	LumImagePyramid pyramid(iv, opts.downscaleThreshold() * opts.tryDownscale(), opts.downscaleFactor());

	Barcodes res;
	int maxSymbols = opts.maxNumberOfSymbols() ? opts.maxNumberOfSymbols() : INT_MAX;
	for (auto&& iv : pyramid.layers) {
		auto bitmap = CreateBitmap(opts.binarizer(), iv);
		for (int close = 0; close <= (closedReader ? 1 : 0); ++close) {
			if (close) {
				// if we already inverted the image in the first round, we need to undo that first
				if (bitmap->inverted())
					bitmap->invert();
				bitmap->close();
			}

			// TODO: check if closing after invert would be beneficial
			for (int invert = 0; invert <= static_cast<int>(opts.tryInvert() && !close); ++invert) {
				if (invert)
					bitmap->invert();
				auto rs = (close ? *closedReader : reader).read(*bitmap, maxSymbols);
				for (auto& r : rs) {
					if (iv.width() != _iv.width())
						r.d->position = Scale(r.position(), _iv.width() / iv.width());
					if (!Contains(res, r)) {
						r.setReaderOptions(opts);
						r.d->isInverted = bitmap->inverted();
						res.push_back(std::move(r));
						--maxSymbols;
					}
				}
				if (maxSymbols <= 0)
					return res;
			}
		}
	}

	return res;
}

#else // ZXING_READERS

Barcode ReadBarcode(const ImageView&, const ReaderOptions&)
{
	throw std::runtime_error("This build of zxing-cpp does not support reading barcodes.");
}

Barcodes ReadBarcodes(const ImageView&, const ReaderOptions&)
{
	throw std::runtime_error("This build of zxing-cpp does not support reading barcodes.");
}

#endif // ZXING_READERS

} // ZXing
