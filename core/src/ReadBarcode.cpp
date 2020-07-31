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
#include "MultiFormatReader.h"
#include "GenericLuminanceSource.h"
#include "GlobalHistogramBinarizer.h"
#include "HybridBinarizer.h"
#include "BinaryBitmap.h"

#include "BitMatrix.h"
#include "BitArray.h"

#include <memory>

namespace ZXing {

class ThresholdBinarizer : public BinaryBitmap
{
	const ImageView _buffer;
	const uint8_t _threshold = 0;
	mutable std::shared_ptr<const BitMatrix> _cache;

public:
	ThresholdBinarizer(const ImageView& buffer, uint8_t threshold = 1) : _buffer(buffer), _threshold(threshold) {}

	int width() const override { return _buffer._width; }
	int height() const override { return _buffer._height; }

	bool getBlackRow(int y, BitArray& row) const override
	{
		const int channel = GreenIndex(_buffer._format);

		if (row.size() != width())
			row = BitArray(width());
		else
			row.clearBits();

		for (int x = 0; x < row.size(); ++x)
			if (_buffer.data(x, y)[channel] <= _threshold)
				row.set(x);

		return true;
	}

	std::shared_ptr<const BitMatrix> getBlackMatrix() const override
	{
		if (!_cache) {
			BitMatrix res(width(), height());
#ifdef ZX_FAST_BIT_STORAGE
			auto src = _buffer.data(0, 0) + GreenIndex(_buffer._format);
			for (int y = 0; y < res.height(); ++y)
				for (auto& dst : res.row(y))
					dst = *(src += _buffer._pixStride) <= _threshold;
#else
			const int channel = GreenIndex(_buffer._format);
			for (int y = 0; y < res.height(); ++y)
				for (int x = 0; x < res.width(); ++x)
					res.set(x, y, _buffer.data(x, y)[channel] <= _threshold);
#endif
			_cache = std::make_shared<const BitMatrix>(std::move(res));
		}
		return _cache;
	}
};

static Result ReadBarcode(GenericLuminanceSource&& source, const DecodeHints& hints)
{
	MultiFormatReader reader(hints);
	auto srcPtr = std::shared_ptr<LuminanceSource>(&source, [](void*) {});

	if (hints.binarizer() == Binarizer::LocalAverage)
		return reader.read(HybridBinarizer(srcPtr));
	else
		return reader.read(GlobalHistogramBinarizer(srcPtr));
}

Result ReadBarcode(const ImageView& iv, const DecodeHints& hints)
{
	switch (hints.binarizer()) {
	case Binarizer::BoolCast: return MultiFormatReader(hints).read(ThresholdBinarizer(iv, 0));
	case Binarizer::FixedThreshold: return MultiFormatReader(hints).read(ThresholdBinarizer(iv, 127));
	default:
		return ReadBarcode(
			{
				0,
				0,
				iv._width,
				iv._height,
				iv._data,
				iv._rowStride,
				iv._pixStride,
				RedIndex(iv._format),
				GreenIndex(iv._format),
				BlueIndex(iv._format),
				nullptr
			},
			hints);
	}
}

Result ReadBarcode(int width, int height, const uint8_t* data, int rowStride, BarcodeFormats formats, bool tryRotate,
				   bool tryHarder)
{
	return ReadBarcode({0, 0, width, height, data, rowStride, 1, 0, 0, 0, nullptr},
					   DecodeHints().setTryHarder(tryHarder).setTryRotate(tryRotate).setFormats(formats));
}

Result ReadBarcode(int width, int height, const uint8_t* data, int rowStride, int pixelStride, int rIndex, int gIndex,
				   int bIndex, BarcodeFormats formats, bool tryRotate, bool tryHarder)
{
	return ReadBarcode({0, 0, width, height, data, rowStride, pixelStride, rIndex, gIndex, bIndex, nullptr},
					   DecodeHints().setTryHarder(tryHarder).setTryRotate(tryRotate).setFormats(formats));
}

} // ZXing
