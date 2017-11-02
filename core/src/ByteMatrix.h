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
#include <vector>
#include <cstdint>
#include <cstring>

namespace ZXing {

class ByteMatrix
{
	int _width = 0;
	int _height = 0;
	std::vector<int8_t> _data;

	// Nothing wrong to support it, just to make it explicit, instead of by mistake.
	// Use copy() below.
	ByteMatrix(const ByteMatrix &) = default;
	ByteMatrix& operator=(const ByteMatrix &) = delete;

public:
	ByteMatrix() = default;
	ByteMatrix(int width, int height, int val = 0) : _width(width), _height(height), _data(_width * _height, val) { }

	ByteMatrix(ByteMatrix&&) = default;
	ByteMatrix& operator=(ByteMatrix&&) = default;

	ByteMatrix copy() const {
		return *this;
	}

	int height() const {
		return _height;
	}

	int width() const {
		return _width;
	}

	int size() const {
		return static_cast<int>(_data.size());
	}

	const int8_t& get(int x, int y) const {
		return _data[y *_width + x];
	}

	void set(int x, int y, int8_t value) {
		_data[y *_width + x] = value;
	}

	void set(int x, int y, int value) {
		_data[y *_width + x] = static_cast<int8_t>(value);
	}

	void set(int x, int y, bool value) {
		_data[y * _width + x] = static_cast<int8_t>(value);
	}

	/**
	* @return an internal representation as bytes, in row-major order. array[y * width() + x] represents point (x,y)
	*/

	const int8_t* begin() const {
		return _data.data();
	}

	const int8_t* end() const {
		return _data.data() + _width * _height;
	}

	void clear(int8_t value) {
		memset(_data.data(), value, _data.size());
	}
};

} // ZXing
