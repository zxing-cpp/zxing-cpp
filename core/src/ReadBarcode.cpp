/*
* Copyright 2019 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "ReadBarcode.h"

#include "DecodeHints.h"
#include "GlobalHistogramBinarizer.h"
#include "HybridBinarizer.h"
#include "MultiFormatReader.h"
#include "Pattern.h"
#include "ThresholdBinarizer.h"

#include <climits>
#include <memory>
#include <stdexcept>

namespace ZXing {

class LumImage : public ImageView
{
	std::unique_ptr<uint8_t[]> _memory;
	LumImage(std::unique_ptr<uint8_t[]>&& data, int w, int h)
		: ImageView(data.get(), w, h, ImageFormat::Lum), _memory(std::move(data))
	{}

public:
	LumImage() : ImageView(nullptr, 0, 0, ImageFormat::Lum) {}
	LumImage(int w, int h) : LumImage(std::make_unique<uint8_t[]>(w * h), w, h) {}

	uint8_t* data() { return _memory.get(); }
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
		default: throw std::invalid_argument("Invalid DecodeHints::downscaleFactor"); break;
		}
	}

public:
	std::vector<ImageView> layers;

	LumImagePyramid(const ImageView& iv, int threshold, int factor)
	{
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

ImageView SetupLumImageView(ImageView iv, LumImage& lum, const DecodeHints& hints)
{
	if (iv.format() == ImageFormat::None)
		throw std::invalid_argument("Invalid image format");

	if (hints.binarizer() == Binarizer::GlobalHistogram || hints.binarizer() == Binarizer::LocalAverage) {
		if (iv.format() != ImageFormat::Lum) {
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

Result ReadBarcode(const ImageView& _iv, const DecodeHints& hints)
{
	return FirstOrDefault(ReadBarcodes(_iv, DecodeHints(hints).setMaxNumberOfSymbols(1)));
}

Results ReadBarcodes(const ImageView& _iv, const DecodeHints& hints)
{
	if (sizeof(PatternType) < 4 && hints.hasFormat(BarcodeFormat::LinearCodes) && (_iv.width() > 0xffff || _iv.height() > 0xffff))
		throw std::invalid_argument("maximum image width/height is 65535");

	LumImage lum;
	ImageView iv = SetupLumImageView(_iv, lum, hints);
	MultiFormatReader reader(hints);

	if (hints.isPure())
		return {reader.read(*CreateBitmap(hints.binarizer(), iv))};

	std::unique_ptr<MultiFormatReader> closedReader;
#ifdef ZXING_BUILD_EXPERIMENTAL_API
	auto formatsBenefittingFromClosing = BarcodeFormat::Aztec | BarcodeFormat::DataMatrix | BarcodeFormat::QRCode | BarcodeFormat::MicroQRCode;
	DecodeHints closedHints = hints;
	if (hints.tryDenoise() && hints.hasFormat(formatsBenefittingFromClosing)) {
		closedHints.setFormats((hints.formats().empty() ? BarcodeFormat::Any : hints.formats()) & formatsBenefittingFromClosing);
		closedReader = std::make_unique<MultiFormatReader>(closedHints);
	}
#endif
	LumImagePyramid pyramid(iv, hints.downscaleThreshold() * hints.tryDownscale(), hints.downscaleFactor());

	Results results;
	int maxSymbols = hints.maxNumberOfSymbols() ? hints.maxNumberOfSymbols() : INT_MAX;
	for (auto&& iv : pyramid.layers) {
		auto bitmap = CreateBitmap(hints.binarizer(), iv);
		for (int close = 0; close <= (closedReader ? 1 : 0); ++close) {
			if (close)
				bitmap->close();

			// TODO: check if closing after invert would be beneficial
			for (int invert = 0; invert <= static_cast<int>(hints.tryInvert() && !close); ++invert) {
				if (invert)
					bitmap->invert();
				auto rs = (close ? *closedReader : reader).readMultiple(*bitmap, maxSymbols);
				for (auto& r : rs) {
					if (iv.width() != _iv.width())
						r.setPosition(Scale(r.position(), _iv.width() / iv.width()));
					if (!Contains(results, r)) {
						r.setDecodeHints(hints);
						r.setIsInverted(bitmap->inverted());
						results.push_back(std::move(r));
						--maxSymbols;
					}
				}
				if (maxSymbols <= 0)
					return results;
			}
		}
	}

	return results;
}

} // ZXing
