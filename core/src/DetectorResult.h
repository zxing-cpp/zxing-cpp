#pragma once
/*
* Copyright 2016 Nu-book Inc.
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

#include "BitMatrix.h"
#include "Quadrilateral.h"

#include <utility>

namespace ZXing {

/**
* Encapsulates the result of detecting a barcode in an image. This includes the raw
* matrix of black/white pixels corresponding to the barcode and the position of the code
* in the input image.
*/
class DetectorResult
{
	BitMatrix _bits;
	QuadrilateralI _position;

	DetectorResult(const DetectorResult&) = delete;
	DetectorResult& operator=(const DetectorResult&) = delete;

public:
	DetectorResult() = default;
	DetectorResult(DetectorResult&&) = default;
	DetectorResult& operator=(DetectorResult&&) = default;

	DetectorResult(BitMatrix&& bits, QuadrilateralI&& position)
		: _bits(std::move(bits)), _position(std::move(position))
	{}

	const BitMatrix& bits() const & { return _bits; }
	BitMatrix&& bits() && { return std::move(_bits); }
	const QuadrilateralI& position() const & { return _position; }
	QuadrilateralI&& position() && { return std::move(_position); }

	bool isValid() const { return !_bits.empty(); }
};

} // ZXing
