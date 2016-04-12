#pragma once

namespace ZXing {

/**
* <p>Encapsulates a point of interest in an image containing a barcode. Typically, this
* would be the location of a finder pattern or the corner of the barcode, for example.</p>
*/
class ResultPoint
{
	float _x;
	float _y;

public:
	ResultPoint() : _x(0), _y(0) {}
	ResultPoint(float x, float y) : _x(x), _y(y) {}

	float x() const {
		return _x;
	}

	float y() const {
		return _y;
	}

	bool operator==(const ResultPoint& other) const
	{
		return x == other.x && y == other.y;
	}

	//String toString() {
	//	StringBuilder result = new StringBuilder(25);
	//	result.append('(');
	//	result.append(x);
	//	result.append(',');
	//	result.append(y);
	//	result.append(')');
	//	return result.toString();
	//}

	/**
	* Orders an array of three ResultPoints in an order [A,B,C] such that AB is less than AC
	* and BC is less than AC, and the angle between BC and BA is less than 180 degrees.
	*
	* @param patterns array of three {@code ResultPoint} to order
	*/
	static void OrderByBestPatterns(ResultPoint* patterns);
};

} // ZXing
