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

namespace ZXing {

/**
* <p>This class implements a perspective transform in two dimensions. Given four source and four
* destination points, it will compute the transformation implied between them. The code is based
* directly upon section 3.4.2 of George Wolberg's "Digital Image Warping"; see pages 54-56.</p>
*
* @author Sean Owen
*/
class PerspectiveTransform
{
	float a11 = 1.0f;
	float a12 = 0.0f;
	float a13 = 0.0f;
	float a21 = 0.0f;
	float a22 = 1.0f;
	float a23 = 0.0f;
	float a31 = 0.0f;
	float a32 = 0.0f;
	float a33 = 1.0f;

public:
	PerspectiveTransform(float a11, float a21, float a31, float a12, float a22, float a32, float a13, float a23, float a33);

	void transformPoints(float* points, int count) const;
	void transformPoints(float* xValues, float* yValues, int count) const;

	PerspectiveTransform buildAdjoint() const;
	PerspectiveTransform times(const PerspectiveTransform& other) const;

	static PerspectiveTransform QuadrilateralToQuadrilateral(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, float x0p, float y0p, float x1p, float y1p, float x2p, float y2p, float x3p, float y3p);
	static PerspectiveTransform SquareToQuadrilateral(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3);
	static PerspectiveTransform QuadrilateralToSquare(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3);
};

inline PerspectiveTransform operator*(const PerspectiveTransform& a, const PerspectiveTransform& b) {
	return a.times(b);
}

} // ZXing
