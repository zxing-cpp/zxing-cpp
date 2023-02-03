/*
* Copyright 2020 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "BinaryBitmap.h"

#include "BitMatrix.h"

#include <mutex>

namespace ZXing {

struct BinaryBitmap::Cache
{
	std::once_flag once;
	std::shared_ptr<const BitMatrix> matrix;
};

BitMatrix BinaryBitmap::binarize(const uint8_t threshold) const
{
	BitMatrix res(width(), height());

	if (_buffer.pixStride() == 1 && _buffer.rowStride() == _buffer.width()) {
		// Specialize for a packed buffer with pixStride 1 to support auto vectorization (16x speedup on AVX2)
		auto dst = res.row(0).begin();
		for (auto src = _buffer.data(0, 0), end = _buffer.data(0, height()); src != end; ++src, ++dst)
			*dst = (*src <= threshold) * BitMatrix::SET_V;
	} else {
		auto processLine = [&res, threshold](int y, const auto* src, const int stride) {
			for (auto& dst : res.row(y)) {
				dst = (*src <= threshold) * BitMatrix::SET_V;
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

	return res;
}

BinaryBitmap::BinaryBitmap(const ImageView& buffer) : _cache(new Cache), _buffer(buffer) {}

BinaryBitmap::~BinaryBitmap() = default;

const BitMatrix* BinaryBitmap::getBitMatrix() const
{
	std::call_once(_cache->once, [&](){_cache->matrix = getBlackMatrix();});
	return _cache->matrix.get();
}

void BinaryBitmap::invert()
{
	if (auto matrix = const_cast<BitMatrix*>(getBitMatrix()))
		matrix->flipAll();
	_inverted = true;
}

} // ZXing
