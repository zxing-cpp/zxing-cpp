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

#include "ResultPoint.h"

#include <vector>
#include <memory>

namespace ZXing {

class BitMatrix;

/**
* <p>Encapsulates the result of detecting a barcode in an image. This includes the raw
* matrix of black/white pixels corresponding to the barcode, and possibly points of interest
* in the image, like the location of finder patterns or corners of the barcode in the image.</p>
*
* @author Sean Owen
*/
class DetectorResult
{
	std::shared_ptr<const BitMatrix> _bits;
	std::vector<ResultPoint> _points;

public:
	DetectorResult() {}
	DetectorResult(const DetectorResult&) = delete;
	DetectorResult& operator=(const DetectorResult&) = delete;

	std::shared_ptr<const BitMatrix> bits() const { return _bits; }
	void setBits(const std::shared_ptr<const BitMatrix>& bits) { _bits = bits; }
	const std::vector<ResultPoint>& points() const { return _points; }
	void setPoints(const std::vector<ResultPoint>& points) { _points = points; }
	void setPoints(std::initializer_list<ResultPoint> list) { _points.assign(list); }
};

} // ZXing
