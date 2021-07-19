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
