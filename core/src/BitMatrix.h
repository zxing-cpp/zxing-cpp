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

#include <cstdint>
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
	std::vector<uint32_t> _bits;

	// There is nothing wrong to support this but disable to make it explicit since we may copy something very big here.
	// Use copy() below.
	BitMatrix(const BitMatrix&) = default;
	BitMatrix& operator=(const BitMatrix&) = delete;

public:
	BitMatrix() {}
	BitMatrix(int width, int height) : _width(width), _height(height), _rowSize((width + 31) / 32), _bits(((width + 31) / 32) * _height, 0) {}

	explicit BitMatrix(int dimension) : BitMatrix(dimension, dimension) {} // Construct a square matrix.

	BitMatrix(const ByteMatrix& other, int blackValue);

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

	/**
	* <p>Gets the requested bit, where true means black.</p>
	*
	* @param x The horizontal component (i.e. which column)
	* @param y The vertical component (i.e. which row)
	* @return value of given bit in matrix
	*/
	bool get(int x, int y) const {
		return ((_bits.at(y * _rowSize + (x / 32)) >> (x & 0x1f)) & 1) != 0;
	}

	/**
	* <p>Sets the given bit to true.</p>
	*
	* @param x The horizontal component (i.e. which column)
	* @param y The vertical component (i.e. which row)
	*/
	void set(int x, int y) {
		_bits.at(y * _rowSize + (x / 32)) |= 1 << (x & 0x1f);
	}

	void unset(int x, int y) {
		_bits.at(y * _rowSize + (x / 32)) &= ~(1 << (x & 0x1f));
	}

	/**
	* <p>Flips the given bit.</p>
	*
	* @param x The horizontal component (i.e. which column)
	* @param y The vertical component (i.e. which row)
	*/
	void flip(int x, int y) {
		_bits.at(y * _rowSize + (x / 32)) ^= 1 << (x & 0x1f);
	}

	void flipAll() {
		for (auto& i : _bits) {
			i = ~i;
		}
	}

	/**
	* Exclusive-or (XOR): Flip the bit in this {@code BitMatrix} if the corresponding
	* mask bit is set.
	*
	* @param mask XOR mask
	*/
	//void xor(const BitMatrix& mask);

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
	* @param y row to set
	* @param row {@link BitArray} to copy from
	*/
	void setRow(int y, const BitArray& row);

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
	* This is useful in detecting the enclosing rectangle of a 'pure' barcode.
	*
	* @return {@code left,top,width,height} enclosing rectangle of all 1 bits, or null if it is all white
	*/
	bool getEnclosingRectangle(int &left, int& top, int& width, int& height) const;

	/**
	* This is useful in detecting a corner of a 'pure' barcode.
	*
	* @return {@code x,y} coordinate of top-left-most 1 bit, or null if it is all white
	*/
	bool getTopLeftOnBit(int &left, int& top) const;

	bool getBottomRightOnBit(int &right, int& bottom) const;

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

} // ZXing
