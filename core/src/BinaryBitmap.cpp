/*
* Copyright 2020 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "BinaryBitmap.h"

#include <mutex>

namespace ZXing {

struct BinaryBitmap::Cache
{
	std::once_flag once;
	std::shared_ptr<const BitMatrix> matrix;
};

static inline bool _positive(int value, int threshold)
{
	return value <= threshold;
}

static inline bool _negative(int value, int threshold)
{
	return value > threshold;
}

BinaryBitmap::BinaryBitmap(const ImageView& buffer, bool invert) : _cache(new Cache), _buffer(buffer), _step(invert ? _negative : _positive) {}

BinaryBitmap::~BinaryBitmap() = default;

const BitMatrix* BinaryBitmap::getBitMatrix() const
{
	std::call_once(_cache->once, [&](){_cache->matrix = getBlackMatrix();});
	return _cache->matrix.get();
}

} // ZXing
