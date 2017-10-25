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

#include "aztec/AZDetector.h"
#include "aztec/AZDetectorResult.h"
#include "BitHacks.h"
#include "ZXNumeric.h"
#include "ReedSolomonDecoder.h"
#include "GenericGF.h"
#include "WhiteRectDetector.h"
#include "GridSampler.h"
#include "DecodeStatus.h"
#include "BitMatrix.h"

#include <array>

namespace ZXing {
namespace Aztec {

static const int EXPECTED_CORNER_BITS[] = {
	0xee0,  // 07340  XXX .XX X.. ...
	0x1dc,  // 00734  ... XXX .XX X..
	0x83b,  // 04073  X.. ... XXX .XX
	0x707,  // 03407 .XX X.. ... XXX
};

// return -1 in case of error
static int GetRotation(const std::array<int, 4>& sides, int length)
{
	// In a normal pattern, we expect to See
	//   **    .*             D       A
	//   *      *
	//
	//   .      *
	//   ..    ..             C       B
	//
	// Grab the 3 bits from each of the sides the form the locator pattern and concatenate
	// into a 12-bit integer.  Start with the bit at A
	int cornerBits = 0;
	for (int side : sides) {
		// XX......X where X's are orientation marks
		int t = ((side >> (length - 2)) << 1) + (side & 1);
		cornerBits = (cornerBits << 3) + t;
	}
	// Mov the bottom bit to the top, so that the three bits of the locator pattern at A are
	// together.  cornerBits is now:
	//  3 orientation bits at A || 3 orientation bits at B || ... || 3 orientation bits at D
	cornerBits = ((cornerBits & 1) << 11) + (cornerBits >> 1);
	// The result shift indicates which element of BullsEyeCorners[] goes into the top-left
	// corner. Since the four rotation values have a Hamming distance of 8, we
	// can easily tolerate two errors.
	for (int shift = 0; shift < 4; shift++) {
		if (BitHacks::CountBitsSet(cornerBits ^ EXPECTED_CORNER_BITS[shift]) <= 2) {
			return shift;
		}
	}
	return -1;
}

inline static bool IsValidPoint(int x, int y, int imgWidth, int imgHeight)
{
	return x >= 0 && x < imgWidth && y > 0 && y < imgHeight;
}

inline static bool IsValidPoint(const ResultPoint& point, int imgWidth, int imgHeight)
{
	return IsValidPoint(RoundToNearest(point.x()), RoundToNearest(point.y()), imgWidth, imgHeight);
}

//private static float distance(Point a, Point b) {
//	return MathUtils.distance(a.getX(), a.getY(), b.getX(), b.getY());
//}
//
//private static float distance(ResultPoint a, ResultPoint b) {
//	return MathUtils.distance(a.getX(), a.getY(), b.getX(), b.getY());
//}

/**
* Samples a line.
*
* @param p1   start point (inclusive)
* @param p2   end point (exclusive)
* @param size number of bits
* @return the array of bits as an int (first bit is high-order bit of result)
*/
static int SampleLine(const BitMatrix& image, const ResultPoint& p1, const ResultPoint& p2, int size)
{
	int result = 0;

	float d = ResultPoint::Distance(p1, p2);
	float moduleSize = d / size;
	float px = p1.x();
	float py = p1.y();
	float dx = moduleSize * (p2.x() - p1.x()) / d;
	float dy = moduleSize * (p2.y() - p1.y()) / d;
	for (int i = 0; i < size; i++) {
		if (image.get(RoundToNearest(px + i * dx), RoundToNearest(py + i * dy))) {
			result |= 1 << (size - i - 1);
		}
	}
	return result;
}

/**
* Corrects the parameter bits using Reed-Solomon algorithm.
*
* @param parameterData parameter bits
* @param compact true if this is a compact Aztec code
*/
static bool GetCorrectedParameterData(int64_t parameterData, bool compact, int& result)
{
	int numCodewords;
	int numDataCodewords;

	if (compact) {
		numCodewords = 7;
		numDataCodewords = 2;
	}
	else {
		numCodewords = 10;
		numDataCodewords = 4;
	}

	int numECCodewords = numCodewords - numDataCodewords;
	std::vector<int> parameterWords(numCodewords);
	for (int i = numCodewords - 1; i >= 0; --i) {
		parameterWords[i] = (int)parameterData & 0xF;
		parameterData >>= 4;
	}
	if (!ReedSolomonDecoder::Decode(GenericGF::AztecParam(), parameterWords, numECCodewords))
		return false;

	// Toss the error correction.  Just return the data as an integer
	result = 0;
	for (int i = 0; i < numDataCodewords; i++) {
		result = (result << 4) + parameterWords[i];
	}
	return true;
}

/**
* Extracts the number of data layers and data blocks from the layer around the bull's eye.
*
* @param bullsEyeCorners the array of bull's eye corners
* @throws NotFoundException in case of too many errors or invalid parameters
*/
static bool ExtractParameters(const BitMatrix& image, const std::array<ResultPoint, 4>& bullsEyeCorners, bool compact, int nbCenterLayers, int& nbLayers, int& nbDataBlocks, int& shift)
{
	if (!IsValidPoint(bullsEyeCorners[0], image.width(), image.height()) || !IsValidPoint(bullsEyeCorners[1], image.width(), image.height()) ||
		!IsValidPoint(bullsEyeCorners[2], image.width(), image.height()) || !IsValidPoint(bullsEyeCorners[3], image.width(), image.height())) {
		return false;
	}
	int length = 2 * nbCenterLayers;
	// Get the bits around the bull's eye
	std::array<int, 4> sides = {
		SampleLine(image, bullsEyeCorners[0], bullsEyeCorners[1], length), // Right side
		SampleLine(image, bullsEyeCorners[1], bullsEyeCorners[2], length), // Bottom 
		SampleLine(image, bullsEyeCorners[2], bullsEyeCorners[3], length), // Left side
		SampleLine(image, bullsEyeCorners[3], bullsEyeCorners[0], length)  // Top 
	};

	// bullsEyeCorners[shift] is the corner of the bulls'eye that has three 
	// orientation marks.  
	// sides[shift] is the row/column that goes from the corner with three
	// orientation marks to the corner with two.
	shift = GetRotation(sides, length);
	if (shift < 0) {
		return false;
	}

	// Flatten the parameter bits into a single 28- or 40-bit long
	int64_t parameterData = 0;
	for (int i = 0; i < 4; i++) {
		int side = sides[(shift + i) % 4];
		if (compact) {
			// Each side of the form ..XXXXXXX. where Xs are parameter data
			parameterData <<= 7;
			parameterData += (side >> 1) & 0x7F;
		}
		else {
			// Each side of the form ..XXXXX.XXXXX. where Xs are parameter data
			parameterData <<= 10;
			parameterData += ((side >> 2) & (0x1f << 5)) + ((side >> 1) & 0x1F);
		}
	}

	// Corrects parameter data using RS.  Returns just the data portion
	// without the error correction.
	int correctedData;
	if (!GetCorrectedParameterData(parameterData, compact, correctedData)) {
		return false;
	}

	if (compact) {
		// 8 bits:  2 bits layers and 6 bits data blocks
		nbLayers = (correctedData >> 6) + 1;
		nbDataBlocks = (correctedData & 0x3F) + 1;
	}
	else {
		// 16 bits:  5 bits layers and 11 bits data blocks
		nbLayers = (correctedData >> 11) + 1;
		nbDataBlocks = (correctedData & 0x7FF) + 1;
	}
	return true;
}

struct PixelPoint
{
	int x;
	int y;

	ResultPoint toResultPoint() const { return {x, y}; }
};


inline static float Distance(const PixelPoint& a, const PixelPoint& b)
{
	return ResultPoint::Distance(a.x, a.y, b.x, b.y);
}


/**
* Gets the color of a segment
*
* @return 1 if segment more than 90% black, -1 if segment is more than 90% white, 0 else
*/
static int GetColor(const BitMatrix& image, const PixelPoint& p1, const PixelPoint& p2)
{
	float d = Distance(p1, p2);
	float dx = (p2.x - p1.x) / d;
	float dy = (p2.y - p1.y) / d;
	int error = 0;

	float px = static_cast<float>(p1.x);
	float py = static_cast<float>(p1.y);

	bool colorModel = image.get(p1.x, p1.y);
	int iMax = (int)std::ceil(d);
	for (int i = 0; i < iMax; i++) {
		px += dx;
		py += dy;
		if (image.get(RoundToNearest(px), RoundToNearest(py)) != colorModel) {
			error++;
		}
	}

	float errRatio = error / d;

	if (errRatio > 0.1f && errRatio < 0.9f) {
		return 0;
	}

	return (errRatio <= 0.1f) == colorModel ? 1 : -1;
}

/**
* @return true if the border of the rectangle passed in parameter is compound of white points only
*         or black points only
*/
static bool IsWhiteOrBlackRectangle(const BitMatrix& image, const PixelPoint& pt1, const PixelPoint& pt2, const PixelPoint& pt3, const PixelPoint& pt4) {

	int corr = 3;

	PixelPoint p1{ pt1.x - corr, pt1.y + corr };
	PixelPoint p2{ pt2.x - corr, pt2.y - corr };
	PixelPoint p3{ pt3.x + corr, pt3.y - corr };
	PixelPoint p4{ pt4.x + corr, pt4.y + corr };

	int cInit = GetColor(image, p4, p1);

	if (cInit == 0) {
		return false;
	}

	int c = GetColor(image, p1, p2);

	if (c != cInit) {
		return false;
	}

	c = GetColor(image, p2, p3);

	if (c != cInit) {
		return false;
	}

	c = GetColor(image, p3, p4);

	return c == cInit;

}

/**
* Gets the coordinate of the first point with a different color in the given direction
*/
static PixelPoint GetFirstDifferent(const BitMatrix& image, const PixelPoint& init, bool color, int dx, int dy) {
	int x = init.x + dx;
	int y = init.y + dy;

	while (IsValidPoint(x, y, image.width(), image.height()) && image.get(x, y) == color) {
		x += dx;
		y += dy;
	}

	x -= dx;
	y -= dy;

	while (IsValidPoint(x, y, image.width(), image.height()) && image.get(x, y) == color) {
		x += dx;
	}
	x -= dx;

	while (IsValidPoint(x, y, image.width(), image.height()) && image.get(x, y) == color) {
		y += dy;
	}
	y -= dy;

	return PixelPoint{ x, y };
}

/**
* Expand the square represented by the corner points by pushing out equally in all directions
*
* @param cornerPoints the corners of the square, which has the bull's eye at its center
* @param oldSide the original length of the side of the square in the target bit matrix
* @param newSide the new length of the size of the square in the target bit matrix
* @return the corners of the expanded square
*/
static void ExpandSquare(std::array<ResultPoint, 4>& cornerPoints, float oldSide, float newSide)
{
	float ratio = newSide / (2 * oldSide);
	float dx = cornerPoints[0].x() - cornerPoints[2].x();
	float dy = cornerPoints[0].y() - cornerPoints[2].y();
	float centerx = (cornerPoints[0].x() + cornerPoints[2].x()) / 2.0f;
	float centery = (cornerPoints[0].y() + cornerPoints[2].y()) / 2.0f;

	cornerPoints[0] = ResultPoint(centerx + ratio * dx, centery + ratio * dy);
	cornerPoints[2] = ResultPoint(centerx - ratio * dx, centery - ratio * dy);

	dx = cornerPoints[1].x() - cornerPoints[3].x();
	dy = cornerPoints[1].y() - cornerPoints[3].y();
	centerx = (cornerPoints[1].x() + cornerPoints[3].x()) / 2.0f;
	centery = (cornerPoints[1].y() + cornerPoints[3].y()) / 2.0f;
	cornerPoints[1] = ResultPoint(centerx + ratio * dx, centery + ratio * dy);
	cornerPoints[3] = ResultPoint(centerx - ratio * dx, centery - ratio * dy);
}


/**
* Finds the corners of a bull-eye centered on the passed point.
* This returns the centers of the diagonal points just outside the bull's eye
* Returns [topRight, bottomRight, bottomLeft, topLeft]
*
* @param pCenter Center point
* @return The corners of the bull-eye
* @throws NotFoundException If no valid bull-eye can be found
*/
static bool GetBullsEyeCorners(const BitMatrix& image, const PixelPoint& pCenter, std::array<ResultPoint, 4>& result, bool& compact, int& nbCenterLayers)
{
	PixelPoint pina = pCenter;
	PixelPoint pinb = pCenter;
	PixelPoint pinc = pCenter;
	PixelPoint pind = pCenter;

	bool color = true;
	for (nbCenterLayers = 1; nbCenterLayers < 9; nbCenterLayers++) {
		PixelPoint pouta = GetFirstDifferent(image, pina, color, 1, -1);
		PixelPoint poutb = GetFirstDifferent(image, pinb, color, 1, 1);
		PixelPoint poutc = GetFirstDifferent(image, pinc, color, -1, 1);
		PixelPoint poutd = GetFirstDifferent(image, pind, color, -1, -1);

		//d      a
		//
		//c      b

		if (nbCenterLayers > 2) {
			float q = Distance(poutd, pouta) * nbCenterLayers / (Distance(pind, pina) * (nbCenterLayers + 2));
			if (q < 0.75 || q > 1.25 || !IsWhiteOrBlackRectangle(image, pouta, poutb, poutc, poutd)) {
				break;
			}
		}

		pina = pouta;
		pinb = poutb;
		pinc = poutc;
		pind = poutd;

		color = !color;
	}

	if (nbCenterLayers != 5 && nbCenterLayers != 7) {
		return false;
	}

	compact = nbCenterLayers == 5;

	// Expand the square by .5 pixel in each direction so that we're on the border
	// between the white square and the black square
	result[0] = ResultPoint(pina.x + 0.5f, pina.y - 0.5f);
	result[1] = ResultPoint(pinb.x + 0.5f, pinb.y + 0.5f);
	result[2] = ResultPoint(pinc.x - 0.5f, pinc.y + 0.5f);
	result[3] = ResultPoint(pind.x - 0.5f, pind.y - 0.5f);

	// Expand the square so that its corners are the centers of the points
	// just outside the bull's eye.
	ExpandSquare(result, static_cast<float>(2 * nbCenterLayers - 3), static_cast<float>(2 * nbCenterLayers));
	return true;
}

/**
* Finds a candidate center point of an Aztec code from an image
*
* @return the center point
*/
static PixelPoint GetMatrixCenter(const BitMatrix& image)
{
	//Get a white rectangle that can be the border of the matrix in center bull's eye or
	ResultPoint pointA, pointB, pointC, pointD;
	if (!WhiteRectDetector::Detect(image, pointA, pointB, pointC, pointD)) {
		// This exception can be in case the initial rectangle is white
		// In that case, surely in the bull's eye, we try to expand the rectangle.
		int cx = image.width() / 2;
		int cy = image.height() / 2;
		pointA = GetFirstDifferent(image, { cx + 7, cy - 7 }, false, 1, -1).toResultPoint();
		pointB = GetFirstDifferent(image, { cx + 7, cy + 7 }, false, 1, 1).toResultPoint();
		pointC = GetFirstDifferent(image, { cx - 7, cy + 7 }, false, -1, 1).toResultPoint();
		pointD = GetFirstDifferent(image, { cx - 7, cy - 7 }, false, -1, -1).toResultPoint();
	}

	//Compute the center of the rectangle
	int cx = RoundToNearest((pointA.x() + pointD.x() + pointB.x() + pointC.x()) / 4.0f);
	int cy = RoundToNearest((pointA.y() + pointD.y() + pointB.y() + pointC.y()) / 4.0f);

	// Redetermine the white rectangle starting from previously computed center.
	// This will ensure that we end up with a white rectangle in center bull's eye
	// in order to compute a more accurate center.
	if (!WhiteRectDetector::Detect(image, 15, cx, cy, pointA, pointB, pointC, pointD)) {
		// This exception can be in case the initial rectangle is white
		// In that case we try to expand the rectangle.
		pointA = GetFirstDifferent(image, { cx + 7, cy - 7 }, false, 1, -1).toResultPoint();
		pointB = GetFirstDifferent(image, { cx + 7, cy + 7 }, false, 1, 1).toResultPoint();
		pointC = GetFirstDifferent(image, { cx - 7, cy + 7 }, false, -1, 1).toResultPoint();
		pointD = GetFirstDifferent(image, { cx - 7, cy - 7 }, false, -1, -1).toResultPoint();
	}

	// Recompute the center of the rectangle
	cx = RoundToNearest((pointA.x() + pointD.x() + pointB.x() + pointC.x()) / 4.0f);
	cy = RoundToNearest((pointA.y() + pointD.y() + pointB.y() + pointC.y()) / 4.0f);

	return{ cx, cy };
}

static int GetDimension(bool compact, int nbLayers)
{
	if (compact) {
		return 4 * nbLayers + 11;
	}
	if (nbLayers <= 4) {
		return 4 * nbLayers + 15;
	}
	return 4 * nbLayers + 2 * ((nbLayers - 4) / 8 + 1) + 15;
}


/**
* Gets the Aztec code corners from the bull's eye corners and the parameters.
*
* @param bullsEyeCorners the array of bull's eye corners
* @return the array of aztec code corners
*/
static void GetMatrixCornerPoints(std::array<ResultPoint, 4>& bullsEyeCorners, bool compact, int nbLayers, int nbCenterLayers)
{
	ExpandSquare(bullsEyeCorners, static_cast<float>(2 * nbCenterLayers), static_cast<float>(GetDimension(compact, nbLayers)));
}

/**
* Creates a BitMatrix by sampling the provided image.
* topLeft, topRight, bottomRight, and bottomLeft are the centers of the squares on the
* diagonal just outside the bull's eye.
*/
static BitMatrix SampleGrid(const BitMatrix& image, const ResultPoint& topLeft, const ResultPoint& topRight, const ResultPoint& bottomRight, const ResultPoint& bottomLeft, bool compact, int nbLayers, int nbCenterLayers)
{
	int dimension = GetDimension(compact, nbLayers);

	float low = dimension / 2.0f - nbCenterLayers;
	float high = dimension / 2.0f + nbCenterLayers;

	return GridSampler::Instance()->sampleGrid(image,
		dimension,
		dimension,
		low, low,   // topleft
		high, low,  // topright
		high, high, // bottomright
		low, high,  // bottomleft
		topLeft.x(), topLeft.y(),
		topRight.x(), topRight.y(),
		bottomRight.x(), bottomRight.y(),
		bottomLeft.x(), bottomLeft.y());
}


DetectorResult Detector::Detect(const BitMatrix& image, bool isMirror)
{
	// 1. Get the center of the aztec matrix
	auto pCenter = GetMatrixCenter(image);

	// 2. Get the center points of the four diagonal points just outside the bull's eye
	//  [topRight, bottomRight, bottomLeft, topLeft]
	std::array<ResultPoint, 4> bullsEyeCorners;
	bool compact = false;
	int nbCenterLayers = 0;
	if (!GetBullsEyeCorners(image, pCenter, bullsEyeCorners, compact, nbCenterLayers)) {
		return {};
	}

	if (isMirror) {
		std::swap(bullsEyeCorners[0], bullsEyeCorners[2]);
	}

	// 3. Get the size of the matrix and other parameters from the bull's eye
	int nbLayers = 0;
	int nbDataBlocks = 0;
	int shift = 0;
	if (!ExtractParameters(image, bullsEyeCorners, compact, nbCenterLayers, nbLayers, nbDataBlocks, shift)) {
		return {};
	}

	// 4. Sample the grid
	auto bits = SampleGrid(image, bullsEyeCorners[shift % 4], bullsEyeCorners[(shift + 1) % 4], bullsEyeCorners[(shift + 2) % 4], bullsEyeCorners[(shift + 3) % 4], compact, nbLayers, nbCenterLayers);
	if (bits.empty())
		return {};

	// 5. Get the corners of the matrix.
	GetMatrixCornerPoints(bullsEyeCorners, compact, nbLayers, nbCenterLayers);

	return {std::move(bits), {bullsEyeCorners.begin(), bullsEyeCorners.end()}, compact, nbDataBlocks, nbLayers};
}

} // Aztec
} // ZXing
