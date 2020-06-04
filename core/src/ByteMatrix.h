#pragma once
/*
* Copyright 2016 Huy Cuong Nguyen
* Copyright 2016 ZXing authors
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
#include "Matrix.h"

#include <cstdint>

namespace ZXing {

// TODO: If kept at all, this should be replaced by `using ByteMatrix = Matrix<uint8_t>;` to be consistent with ByteArray
// This non-template class is kept for now to stay source-compatible with oder versions of the library.

class ByteMatrix : public Matrix<int8_t>
{
public:
	ByteMatrix(int width, int height, int8_t val = 0) : Matrix<int8_t>(width, height, val) {}
	ByteMatrix(ByteMatrix&&) = default;
	ByteMatrix& operator=(ByteMatrix&&) = default;
};

} // ZXing
