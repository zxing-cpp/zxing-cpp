#pragma once
/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
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

#include "Point.h"
#include "Quadrilateral.h"

#include <array>

namespace ZXing {

/**
* <p>This class implements a perspective transform in two dimensions. Given four source and four
* destination points, it will compute the transformation implied between them. The code is based
* directly upon section 3.4.2 of George Wolberg's "Digital Image Warping"; see pages 54-56.</p>
*/
class PerspectiveTransform
{
	using value_t = PointF::value_t;
	value_t a11, a12, a13, a21, a22, a23, a31, a32, a33;
	bool _isValid = false;

	PerspectiveTransform(value_t a11, value_t a21, value_t a31, value_t a12, value_t a22, value_t a32, value_t a13,
						 value_t a23, value_t a33)
		: a11(a11), a12(a12), a13(a13), a21(a21), a22(a22), a23(a23), a31(a31), a32(a32), a33(a33), _isValid(true)
	{}

	PerspectiveTransform inverse() const;
	PerspectiveTransform times(const PerspectiveTransform& other) const;

	static PerspectiveTransform UnitSquareTo(const QuadrilateralF& q);

public:
	PerspectiveTransform(const QuadrilateralF& src, const QuadrilateralF& dst);

	/// Project from the destination space (grid of modules) into the image space (bit matrix)
	PointF operator()(PointF p) const;

	bool isValid() const { return _isValid; }
};

} // ZXing
