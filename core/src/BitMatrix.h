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

#include "Matrix.h"
#include "Point.h"
#include "ZXConfig.h"

#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>

namespace ZXing {

class BitArray;
class ByteMatrix;

/**
* <p>Represents a 2D matrix of bits. In function arguments below, and throughout the common
* module, x is the column position, and y is the row position. The ordering is always x, y.
* The origin is at the top-left.</p>
*
* <p>Internally the bits are represented in a 1-D array of 32-bit ints. However, each row begins
* with a new int. This is done intentionally so that we can copy out a row into a BitArray very
* efficiently.</p>
*
* <p>The ordering of bits is row-major. Within each int, the least significant bits are used first,
* meaning they represent lower x values. This is compatible with BitArray's implementation.</p>
*
* @author Sean Owen
* @author dswitkin@google.com (Daniel Switkin)
*/
class BitMatrix
{
	int _width = 0;
	int _height = 0;
	int _rowSize = 0;
#ifdef ZX_FAST_BIT_STORAGE
	using data_t = uint8_t;
	//TODO: c++17 inline
	static constexpr data_t SET_V = 0xff; // allows playing with SIMD binarization
	static constexpr data_t UNSET_V = 0;
#else
	using data_t = uint32_t;
#endif
	std::vector<data_t> _bits;
	// There is nothing wrong to support this but disable to make it explicit since we may copy something very big here.
	// Use copy() below.
	BitMatrix(const BitMatrix&) = default;
	BitMatrix& operator=(const BitMatrix&) = delete;

	const data_t& get(int i) const {
#if 1
		return _bits.at(i);
#else
		return _bits[i];
#endif
	}

	data_t& get(int i) { return const_cast<data_t&>(static_cast<const BitMatrix*>(this)->get(i)); }

public:
	BitMatrix() = default;
#ifdef ZX_FAST_BIT_STORAGE
	static bool isSet(data_t v) { return v != 0; } // see SET_V above

	BitMatrix(int width, int height) : _width(width), _height(height), _rowSize(width), _bits(width * height, UNSET_V) {}
#else
	BitMatrix(int width, int height) : _width(width), _height(height), _rowSize((width + 31) / 32), _bits(((width + 31) / 32) * _height, 0) {}
#endif

	explicit BitMatrix(int dimension) : BitMatrix(dimension, dimension) {} // Construct a square matrix.

	BitMatrix(BitMatrix&& other) noexcept : _width(other._width), _height(other._height), _rowSize(other._rowSize), _bits(std::move(other._bits)) {}

	BitMatrix& operator=(BitMatrix&& other) noexcept {
		_width = other._width;
		_height = other._height;
		_rowSize = other._rowSize;
		_bits = std::move(other._bits);
		return *this;
	}

	BitMatrix copy() const {
		return *this;
	}

	[[deprecated]]
	ByteMatrix toByteMatrix(int black = 0, int white = 255) const;

#ifdef ZX_FAST_BIT_STORAGE
	// experimental iterator based access
	template<typename iterator>
	struct Row
	{
		iterator _begin, _end;
		iterator begin() noexcept { return _begin; }
		iterator end() noexcept { return _end; }
	};
	Row<data_t*> row(int y) { return {_bits.data() + y * _width, _bits.data() + (y + 1) * _width}; }
	Row<const data_t*> row(int y) const { return {_bits.data() + y * _width, _bits.data() + (y + 1) * _width}; }
#endif

	/**
	* <p>Gets the requested bit, where true means black.</p>
	*
	* @param x The horizontal component (i.e. which column)
	* @param y The vertical component (i.e. which row)
	* @return value of given bit in matrix
	*/
	bool get(int x, int y) const {
#ifdef ZX_FAST_BIT_STORAGE
		return isSet(get(y * _width + x));
#else
		return ((get(y * _rowSize + (x / 32)) >> (x & 0x1f)) & 1) != 0;
#endif
	}

	/**
	* <p>Sets the given bit to true.</p>
	*
	* @param x The horizontal component (i.e. which column)
	* @param y The vertical component (i.e. which row)
	*/
	void set(int x, int y) {
#ifdef ZX_FAST_BIT_STORAGE
		get(y * _width + x) = SET_V;
#else
		get(y * _rowSize + (x / 32)) |= 1 << (x & 0x1f);
#endif
	}

	void unset(int x, int y) {
#ifdef ZX_FAST_BIT_STORAGE
		get(y * _width + x) = UNSET_V;
#else
		get(y * _rowSize + (x / 32)) &= ~(1 << (x & 0x1f));
#endif
	}

	void set(int x, int y, bool val) {
#ifdef ZX_FAST_BIT_STORAGE
		get(y * _width + x) = val ? SET_V : UNSET_V;
#else
		val ? set(x, y) : unset(x, y);
#endif
	}

	/**
	* <p>Flips the given bit.</p>
	*
	* @param x The horizontal component (i.e. which column)
	* @param y The vertical component (i.e. which row)
	*/
	void flip(int x, int y) {
#ifdef ZX_FAST_BIT_STORAGE
		auto& v =get(y * _width + x);
		v = !v;
#else
		get(y * _rowSize + (x / 32)) ^= 1 << (x & 0x1f);
#endif
	}

	void flipAll() {
		for (auto& i : _bits) {
			i = ~i;
		}
	}

	/**
	* Clears all bits (sets to false).
	*/
	void clear() {
		std::fill(_bits.begin(), _bits.end(), 0);
	}

	/**
	* <p>Sets a square region of the bit matrix to true.</p>
	*
	* @param left The horizontal position to begin at (inclusive)
	* @param top The vertical position to begin at (inclusive)
	* @param width The width of the region
	* @param height The height of the region
	*/
	void setRegion(int left, int top, int width, int height);

	/**
	* A fast method to retrieve one row of data from the matrix as a BitArray.
	*
	* @param y The row to retrieve
	* @param row An optional caller-allocated BitArray, will be allocated if null or too small
	* @return The resulting BitArray - this reference should always be used even when passing
	*         your own row
	*/
	void getRow(int y, BitArray& row) const;

	/**
	* Modifies this {@code BitMatrix} to represent the same but rotated 90 degrees clockwise
	*/
	void rotate90();

	/**
	* Modifies this {@code BitMatrix} to represent the same but rotated 180 degrees
	*/
	void rotate180();

	void mirror();

	/**
	* Find the rectangle that contains all non-white pixels. Useful for detection of 'pure' barcodes.
	*
	* @return True iff this rectangle is at least minWidth x minHeight pixels big
	*/
	bool findBoundingBox(int &left, int& top, int& width, int& height, int minSize = 1) const;

	/**
	* This is useful in detecting a corner of a 'pure' barcode.
	*
	* @return {@code x,y} coordinate of top-left-most 1 bit, or null if it is all white
	*/
	bool getTopLeftOnBit(int &left, int& top) const;

	bool getBottomRightOnBit(int &right, int& bottom) const;

#ifdef ZX_FAST_BIT_STORAGE
	void getPatternRow(int r, std::vector<uint16_t>& p_row) const;
#endif

	/**
	* @return The width of the matrix
	*/
	int width() const {
		return _width;
	}

	/**
	* @return The height of the matrix
	*/
	int height() const {
		return _height;
	}

	/**
	* @return The row size of the matrix. That is the number of 32-bits blocks that one row takes.
	*/
	int rowSize() const {
		return _rowSize;
	}

	bool empty() const {
		return _bits.empty();
	}

	friend bool operator==(const BitMatrix& a, const BitMatrix& b)
	{
		return a._width == b._width && a._height == b._height && a._rowSize == b._rowSize && a._bits == b._bits;
	}

	bool isIn(PointI p, int b = 0) const noexcept
	{
		return b <= p.x && p.x < width() - b && b <= p.y && p.y < height() - b;
	}
	bool isIn(PointF p) const  noexcept { return isIn(PointI(p)); }

	bool get(PointI p) const { return get(p.x, p.y); }
	bool get(PointF p) const { return get(PointI(p)); }
	void set(PointI p, bool v = true) { set(p.x, p.y, v); }
	void set(PointF p, bool v = true) { set(PointI(p), v); }
};

/**
 * @brief Inflate scales a BitMatrix up and adds a quite Zone plus padding
 * @param matrix input to be expanded
 * @param width new width in bits (pixel)
 * @param height new height in bits (pixel)
 * @param quietZone size of quite zone to add in modules
 * @return expanded BitMatrix, maybe move(input) if size did not change
 */
BitMatrix Inflate(BitMatrix&& input, int width, int height, int quietZone);

/**
 * @brief Deflate (crop + subsample) a bit matrix
 * @param matrix
 * @param width new width
 * @param height new height
 * @param top cropping starts at top row
 * @param left cropping starts at left col
 * @param subSampling typically the module size
 * @return deflated input
 */
BitMatrix Deflate(const BitMatrix& matrix, int width, int height, int top, int left, int subSampling);

template<typename T>
BitMatrix ToBitMatrix(const Matrix<T>& in, T trueValue = {true})
{
	BitMatrix out(in.width(), in.height());
	for (int y = 0; y < in.height(); ++y)
		for (int x = 0; x < in.width(); ++x)
			if (in.get(x, y) == trueValue)
				out.set(x, y);
	return out;
}

template<typename T>
Matrix<T> ToMatrix(const BitMatrix& in, T black = 0, T white = ~0)
{
	Matrix<T> res(in.width(), in.height());
	for (int y = 0; y < in.height(); ++y)
		for (int x = 0; x < in.width(); ++x)
			res.set(x, y, in.get(x, y) ? black : white);
	return res;
}

} // ZXing
