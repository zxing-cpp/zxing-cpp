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
* <p>Encapsulates a point of interest in an image containing a barcode. Typically, this
* would be the location of a finder pattern or the corner of the barcode, for example.</p>
*
* @author Sean Owen
*/
class ResultPoint
{
	float _x = 0.f;
	float _y = 0.f;

public:
	ResultPoint() {}
	ResultPoint(float x, float y) : _x(x), _y(y) {}
	ResultPoint(int x, int y) : _x(static_cast<float>(x)), _y(static_cast<float>(y)) {}

	float x() const {
		return _x;
	}

	float y() const {
		return _y;
	}

	bool operator==(const ResultPoint& other) const
	{
		return _x == other._x && _y == other._y;
	}

	void set(float x, float y) {
		_x = x;
		_y = y;
	}

	static float Distance(const ResultPoint& a, const ResultPoint& b);
	static float Distance(float aX, float aY, float bX, float bY);
	static float Distance(int aX, int aY, int bX, int bY);

	static float SquaredDistance(const ResultPoint& a, const ResultPoint& b);
};

} // ZXing
