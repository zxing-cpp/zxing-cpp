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

namespace ZXing {

class ByteMatrix
{
	int _width;
	int _height;
	std::vector<int8_t> _data;

public:
	ByteMatrix(int width, int height, int val = 0) : _width(width), _height(height), _data(_width * _height, val) { }

	ByteMatrix() : _width(0), _height(0) { }

	void init(int width, int height, int val = 0)
	{
		_width = width;
		_height = height;
		_data.clear();
		_data.resize(_width * _height, val);
	}

	int height() const {
		return _height;
	}

	int width() const {
		return _width;
	}

	int8_t get(int x, int y) const {
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
	const int8_t* getArray() const {
		return _data.data();
	}

	int8_t* getArray() {
		return _data.data();
	}

	void clear(int8_t value) {
		memset(_data.data(), value, _data.size());
	}
};

} // ZXing
