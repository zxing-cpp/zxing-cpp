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
#include <string>

namespace ZXing {

class BitMatrix;

namespace DataMatrix {

enum class SymbolShape;

class Writer
{
public:
	Writer();

	Writer& setMargin(int margin) {
		_quiteZone = margin;
		return *this;
	}

	Writer& setShapeHint(SymbolShape shape) {
		_shapeHint = shape;
		return *this;
	}

	Writer& setMinSize(int width, int height) {
		_minWidth = width;
		_minHeight = height;
		return *this;
	}

	Writer& setMaxSize(int width, int height) {
		_maxWidth = width;
		_maxHeight = height;
		return *this;
	}

	BitMatrix encode(const std::wstring& contents, int width, int height) const;

private:
	SymbolShape _shapeHint;
	int _quiteZone = 1, _minWidth = -1, _minHeight = -1, _maxWidth = -1, _maxHeight = -1;
};

} // DataMatrix
} // ZXing
