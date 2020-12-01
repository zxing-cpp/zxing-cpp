#pragma once
/*
* Copyright 2020 Axel Waggershauser
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

#include "BinaryBitmap.h"
#include "BitMatrix.h"
#include "ReadBarcode.h"

#include <cstdint>
#include <memory>

namespace ZXing {

class LuminanceSource;

class ThresholdBinarizer : public BinaryBitmap
{
	const ImageView _buffer;
	const uint8_t _threshold = 0;
	mutable std::shared_ptr<const BitMatrix> _cache;

public:
	ThresholdBinarizer(const ImageView& buffer, uint8_t threshold = 1) : _buffer(buffer), _threshold(threshold) {}

	int width() const override { return _buffer._width; }
	int height() const override { return _buffer._height; }

	bool getPatternRow(int y, PatternRow& res) const override
	{
		const int stride = _buffer._pixStride;
		const uint8_t* begin = _buffer.data(0, y) + GreenIndex(_buffer._format);
		const uint8_t* end = begin + _buffer._width * stride;

		auto* lastPos = begin;
		bool lastVal = false;

		for (const uint8_t* p = begin; p < end; p += stride) {
			bool val = *p <= _threshold;
			if (val != lastVal) {
				res.push_back(static_cast<PatternRow::value_type>(p - lastPos) / stride);
				lastVal = val;
				lastPos = p;
			}
		}

		res.push_back(static_cast<PatternRow::value_type>(end - lastPos) / stride);

		if (*(end - stride) <= _threshold)
			res.push_back(0); // last value is number of white pixels, here 0

		return true;
	}

	std::shared_ptr<const BitMatrix> getBlackMatrix() const override
	{
		if (!_cache) {
			BitMatrix res(width(), height());
#ifdef ZX_FAST_BIT_STORAGE
			if (_buffer._pixStride == 1 && _buffer._rowStride == _buffer._width) {
				// Specialize for a packed buffer with pixStride 1 to support auto vectorization (16x speedup on AVX2)
				auto dst = res.row(0).begin();
				for (auto src = _buffer.data(0, 0), end = _buffer.data(0, height()); src != end; ++src, ++dst)
					*dst = *src <= _threshold;
			} else {
				auto processLine = [this, &res](int y, const auto* src, const int stride) {
					for (auto& dst : res.row(y)) {
						dst = *src <= _threshold;
						src += stride;
					}
				};
				for (int y = 0; y < res.height(); ++y) {
					auto src = _buffer.data(0, y) + GreenIndex(_buffer._format);
					// Specialize the inner loop for strides 1 and 4 to support auto vectorization
					switch (_buffer._pixStride) {
					case 1: processLine(y, src, 1); break;
					case 4: processLine(y, src, 4); break;
					default: processLine(y, src, _buffer._pixStride); break;
					}
				}
			}
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

} // ZXing
