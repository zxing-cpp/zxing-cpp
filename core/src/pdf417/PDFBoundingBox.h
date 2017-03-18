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

#include "ZXNullable.h"
#include "ResultPoint.h"

namespace ZXing {
namespace Pdf417 {

/**
* @author Guenther Grau
*/
class BoundingBox
{
	int _imgWidth;
	int _imgHeight;
	Nullable<ResultPoint> _topLeft;
	Nullable<ResultPoint> _bottomLeft;
	Nullable<ResultPoint> _topRight;
	Nullable<ResultPoint> _bottomRight;
	int _minX;
	int _maxX;
	int _minY;
	int _maxY;

public:
	BoundingBox();

	int minX() const {
		return _minX;
	}

	int maxX() const {
		return _maxX;
	}

	int minY() const {
		return _minY;
	}

	int maxY() const {
		return _maxY;
	}

	Nullable<ResultPoint> topLeft() const {
		return _topLeft;
	}

	Nullable<ResultPoint> topRight() const {
		return _topRight;
	}

	Nullable<ResultPoint> bottomLeft() const {
		return _bottomLeft;
	}

	Nullable<ResultPoint> bottomRight() const {
		return _bottomRight;
	}

	static bool Create(int imgWidth, int imgHeight, const Nullable<ResultPoint>& topLeft, const Nullable<ResultPoint>& bottomLeft, const Nullable<ResultPoint>& topRight, const Nullable<ResultPoint>& bottomRight, BoundingBox& result);
	static bool Merge(const Nullable<BoundingBox>& leftBox, const Nullable<BoundingBox>& rightBox, Nullable<BoundingBox>& result);
	static bool AddMissingRows(const BoundingBox&box, int missingStartRows, int missingEndRows, bool isLeft, BoundingBox& result);

private:
	void calculateMinMaxValues();
};

} // Pdf417
} // ZXing
