/*
* Copyright 2020 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "BinaryBitmap.h"
#include "BitMatrix.h"

#include <cstdint>

namespace ZXing {

class ThresholdBinarizer : public BinaryBitmap
{
	const uint8_t _threshold = 0;

public:
	ThresholdBinarizer(const ImageView& buffer, uint8_t threshold = 1) : BinaryBitmap(buffer), _threshold(threshold) {}

	bool getPatternRow(int row, int rotation, PatternRow& res) const override
	{
		auto buffer = _buffer.rotated(rotation);

		const int stride = buffer.pixStride();
		const uint8_t* begin = buffer.data(0, row) + GreenIndex(buffer.format());
		const uint8_t* end = begin + buffer.width() * stride;

		auto* lastPos = begin;
		bool lastVal = false;

		res.clear();

		for (const uint8_t* p = begin; p < end; p += stride) {
			bool val = *p <= _threshold;
			if (val != lastVal) {
				res.push_back(narrow_cast<PatternRow::value_type>(p - lastPos) / stride);
				lastVal = val;
				lastPos = p;
			}
		}

		res.push_back(narrow_cast<PatternRow::value_type>(end - lastPos) / stride);

		if (*(end - stride) <= _threshold)
			res.push_back(0); // last value is number of white pixels, here 0

		return true;
	}

	std::shared_ptr<const BitMatrix> getBlackMatrix() const override
	{
		BitMatrix res(width(), height());

		if (_buffer.pixStride() == 1 && _buffer.rowStride() == _buffer.width()) {
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
				auto src = _buffer.data(0, y) + GreenIndex(_buffer.format());
				// Specialize the inner loop for strides 1 and 4 to support auto vectorization
				switch (_buffer.pixStride()) {
				case 1: processLine(y, src, 1); break;
				case 4: processLine(y, src, 4); break;
				default: processLine(y, src, _buffer.pixStride()); break;
				}
			}
		}

		return std::make_shared<const BitMatrix>(std::move(res));
	}
};

} // ZXing
