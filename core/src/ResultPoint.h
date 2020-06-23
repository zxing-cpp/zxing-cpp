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

#include "Point.h"

namespace ZXing {

/**
* <p>Encapsulates a point of interest in an image containing a barcode. Typically, this
* would be the location of a finder pattern or the corner of the barcode, for example.</p>
*
* @author Sean Owen
*/
class ResultPoint : public PointF
{
public:
	ResultPoint() = default;
	ResultPoint(float x, float y) : PointF(x, y) {}
	ResultPoint(int x, int y) : PointF(x, y) {}
	template <typename T> ResultPoint(PointT<T> p) : PointF(p) {}

	float x() const { return static_cast<float>(PointF::x); }
	float y() const { return static_cast<float>(PointF::y); }

	void set(float x, float y) { *this = PointF(x, y); }

	static float Distance(int aX, int aY, int bX, int bY);
};

} // ZXing
