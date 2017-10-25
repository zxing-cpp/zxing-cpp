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

#include <memory>

namespace ZXing {

class BitArray;
class BitMatrix;

/**
* This class is the core bitmap class used by ZXing to represent 1 bit data. Reader objects
* accept a BinaryBitmap and attempt to decode it.
*
* @author dswitkin@google.com (Daniel Switkin)
*/
class BinaryBitmap
{
public:
	virtual ~BinaryBitmap() {}

	/**
	* Image is a pure monochrome image of a barcode.
	*/
	virtual bool isPureBarcode() const = 0;

	/**
	* @return The width of the bitmap.
	*/
	virtual int width() const = 0;

	/**
	* @return The height of the bitmap.
	*/
	virtual int height() const = 0;

	/**
	* Converts one row of luminance data to 1 bit data.
	* This method is intended for decoding 1D barcodes and may choose to apply sharpening.
	*
	* @param y The row to fetch, which must be in [0, bitmap height)
	* @param row An optional preallocated array. If null or too small, it will be ignored.
	*            If used, the Binarizer will call BitArray.clearBits(). Always use the returned object.
	* @return The array of bits for this row (true means black).
	* @throws NotFoundException if row can't be binarized
	*/
	virtual bool getBlackRow(int y, BitArray& outArray) const = 0;

	/**
	* Converts a 2D array of luminance data to 1 bit. This method is intended for decoding 2D
	* barcodes and may or may not apply sharpening. Therefore, a row from this matrix may not be
	* identical to one fetched using getBlackRow(), so don't mix and match between them.
	*
	* @return The 2D array of bits for the image (true means black).
	* @return null if image can't be binarized to make a matrix
	*/
	virtual std::shared_ptr<const BitMatrix> getBlackMatrix() const = 0;

	/**
	* @return Whether this bitmap can be cropped.
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
	virtual std::shared_ptr<BinaryBitmap> cropped(int left, int top, int width, int height) const = 0;

	/**
	* @return Whether this bitmap supports counter-clockwise rotation.
	*/
	virtual bool canRotate() const = 0;

	/**
	* Returns a new object with rotated image data by 90 degrees clockwise.
	* Only callable if {@link #isRotateSupported()} is true.
	*
	* @param degreeCW degree in clockwise direction, possible values are 90, 180 and 270
	* @return A rotated version of this object.
	*/
	virtual std::shared_ptr<BinaryBitmap> rotated(int degreeCW) const = 0;
};

} // ZXing
