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
#include "ZXContainerAlgorithms.h"

#include <algorithm>
#include <cassert>
#include <vector>

namespace ZXing {

template <class T>
class Matrix
{
public:
	using value_t = T;

private:
	int _width = 0;
	int _height = 0;
	std::vector<value_t> _data;

	// Nothing wrong to support it, just to make it explicit, instead of by mistake.
	// Use copy() below.
	Matrix(const Matrix &) = default;
	Matrix& operator=(const Matrix &) = delete;

public:
	Matrix() = default;
	Matrix(int width, int height, value_t val = {}) : _width(width), _height(height), _data(_width * _height, val) { }

	Matrix(Matrix&&) = default;
	Matrix& operator=(Matrix&&) = default;

	Matrix copy() const {
		return *this;
	}

	int height() const {
		return _height;
	}

	int width() const {
		return _width;
	}

	int size() const {
		return Size(_data);
	}

	value_t& operator()(int x, int y)
	{
		assert(x >= 0 && x < _width && y >= 0 && y < _height);
		return _data[y * _width + x];
	}

	const T& operator()(int x, int y) const
	{
		assert(x >= 0 && x < _width && y >= 0 && y < _height);
		return _data[y * _width + x];
	}

	const value_t& get(int x, int y) const {
		return operator()(x, y);
	}

	void set(int x, int y, value_t value) {
		operator()(x, y) = value;
	}

	const value_t* data() const {
		return _data.data();
	}

	const value_t* begin() const {
		return _data.data();
	}

	const value_t* end() const {
		return _data.data() + _width * _height;
	}

	void clear(value_t value = {}) {
		std::fill(_data.begin(), _data.end(), value);
	}
};

} // ZXing
