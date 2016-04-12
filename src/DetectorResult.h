#pragma once
#include "utils/BitMatrix.h"
#include "ResultPoint.h"

#include <vector>

namespace ZXing {

/**
* <p>Encapsulates the result of detecting a barcode in an image. This includes the raw
* matrix of black/white pixels corresponding to the barcode, and possibly points of interest
* in the image, like the location of finder patterns or corners of the barcode in the image.</p>
*
* @author Sean Owen
*/
class DetectorResult
{
	BitMatrix _bits;
	std::vector<ResultPoint> _points;

public:
	DetectorResult(const BitMatrix& bits, const std::vector<ResultPoint>& points) : _bits(bits), _points(points) {}

	const BitMatrix& bits() const { return _bits; }
	const std::vector<ResultPoint>& points() const { return _points; }
};

} // ZXing
