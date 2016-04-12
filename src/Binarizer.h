#pragma once

#include <memory>

namespace ZXing {

class LuminanceSource;
class BitArray;
class BitMatrix;

/**
* This class hierarchy provides a set of methods to convert luminance data to 1 bit data.
* It allows the algorithm to vary polymorphically, for example allowing a very expensive
* thresholding technique for servers and a fast one for mobile. It also permits the implementation
* to vary, e.g. a JNI version for Android and a Java fallback version for other platforms.
*/
class Binarizer
{
	std::shared_ptr<LuminanceSource> _source;

public:
	virtual ~Binarizer();

	std::shared_ptr<LuminanceSource> luminanceSource() const { return _source; }

	int width() const;

	int height() const;

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
	virtual bool getBlackRow(int y, BitArray& outArray) const = 0;

	/**
	* Converts a 2D array of luminance data to 1 bit data. As above, assume this method is expensive
	* and do not call it repeatedly. This method is intended for decoding 2D barcodes and may or
	* may not apply sharpening. Therefore, a row from this matrix may not be identical to one
	* fetched using getBlackRow(), so don't mix and match between them.
	*
	* @return The 2D array of bits for the image (true means black).
	* @throws NotFoundException if image can't be binarized to make a matrix
	*/
	virtual bool getBlackMatrix(BitMatrix& outMatrix) const = 0;

	/**
	* Creates a new object with the same type as this Binarizer implementation, but with pristine
	* state. This is needed because Binarizer implementations may be stateful, e.g. keeping a cache
	* of 1 bit data. See Effective Java for why we can't use Java's clone() method.
	*
	* @param source The LuminanceSource this Binarizer will operate on.
	* @return A new concrete Binarizer implementation object.
	*/
	virtual std::shared_ptr<Binarizer> createBinarizer(const std::shared_ptr<LuminanceSource>& source) const = 0;

protected:
	Binarizer(const std::shared_ptr<LuminanceSource>& source) : _source(source) {}
};

} // ZXing