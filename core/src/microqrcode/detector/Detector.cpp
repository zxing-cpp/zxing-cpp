/*
 * Copyright 2017 KURZ Digital Solutions GmbH & Co. KG
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
/**
 * <p>Encapsulates logic that can detect a Micro QR Code in an image, even if the QR Code
 * is rotated or skewed, or partially obscured.</p>
 */

#include "Detector.h"

#include "DecodeHints.h"
#include "Dimension.h"
#include "FinderPattern.h"
#include "FinderPatternFinder.h"
#include "GridSampler.h"
#include "ReaderException.h"

#include <algorithm> // vs12, std::min and std:max
#include <cstdlib>
#include <sstream>

using ZXing::BitMatrix;
using ZXing::DetectorResult;
using ZXing::PerspectiveTransform;
using ZXing::MicroQRCode::Detector;

// VC++
using ZXing::DecodeHints;
using ZXing::ResultPoint;
using ZXing::MicroQRCode::FinderPatternFinder;
using ZXing::MicroQRCode::FinderPatternInfo;

Detector::Detector(const BitMatrix& image)
{
	image_ = image.copy();
}

/**
 * <p>Detects a Micro QR Code in an image.</p>
 *
 * @param hints optional hints to detector
 * @return {@link DetectorResult} encapsulating results of detecting a Micro QR Code
 * @throws NotFoundException if Micro QR Code cannot be found
 */
DetectorResult Detector::detect(DecodeHints const& hints) const
{
	if (hints.isPure())
		return detectPure();

	FinderPatternFinder finder(image_);
	const auto codeEnclosingRect = finder.findCorners(hints);
	const auto patternInfo = finder.findCenters(hints);
	float moduleSize = patternInfo.getActualTopLeft().getEstimatedModuleSize();
	if (moduleSize < 2.0f)
		throw ReaderException("Module size too small.");

	// Calculating dimension from centers and from corners as the center dimension is highly vulnerable for
	// perspective transformed Micro QR Codes. Therefore, if the two dimensions differ we will work with the
	// code enclosing rect for detection. We are not using this rect for every detection as there are some cases
	// in which not all 4 corners of the Micro QR Code are detected correctly. The detection with the fake centers
	// depends only on 3 corners (tl, tr, bl) and  will therefore give better results in many situations.
	int dimensionFromCenters = computeDimension(patternInfo.getActualTopLeft(), patternInfo.getFakeTopRight(),
												patternInfo.getFakeBottomLeft(), moduleSize);
	int dimensionFromCorners = computeDimension(codeEnclosingRect, moduleSize);
	if (dimensionFromCenters != dimensionFromCorners) {
		return processCodeEnclosingRect(codeEnclosingRect, dimensionFromCorners);
	}
	return processFinderPatternInfo(patternInfo, dimensionFromCenters);
}

DetectorResult Detector::detectPure() const
{
	// Now need to determine module size in pixels
	int height = image_.height();
	int width = image_.width();
	int minDimension = std::min(height, width);

	// First, skip white border by tracking diagonally from the top left down and to the right:
	int borderWidth = 0;
	while (borderWidth < minDimension && !image_.get(borderWidth, borderWidth)) {
		borderWidth++;
	}
	if (borderWidth == minDimension) {
		throw ReaderException("border width equal to min dimension");
	}

	// And then keep tracking across the top-left black module to determine module size
	int moduleEnd = borderWidth;
	while (moduleEnd < minDimension && image_.get(moduleEnd, moduleEnd)) {
		moduleEnd++;
	}
	if (moduleEnd == minDimension) {
		throw ReaderException("module end equal to min dimension");
	}

	int moduleSize = moduleEnd - borderWidth;

	// And now find where the rightmost black module on the first row ends
	int rowEndOfSymbol = width - 1;
	while (rowEndOfSymbol >= 0 && !image_.get(rowEndOfSymbol, borderWidth)) {
		rowEndOfSymbol--;
	}
	if (rowEndOfSymbol < 0) {
		throw ReaderException("Row end of symbol less than 0.");
	}
	rowEndOfSymbol++;

	// Make sure width of barcode is a multiple of module size
	if ((rowEndOfSymbol - borderWidth) % moduleSize != 0) {
		throw ReaderException("Barcode width is not a multiple of module size.");
	}
	int dimension = (rowEndOfSymbol - borderWidth) / moduleSize;

	// Push in the "border" by half the module width so that we start
	// sampling in the middle of the module. Just in case the image is a
	// little off, this will help recover.
	borderWidth += (moduleSize >> 1);

	int sampleDimension = borderWidth + (dimension - 1) * moduleSize;
	if (sampleDimension >= width || sampleDimension >= height) {
		throw ReaderException("Sample dimension must be less than width or height");
	}

	// Now just read off the bits
	BitMatrix bits{dimension};
	for (int i = 0; i < dimension; i++) {
		int iOffset = borderWidth + i * moduleSize;
		for (int j = 0; j < dimension; j++) {
			if (image_.get(borderWidth + j * moduleSize, iOffset)) {
				bits.set(j, i);
			}
		}
	}
	return {std::move(bits),
			{{borderWidth, borderWidth},
			 {rowEndOfSymbol, borderWidth},
			 {rowEndOfSymbol, rowEndOfSymbol},
			 {borderWidth, rowEndOfSymbol}}};
}

DetectorResult Detector::processCodeEnclosingRect(const std::vector<ResultPoint>& codeEnclosingRect,
												  int dimension) const
{
	PerspectiveTransform transform = createTransform(codeEnclosingRect, dimension);
	return sampleGrid(image_, transform, dimension);
}

DetectorResult Detector::processFinderPatternInfo(const FinderPatternInfo& patternInfo, int dimension) const
{
	FinderPattern actualTopLeft(patternInfo.getActualTopLeft());
	FinderPattern fakeTopRight(patternInfo.getFakeTopRight());
	FinderPattern fakeBottomLeft(patternInfo.getFakeBottomLeft());

	PerspectiveTransform transform = createTransform(actualTopLeft, fakeTopRight, fakeBottomLeft, dimension);
	return sampleGrid(image_, transform, dimension);
}

/**
 * Create a transform that converts points from the correctly oriented code domain to the input image domain.
 * @param rect vector containing the positions of the corners of the code.
 * @param dimension the dimension of the code.
 * @return Transform from registered QR code to input image.
 */
PerspectiveTransform Detector::createTransform(const std::vector<ResultPoint>& rect, int dimension) const
{
	const QuadrilateralF codeDomain = {PointF{0.0f, 0.0f}, PointF{static_cast<float>(dimension), 0.0f},
									   PointF{static_cast<float>(dimension), static_cast<float>(dimension)},
									   PointF{0.0f, static_cast<float>(dimension)}};
	const QuadrilateralF imageDomain = {rect[0], rect[2], rect[3], rect[1]};
	return PerspectiveTransform{codeDomain, imageDomain};
}

/**
 * Create a transform that converts points from the correctly oriented code domain to the input image domain.
 * @param topLeft position of the center of the finder pattern
 * @param topRight fake position of a finder pattern
 * @param bottomLeft fake position of a finder pattern
 * @return Transform from registered QR code to input image.
 */
PerspectiveTransform Detector::createTransform(const ResultPoint& topLeft, const ResultPoint& topRight,
											   const ResultPoint& bottomLeft, int dimension) const
{
	const float PATTERN_CENTER_POS = 3.5f;
	float dimMinusThree = (float)dimension - PATTERN_CENTER_POS;

	// Don't have an alignment pattern, just make up the bottom-right point
	float bottomRightX = (topRight.x() - topLeft.x()) + bottomLeft.x();
	float bottomRightY = (topRight.y() - topLeft.y()) + bottomLeft.y();

	const QuadrilateralF codeDomain = {PointF{PATTERN_CENTER_POS, PATTERN_CENTER_POS},
									   PointF{dimMinusThree, PATTERN_CENTER_POS}, PointF{dimMinusThree, dimMinusThree},
									   PointF{PATTERN_CENTER_POS, dimMinusThree}};
	const QuadrilateralF imageDomain = {topLeft, topRight, PointF{bottomRightX, bottomRightY}, bottomLeft};
	return PerspectiveTransform{codeDomain, imageDomain};
}

DetectorResult Detector::sampleGrid(const BitMatrix& image, const PerspectiveTransform& transform, int dimension) const
{
	return SampleGrid(image, dimension, dimension, transform);
}

/**
 * <p>Computes the dimension (number of modules on a size) of the Micro QR Code based on the position
 * of the finder patterns and estimated module size.</p>
 */
int Detector::computeDimension(const ResultPoint& topLeft, const ResultPoint& topRight, const ResultPoint& bottomLeft,
							   float moduleSize) const
{
	float tltrCentersDimension = ResultPoint::Distance(std::lround(topLeft.x()), std::lround(topLeft.y()),
													   std::lround(topRight.x()), std::lround(topRight.y())) /
								 moduleSize;
	float tlblCentersDimension = ResultPoint::Distance(std::lround(topLeft.x()), std::lround(topLeft.y()),
													   std::lround(bottomLeft.x()), std::lround(bottomLeft.y())) /
								 moduleSize;
	// 7 is added to the dimension because the centers of each pattern should be at (3.5, 3.5) away from the nearest
	// corner of the code.
	int estimatedDimension = std::lround(((tltrCentersDimension + tlblCentersDimension) / 2.0f) + 7);

	return Dimension::computeRoundOff(estimatedDimension);
}

/**
 * <p>Computes the dimension (number of modules on a size) of the Micro QR Code based on the position
 * of the corners and estimated module size.</p>
 */
int Detector::computeDimension(const std::vector<ResultPoint>& codeEnclosingRect, float moduleSize) const
{
	float tltrDimension =
		ResultPoint::Distance(std::lround(codeEnclosingRect[0].x()), std::lround(codeEnclosingRect[0].y()),
							  std::lround(codeEnclosingRect[2].x()), std::lround(codeEnclosingRect[2].y())) /
		moduleSize;
	float tlblDimension =
		ResultPoint::Distance(std::lround(codeEnclosingRect[0].x()), std::lround(codeEnclosingRect[0].y()),
							  std::lround(codeEnclosingRect[1].x()), std::lround(codeEnclosingRect[1].y())) /
		moduleSize;
	int estimatedDimension = std::lround((tltrDimension + tlblDimension) / 2.0f);

	return Dimension::computeRoundUp(estimatedDimension);
}