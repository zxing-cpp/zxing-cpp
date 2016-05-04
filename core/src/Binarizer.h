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

#include <memory>

namespace ZXing {

class BitArray;
class BitMatrix;
enum class ErrorStatus;

/**
* This class hierarchy provides a set of methods to convert luminance data to 1 bit data.
* It allows the algorithm to vary polymorphically, for example allowing a very expensive
* thresholding technique for servers and a fast one for mobile. It also permits the implementation
* to vary, e.g. a JNI version for Android and a Java fallback version for other platforms.
*
* @author dswitkin@google.com (Daniel Switkin)
*/
class Binarizer
{
public:
	virtual ~Binarizer() {}

	virtual int width() const = 0;

	virtual int height() const = 0;

	/**
	* Converts one row of luminance data to 1 bit data. May actually do the conversion, or return
	* cached data. Callers should assume this method is expensive and call it as seldom as possible.
	* This method is intended for decoding 1D barcodes and may choose to apply sharpening.
	* For callers which only examine one row of pixels at a time, the same BitArray should be reused
	* and passed in with each call for performance. However it is legal to keep more than one row
	* at a time if needed.
	*
	* @param y The row to fetch, which must be in [0, bitmap height)
	* @param row An optional preallocated array. If null or too small, it will be ignored.
	*            If used, the Binarizer will call BitArray.clear(). Always use the returned object.
	* @return The array of bits for this row (true means black).
	* @throws NotFoundException if row can't be binarized
	*/
	virtual ErrorStatus getBlackRow(int y, BitArray& outArray) const = 0;

	/**
	* Converts a 2D array of luminance data to 1 bit data. As above, assume this method is expensive
	* and do not call it repeatedly. This method is intended for decoding 2D barcodes and may or
	* may not apply sharpening. Therefore, a row from this matrix may not be identical to one
	* fetched using getBlackRow(), so don't mix and match between them.
	*
	* @return The 2D array of bits for the image (true means black).
	* @throws NotFoundException if image can't be binarized to make a matrix
	*/
	virtual ErrorStatus getBlackMatrix(BitMatrix& outMatrix) const = 0;

	/**
	* @return Whether this subclass supports cropping.
	*/
	virtual bool canCrop() const = 0;

	/**
	* Returns a new object with cropped image data. Implementations may keep a reference to the
	* original data rather than a copy. Only callable if isCropSupported() is true.
	*
	* @param left The left coordinate, which must be in [0,getWidth())
	* @param top The top coordinate, which must be in [0,getHeight())
	* @param width The width of the rectangle to crop.
	* @param height The height of the rectangle to crop.
	* @return A cropped version of this object.
	*/
	virtual std::shared_ptr<Binarizer> cropped(int left, int top, int width, int height) const = 0;

	/**
	* @return Whether this subclass supports counter-clockwise rotation.
	*/
	virtual bool canRotate() const = 0;

	/**
	* Returns a new object with rotated image data by 90 degrees counterclockwise.
	* Only callable if {@link #isRotateSupported()} is true.
	*
	* @return A rotated version of this object.
	*/
	virtual std::shared_ptr<Binarizer> rotatedCCW90() const = 0;

	/**
	* Returns a new object with rotated image data by 45 degrees counterclockwise.
	* Only callable if {@link #isRotateSupported()} is true.
	*
	* @return A rotated version of this object.
	*/
	virtual std::shared_ptr<Binarizer> rotatedCCW45() const = 0;
};

} // ZXing
