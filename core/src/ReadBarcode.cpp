/*
* Copyright 2019 Axel Waggershauser
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

#include "ReadBarcode.h"

#include "DecodeHints.h"
#include "GlobalHistogramBinarizer.h"
#include "HybridBinarizer.h"
#include "MultiFormatReader.h"
#include "ThresholdBinarizer.h"

#include <memory>

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

static uint8_t RGBToGray(unsigned r, unsigned g, unsigned b)
{
	// .299R + 0.587G + 0.114B (YUV/YIQ for PAL and NTSC),
	// (306*R) >> 10 is approximately equal to R*0.299, and so on.
	// 0x200 >> 10 is 0.5, it implements rounding.
	return static_cast<uint8_t>((306 * r + 601 * g + 117 * b + 0x200) >> 10);
}

template<typename P>
static LumImage ExtractLum(const ImageView& iv, P projection)
{
	LumImage res(iv.width(), iv.height());

	auto* dst = res.data();
	for(int y = 0; y < iv.height(); ++y)
		for(int x = 0; x < iv.width(); ++x)
			*dst++ = projection(iv.data(x, y));

	return res;
}

ImageView SetupLumImageView(const ImageView& iv, LumImage& lum, const DecodeHints& hints)
{
	if (hints.binarizer() == Binarizer::GlobalHistogram || hints.binarizer() == Binarizer::LocalAverage) {
		if (iv.format() != ImageFormat::Lum) {
			lum = ExtractLum(iv, [r = RedIndex(iv.format()), g = GreenIndex(iv.format()), b = BlueIndex(iv.format())](
									 const uint8_t* src) { return RGBToGray(src[r], src[g], src[b]); });
		} else if (iv.pixStride() != 1) {
			// GlobalHistogram and LocalAverage need dense line memory layout
			lum = ExtractLum(iv, [](const uint8_t* src) { return *src; });
		}
		if (lum.data())
			return lum;
	}
	return iv;
}

Result ReadBarcode(const ImageView& _iv, const DecodeHints& hints)
{
	LumImage lum;
	ImageView iv = SetupLumImageView(_iv, lum, hints);

	switch (hints.binarizer()) {
	case Binarizer::BoolCast: return MultiFormatReader(hints).read(ThresholdBinarizer(iv, 0));
	case Binarizer::FixedThreshold: return MultiFormatReader(hints).read(ThresholdBinarizer(iv, 127));
	case Binarizer::GlobalHistogram: return MultiFormatReader(hints).read(GlobalHistogramBinarizer(iv));
	case Binarizer::LocalAverage: return MultiFormatReader(hints).read(HybridBinarizer(iv));
	}
}

Results ReadBarcodes(const ImageView& _iv, const DecodeHints& hints)
{
	LumImage lum;
	ImageView iv = SetupLumImageView(_iv, lum, hints);

	switch (hints.binarizer()) {
	case Binarizer::BoolCast: return MultiFormatReader(hints).readMultiple(ThresholdBinarizer(iv, 0));
	case Binarizer::FixedThreshold: return MultiFormatReader(hints).readMultiple(ThresholdBinarizer(iv, 127));
	case Binarizer::GlobalHistogram: return MultiFormatReader(hints).readMultiple(GlobalHistogramBinarizer(iv));
	case Binarizer::LocalAverage: return MultiFormatReader(hints).readMultiple(HybridBinarizer(iv));
	}
}

} // ZXing
