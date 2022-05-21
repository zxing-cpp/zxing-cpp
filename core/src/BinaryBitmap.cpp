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

BinaryBitmap::BinaryBitmap(const ImageView& buffer) : _cache(new Cache), _buffer(buffer) {}

BinaryBitmap::~BinaryBitmap() = default;

const BitMatrix* BinaryBitmap::getBitMatrix() const
{
	std::call_once(_cache->once, [&](){_cache->matrix = getBlackMatrix();});
	return _cache->matrix.get();
}

} // ZXing
