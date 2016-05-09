#pragma once
/*
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
	float _x;
	float _y;

public:
	ResultPoint() : _x(0), _y(0) {}
	ResultPoint(float x, float y) : _x(x), _y(y) {}

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

	//String toString() {
	//	StringBuilder result = new StringBuilder(25);
	//	result.append('(');
	//	result.append(x);
	//	result.append(',');
	//	result.append(y);
	//	result.append(')');
	//	return result.toString();
	//}

	/**
	* Orders an array of three ResultPoints in an order [A,B,C] such that AB is less than AC
	* and BC is less than AC, and the angle between BC and BA is less than 180 degrees.
	*
	* @param patterns array of three {@code ResultPoint} to order
	*/
	static void OrderByBestPatterns(ResultPoint& p0, ResultPoint& p1, ResultPoint& p2);
	static void OrderByBestPatterns(const ResultPoint*& p0, const ResultPoint*& p1, const ResultPoint*& p2);

	static float Distance(const ResultPoint& a, const ResultPoint& b);
	static float Distance(float aX, float aY, float bX, float bY);
	static float Distance(int aX, int aY, int bX, int bY);

};

} // ZXing
